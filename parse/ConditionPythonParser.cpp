#include "ConditionPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

namespace {
    template <typename T, typename... Args>
        requires std::is_base_of_v<Condition::Condition, std::decay_t<T>>
    auto make_wrapped(Args&&... args)
    { return condition_wrapper(std::make_shared<T>(std::forward<Args>(args)...)); }

    template <typename T>
    auto make_constant(auto&& arg)
    {
        if constexpr (std::is_enum_v<T>) {
            auto val = boost::python::extract<enum_wrapper<T>>(std::forward<decltype(arg)>(arg))().value;
            return std::make_unique<ValueRef::Constant<T>>(val);
        } else {
            return std::make_unique<ValueRef::Constant<T>>(std::forward<decltype(arg)>(arg));
        }
    }
}

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    auto lhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(lhs.condition);
    auto rhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(rhs.condition);

    if (lhs_cond && rhs_cond) {
        const auto lhs_vals = lhs_cond->ValuesDouble();
        const auto rhs_vals = rhs_cond->ValuesDouble();

        if (!lhs_vals[2] && !rhs_vals[2] && lhs_vals[1] && rhs_vals[0] && (*lhs_vals[1] == *rhs_vals[0])) {
            return make_wrapped<Condition::ValueTest>(
                lhs_vals[0] ? lhs_vals[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals[1] ? rhs_vals[1]->Clone() : nullptr
            );
        }

        const auto lhs_vals_i = lhs_cond->ValuesInt();
        const auto rhs_vals_i = rhs_cond->ValuesInt();

        if (!lhs_vals_i[2] && !rhs_vals_i[2] && lhs_vals_i[1] && rhs_vals_i[0] && (*lhs_vals_i[1] == *rhs_vals_i[0])) {
            return make_wrapped<Condition::ValueTest>(
                lhs_vals_i[0] ? lhs_vals_i[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_i[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_i[1] ? rhs_vals_i[1]->Clone() : nullptr
            );
        }

        const auto lhs_vals_s = lhs_cond->ValuesString();
        const auto rhs_vals_s = rhs_cond->ValuesString();

        if (!lhs_vals_s[2] && !rhs_vals_s[2] && lhs_vals_s[1] && rhs_vals_s[0] && (*lhs_vals_s[1] == *rhs_vals_s[0])) {
            return make_wrapped<Condition::ValueTest>(
                lhs_vals_s[0] ? lhs_vals_s[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_s[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_s[1] ? rhs_vals_s[1]->Clone() : nullptr
            );
        }
    }

    return make_wrapped<Condition::And>(lhs.condition->Clone(), rhs.condition->Clone());
}

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<double>& rhs)
{ return lhs & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<double>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() & rhs; }

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs.operator condition_wrapper() & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs)
{ return lhs.operator condition_wrapper() & rhs.operator condition_wrapper(); }

condition_wrapper operator&(const value_ref_wrapper<int>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() & rhs; }


condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs)
{ return make_wrapped<Condition::Or>(lhs.condition->Clone(), rhs.condition->Clone()); }

condition_wrapper operator|(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs | rhs.operator condition_wrapper(); }

condition_wrapper operator|(const value_ref_wrapper<int>& lhs, const condition_wrapper& rhs)
{ return lhs.operator condition_wrapper() | rhs; }

condition_wrapper operator|(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs)
{ return lhs.operator condition_wrapper() | rhs.operator condition_wrapper(); }

condition_wrapper operator~(const condition_wrapper& lhs)
{ return make_wrapped<Condition::Not>(lhs.condition->Clone()); }

condition_wrapper operator~(const value_ref_wrapper<double>& lhs)
{ return ~(lhs.operator condition_wrapper()); }

condition_wrapper operator~(const value_ref_wrapper<int>& lhs)
{ return ~(lhs.operator condition_wrapper()); }

namespace {
    using ValueRef::CloneUnique;
    using boost::python::extract;

    condition_wrapper insert_owned_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;

        if (kw.has_key("empire")) {
            auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = CloneUnique(empire_args().value_ref);
            } else {
                empire = make_constant<int>(extract<int>(kw["empire"])());
            }
        }

        if (kw.has_key("affiliation")) {
            affiliation = extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        return make_wrapped<Condition::EmpireAffiliation>(std::move(empire), affiliation);
    }

    condition_wrapper insert_contained_by_(const condition_wrapper& cond)
    { return make_wrapped<Condition::ContainedBy<>>(CloneUnique(cond.condition)); }

    condition_wrapper insert_contains_(const condition_wrapper& cond)
    { return make_wrapped<Condition::Contains<>>(CloneUnique(cond.condition)); }

    condition_wrapper insert_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw, MeterType m) {
        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check())
                low = CloneUnique(low_args().value_ref);
            else
                low = make_constant<double>(extract<double>(kw["low"])());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check())
                high = CloneUnique(high_args().value_ref);
            else
                high = make_constant<double>(extract<double>(kw["high"])());
        }
        return make_wrapped<Condition::MeterValue>(m, std::move(low), std::move(high));
    }

    condition_wrapper insert_sorted_number_of_(const boost::python::tuple& args, const boost::python::dict& kw, Condition::SortingMethod method) {
        std::unique_ptr<ValueRef::ValueRef<int>> number;
        if (kw.has_key("number")) {
            auto number_args = boost::python::extract<value_ref_wrapper<int>>(kw["number"]);
            if (number_args.check())
                number = ValueRef::CloneUnique(number_args().value_ref);
            else
                number = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["number"])());
        } else {
            number = make_constant<int>(std::numeric_limits<int>::max());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> sortkey;
        std::unique_ptr<ValueRef::ValueRef<std::string>> sortkey_str;
        if (kw.has_key("sortkey")) {
            auto sortkey_args = extract<value_ref_wrapper<double>>(kw["sortkey"]);
            if (sortkey_args.check()) {
                sortkey = CloneUnique(sortkey_args().value_ref);
            } else {
                auto sortkey_str_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["sortkey"]);
                if (sortkey_str_args.check())
                    sortkey_str = ValueRef::CloneUnique(sortkey_str_args().value_ref);
                else
                    sortkey = make_constant<double>(extract<double>(kw["sortkey"])());
            }
        }

        auto condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["condition"])().condition);
        if (sortkey_str) {
            return make_wrapped<Condition::SortedNumberOf>(std::move(number),
                                                           std::move(sortkey_str),
                                                           method,
                                                           std::move(condition));

        } else {
            return make_wrapped<Condition::SortedNumberOf>(std::move(number),
                                                           std::move(sortkey),
                                                           method,
                                                           std::move(condition));
        }
    }

    condition_wrapper insert_visible_to_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check())
            empire = CloneUnique(empire_args().value_ref);
        else
            empire = make_constant<int>(extract<int>(kw["empire"])());

        if (kw.has_key("turn"))
            throw std::runtime_error(std::string("Not implemented ") + __func__);

        if (kw.has_key("visibility"))
            throw std::runtime_error(std::string("Not implemented ") + __func__);

        return make_wrapped<Condition::VisibleToEmpire>(std::move(empire));
    }

    condition_wrapper insert_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("type")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>> types;
            std::vector<::PlanetType> type_vals;

            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
            bool have_refs = false;
            for (auto it = it_begin; it != it_end; ++it) {
                auto type_arg = extract<value_ref_wrapper< ::PlanetType>>(*it);
                if (type_arg.check()) {
                    types.push_back(CloneUnique(type_arg().value_ref));
                    have_refs = true;
                } else if (auto constant = make_constant< ::PlanetType>(*it)) {
                    type_vals.push_back(constant->Value());
                    types.push_back(std::move(constant));
                }
            }
            if (have_refs)
                return make_wrapped<Condition::PlanetType<>>(std::move(types));
            else // have only constants
                return make_wrapped<Condition::PlanetType<::PlanetType>>(std::move(type_vals));

        } else if (kw.has_key("size")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>> sizes;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["size"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto size_arg = extract<value_ref_wrapper< ::PlanetSize>>(*it);
                if (size_arg.check())
                    sizes.push_back(CloneUnique(size_arg().value_ref));
                else
                    sizes.push_back(make_constant< ::PlanetSize>(*it));
            }
            return make_wrapped<Condition::PlanetSize>(std::move(sizes));

        } else if (kw.has_key("environment")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["environment"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto env_arg = extract<value_ref_wrapper< ::PlanetEnvironment>>(*it);
                if (env_arg.check())
                    environments.push_back(CloneUnique(env_arg().value_ref));
                else
                    environments.push_back(make_constant< ::PlanetEnvironment>(*it));
            }
            std::unique_ptr<ValueRef::ValueRef<std::string>> species = nullptr;
            if (kw.has_key("species"))
                species = pyobject_to_vref<std::string>(kw["species"]);
            return make_wrapped<Condition::PlanetEnvironment>(std::move(environments), std::move(species));
        }
        return make_wrapped<Condition::Type>(UniverseObjectType::OBJ_PLANET);
    }

    condition_wrapper insert_homeworld_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto name_arg = extract<value_ref_wrapper<std::string>>(*it);
                if (name_arg.check())
                    names.push_back(CloneUnique(name_arg().value_ref));
                else
                    names.push_back(make_constant<std::string>(extract<std::string>(*it)()));
            }
            return make_wrapped<Condition::Homeworld>(std::move(names));
        }
        return make_wrapped<Condition::Homeworld>();
    }

    condition_wrapper insert_has_special_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (!kw.has_key("name"))
            return make_wrapped<Condition::HasSpecial>();

        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check())
            return make_wrapped<Condition::HasSpecial>(CloneUnique(name_args().value_ref));

        auto name_arg = extract<std::string>(kw["name"]);
        if (name_arg.check())
            return make_wrapped<Condition::HasSpecial>(make_constant<std::string>(name_arg()));

        return make_wrapped<Condition::HasSpecial>();
    }

    condition_wrapper insert_has_species_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (!kw.has_key("name"))
            return make_wrapped<Condition::Species>();

        auto name_arg_string = extract<std::string>(kw["name"]);
        if (name_arg_string.check())
            return make_wrapped<Condition::Species>(name_arg_string());

        auto name_arg_ref = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_arg_ref.check())
            return make_wrapped<Condition::Species>(CloneUnique(name_arg_ref().value_ref));

        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            auto name_arg = extract<value_ref_wrapper<std::string>>(*it);
            if (name_arg.check())
                names.push_back(CloneUnique(name_arg().value_ref));
            else
                names.push_back(make_constant<std::string>(extract<std::string>(*it)()));
        }
        return make_wrapped<Condition::Species>(std::move(names));
    }

    condition_wrapper insert_is_field_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
        if (kw.has_key("name")) {
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto name_arg = extract<value_ref_wrapper<std::string>>(*it);
                if (name_arg.check()) {
                    names.push_back(CloneUnique(name_arg().value_ref));
                } else {
                    names.push_back(make_constant<std::string>(extract<std::string>(*it)()));
                }
            }
        }
        return make_wrapped<Condition::Field>(std::move(names));
    }

    condition_wrapper insert_has_tag_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = CloneUnique(name_args().value_ref);
            } else {
                name = make_constant<std::string>(extract<std::string>(kw["name"])());
            }
        }
        return make_wrapped<Condition::HasTag>(std::move(name));
    }

    condition_wrapper insert_has_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto from = CloneUnique(extract<condition_wrapper>(kw["from_"])().condition);
        return make_wrapped<Condition::HasStarlaneTo>(std::move(from));
    }

    condition_wrapper insert_starlane_to_would_cross_existing_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto from = CloneUnique(extract<condition_wrapper>(kw["from_"])().condition);
        return make_wrapped<Condition::StarlaneToWouldCrossExistingStarlane>(std::move(from));
    }

    condition_wrapper insert_starlane_to_would_be_angularly_close_to_existing_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto from = CloneUnique(extract<condition_wrapper>(kw["from_"])().condition);
        double maxdotprod = extract<double>(kw["maxdotprod"])();

        return make_wrapped<Condition::StarlaneToWouldBeAngularlyCloseToExistingStarlane>(std::move(from), maxdotprod);
    }

    condition_wrapper insert_starlane_to_would_be_close_to_object_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto from = CloneUnique(extract<condition_wrapper>(kw["from_"])().condition);
        auto closeto = CloneUnique(extract<condition_wrapper>(kw["closeto"])().condition);
        double distance = extract<double>(kw["distance"])();

        return make_wrapped<Condition::StarlaneToWouldBeCloseToObject>(
            std::move(from),
            std::move(closeto),
            distance);
    }

    condition_wrapper insert_focus_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> types;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            types.push_back(pyobject_to_vref<std::string>(*it));
        }
        return make_wrapped<Condition::FocusType>(std::move(types));
    }

    condition_wrapper insert_empire_stockpile_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = CloneUnique(empire_args().value_ref);
        } else {
            empire = make_constant<int>(extract<int>(kw["empire"])());
        }

        auto resource = extract<enum_wrapper<ResourceType>>(kw["resource"])();

        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = CloneUnique(low_args().value_ref);
            } else {
                low = make_constant<double>(extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = CloneUnique(high_args().value_ref);
            } else {
                high = make_constant<double>(extract<double>(kw["high"])());
            }
        }

        return make_wrapped<Condition::EmpireStockpileValue>(
            std::move(empire),
            resource.value,
            std::move(low),
            std::move(high));
    }

    condition_wrapper insert_empire_has_adopted_policy_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = CloneUnique(empire_args().value_ref);
            } else {
                empire = make_constant<int>(extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = CloneUnique(name_args().value_ref);
        } else {
            name = make_constant<std::string>(extract<std::string>(kw["name"])());
        }

        return make_wrapped<Condition::EmpireHasAdoptedPolicy>(
            std::move(empire),
            std::move(name));
    }

    condition_wrapper insert_resupplyable_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = CloneUnique(empire_args().value_ref);
        } else {
            empire = make_constant<int>(extract<int>(kw["empire"])());
        }

        return make_wrapped<Condition::FleetSupplyableByEmpire>(std::move(empire));
    }

    condition_wrapper insert_design_has_part_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = CloneUnique(name_args().value_ref);
        } else {
            name = make_constant<std::string>(extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = CloneUnique(low_args().value_ref);
            } else {
                low = make_constant<int>(extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = CloneUnique(high_args().value_ref);
            } else {
                high = make_constant<int>(extract<int>(kw["high"])());
            }
        }

        return make_wrapped<Condition::DesignHasPart>(
            std::move(name),
            std::move(low),
            std::move(high));
    }

    condition_wrapper insert_building_(const boost::python::tuple& args, const boost::python::dict& kw) {
        BuildingType::SubType subtype = BuildingType::SubType::NONE;
        if (kw.has_key("subtype")) {
            const auto subtype_name = extract<std::string>(kw["subtype"])();
            if (subtype_name == "colony")
                subtype = BuildingType::SubType::COLONY;
            else if (subtype_name == "shipyard")
                subtype = BuildingType::SubType::SHIPYARD;

        }
        if (!kw.has_key("name"))
            return make_wrapped<Condition::Building>(subtype);

        auto name_arg_string = extract<std::string>(kw["name"]);
        if (name_arg_string.check())
            return make_wrapped<Condition::Building>(name_arg_string(), subtype);

        auto name_arg_ref = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_arg_ref.check())
            return make_wrapped<Condition::Building>(CloneUnique(name_arg_ref().value_ref), subtype);

        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            auto name_arg = extract<value_ref_wrapper<std::string>>(*it);
            if (name_arg.check())
                names.push_back(CloneUnique(name_arg().value_ref));
            else
                names.push_back(make_constant<std::string>(extract<std::string>(*it)()));
        }

        return make_wrapped<Condition::Building>(std::move(names), subtype);
    }

    condition_wrapper insert_location_(const boost::python::tuple& args, const boost::python::dict& kw) {
        Condition::ContentType content_type = extract<enum_wrapper<Condition::ContentType>>(kw["type"])().value;

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check())
            name = CloneUnique(name_args().value_ref);
        else
            name = make_constant<std::string>(extract<std::string>(kw["name"])());

        std::unique_ptr<ValueRef::ValueRef<std::string>> name2;
        if (kw.has_key("name2")) {
            auto name2_args = extract<value_ref_wrapper<std::string>>(kw["name2"]);
            if (name2_args.check())
                name2 = CloneUnique(name2_args().value_ref);
            else
                name2 = make_constant<std::string>(extract<std::string>(kw["name2"])());
        }

        return make_wrapped<Condition::Location>(
            content_type,
            std::move(name),
            std::move(name2));
    }

    condition_wrapper insert_enqueued_(const boost::python::tuple& args, const boost::python::dict& kw) {
        BuildType build_type = BuildType::INVALID_BUILD_TYPE;
        if (kw.has_key("type"))
            build_type = extract<enum_wrapper<BuildType>>(kw["type"])().value;

        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check())
                low = CloneUnique(low_args().value_ref);
            else
                low = make_constant<int>(extract<int>(kw["low"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check())
                high = CloneUnique(high_args().value_ref);
            else
                high = make_constant<int>(extract<int>(kw["high"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check())
                empire = CloneUnique(empire_args().value_ref);
            else
                empire = make_constant<int>(extract<int>(kw["empire"])());
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check())
                name = CloneUnique(name_args().value_ref);
            else
                name = make_constant<std::string>(extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> design;
        if (kw.has_key("design")) {
            auto design_args = extract<value_ref_wrapper<int>>(kw["design"]);
            if (design_args.check())
                design = CloneUnique(design_args().value_ref);
            else
                design = make_constant<int>(extract<int>(kw["design"])());
        }

        BuildingType::SubType subtype = BuildingType::SubType::NONE;
        if (kw.has_key("subtype")) {
            const auto subtype_name = extract<std::string>(kw["subtype"])();
            if (subtype_name == "colony")
                subtype = BuildingType::SubType::COLONY;
            else if (subtype_name == "shipyard")
                subtype = BuildingType::SubType::SHIPYARD;
        }

        if (build_type == BuildType::BT_SHIP) {
            return make_wrapped<Condition::Enqueued>(
                std::move(design),
                std::move(empire),
                std::move(low),
                std::move(high));

        } else if (subtype != BuildingType::SubType::NONE) {
            return make_wrapped<Condition::Enqueued>(
                subtype,
                std::move(empire),
                std::move(low),
                std::move(high));

        } else {
            return make_wrapped<Condition::Enqueued>(
                build_type,
                std::move(name),
                std::move(empire),
                std::move(low),
                std::move(high));
        }
    }

    condition_wrapper insert_number_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check())
                low = CloneUnique(low_args().value_ref);
            else
                low = make_constant<int>(extract<int>(kw["low"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check())
                high = CloneUnique(high_args().value_ref);
            else
                high = make_constant<int>(extract<int>(kw["high"])());
        }

        auto condition = CloneUnique(extract<condition_wrapper>(kw["condition"])().condition);

        return make_wrapped<Condition::Number>(
            std::move(low),
            std::move(high),
            std::move(condition));
    }

    condition_wrapper insert_produced_by_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check())
            empire = CloneUnique(empire_args().value_ref);
        else
            empire = make_constant<int>(extract<int>(kw["empire"])());

        return make_wrapped<Condition::ProducedByEmpire>(std::move(empire));
    }

    condition_wrapper insert_owner_has_tech_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check())
            name = CloneUnique(name_args().value_ref);
        else
            name = make_constant<std::string>(extract<std::string>(kw["name"])());
        return make_wrapped<Condition::OwnerHasTech>(std::move(name));
    }

    condition_wrapper insert_random_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> probability;
        auto p_args = extract<value_ref_wrapper<double>>(kw["probability"]);
        if (p_args.check()) {
            probability = CloneUnique(p_args().value_ref);
        } else {
            probability = make_constant<double>(extract<double>(kw["probability"])());
        }
        return make_wrapped<Condition::Chance>(std::move(probability));
    }

    condition_wrapper insert_star_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>> types;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            auto type_arg = extract<value_ref_wrapper< ::StarType>>(*it);
            if (type_arg.check()) {
                types.push_back(CloneUnique(type_arg().value_ref));
            } else {
                types.push_back(make_constant< ::StarType>(*it));
            }
        }
        return make_wrapped<Condition::StarType>(std::move(types));
    }

    condition_wrapper insert_in_system_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> system_id;
        if (kw.has_key("id")) {
            auto id_args = extract<value_ref_wrapper<int>>(kw["id"]);
            if (id_args.check()) {
                system_id = CloneUnique(id_args().value_ref);
            } else {
                system_id = make_constant<int>(extract<int>(kw["id"])());
            }
        }
        
        return make_wrapped<Condition::InOrIsSystem>(std::move(system_id));
    }

    condition_wrapper insert_on_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> planet_id;
        if (kw.has_key("id")) {
            auto id_args = extract<value_ref_wrapper<int>>(kw["id"]);
            if (id_args.check()) {
                planet_id = CloneUnique(id_args().value_ref);
            } else {
                planet_id = make_constant<int>(extract<int>(kw["id"])());
            }
        }

        return make_wrapped<Condition::OnPlanet>(std::move(planet_id));
    }

    condition_wrapper insert_turn_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = CloneUnique(low_args().value_ref);
            } else {
                auto low_args_double = extract<value_ref_wrapper<double>>(kw["low"]);
                if (low_args_double.check()) {
                    low = std::make_unique<ValueRef::StaticCast<double, int>>(CloneUnique(low_args_double().value_ref));
                } else {
                    low = make_constant<int>(extract<int>(kw["low"])());
                }
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = CloneUnique(high_args().value_ref);
            } else {
                auto high_args_double = extract<value_ref_wrapper<double>>(kw["high"]);
                if (high_args_double.check()) {
                    high = std::make_unique<ValueRef::StaticCast<double, int>>(CloneUnique(high_args_double().value_ref));
                } else {
                    high = make_constant<int>(extract<int>(kw["high"])());
                }
            }
        }

        return make_wrapped<Condition::Turn>(std::move(low), std::move(high));
    }

    condition_wrapper insert_resource_supply_connected_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = CloneUnique(empire_args().value_ref);
        } else {
            empire = make_constant<int>(extract<int>(kw["empire"])());
        }

        auto condition = extract<condition_wrapper>(kw["condition"])();
        return make_wrapped<Condition::ResourceSupplyConnectedByEmpire>(
            std::move(empire),
            CloneUnique(condition.condition));
    }

    condition_wrapper insert_within_starlane_jumps_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = extract<condition_wrapper>(kw["condition"])();

        std::unique_ptr<ValueRef::ValueRef<int>> jumps;
        auto jumps_args = extract<value_ref_wrapper<int>>(kw["jumps"]);
        if (jumps_args.check()) {
            jumps = CloneUnique(jumps_args().value_ref);
        } else {
            jumps = make_constant<int>(extract<int>(kw["jumps"])());
        }

        return make_wrapped<Condition::WithinStarlaneJumps>(
            std::move(jumps),
            CloneUnique(condition.condition));
    }

    condition_wrapper insert_within_distance_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = extract<condition_wrapper>(kw["condition"])();

        std::unique_ptr<ValueRef::ValueRef<double>> distance;
        auto distance_args = extract<value_ref_wrapper<double>>(kw["distance"]);
        if (distance_args.check()) {
            distance = CloneUnique(distance_args().value_ref);
        } else {
            distance = make_constant<double>(extract<double>(kw["distance"])());
        }

        return make_wrapped<Condition::WithinDistance>(
            std::move(distance),
            CloneUnique(condition.condition));
    }

    condition_wrapper insert_object_id_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> id;
        auto id_args = extract<value_ref_wrapper<int>>(kw["id"]);
        if (id_args.check()) {
            id = CloneUnique(id_args().value_ref);
        } else {
            id = make_constant<int>(extract<int>(kw["id"])());
        }
        return make_wrapped<Condition::ObjectID>(std::move(id));
    }

    condition_wrapper insert_described_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto description = extract<std::string>(kw["description"])();
        auto condition = extract<condition_wrapper>(kw["condition"])();

        return make_wrapped<Condition::Described>(
            CloneUnique(condition.condition),
            std::move(description));
    }

    condition_wrapper insert_design_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name") && !kw.has_key("design")) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> name;
            auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = CloneUnique(name_args().value_ref);
            } else {
                name = make_constant<std::string>(extract<std::string>(kw["name"])());
            }
            return make_wrapped<Condition::PredefinedShipDesign>(std::move(name));

        } else if (kw.has_key("design") && !kw.has_key("name")) {
            std::unique_ptr<ValueRef::ValueRef<int>> design;
            auto design_args = extract<value_ref_wrapper<int>>(kw["design"]);
            if (design_args.check()) {
                design = CloneUnique(design_args().value_ref);
            } else {
                design = make_constant<int>(extract<int>(kw["design"])());
            }
            return make_wrapped<Condition::NumberedShipDesign>(std::move(design));
        }

        throw std::runtime_error("Design requires only name or design keyword");
    }

    condition_wrapper insert_species_opinion_(const boost::python::tuple& args, const boost::python::dict& kw, Condition::ComparisonType cmp) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> species;
        if (kw.has_key("species")) {
            auto species_args = extract<value_ref_wrapper<std::string>>(kw["species"]);
            if (species_args.check()) {
                species = CloneUnique(species_args().value_ref);
            } else {
                species = make_constant<std::string>(extract<std::string>(kw["species"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = CloneUnique(name_args().value_ref);
        } else {
            name = make_constant<std::string>(extract<std::string>(kw["name"])());
        }
        return make_wrapped<Condition::SpeciesOpinion>(
            std::move(species),
            std::move(name),
            cmp);
    }
}

void RegisterGlobalsConditions(boost::python::dict& globals) {
    globals["All"] = make_wrapped<Condition::All>();
    globals["Ship"] = make_wrapped<Condition::Type>(UniverseObjectType::OBJ_SHIP);
    globals["System"] = make_wrapped<Condition::Type>(UniverseObjectType::OBJ_SYSTEM);
    globals["Fleet"] = make_wrapped<Condition::Type>(UniverseObjectType::OBJ_FLEET);
    globals["Monster"] = make_wrapped<Condition::Monster>();
    globals["Capital"] = make_wrapped<Condition::Capital>();
    globals["Stationary"] = make_wrapped<Condition::Stationary>();

    globals["NoOpCondition"] = make_wrapped<Condition::NoOp>();

    globals["Unowned"] = make_wrapped<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE);
    globals["IsHuman"] = make_wrapped<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_HUMAN);

    globals["OwnerHasTech"] = boost::python::raw_function(insert_owner_has_tech_);
    globals["Random"] = boost::python::raw_function(insert_random_);
    globals["Star"] = boost::python::raw_function(insert_star_);
    globals["InSystem"] = boost::python::raw_function(insert_in_system_);
    globals["OnPlanet"] = boost::python::raw_function(insert_on_planet_);
    globals["ResupplyableBy"] = boost::python::raw_function(insert_resupplyable_by_);
    globals["DesignHasPart"] = boost::python::raw_function(insert_design_has_part_);
    globals["IsBuilding"] = boost::python::raw_function(insert_building_);
    globals["Location"] = boost::python::raw_function(insert_location_);
    globals["Enqueued"] = boost::python::raw_function(insert_enqueued_);
    globals["Number"] = boost::python::raw_function(insert_number_);
    globals["ProducedByEmpire"] = boost::python::raw_function(insert_produced_by_empire_);

    // non_ship_part_meter_enum_grammar
    for (const auto& [name, mt] : std::initializer_list<std::pair<const char*, MeterType>>{
            {"TargetConstruction", MeterType::METER_TARGET_CONSTRUCTION},
            {"TargetIndustry",     MeterType::METER_TARGET_INDUSTRY},
            {"TargetPopulation",   MeterType::METER_TARGET_POPULATION},
            {"TargetResearch",     MeterType::METER_TARGET_RESEARCH},
            {"TargetInfluence",    MeterType::METER_TARGET_INFLUENCE},
            {"TargetHappiness",    MeterType::METER_TARGET_HAPPINESS},
            {"MaxDefense",         MeterType::METER_MAX_DEFENSE},
            {"MaxFuel",            MeterType::METER_MAX_FUEL},
            {"MaxShield",          MeterType::METER_MAX_SHIELD},
            {"MaxStructure",       MeterType::METER_MAX_STRUCTURE},
            {"MaxTroops",          MeterType::METER_MAX_TROOPS},
            {"MaxSupply",          MeterType::METER_MAX_SUPPLY},
            {"MaxStockpile",       MeterType::METER_MAX_STOCKPILE},

            {"Construction",       MeterType::METER_CONSTRUCTION},
            {"Industry",           MeterType::METER_INDUSTRY},
            {"Population",         MeterType::METER_POPULATION},
            {"Research",           MeterType::METER_RESEARCH},
            {"Influence",          MeterType::METER_INFLUENCE},
            {"Happiness",          MeterType::METER_HAPPINESS},

            {"Defense",            MeterType::METER_DEFENSE},
            {"Fuel",               MeterType::METER_FUEL},
            {"Shield",             MeterType::METER_SHIELD},
            {"Structure",          MeterType::METER_STRUCTURE},
            {"Troops",             MeterType::METER_TROOPS},
            {"Supply",             MeterType::METER_SUPPLY},
            {"Stockpile",          MeterType::METER_STOCKPILE},

            {"RebelTroops",        MeterType::METER_REBEL_TROOPS},
            {"Stealth",            MeterType::METER_STEALTH},
            {"Detection",          MeterType::METER_DETECTION},
            {"Speed",              MeterType::METER_SPEED},

            {"Size",               MeterType::METER_SIZE}})
    {
        const auto f_insert_meter_value = [mt=mt](const auto& args, const auto& kw)
        { return insert_meter_value_(args, kw, mt); };
        globals[name] = boost::python::raw_function(f_insert_meter_value);
    }

    for (const auto& [name, ct] : std::initializer_list<std::pair<const char*, Condition::ContentType>>{
            {"ContentBuilding", Condition::ContentType::CONTENT_BUILDING},
            {"ContentSpecies",  Condition::ContentType::CONTENT_SPECIES},
            {"ContentHull",     Condition::ContentType::CONTENT_SHIP_HULL},
            {"ContentPart",     Condition::ContentType::CONTENT_SHIP_PART},
            {"ContentSpecial",  Condition::ContentType::CONTENT_SPECIAL},
            {"ContentFocus",    Condition::ContentType::CONTENT_FOCUS}})
    { globals[name] = enum_wrapper<Condition::ContentType>(ct); }

    for (const auto& [name, sm] : std::initializer_list<std::pair<const char*, Condition::SortingMethod>>{
            {"MaximumNumberOf", Condition::SortingMethod::SORT_MAX},
            {"MinimumNumberOf", Condition::SortingMethod::SORT_MIN},
            {"ModeNumberOf",    Condition::SortingMethod::SORT_MODE},
            {"UniqueNumberOf",  Condition::SortingMethod::SORT_UNIQUE},
            {"NumberOf",        Condition::SortingMethod::SORT_RANDOM}})
    {
        globals[name] = boost::python::raw_function([sm=sm](const auto& args, const auto& kw)
                                                    { return insert_sorted_number_of_(args, kw, sm); });
    }

    globals["HasSpecies"] = boost::python::raw_function(insert_has_species_);
    globals["IsField"] = boost::python::raw_function(insert_is_field_);
    globals["CanColonize"] = make_wrapped<Condition::CanColonize>();
    globals["CanProduceShips"] = make_wrapped<Condition::CanProduceShips>();
    globals["Armed"] = make_wrapped<Condition::Armed>();

    globals["IsSource"] = make_wrapped<Condition::Source>();
    globals["IsTarget"] = make_wrapped<Condition::Target>();
    globals["IsRootCandidate"] = make_wrapped<Condition::RootCandidate>();
    globals["IsAnyObject"] = make_wrapped<Condition::All>();
    globals["NoObject"] = make_wrapped<Condition::None>();

    globals["HasTag"] = boost::python::raw_function(insert_has_tag_);
    globals["HasStarlane"] = boost::python::raw_function(insert_has_starlane_);
    globals["StarlaneToWouldCrossExistingStarlane"] = boost::python::raw_function(insert_starlane_to_would_cross_existing_starlane_);
    globals["StarlaneToWouldBeAngularlyCloseToExistingStarlane"] = boost::python::raw_function(insert_starlane_to_would_be_angularly_close_to_existing_starlane_);
    globals["StarlaneToWouldBeCloseToObject"] = boost::python::raw_function(insert_starlane_to_would_be_close_to_object_);
    globals["Planet"] = boost::python::raw_function(insert_planet_);
    globals["Homeworld"] = boost::python::raw_function(insert_homeworld_);
    globals["HasSpecial"] = boost::python::raw_function(insert_has_special_);
    globals["VisibleToEmpire"] = boost::python::raw_function(insert_visible_to_empire_);
    globals["OwnedBy"] = boost::python::raw_function(insert_owned_by_);
    globals["ContainedBy"] = insert_contained_by_;
    globals["Contains"] = insert_contains_;
    globals["Focus"] = boost::python::raw_function(insert_focus_);
    globals["HasEmpireStockpile"] = boost::python::raw_function(insert_empire_stockpile_);
    globals["EmpireHasAdoptedPolicy"] = boost::python::raw_function(insert_empire_has_adopted_policy_);
    globals["Turn"] = boost::python::raw_function(insert_turn_);
    globals["ResourceSupplyConnected"] = boost::python::raw_function(insert_resource_supply_connected_);
    globals["WithinStarlaneJumps"] = boost::python::raw_function(insert_within_starlane_jumps_);
    globals["WithinDistance"] = boost::python::raw_function(insert_within_distance_);
    globals["Object"] = boost::python::raw_function(insert_object_id_);
    globals["Described"] = boost::python::raw_function(insert_described_);
    globals["HasDesign"] = boost::python::raw_function(insert_design_);
    const auto f_insert_species_likes = [](const auto& args, const auto& kw) { return insert_species_opinion_(args, kw, Condition::ComparisonType::GREATER_THAN); };
    globals["SpeciesLikes"] = boost::python::raw_function(f_insert_species_likes);
    const auto f_insert_species_dislikes = [](const auto& args, const auto& kw) { return insert_species_opinion_(args, kw, Condition::ComparisonType::LESS_THAN); };
    globals["SpeciesDislikes"] = boost::python::raw_function(f_insert_species_dislikes);
}

