#include "EffectPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

#include "../universe/Effects.h"
#include "../universe/Enums.h"

#include "EnumPythonParser.h"
#include "PythonParserImpl.h"
#include "ValueRefPythonParser.h"

namespace py = boost::python;

namespace {
    effect_wrapper insert_generate_sit_rep_message_(const py::tuple& args, const py::dict& kw) {
        auto message = py::extract<std::string>(kw["message"])();
        auto label = py::extract<std::string>(kw["label"])();

        bool stringtable_lookup = true;
        if (kw.has_key("NoStringtableLookup")) {
            stringtable_lookup = false;
        }

        std::string icon;
        if (kw.has_key("icon")) {
            icon = py::extract<std::string>(kw["icon"])();
        }

        std::vector<std::pair<std::string, std::unique_ptr<ValueRef::ValueRef<std::string>>>> parameters;
        if (kw.has_key("parameters")) {
            py::dict param_args = py::extract<py::dict>(kw["parameters"]);
            py::stl_input_iterator<std::string> p_begin(param_args.keys()), p_end;
            for (auto it = p_begin; it != p_end; ++it) {
                if (auto p_arg = py::extract<value_ref_wrapper<std::string>>(param_args[*it]); p_arg.check()) {
                    parameters.emplace_back(*it, ValueRef::CloneUnique(p_arg().value_ref));
                } else if (auto p_arg_double = py::extract<value_ref_wrapper<double>>(param_args[*it]); p_arg_double.check()) {
                    parameters.emplace_back(*it, std::make_unique<ValueRef::StringCast<double>>(ValueRef::CloneUnique(p_arg_double().value_ref)));
                } else if (auto p_arg_int = py::extract<value_ref_wrapper<int>>(param_args[*it]); p_arg_int.check()) {
                    parameters.emplace_back(*it, std::make_unique<ValueRef::StringCast<int>>(ValueRef::CloneUnique(p_arg_int().value_ref)));
                } else {
                    parameters.emplace_back(*it, std::make_unique<ValueRef::Constant<std::string>>(py::extract<std::string>(param_args[*it])));
                }
            }
        }

        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = py::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(py::extract<int>(kw["empire"])());
            }
        }

        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;
        if (kw.has_key("affiliation")) {
            affiliation = py::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        std::unique_ptr<Condition::Condition> condition;
        if (kw.has_key("condition")) {
            condition = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["condition"])().condition);
        }

        return effect_wrapper(std::make_shared<Effect::GenerateSitRepMessage>(
                                    message,
                                    icon,
                                    std::move(parameters),
                                    std::move(empire),
                                    affiliation,
                                    std::move(label),
                                    stringtable_lookup
                              ));
    }

    effect_wrapper insert_if_(const py::tuple& args, const py::dict& kw) {
        auto condition = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["condition"])().condition);

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        py_parse::detail::flatten_list<effect_wrapper>(kw["effects"], [](const effect_wrapper& o, std::vector<std::unique_ptr<Effect::Effect>> &v) {
            v.push_back(ValueRef::CloneUnique(o.effect));
        }, effects);
        
        std::vector<std::unique_ptr<Effect::Effect>> else_;
        py_parse::detail::flatten_list<effect_wrapper>(kw["else_"], [](const effect_wrapper& o, std::vector<std::unique_ptr<Effect::Effect>> &v) {
            v.push_back(ValueRef::CloneUnique(o.effect));
        }, else_);

        return effect_wrapper(std::make_shared<Effect::Conditional>(std::move(condition),
                                                                    std::move(effects),
                                                                    std::move(else_)));
    }

    template <MeterType M>
    effect_wrapper insert_set_meter_(const py::tuple& args, const py::dict& kw) {
        auto value = py::extract<value_ref_wrapper<double>>(kw["value"])();

        boost::optional<std::string> accountinglabel;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = py::extract<std::string>(kw["accountinglabel"])();
        }
        return effect_wrapper(std::make_shared<Effect::SetMeter>(
            M,
            ValueRef::CloneUnique(value.value_ref),
            accountinglabel));
    }

    unlockable_item_wrapper insert_item_(const py::tuple& args, const py::dict& kw) {
        auto type = py::extract<enum_wrapper<UnlockableItemType>>(kw["type"])();
        auto name = py::extract<std::string>(kw["name"])();
        return unlockable_item_wrapper(UnlockableItem(type.value, std::move(name)));
    }

    effect_group_wrapper insert_effects_group_(const py::tuple& args, const py::dict& kw) {
        auto scope = py::extract<condition_wrapper>(kw["scope"])();
        int priority = 100;
        if (kw.has_key("priority")) {
            priority = py::extract<int>(kw["priority"])();
        }

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        auto effects_args = py::extract<py::list>(kw["effects"]);
        if (effects_args.check()) {
            py::stl_input_iterator<effect_wrapper> effects_begin(effects_args), effects_end;
            for (auto it = effects_begin; it != effects_end; ++it)
                effects.push_back(ValueRef::CloneUnique(it->effect));
        } else {
            effects.push_back(ValueRef::CloneUnique(py::extract<effect_wrapper>(kw["effects"])().effect));
        }

        std::string stackinggroup;
        if (kw.has_key("stackinggroup")) {
            stackinggroup = py::extract<std::string>(kw["stackinggroup"])();
        }

        std::unique_ptr<Condition::Condition> activation;
        if (kw.has_key("activation")) {
           activation = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["activation"])().condition);
        }

        std::string accountinglabel;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = py::extract<std::string>(kw["accountinglabel"])();
        }
        // ToDo: implement other arguments later

        return effect_group_wrapper(std::make_shared<Effect::EffectsGroup>(ValueRef::CloneUnique(scope.condition),
                                                      std::move(activation),
                                                      std::move(effects),
                                                      std::move(accountinglabel),
                                                      std::move(stackinggroup),
                                                      priority,
                                                      "",
                                                      ""));
    }

    effect_wrapper set_empire_meter(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto meter = boost::python::extract<std::string>(kw["meter"])();
        auto value = ValueRef::CloneUnique(boost::python::extract<value_ref_wrapper<double>>(kw["value"])().value_ref);
        if (kw.has_key("empire")) {
            auto empire = ValueRef::CloneUnique(boost::python::extract<value_ref_wrapper<int>>(kw["empire"])().value_ref);
            return effect_wrapper(std::make_shared<Effect::SetEmpireMeter>(std::move(empire),
                                                                           meter,
                                                                           std::move(value)));
        } else {
            return effect_wrapper(std::make_shared<Effect::SetEmpireMeter>(meter,
                                                                           std::move(value)));
        }
    }


    effect_wrapper insert_set_empire_stockpile(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        auto resource = boost::python::extract<enum_wrapper<ResourceType>>(kw["resource"])();

        std::unique_ptr<ValueRef::ValueRef<double>> value;
        auto value_args = boost::python::extract<value_ref_wrapper<double>>(kw["value"]);
        if (value_args.check()) {
            value = ValueRef::CloneUnique(value_args().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
        }

        if (empire)
            return effect_wrapper(std::make_shared<Effect::SetEmpireStockpile>(std::move(empire), resource.value, std::move(value)));
        else 
            return effect_wrapper(std::make_shared<Effect::SetEmpireStockpile>(resource.value, std::move(value)));
    }
}

