#include "EffectPythonParser.h"

#include <boost/python/extract.hpp>
#include <boost/python/raw_function.hpp>
#include <boost/python/stl_iterator.hpp>

#include "../universe/Effects.h"
#include "../universe/Enums.h"
#include "../universe/Species.h"

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
        boost::python::stl_input_iterator<effect_wrapper> effects_begin(kw["effects"]), effects_end;
        for (auto it = effects_begin; it != effects_end; ++ it) {
            effects.push_back(ValueRef::CloneUnique(it->effect));
        }

        std::vector<std::unique_ptr<Effect::Effect>> else_;
        if (kw.has_key("else_")) {
            boost::python::stl_input_iterator<effect_wrapper> else_begin(kw["else_"]);
            for (auto it = else_begin; it != effects_end; ++ it) {
                    else_.push_back(ValueRef::CloneUnique(it->effect));
            }
        }

        return effect_wrapper(std::make_shared<Effect::Conditional>(std::move(condition),
                                                                    std::move(effects),
                                                                    std::move(else_)));
    }

    effect_wrapper insert_set_meter_(const MeterType m, const py::tuple& args, const py::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> value;
        auto value_arg = py::extract<value_ref_wrapper<double>>(kw["value"]);
        if (value_arg.check()) {
            value = ValueRef::CloneUnique(value_arg().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
        }

        boost::optional<std::string> accountinglabel;
        if (kw.has_key("accountinglabel")) {
            accountinglabel = py::extract<std::string>(kw["accountinglabel"])();
        }
        return effect_wrapper(std::make_shared<Effect::SetMeter>(
            m,
            std::move(value),
            accountinglabel));
    }

    effect_wrapper insert_ship_part_set_meter_(const MeterType m, const py::tuple& args, const py::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> value;
        auto value_arg = py::extract<value_ref_wrapper<double>>(kw["value"]);
        if (value_arg.check()) {
            value = ValueRef::CloneUnique(value_arg().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
        }

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
            std::move(value)));
    }

    unlockable_item_wrapper insert_item_(const py::tuple& args, const py::dict& kw) {
        auto type = py::extract<enum_wrapper<UnlockableItemType>>(kw["type"])();
        auto name = py::extract<std::string>(kw["name"])();
        return unlockable_item_wrapper(UnlockableItem(type.value, std::move(name)));
    }

    effect_group_wrapper insert_effects_group_(const py::tuple& args, const py::dict& kw) {
        auto scope = py::extract<condition_wrapper>(kw["scope"])();
        int priority = 100;
        if (kw.has_key("priority"))
            priority = py::extract<int>(kw["priority"])();

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
        if (kw.has_key("stackinggroup"))
            stackinggroup = py::extract<std::string>(kw["stackinggroup"])();

        std::unique_ptr<Condition::Condition> activation;
        if (kw.has_key("activation")) {
           if (py::extract<py::object>(kw["activation"])().is_none()) {
               activation = std::make_unique<Condition::None>();
           } else {
               activation = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["activation"])().condition);
           }
        }

        std::string accountinglabel;
        if (kw.has_key("accountinglabel"))
            accountinglabel = py::extract<std::string>(kw["accountinglabel"])();

        std::string description;
        if (kw.has_key("description"))
            description = py::extract<std::string>(kw["description"])();

        return effect_group_wrapper(std::make_shared<Effect::EffectsGroup>(ValueRef::CloneUnique(scope.condition),
                                                      std::move(activation),
                                                      std::move(effects),
                                                      std::move(accountinglabel),
                                                      std::move(stackinggroup),
                                                      priority,
                                                      std::move(description),
                                                      ""));
    }

    effect_wrapper set_empire_meter(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto meter = boost::python::extract<std::string>(kw["meter"])();

        std::unique_ptr<ValueRef::ValueRef<double>> value;
        auto value_args = boost::python::extract<value_ref_wrapper<double>>(kw["value"]);
        if (value_args.check()) {
            value = ValueRef::CloneUnique(value_args().value_ref);
        } else {
            value = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["value"])());
        }

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

    effect_wrapper insert_set_star_type_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef< ::StarType>> star_type;
        auto star_type_arg = py::extract<value_ref_wrapper< ::StarType>>(kw["type"]);
        if (star_type_arg.check()) {
            star_type = ValueRef::CloneUnique(star_type_arg().value_ref);
        } else {
            star_type = std::make_unique<ValueRef::Constant< ::StarType>>(boost::python::extract<enum_wrapper< ::StarType>>(kw["type"])().value);
        }
        return effect_wrapper(std::make_shared<Effect::SetStarType>(std::move(star_type)));
    }

    effect_wrapper insert_move_to_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto destination = py::extract<condition_wrapper>(kw["destination"])();
        return effect_wrapper(std::make_shared<Effect::MoveTo>(ValueRef::CloneUnique(destination.condition)));
    }

    effect_wrapper insert_move_towards_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<double>> speed;
        auto speed_args = boost::python::extract<value_ref_wrapper<double>>(kw["speed"]);
        if (speed_args.check()) {
            speed = ValueRef::CloneUnique(speed_args().value_ref);
        } else {
            speed = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["speed"])());
        }

        if (kw.has_key("target")) {
            auto target = py::extract<condition_wrapper>(kw["target"])();
            return effect_wrapper(std::make_shared<Effect::MoveTowards>(std::move(speed),
                ValueRef::CloneUnique(target.condition)));
        } else if (kw.has_key("x") && kw.has_key("y")) {
            std::unique_ptr<ValueRef::ValueRef<double>> x;
            auto x_args = boost::python::extract<value_ref_wrapper<double>>(kw["x"]);
            if (x_args.check()) {
                x = ValueRef::CloneUnique(x_args().value_ref);
            } else {
                x = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["x"])());
            }

            std::unique_ptr<ValueRef::ValueRef<double>> y;
            auto y_args = boost::python::extract<value_ref_wrapper<double>>(kw["y"]);
            if (y_args.check()) {
                y = ValueRef::CloneUnique(y_args().value_ref);
            } else {
                y = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["y"])());
            }

            return effect_wrapper(std::make_shared<Effect::MoveTowards>(std::move(speed),
                std::move(x),
                std::move(y)));
        }

        throw std::runtime_error(std::string("Not implemented in ") + __func__);
    }

    effect_wrapper insert_set_planet_size_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<PlanetSize>> planetsize;
        auto size_arg = boost::python::extract<value_ref_wrapper< ::PlanetSize>>(kw["planetsize"]);
        if (size_arg.check()) {
            planetsize = ValueRef::CloneUnique(size_arg().value_ref);
        } else {
            planetsize = std::make_unique<ValueRef::Constant< ::PlanetSize>>(boost::python::extract<enum_wrapper< ::PlanetSize>>(kw["planetsize"])().value);
        }
        return effect_wrapper(std::make_shared<Effect::SetPlanetSize>(std::move(planetsize)));
    }

    effect_wrapper insert_set_planet_type_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<PlanetType>> type;
        auto type_arg = boost::python::extract<value_ref_wrapper< ::PlanetType>>(kw["type"]);
        if (type_arg.check()) {
            type = ValueRef::CloneUnique(type_arg().value_ref);
        } else {
            type = std::make_unique<ValueRef::Constant< ::PlanetType>>(boost::python::extract<enum_wrapper< ::PlanetType>>(kw["type"])().value);
        }
        return effect_wrapper(std::make_shared<Effect::SetPlanetType>(std::move(type)));
    }

    effect_wrapper insert_set_visibility_(const boost::python::tuple& args, const boost::python::dict& kw) {
        EmpireAffiliationType affiliation = EmpireAffiliationType::AFFIL_ANY;
        std::unique_ptr<ValueRef::ValueRef<int>> empire;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check())
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            else
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            affiliation = EmpireAffiliationType::AFFIL_SELF;
        }

        if (kw.has_key("affiliation"))
            affiliation = py::extract<enum_wrapper<EmpireAffiliationType>>(kw["affiliation"])().value;

        std::unique_ptr<ValueRef::ValueRef<Visibility>> visibility;
        auto vis_arg = boost::python::extract<value_ref_wrapper<Visibility>>(kw["visibility"]);
        if (vis_arg.check())
            visibility = ValueRef::CloneUnique(vis_arg().value_ref);
        else
            visibility = std::make_unique<ValueRef::Constant<Visibility>>(boost::python::extract<enum_wrapper<Visibility>>(kw["visibility"])().value);

        std::unique_ptr<Condition::Condition> condition;
        if (kw.has_key("condition"))
            condition = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["condition"])().condition);

        return effect_wrapper(std::make_shared<Effect::SetVisibility>(std::move(visibility),
            affiliation,
            std::move(empire),
            std::move(condition)));
    }

    effect_wrapper insert_add_starlanes_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> endpoint = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["endpoint"])().condition);

        return effect_wrapper(std::make_shared<Effect::AddStarlanes>(std::move(endpoint)));
    }

    effect_wrapper insert_remove_starlanes_(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<Condition::Condition> endpoint = ValueRef::CloneUnique(py::extract<condition_wrapper>(kw["endpoint"])().condition);

        return effect_wrapper(std::make_shared<Effect::RemoveStarlanes>(std::move(endpoint)));
    }

    effect_wrapper insert_give_empire_item_(UnlockableItemType item, const boost::python::tuple& args, const boost::python::dict& kw) {
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

        return effect_wrapper(std::make_shared<Effect::GiveEmpireContent>(std::move(name),
            item,
            std::move(empire)));
    }

    effect_wrapper insert_set_species_opinion_(bool target, const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> species_name;
        auto species_name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["species"]);
        if (species_name_args.check()) {
            species_name = ValueRef::CloneUnique(species_name_args().value_ref);
        } else {
            species_name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["species"])());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> opinion;
        auto opinion_args = boost::python::extract<value_ref_wrapper<double>>(kw["opinion"]);
        if (opinion_args.check()) {
            opinion = ValueRef::CloneUnique(opinion_args().value_ref);
        } else {
            opinion = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["opinion"])());
        }

        if (kw.has_key("empire")) {
            std::unique_ptr<ValueRef::ValueRef<int>> empire;
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }

            return effect_wrapper(std::make_shared<Effect::SetSpeciesEmpireOpinion>(std::move(species_name),
                std::move(empire),
                std::move(opinion),
                target));
        } else if (kw.has_key("species2")) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> species2_name;
            auto species2_name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["species2"]);
            if (species2_name_args.check()) {
                species2_name = ValueRef::CloneUnique(species2_name_args().value_ref);
            } else {
                species2_name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["species2"])());
            }

            return effect_wrapper(std::make_shared<Effect::SetSpeciesSpeciesOpinion>(std::move(species_name),
                std::move(species2_name),
                std::move(opinion),
                target));
        } else {
            throw std::runtime_error(std::string("Unknown species opinion ") + __func__);
        }
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

    effect_wrapper create_ship(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<int>> empire_id;
        if (kw.has_key("empire")) {
            auto empire_args = boost::python::extract<value_ref_wrapper<int>>(kw["empire"]);
            if (empire_args.check()) {
                empire_id = ValueRef::CloneUnique(empire_args().value_ref);
            } else {
                empire_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["empire"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> species_name;
        if (kw.has_key("species")) {
            auto species_name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["species"]);
            if (species_name_args.check()) {
                species_name = ValueRef::CloneUnique(species_name_args().value_ref);
            } else {
                species_name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["species"])());
            }
        }

        std::unique_ptr<ValueRef::ValueRef<std::string>> ship_name;
        if (kw.has_key("name")) {
            auto ship_name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
            if (ship_name_args.check()) {
                ship_name = ValueRef::CloneUnique(ship_name_args().value_ref);
            } else {
                ship_name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
            }
        }

        std::vector<std::unique_ptr<Effect::Effect>> effects_to_apply_after;
        if (kw.has_key("effects")) {
            boost::python::stl_input_iterator<effect_wrapper> it_begin(kw["effects"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                effects_to_apply_after.push_back(ValueRef::CloneUnique(it->effect));
            }
        }

        if (kw.has_key("designid")) {
            std::unique_ptr<ValueRef::ValueRef<int>> ship_design_id;
            auto designid_args = boost::python::extract<value_ref_wrapper<int>>(kw["designid"]);
            if (designid_args.check()) {
                ship_design_id = ValueRef::CloneUnique(designid_args().value_ref);
            } else {
                ship_design_id = std::make_unique<ValueRef::Constant<int>>(boost::python::extract<int>(kw["designid"])());
            }
            return effect_wrapper(std::make_shared<Effect::CreateShip>(std::move(ship_design_id),
                std::move(empire_id),
                std::move(species_name),
                std::move(ship_name),
                std::move(effects_to_apply_after)));
        } else if (kw.has_key("designname")) {
            std::unique_ptr<ValueRef::ValueRef<std::string>> predefined_ship_design_name;
            auto designname_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["designname"]);
            if (designname_args.check()) {
                predefined_ship_design_name = ValueRef::CloneUnique(designname_args().value_ref);
            } else {
                predefined_ship_design_name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["designname"])());
            }
            return effect_wrapper(std::make_shared<Effect::CreateShip>(std::move(predefined_ship_design_name),
                std::move(empire_id),
                std::move(species_name),
                std::move(ship_name),
                std::move(effects_to_apply_after)));
        } else {
            throw std::runtime_error(std::string(" ") + __func__);
        }
    }

    effect_wrapper create_building(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> type;
        auto type_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["type"]);
        if (type_args.check()) {
            type = ValueRef::CloneUnique(type_args().value_ref);
        } else {
            type = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["type"])());
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

        std::vector<std::unique_ptr<Effect::Effect>> effects_to_apply_after;
        if (kw.has_key("effects")) {
            boost::python::stl_input_iterator<effect_wrapper> it_begin(kw["effects"]), it_end;
            for (auto it = it_begin; it != it_end; ++it) {
                effects_to_apply_after.push_back(ValueRef::CloneUnique(it->effect));
            }
        }

        return effect_wrapper(std::make_shared<Effect::CreateBuilding>(std::move(type),
            std::move(name),
            std::move(effects_to_apply_after)));
    }

    effect_wrapper set_focus(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        return effect_wrapper(std::make_shared<Effect::SetFocus>(std::move(name)));
    }

    effect_wrapper set_species(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> name;
        auto name_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["name"]);
        if (name_args.check()) {
            name = ValueRef::CloneUnique(name_args().value_ref);
        } else {
            name = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["name"])());
        }

        return effect_wrapper(std::make_shared<Effect::SetSpecies>(std::move(name)));
    }

    effect_wrapper create_field(const boost::python::tuple& args, const boost::python::dict& kw) {
        std::unique_ptr<ValueRef::ValueRef<std::string>> type;
        auto type_args = boost::python::extract<value_ref_wrapper<std::string>>(kw["type"]);
        if (type_args.check()) {
            type = ValueRef::CloneUnique(type_args().value_ref);
        } else {
            type = std::make_unique<ValueRef::Constant<std::string>>(boost::python::extract<std::string>(kw["type"])());
        }

        std::unique_ptr<ValueRef::ValueRef<double>> size;
        auto size_arg = py::extract<value_ref_wrapper<double>>(kw["size"]);
        if (size_arg.check()) {
            size = ValueRef::CloneUnique(size_arg().value_ref);
        } else {
            size = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["size"])());
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

        std::vector<std::unique_ptr<Effect::Effect>> effects;
        if (kw.has_key("effects")) {
            py::stl_input_iterator<effect_wrapper> effects_begin(kw["effects"]), effects_end;
            for (auto it = effects_begin; it != effects_end; ++ it) {
                effects.push_back(ValueRef::CloneUnique(it->effect));
            }
        }

        if (kw.has_key("x") && kw.has_key("y")) {
            std::unique_ptr<ValueRef::ValueRef<double>> x;
            auto x_arg = py::extract<value_ref_wrapper<double>>(kw["x"]);
            if (x_arg.check()) {
                x = ValueRef::CloneUnique(x_arg().value_ref);
            } else {
                x = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["x"])());
            }

            std::unique_ptr<ValueRef::ValueRef<double>> y;
            auto y_arg = py::extract<value_ref_wrapper<double>>(kw["y"]);
            if (y_arg.check()) {
                y = ValueRef::CloneUnique(y_arg().value_ref);
            } else {
                y = std::make_unique<ValueRef::Constant<double>>(boost::python::extract<double>(kw["y"])());
            }

            return effect_wrapper(std::make_shared<Effect::CreateField>(std::move(type),
                                                                        std::move(x),
                                                                        std::move(y),
                                                                        std::move(size),
                                                                        std::move(name),
                                                                        std::move(effects)));
        } else {
            return effect_wrapper(std::make_shared<Effect::CreateField>(std::move(type),
                                                                        std::move(size),
                                                                        std::move(name),
                                                                        std::move(effects)));
        }
    }

    FocusType insert_focus_type_(const boost::python::tuple& args, const boost::python::dict& kw) {
        auto name = boost::python::extract<std::string>(kw["name"])();
        auto description = boost::python::extract<std::string>(kw["description"])();
        auto location = boost::python::extract<condition_wrapper>(kw["location"])();
        auto graphic = boost::python::extract<std::string>(kw["graphic"])();

        return {std::move(name),
            std::move(description),
            ValueRef::CloneUnique(location.condition),
            std::move(graphic)};
    }
}

