#include "ConditionPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    auto lhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(lhs.condition);
    auto rhs_cond = std::dynamic_pointer_cast<const Condition::ValueTest>(rhs.condition);

    if (lhs_cond && rhs_cond) {
        const auto lhs_vals = lhs_cond->ValuesDouble();
        const auto rhs_vals = rhs_cond->ValuesDouble();

        if (!lhs_vals[2] && !rhs_vals[2] && lhs_vals[1] && rhs_vals[0] && (*lhs_vals[1] == *rhs_vals[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals[0] ? lhs_vals[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals[1] ? rhs_vals[1]->Clone() : nullptr
            ));
        }

        const auto lhs_vals_i = lhs_cond->ValuesInt();
        const auto rhs_vals_i = rhs_cond->ValuesInt();

        if (!lhs_vals_i[2] && !rhs_vals_i[2] && lhs_vals_i[1] && rhs_vals_i[0] && (*lhs_vals_i[1] == *rhs_vals_i[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals_i[0] ? lhs_vals_i[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_i[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_i[1] ? rhs_vals_i[1]->Clone() : nullptr
            ));
        }

        const auto lhs_vals_s = lhs_cond->ValuesString();
        const auto rhs_vals_s = rhs_cond->ValuesString();

        if (!lhs_vals_s[2] && !rhs_vals_s[2] && lhs_vals_s[1] && rhs_vals_s[0] && (*lhs_vals_s[1] == *rhs_vals_s[0])) {
            return condition_wrapper(std::make_shared<Condition::ValueTest>(
                lhs_vals_s[0] ? lhs_vals_s[0]->Clone() : nullptr,
                lhs_cond->CompareTypes()[0],
                lhs_vals_s[1]->Clone(),
                rhs_cond->CompareTypes()[0],
                rhs_vals_s[1] ? rhs_vals_s[1]->Clone() : nullptr
            ));
        }
    }

    return condition_wrapper(std::make_shared<Condition::And>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<double>& rhs) {
    return lhs & rhs.operator condition_wrapper();
}

condition_wrapper operator&(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs) {
    return lhs & rhs.operator condition_wrapper();
}

condition_wrapper operator&(const value_ref_wrapper<double>& lhs, const value_ref_wrapper<double>& rhs) {
    return lhs.operator condition_wrapper() & rhs.operator condition_wrapper();
}

condition_wrapper operator&(const value_ref_wrapper<int>& lhs, const condition_wrapper& rhs) {
    return lhs.operator condition_wrapper() & rhs;
}


condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::Or>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator|(const condition_wrapper& lhs, const value_ref_wrapper<int>& rhs) {
    return lhs | rhs.operator condition_wrapper();
}

condition_wrapper operator|(const value_ref_wrapper<int>& lhs, const value_ref_wrapper<int>& rhs) {
    return lhs.operator condition_wrapper() | rhs.operator condition_wrapper();
}

condition_wrapper operator~(const condition_wrapper& lhs) {
    return condition_wrapper(std::make_shared<Condition::Not>(
        lhs.condition->Clone()
    ));
}


namespace {
    condition_wrapper insert_owned_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;

        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        if (kw.has_key("affiliation")) {
            affiliation = boost::python::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        return condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(std::move(empire), affiliation));
    }

    condition_wrapper insert_contained_by_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::ContainedBy>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_contains_(const condition_wrapper& cond) {
        return condition_wrapper(std::make_shared<Condition::Contains<>>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw, MeterType m) {
        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["high"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::MeterValue>(m, std::move(low), std::move(high)));
    }

    condition_wrapper insert_sorted_number_of_(const boost::python::tuple& args, const boost::python::dict& kw, Condition::SortingMethod method) {
        std::unique_ptr<ValueRef::ValueRef<int>> number;
        auto number_args = boost::python::extract<value_ref_wrapper<int>>(kw["number"]);
        if (number_args.check()) {
            number = ValueRef::CloneUnique(number_args().value_ref);
        } else {
            number = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["number"])());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> sortkey;
        if (kw.has_key("sortkey")) {
            auto sortkey_args = boost::python::extract<value_ref_wrapper<double>>(kw["sortkey"]);
            if (sortkey_args.check()) {
                sortkey = ValueRef::CloneUnique(sortkey_args().value_ref);
            } else {
                sortkey = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["sortkey"])());
            }
        }

        auto condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["condition"])().condition);
        return condition_wrapper(std::make_shared<Condition::SortedNumberOf>(std::move(number),
                                                                             std::move(sortkey),
                                                                             method,
                                                                             std::move(condition)));
    }

    condition_wrapper insert_visible_to_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        if (kw.has_key("turn")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        if (kw.has_key("visibility")) {
            throw std::runtime_error(std::string("Not implemented ") + __func__);
        }

        return condition_wrapper(std::make_shared<Condition::VisibleToEmpire>(std::move(empire)));
    }

    condition_wrapper insert_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("type")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>> types;
            std::vector<::PlanetType> type_vals;

            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
            bool have_refs = false;
            for (auto it = it_begin; it != it_end; ++it) {
                auto type_arg = boost::python::extract<value_ref_wrapper< ::PlanetType>>(*it);
                if (type_arg.check()) {
                    types.push_back(ValueRef::CloneUnique(type_arg().value_ref));
                    have_refs = true;
                } else {
                    auto val = boost::python::extract<enum_wrapper< ::PlanetType>>(*it)().value;
                    types.push_back(std::make_unique<ValueRef::Constant< ::PlanetType>>(val));
                    type_vals.push_back(val);
                }
            }
            if (have_refs) {
                return condition_wrapper(std::make_shared<Condition::PlanetType<>>(std::move(types)));
            } else { // have only constants
                return condition_wrapper(std::make_shared<Condition::PlanetType<::PlanetType>>(std::move(type_vals)));
            }
        } else if (kw.has_key("size")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>> sizes;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["size"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto size_arg = boost::python::extract<value_ref_wrapper< ::PlanetSize>>(*it);
                if (size_arg.check()) {
                    sizes.push_back(ValueRef::CloneUnique(size_arg().value_ref));
                } else {
                    sizes.push_back(std::make_unique<ValueRef::Constant< ::PlanetSize>>(boost::python::extract<enum_wrapper< ::PlanetSize>>(*it)().value));
                }
            }
            return condition_wrapper(std::make_shared<Condition::PlanetSize>(std::move(sizes)));
        } else if (kw.has_key("environment")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["environment"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto env_arg = boost::python::extract<value_ref_wrapper< ::PlanetEnvironment>>(*it);
                if (env_arg.check()) {
                    environments.push_back(ValueRef::CloneUnique(env_arg().value_ref));
                } else {
                    environments.push_back(std::make_unique<ValueRef::Constant< ::PlanetEnvironment>>(boost::python::extract<enum_wrapper< ::PlanetEnvironment>>(*it)().value));
                }
            }
            return condition_wrapper(std::make_shared<Condition::PlanetEnvironment>(std::move(environments)));
        }
        return condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_PLANET));
    }

    condition_wrapper insert_homeworld_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto name_arg = boost::python::extract<value_ref_wrapper<std::string>>(*it);
                if (name_arg.check()) {
                    names.push_back(ValueRef::CloneUnique(name_arg().value_ref));
                } else {
                    names.push_back(std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(*it)()));
                }
            }
            return condition_wrapper(std::make_shared<Condition::Homeworld>(std::move(names)));
        }
        return condition_wrapper(std::make_shared<Condition::Homeworld>());
    }

    condition_wrapper insert_has_special_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name")) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> name;
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
            return condition_wrapper(std::make_shared<Condition::HasSpecial>(std::move(name)));
        }
        return condition_wrapper(std::make_shared<Condition::HasSpecial>());
    }

    condition_wrapper insert_has_species_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto name_arg = boost::python::extract<value_ref_wrapper<std::string>>(*it);
                if (name_arg.check()) {
                    names.push_back(ValueRef::CloneUnique(name_arg().value_ref));
                } else {
                    names.push_back(std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(*it)()));
                }
            }
            return condition_wrapper(std::make_shared<Condition::Species>(std::move(names)));
        }
        return condition_wrapper(std::make_shared<Condition::Species>());
    }

    condition_wrapper insert_is_field_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            auto name_arg = boost::python::extract<value_ref_wrapper<std::string>>(*it);
            if (name_arg.check()) {
                names.push_back(ValueRef::CloneUnique(name_arg().value_ref));
            } else {
                names.push_back(std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(*it)()));
            }
        }
        return condition_wrapper(std::make_shared<Condition::Field>(std::move(names)));
    }

    condition_wrapper insert_has_tag_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }
        return condition_wrapper(std::make_shared<Condition::HasTag>(std::move(name)));
    }

    condition_wrapper insert_has_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> from = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["from_"])().condition);

        return condition_wrapper(std::make_shared<Condition::HasStarlaneTo>(std::move(from)));
    }

    condition_wrapper insert_starlane_to_would_cross_existing_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> from = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["from_"])().condition);

        return condition_wrapper(std::make_shared<Condition::StarlaneToWouldCrossExistingStarlane>(std::move(from)));
    }

    condition_wrapper insert_starlane_to_would_be_angularly_close_to_existing_starlane_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> from = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["from_"])().condition);
        double maxdotprod = boost::python::extract<double>(kw["maxdotprod"])();

        return condition_wrapper(std::make_shared<Condition::StarlaneToWouldBeAngularlyCloseToExistingStarlane>(std::move(from), maxdotprod));
    }

    condition_wrapper insert_starlane_to_would_be_close_to_object_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> from = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["from_"])().condition);
        std::unique_ptr<Condition::Condition> closeto = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["closeto"])().condition);
        double distance = boost::python::extract<double>(kw["distance"])();

        return condition_wrapper(std::make_shared<Condition::StarlaneToWouldBeCloseToObject>(std::move(from),
                                std::move(closeto),
                                distance));
    }

    condition_wrapper insert_focus_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> types;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            types.push_back(pyobject_to_vref<std::string>(*it));
        }
        return condition_wrapper(std::make_shared<Condition::FocusType>(std::move(types)));
    }

    condition_wrapper insert_empire_stockpile_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        auto resource = boost::python::extract<enum_wrapper<ResourceType>>(kw["resource"])();

        std::unique_ptr<ValueRef::ValueRef<double>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<double>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["high"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::EmpireStockpileValue>(
            std::move(empire),
            resource.value,
            std::move(low),
            std::move(high)));
    }

    condition_wrapper insert_empire_has_adopted_policy_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        return condition_wrapper(std::make_shared<Condition::EmpireHasAdoptedPolicy>(
            std::move(empire),
            std::move(name)));
    }

    condition_wrapper insert_resupplyable_by_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        return condition_wrapper(std::make_shared<Condition::FleetSupplyableByEmpire>(std::move(empire)));
    }

    condition_wrapper insert_design_has_part_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["high"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::DesignHasPart>(
            std::move(name),
            std::move(low),
            std::move(high)));
    }

    condition_wrapper insert_building_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef<std::string>>> names;
        
        if (kw.has_key("name")) {
            boost::python::stl_input_iterator<boost::python::object> it_begin(kw["name"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                auto name_arg = boost::python::extract<value_ref_wrapper<std::string>>(*it);
                if (name_arg.check()) {
                    names.push_back(ValueRef::CloneUnique(name_arg().value_ref));
                } else {
                    names.push_back(std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(*it)()));
                }
            }
        }

        return condition_wrapper(std::make_shared<Condition::Building>(std::move(names)));
    }

    condition_wrapper insert_location_(const boost::python::tuple& args, const boost::python::dict& kw) {
        Condition::ContentType content_type = boost::python::extract<enum_wrapper<Condition::ContentType>>(kw["type"])().value;

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name2;
        if (kw.has_key("name2")) {
            auto name2_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name2"]);
            if (name2_args.check()) {
                name2 = ValueRef::CloneUnique(name2_args().value_ref);
            } else {
                name2 = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name2"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::Location>(
            content_type,
            std::move(name),
            std::move(name2)));
    }

    condition_wrapper insert_enqueued_(const boost::python::tuple& args, const boost::python::dict& kw) {
        BuildType build_type = BuildType::INVALID_BUILD_TYPE;
        if (kw.has_key("type")) {
            build_type = boost::python::extract<enum_wrapper<BuildType>>(kw["type"])().value;
        }

        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["high"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        if (kw.has_key("name")) {
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> design;
        if (kw.has_key("design")) {
            auto design_args = boost::python::extract<value_ref_wrapper<int>>(kw["design"]);
            if (design_args.check()) {
                design = ValueRef::CloneUnique(design_args().value_ref);
            } else {
                design = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["design"])());
            }
        }

        if (build_type == BuildType::BT_SHIP) {
            return condition_wrapper(std::make_shared<Condition::Enqueued>(
                std::move(design),
                std::move(empire),
                std::move(low),
                std::move(high)));
        } else {
            return condition_wrapper(std::make_shared<Condition::Enqueued>(
                build_type,
                std::move(name),
                std::move(empire),
                std::move(low),
                std::move(high)));
        }
    }

    condition_wrapper insert_number_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["high"])());
            }
        }

        auto condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["condition"])().condition);

        return condition_wrapper(std::make_shared<Condition::Number>(
            std::move(low),
            std::move(high),
            std::move(condition)));
    }

    condition_wrapper insert_produced_by_empire_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        return condition_wrapper(std::make_shared<Condition::ProducedByEmpire>(std::move(empire)));
    }

    condition_wrapper insert_owner_has_tech_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }
        return condition_wrapper(std::make_shared<Condition::OwnerHasTech>(std::move(name)));
    }

    condition_wrapper insert_random_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> probability;
        auto p_args = boost::python::extract<value_ref_wrapper<double>>(kw["probability"]);
        if (p_args.check()) {
            probability = ValueRef::CloneUnique(p_args().value_ref);
        } else {
            probability = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["probability"])());
        }
        return condition_wrapper(std::make_shared<Condition::Chance>(std::move(probability)));
    }

    condition_wrapper insert_star_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::vector<std::unique_ptr<ValueRef::ValueRef< ::StarType>>> types;
        boost::python::stl_input_iterator<boost::python::object> it_begin(kw["type"]), it_end;
        for (auto it = it_begin; it != it_end; ++it) {
            auto type_arg = boost::python::extract<value_ref_wrapper< ::StarType>>(*it);
            if (type_arg.check()) {
                types.push_back(ValueRef::CloneUnique(type_arg().value_ref));
            } else {
                types.push_back(std::make_unique<ValueRef::Constant< ::StarType>>(boost::python::extract<enum_wrapper< ::StarType>>(*it)().value));
            }
        }
        return condition_wrapper(std::make_shared<Condition::StarType>(std::move(types)));
    }

    condition_wrapper insert_in_system_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> system_id;
        if (kw.has_key("id")) {
            auto id_args = boost::python::extract<value_ref_wrapper<int>>(kw["id"]);
            if (id_args.check()) {
                system_id = ValueRef::CloneUnique(id_args().value_ref);
            } else {
                system_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["id"])());
            }
        }
        
        return condition_wrapper(std::make_shared<Condition::InOrIsSystem>(std::move(system_id)));
    }

    condition_wrapper insert_on_planet_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> planet_id;
        if (kw.has_key("id")) {
            auto id_args = boost::python::extract<value_ref_wrapper<int>>(kw["id"]);
            if (id_args.check()) {
                planet_id = ValueRef::CloneUnique(id_args().value_ref);
            } else {
                planet_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["id"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::OnPlanet>(std::move(planet_id)));
    }

    condition_wrapper insert_turn_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> low;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<int>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> high;
        if (kw.has_key("high")) {
            auto high_args = boost::python::extract<value_ref_wrapper<int>>(kw["high"]);
            if (high_args.check()) {
                high = ValueRef::CloneUnique(high_args().value_ref);
            } else {
                high = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["high"])());
            }
        }

        return condition_wrapper(std::make_shared<Condition::Turn>(std::move(low), std::move(high)));
    }

    condition_wrapper insert_resource_supply_connected_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
        if (empire_args.check()) {
            empire = ValueRef::CloneUnique(empire_args().value_ref);
        } else {
            empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
        }

        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();
        return condition_wrapper(std::make_shared<Condition::ResourceSupplyConnectedByEmpire>(std::move(empire),
            ValueRef::CloneUnique(condition.condition)));
    }

    condition_wrapper insert_within_starlane_jumps_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();

        std::unique_ptr<ValueRef::ValueRef<int>> jumps;
        auto jumps_args = boost::python::extract<value_ref_wrapper<int>>(kw["jumps"]);
        if (jumps_args.check()) {
            jumps = ValueRef::CloneUnique(jumps_args().value_ref);
        } else {
            jumps = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["jumps"])());
        }

        return condition_wrapper(std::make_shared<Condition::WithinStarlaneJumps>(std::move(jumps),
            ValueRef::CloneUnique(condition.condition)));
    }

    condition_wrapper insert_within_distance_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();

        std::unique_ptr<ValueRef::ValueRef<double>> distance;
        auto distance_args = boost::python::extract<value_ref_wrapper<double>>(kw["distance"]);
        if (distance_args.check()) {
            distance = ValueRef::CloneUnique(distance_args().value_ref);
        } else {
            distance = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["distance"])());
        }

        return condition_wrapper(std::make_shared<Condition::WithinDistance>(std::move(distance),
            ValueRef::CloneUnique(condition.condition)));
    }

    condition_wrapper insert_object_id_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> id;
        auto id_args = boost::python::extract<value_ref_wrapper<int>>(kw["id"]);
        if (id_args.check()) {
            id = ValueRef::CloneUnique(id_args().value_ref);
        } else {
            id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["id"])());
        }
        return condition_wrapper(std::make_shared<Condition::ObjectID>(std::move(id)));
    }

    condition_wrapper insert_described_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto description = boost::python::extract<std::string>(kw["description"])();
        auto condition = boost::python::extract<condition_wrapper>(kw["condition"])();

        return condition_wrapper(std::make_shared<Condition::Described>(
                                    ValueRef::CloneUnique(condition.condition),
                                    std::move(description)));
    }

    condition_wrapper insert_design_(const boost::python::tuple& args, const boost::python::dict& kw) {
        if (kw.has_key("name") && !kw.has_key("design")) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> name;
            auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (name_args.check()) {
                name = ValueRef::CloneUnique(name_args().value_ref);
            } else {
                name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
            return condition_wrapper(std::make_shared<Condition::PredefinedShipDesign>(std::move(name)));           
        } else if (kw.has_key("design") && !kw.has_key("name")) {
            std::unique_ptr<ValueRef::ValueRef<int>> design;
            auto design_args = boost::python::extract<value_ref_wrapper<int>>(kw["design"]);
            if (design_args.check()) {
                design = ValueRef::CloneUnique(design_args().value_ref);
            } else {
                design = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["design"])());
            }
            return condition_wrapper(std::make_shared<Condition::NumberedShipDesign>(std::move(design)));
        }

        throw std::runtime_error("Design requires only name or design keyword");
    }

    condition_wrapper insert_species_opinion_(const boost::python::tuple& args, const boost::python::dict& kw, Condition::ComparisonType cmp) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> species;
        if (kw.has_key("species")) {
            auto species_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["species"]);
            if (species_args.check()) {
                species = ValueRef::CloneUnique(species_args().value_ref);
            } else {
                species = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["species"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }
        return condition_wrapper(std::make_shared<Condition::SpeciesOpinion>(std::move(species),
            std::move(name),
            cmp));
    }
}

