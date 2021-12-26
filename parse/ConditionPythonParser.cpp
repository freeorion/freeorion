#include "ConditionPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>

#include "../universe/Conditions.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

condition_wrapper operator&(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::And>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
}

condition_wrapper operator|(const condition_wrapper& lhs, const condition_wrapper& rhs) {
    return condition_wrapper(std::make_shared<Condition::Or>(
        lhs.condition->Clone(),
        rhs.condition->Clone()
    ));
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
        return condition_wrapper(std::make_shared<Condition::Contains>(ValueRef::CloneUnique(cond.condition)));
    }

    condition_wrapper insert_meter_value_(const boost::python::tuple& args, const boost::python::dict& kw, MeterType m) {
        std::unique_ptr<ValueRef::ValueRef<double>> low = nullptr;
        if (kw.has_key("low")) {
            auto low_args = boost::python::extract<value_ref_wrapper<double>>(kw["low"]);
            if (low_args.check()) {
                low = ValueRef::CloneUnique(low_args().value_ref);
            } else {
                low = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["low"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<double>> high = nullptr;
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
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetType>>(kw["type"], [](const value_ref_wrapper< ::PlanetType>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetType>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, types);
            return condition_wrapper(std::make_shared<Condition::PlanetType>(std::move(types)));
        } else if (kw.has_key("size")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>> sizes;
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetSize>>(kw["size"], [](const value_ref_wrapper< ::PlanetSize>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetSize>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, sizes);
            return condition_wrapper(std::make_shared<Condition::PlanetSize>(std::move(sizes)));

        } else if (kw.has_key("environment")) {
            std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>> environments;
            py_parse::detail::flatten_list<value_ref_wrapper< ::PlanetEnvironment>>(kw["environment"], [](const value_ref_wrapper< ::PlanetEnvironment>& o, std::vector<std::unique_ptr<ValueRef::ValueRef< ::PlanetEnvironment>>>& v) {
                v.push_back(ValueRef::CloneUnique(o.value_ref));
            }, environments);
            return condition_wrapper(std::make_shared<Condition::PlanetEnvironment>(std::move(environments)));
        }
        return condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_PLANET));
    }

    condition_wrapper insert_has_tag_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name = nullptr;
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
}

void RegisterGlobalsConditions(boost::python::dict& globals) {
    globals["Ship"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SHIP));
    globals["System"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_SYSTEM));
    globals["Fleet"] = condition_wrapper(std::make_shared<Condition::Type>(UniverseObjectType::OBJ_FLEET));

    globals["Unowned"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_NONE));
    globals["Human"] = condition_wrapper(std::make_shared<Condition::EmpireAffiliation>(EmpireAffiliationType::AFFIL_HUMAN));

    globals["Structure"] = boost::python::raw_function([] (auto args, auto kw) -> auto { return insert_meter_value_(args, kw, MeterType::METER_STRUCTURE);});

    globals["Species"] = condition_wrapper(std::make_shared<Condition::Species>());

    globals["HasTag"] = boost::python::raw_function(insert_has_tag_);
    globals["Planet"] = boost::python::raw_function(insert_planet_);
    globals["VisibleToEmpire"] = boost::python::raw_function(insert_visible_to_empire_);
    globals["OwnedBy"] = boost::python::raw_function(insert_owned_by_);
    globals["ContainedBy"] = insert_contained_by_;
    globals["Contains"] = insert_contains_;
}

