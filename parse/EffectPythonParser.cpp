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

    effect_wrapper insert_conditional_(const py::tuple& args, const py::dict& kw) {
        auto condition = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["condition"])().condition);

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        py_parse::detail::flatten_list<effect_wrapper>(kw["effects"], [](const effect_wrapper& o, std::vector<std::unique_ptr<Effect::Effect>> &v) {
            v.push_back(ValueRef::CloneUnique(o.effect));
        }, effects);
        
        std::vector<std::unique_ptr<Effect::Effect>> else_;
        if (kw.has_key("else_")) {
            py_parse::detail::flatten_list<effect_wrapper>(kw["else_"], [](const effect_wrapper& o, std::vector<std::unique_ptr<Effect::Effect>> &v) {
                v.push_back(ValueRef::CloneUnique(o.effect));
            }, else_);
        }

        return effect_wrapper(std::make_shared<Effect::Conditional>(std::move(condition),
                                                                    std::move(effects),
                                                                    std::move(else_)));
    }

    effect_wrapper insert_set_meter_(const MeterType m, const py::tuple& args, const py::dict& kw) {
        auto value = py::extract<value_ref_wrapper<double>>(kw["value"])();

        boost::optional<std::string> accountinglabel;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = py::extract<std::string>(kw["accountinglabel"])();
        }
        return effect_wrapper(std::make_shared<Effect::SetMeter>(
            m,
            ValueRef::CloneUnique(value.value_ref),
            accountinglabel));
    }

    effect_wrapper insert_ship_part_set_meter_(const MeterType m, const py::tuple& args, const py::dict& kw) {
        auto value = py::extract<value_ref_wrapper<double>>(kw["value"])();

        std::unique_ptr<ValueRef::ValueRef<std::string>> partname;
        auto partname_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["partname"]);
        if (partname_args.check()) {
            partname = ValueRef::CloneUnique(partname_args().value_ref);
        } else {
            partname = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["partname"])());
        }

        return effect_wrapper(std::make_shared<Effect::SetShipPartMeter>(
            m,
            std::move(partname),
            ValueRef::CloneUnique(value.value_ref)));
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

    effect_wrapper insert_set_owner_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }
       
        return effect_wrapper(std::make_shared<Effect::SetOwner>(std::move(empire)));
    }

    effect_wrapper victory(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto reason = boost::python::extract<std::string>(kw["reason"])();
        return effect_wrapper(std::make_shared<Effect::Victory>(reason));
    }

    effect_wrapper add_special(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> capacity;
        if (kw.has_key("capacity")) {
            auto capacity_args = boost::python::extract<value_ref_wrapper<double>>(kw["capacity"]);
            if (capacity_args.check()) {
                capacity = ValueRef::CloneUnique(capacity_args().value_ref);
            } else {
                capacity = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["capacity"])());
            }
        }

        return effect_wrapper(std::make_shared<Effect::AddSpecial>(std::move(name), std::move(capacity)));
    }

    effect_wrapper remove_special(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        return effect_wrapper(std::make_shared<Effect::RemoveSpecial>(std::move(name)));
    }
}

void RegisterGlobalsEffects(py::dict& globals) {
    globals["EffectsGroup"] = py::raw_function(insert_effects_group_);
    globals["Item"] = py::raw_function(insert_item_);

    globals["Destroy"] = effect_wrapper(std::make_shared<Effect::Destroy>());

    globals["GenerateSitRepMessage"] = py::raw_function(insert_generate_sit_rep_message_);
    globals["Conditional"] = py::raw_function(insert_conditional_);

    globals["SetEmpireMeter"] = py::raw_function(set_empire_meter);
    globals["Victory"] = py::raw_function(victory);
    globals["AddSpecial"] = py::raw_function(add_special);
    globals["RemoveSpecial"] = py::raw_function(remove_special);

    // set_non_ship_part_meter_enum_grammar
    for (const auto& meter : std::initializer_list<std::pair<const char*, MeterType>>{
            {"SetTargetConstruction", MeterType::METER_TARGET_CONSTRUCTION},
            {"SetTargetIndustry",     MeterType::METER_TARGET_INDUSTRY},
            {"SetTargetPopulation",   MeterType::METER_TARGET_POPULATION},
            {"SetTargetResearch",     MeterType::METER_TARGET_RESEARCH},
            {"SetTargetInfluence",    MeterType::METER_TARGET_INFLUENCE},
            {"SetTargetHappiness",    MeterType::METER_TARGET_HAPPINESS},

            {"SetMaxDefense",         MeterType::METER_MAX_DEFENSE},
            {"SetMaxFuel",            MeterType::METER_MAX_FUEL},
            {"SetMaxShield",          MeterType::METER_MAX_SHIELD},
            {"SetMaxStructure",       MeterType::METER_MAX_STRUCTURE},
            {"SetMaxTroops",          MeterType::METER_MAX_TROOPS},
            {"SetMaxSupply",          MeterType::METER_MAX_SUPPLY},
            {"SetMaxStockpile",       MeterType::METER_MAX_STOCKPILE},

            {"SetConstruction",       MeterType::METER_CONSTRUCTION},
            {"SetIndustry",           MeterType::METER_INDUSTRY},
            {"SetPopulation",         MeterType::METER_POPULATION},
            {"SetResearch",           MeterType::METER_RESEARCH},
            {"SetInfluence",          MeterType::METER_INFLUENCE},
            {"SetHappiness",          MeterType::METER_HAPPINESS},

            {"SetDefense",            MeterType::METER_DEFENSE},
            {"SetFuel",               MeterType::METER_FUEL},
            {"SetShield",             MeterType::METER_SHIELD},
            {"SetStructure",          MeterType::METER_STRUCTURE},
            {"SetTroops",             MeterType::METER_TROOPS},
            {"SetSupply",             MeterType::METER_SUPPLY},
            {"SetStockpile",          MeterType::METER_STOCKPILE},

            {"SetRebelTroops",        MeterType::METER_REBEL_TROOPS},
            {"SetStealth",            MeterType::METER_STEALTH},
            {"SetDetection",          MeterType::METER_DETECTION},
            {"SetSpeed",              MeterType::METER_SPEED},

            {"SetSize",               MeterType::METER_SIZE}})
    {
        const auto m = meter.second;
        const auto f_insert_set_meter = [m](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_set_meter_(m, args, kw); };
        globals[meter.first] = boost::python::raw_function(f_insert_set_meter);
    }

    // set_ship_part_meter_enum_grammar
    for (const auto& meter : std::initializer_list<std::pair<const char*, MeterType>>{
            {"SetMaxCapacity",      MeterType::METER_MAX_CAPACITY},
            {"SetMaxDamage",        MeterType::METER_MAX_CAPACITY},
            {"SetMaxSecondaryStat", MeterType::METER_MAX_SECONDARY_STAT},
            {"SetCapacity",         MeterType::METER_CAPACITY},
            {"SetDamage",           MeterType::METER_CAPACITY},
            {"SetSecondaryStat",    MeterType::METER_SECONDARY_STAT}})
    {
        const auto m = meter.second;
        const auto f_insert_set_meter = [m](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_ship_part_set_meter_(m, args, kw); };
        globals[meter.first] = boost::python::raw_function(f_insert_set_meter);
    }

    globals["SetEmpireStockpile"] = py::raw_function(insert_set_empire_stockpile);
    globals["SetOwner"] = py::raw_function(insert_set_owner_);
}