void RegisterGlobalsEffects(py::dict& globals) {
    globals["EffectsGroup"] = py::raw_function(insert_effects_group_);
    globals["Item"] = py::raw_function(insert_item_);

    globals["Policy"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_POLICY);
    globals["Building"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_BUILDING);
    globals["ShipPart"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_SHIP_PART);

    globals["Destroy"] = effect_wrapper(std::make_shared<Effect::Destroy>());

    globals["GenerateSitRepMessage"] = py::raw_function(insert_generate_sit_rep_message_);
    globals["If"] = py::raw_function(insert_if_);

    globals["SetEmpireMeter"] = py::raw_function(set_empire_meter);

    globals["SetConstruction"] = py::raw_function(insert_set_meter_<MeterType::METER_CONSTRUCTION>);
    globals["SetDefense"] = py::raw_function(insert_set_meter_<MeterType::METER_DEFENSE>);
    globals["SetHappiness"] = py::raw_function(insert_set_meter_<MeterType::METER_HAPPINESS>);   
    globals["SetIndustry"] = py::raw_function(insert_set_meter_<MeterType::METER_INDUSTRY>);
    globals["SetMaxShield"] = py::raw_function(insert_set_meter_<MeterType::METER_MAX_SHIELD>);
    globals["SetMaxSupply"] = py::raw_function(insert_set_meter_<MeterType::METER_MAX_SUPPLY>);
    globals["SetMaxTroops"] = py::raw_function(insert_set_meter_<MeterType::METER_MAX_TROOPS>);
    globals["SetResearch"] = py::raw_function(insert_set_meter_<MeterType::METER_RESEARCH>);
    globals["SetShield"] = py::raw_function(insert_set_meter_<MeterType::METER_SHIELD>);
    globals["SetStockpile"] = py::raw_function(insert_set_meter_<MeterType::METER_STOCKPILE>);
    globals["SetStructure"] = py::raw_function(insert_set_meter_<MeterType::METER_STRUCTURE>);
    globals["SetTargetConstruction"] = py::raw_function(insert_set_meter_<MeterType::METER_TARGET_CONSTRUCTION>);
    globals["SetTargetPopulation"] = py::raw_function(insert_set_meter_<MeterType::METER_TARGET_POPULATION>);
    globals["SetTroops"] = py::raw_function(insert_set_meter_<MeterType::METER_TROOPS>);

    globals["SetEmpireStockpile"] = py::raw_function(insert_set_empire_stockpile);
}