void RegisterGlobalsConditions(boost::python::dict& globals) {
    globals["Ship"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SHIP));
    globals["System"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SYSTEM));
    globals["Fleet"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_FLEET));
    globals["Monster"] = condition_wrapper(std::make_shared<Condition::Monster>());
    globals["Capital"] = condition_wrapper(std::make_shared<Condition::Capital>());
    globals["Stationary"] = condition_wrapper(std::make_shared<Condition::Stationary>());

    globals["Unowned"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE));
    globals["IsHuman"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_HUMAN));

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
    for (const auto& meter : std::initializer_list<std::pair<const char*, MeterType>>{
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
        const auto m = meter.second;
        const auto f_insert_meter_value = [m](const auto& args, const auto& kw) { return insert_meter_value_(args, kw, m); };
        globals[meter.first] = boost::python::raw_function(f_insert_meter_value);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, Condition::ContentType>>{
            {"ContentBuilding", Condition::ContentType::CONTENT_BUILDING},
            {"ContentSpecies",  Condition::ContentType::CONTENT_SPECIES},
            {"ContentHull",     Condition::ContentType::CONTENT_SHIP_HULL},
            {"ContentPart",     Condition::ContentType::CONTENT_SHIP_PART},
            {"ContentSpecial",  Condition::ContentType::CONTENT_SPECIAL},
            {"ContentFocus",    Condition::ContentType::CONTENT_FOCUS}})
    {
        globals[op.first] = enum_wrapper<Condition::ContentType>(op.second);
    }

    for (const auto& op : std::initializer_list<std::pair<const char*, Condition::SortingMethod>>{
            {"MaximumNumberOf", Condition::SortingMethod::SORT_MAX},
            {"MinimumNumberOf", Condition::SortingMethod::SORT_MIN},
            {"ModeNumberOf",    Condition::SortingMethod::SORT_MODE},
            {"UniqueNumberOf",  Condition::SortingMethod::SORT_UNIQUE}})
    {
        const auto sm = op.second;
        globals[op.first] = boost::python::raw_function([sm](const auto& args, const auto& kw) { return insert_sorted_number_of_(args, kw, sm); });
    }
    globals["NumberOf"] = boost::python::raw_function([](const auto& args, const auto& kw) { return insert_sorted_number_of_(args, kw, Condition::SortingMethod::SORT_RANDOM); });

    globals["HasSpecies"] = boost::python::raw_function(insert_has_species_);
    globals["IsField"] = boost::python::raw_function(insert_is_field_);
    globals["CanColonize"] = condition_wrapper(std::make_shared<Condition::CanColonize>());
    globals["Armed"] = condition_wrapper(std::make_shared<Condition::Armed>());

    globals["IsSource"] = condition_wrapper(std::make_shared<Condition::Source>());
    globals["IsTarget"] = condition_wrapper(std::make_shared<Condition::Target>());
    globals["IsRootCandidate"] = condition_wrapper(std::make_shared<Condition::RootCandidate>());
    globals["IsAnyObject"] = condition_wrapper(std::make_shared<Condition::All>());
    globals["NoObject"] = condition_wrapper(std::make_shared<Condition::None>());

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

