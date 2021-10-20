#include "EffectPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

#include "../universe/Effects.h"
#include "../universe/Enums.h"

#include "EnumPythonParser.h"
#include "ValueRefPythonParser.h"

namespace {
    effect_wrapper insert_generate_sit_rep_message_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto message = boost::python::extract<std::string>(kw["message"])();
        auto label = boost::python::extract<std::string>(kw["label"])();

        bool stringtable_lookup = true;
        if (kw.has_key("NoStringtableLookup")) {
            stringtable_lookup = false;
        }

        std::string icon = "";
        if (kw.has_key("icon")) {
            icon = boost::python::extract<std::string>(kw["icon"])();
        }

        std::vector<std::pair<std::string, std::unique_ptr<ValueRef::ValueRef<std::string>>>> parameters;
        if (kw.has_key("parameters")) {
            boost::python::dict param_args = boost::python::extract<boost::python::dict>(kw["parameters"]);
            boost::python::stl_input_iterator<std::string> p_begin(param_args.keys()), p_end;
            for (auto it = p_begin; it != p_end; ++it) {
                auto p_arg = boost::python::extract<value_ref_wrapper<std::string>>(param_args[*it]);
                if (p_arg.check()) {
                    parameters.emplace_back(*it, ValueRef::CloneUnique(p_arg().value_ref));
                } else {
                    auto p_arg_double = boost::python::extract<value_ref_wrapper<double>>(param_args[*it]);
                    if (p_arg_double.check()) {
                        parameters.emplace_back(*it, std::make_unique<ValueRef::StringCast<double>>(ValueRef::CloneUnique(p_arg_double().value_ref)));
                    } else {
                        auto p_arg_int = boost::python::extract<value_ref_wrapper<int>>(param_args[*it]);
                        if (p_arg_int.check())
                            parameters.emplace_back(*it, std::make_unique<ValueRef::StringCast<int>>(ValueRef::CloneUnique(p_arg_int().value_ref)));
                        else
                            parameters.emplace_back(*it, std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(param_args[*it])));
                    }
                }
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

        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_SELF;
        if (kw.has_key("affiliation")) {
            affiliation = boost::python::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;
        }

        std::unique_ptr<Condition::Condition> condition;
        if (kw.has_key("condition")) {
            condition = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["condition"])().condition);
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


    template <MeterType M>
    effect_wrapper insert_set_meter_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto value = boost::python::extract<value_ref_wrapper<double>>(kw["value"])();

        boost::optional<std::string> accountinglabel = boost::none;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = boost::python::extract<std::string>(kw["accountinglabel"])();
        }
        return effect_wrapper(std::make_shared<Effect::SetMeter>(M,
                                                                 ValueRef::CloneUnique(value.value_ref),
                                                                 accountinglabel));
    }

    unlockable_item_wrapper insert_item_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto type = boost::python::extract<enum_wrapper<UnlockableItemType>>(kw["type"])();
        auto name = boost::python::extract<std::string>(kw["name"])();
        return unlockable_item_wrapper(UnlockableItem(type.value, std::move(name)));
    }

    effect_group_wrapper insert_effects_group_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto scope = boost::python::extract<condition_wrapper>(kw["scope"])();
        int priority = 100;
        if (kw.has_key("priority")) {
            priority = boost::python::extract<int>(kw["priority"])();
        }

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        auto effects_args = boost::python::extract<boost::python::list>(kw["effects"]);
        if (effects_args.check()) {
            boost::python::stl_input_iterator<effect_wrapper> effects_begin(effects_args), effects_end;
            for (auto it = effects_begin; it != effects_end; ++it)
                effects.push_back(ValueRef::CloneUnique(it->effect));
        } else {
            effects.push_back(ValueRef::CloneUnique(boost::python::extract<effect_wrapper>(kw["effects"])().effect));
        }

        std::string stackinggroup = "";
        if (kw.has_key("stackinggroup")) {
            stackinggroup = boost::python::extract<std::string>(kw["stackinggroup"]);
        }

        std::unique_ptr<Condition::Condition> activation = nullptr;
        if (kw.has_key("activation")) {
           activation = ValueRef::CloneUnique(boost::python::extract<condition_wrapper>(kw["activation"])().condition);
        }
        // ToDo: implement other arguments later

        return effect_group_wrapper(std::make_shared<Effect::EffectsGroup>(ValueRef::CloneUnique(scope.condition),
                                                      std::move(activation),
                                                      std::move(effects),
                                                      "",
                                                      stackinggroup,
                                                      priority,
                                                      "",
                                                      ""));
    }
}

void RegisterGlobalsEffects(boost::python::dict& globals) {
    globals["EffectsGroup"] = boost::python::raw_function(insert_effects_group_);
    globals["Item"] = boost::python::raw_function(insert_item_);

    globals["Policy"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_POLICY);
    globals["Building"] = enum_wrapper<UnlockableItemType>(UnlockableItemType::UIT_BUILDING);

    globals["Destroy"] = effect_wrapper(std::make_shared<Effect::Destroy>());

    globals["GenerateSitRepMessage"] = boost::python::raw_function(insert_generate_sit_rep_message_);

    globals["SetMaxShield"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_MAX_SHIELD>);
    globals["SetShield"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_SHIELD>);
    globals["SetTargetPopulation"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_TARGET_POPULATION>);
    globals["SetDefense"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_DEFENSE>);
    globals["SetTroops"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_TROOPS>);
    globals["SetStructure"] = boost::python::raw_function(insert_set_meter_<MeterType::METER_STRUCTURE>);
}