void RegisterGlobalsEffects(py::dict& globals) {
    globals["FocusType"] = py::raw_function(insert_focus_type_);

    globals["EffectsGroup"] = py::raw_function(insert_effects_group_);
    globals["Item"] = py::raw_function(insert_item_);

    globals["Destroy"] = effect_wrapper(std::make_shared<Effect::Destroy>());
    globals["NoEffect"] = effect_wrapper(std::make_shared<Effect::NoOp>());

    globals["GenerateSitRepMessage"] = py::raw_function(insert_generate_sit_rep_message_);
    globals["Conditional"] = py::raw_function(insert_conditional_);

    globals["SetEmpireMeter"] = py::raw_function(set_empire_meter);
    globals["Victory"] = py::raw_function(victory);
    globals["AddSpecial"] = py::raw_function(add_special);
    globals["SetSpecialCapacity"] = py::raw_function(add_special);
    globals["RemoveSpecial"] = py::raw_function(remove_special);
    globals["CreateShip"] = py::raw_function(create_ship);
    globals["CreateBuilding"] = py::raw_function(create_building);
    globals["SetFocus"] = py::raw_function(set_focus);
    globals["SetSpecies"] = py::raw_function(set_species);
    globals["CreateField"] = py::raw_function(create_field);

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
        globals[meter.first] = boost::python::raw_function([m](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_set_meter_(m, args, kw); });
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
        globals[meter.first] = boost::python::raw_function([m](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_ship_part_set_meter_(m, args, kw); });
    }

    globals["SetEmpireStockpile"] = py::raw_function(insert_set_empire_stockpile);
    globals["SetOwner"] = py::raw_function(insert_set_owner_);
    globals["SetStarType"] = py::raw_function(insert_set_star_type_);
    globals["MoveTo"] = py::raw_function(insert_move_to_);
    globals["MoveTowards"] = py::raw_function(insert_move_towards_);
    globals["SetPlanetSize"] = py::raw_function(insert_set_planet_size_);
    globals["SetPlanetType"] = py::raw_function(insert_set_planet_type_);
    globals["SetVisibility"] = py::raw_function(insert_set_visibility_);
    globals["AddStarlanes"] = py::raw_function(insert_add_starlanes_);
    globals["RemoveStarlanes"] = py::raw_function(insert_remove_starlanes_);

    // give_empire_unlockable_item_enum_grammar
    for (const auto& uit : std::initializer_list<std::pair<const char*, UnlockableItemType>>{
            {"GiveEmpireBuilding", UnlockableItemType::UIT_BUILDING},
            {"GiveEmpireShipPart", UnlockableItemType::UIT_SHIP_PART},
            {"GiveEmpireShipHull", UnlockableItemType::UIT_SHIP_HULL},
            {"GiveEmpireTech", UnlockableItemType::UIT_TECH},
            {"GiveEmpirePolicy", UnlockableItemType::UIT_POLICY}})
    {
        const auto u = uit.second;
        globals[uit.first] = py::raw_function([u](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_give_empire_item_(u, args, kw); });
    }

    // set_species_opinion
    for (const auto& sso : std::initializer_list<std::pair<const char*, bool>>{
            {"SetSpeciesOpinion", false},
            {"SetSpeciesTargetOpinion", true}})
    {
        const auto d = sso.second;
        globals[sso.first] = py::raw_function([d](const boost::python::tuple& args, const boost::python::dict& kw) { return insert_set_species_opinion_(d, args, kw); });
    }
}

