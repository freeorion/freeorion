#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Effect.h"     // for Effect::EffectsGroup
#include "Building.h"   // for Building
#include "Special.h"    // for Special
#include "Species.h"    // for Species
#include "Condition.h"
#include "ShipDesign.h"

#include <boost/spirit/include/classic_insert_at_actor.hpp>

using namespace boost::spirit::classic;
using namespace phoenix;

rule<Scanner, BuildingTypeClosure::context_t>       building_type_p;
rule<Scanner, SpecialClosure::context_t>            special_p;
rule<Scanner, SpeciesClosure::context_t>            species_p;
rule<Scanner, TechClosure::context_t>               tech_p;
rule<Scanner, ItemSpecClosure::context_t>           item_spec_p;
rule<Scanner, CategoryClosure::context_t>           category_p;
rule<Scanner, PartStatsClosure::context_t>          part_stats_p;
rule<Scanner, PartClosure::context_t>               part_p;
rule<Scanner, HullStatsClosure::context_t>          hull_stats_p;
rule<Scanner, HullClosure::context_t>               hull_p;
rule<Scanner, ShipDesignClosure::context_t>         ship_design_p;
rule<Scanner, FleetPlanClosure::context_t>          fleet_plan_p;
rule<Scanner, MonsterFleetPlanClosure::context_t>   monster_fleet_plan_p;
rule<Scanner, AlignmentClosure::context_t>          alignment_p;
rule<Scanner, EffectsGroupVecClosure::context_t>    effects_group_vec_p;

struct EffectsGroupClosure : boost::spirit::classic::closure<EffectsGroupClosure, Effect::EffectsGroup*,
                                                             Condition::ConditionBase*, Condition::ConditionBase*,
                                                             std::string, std::vector<Effect::EffectBase*> >
{
    member1 this_;
    member2 scope;
    member3 activation;
    member4 stacking_group;
    member5 effects;
};

struct SlotClosure : boost::spirit::classic::closure<SlotClosure, HullType::Slot, ShipSlotType, double, double>
{
    member1 this_;
    member2 slot_type;
    member3 x;
    member4 y;
};

struct SlotVecClosure : boost::spirit::classic::closure<SlotVecClosure, std::vector<HullType::Slot> >
{
    member1 this_;
};

struct ShipSlotTypeVecClosure : boost::spirit::classic::closure<ShipSlotTypeVecClosure, std::vector<ShipSlotType> >
{
    member1 this_;
};

struct FocusTypeClosure : boost::spirit::classic::closure<FocusTypeClosure, FocusType, std::string, std::string,
                                                          Condition::ConditionBase*, std::string>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 location;
    member5 graphic;
};

struct FocusTypeVecClosure : boost::spirit::classic::closure<FocusTypeVecClosure, std::vector<FocusType> >
{
    member1 this_;
};

struct PlanetTypeEnvironmentMapClosure : boost::spirit::classic::closure<PlanetTypeEnvironmentMapClosure, std::map<PlanetType, PlanetEnvironment>,
                                                                         PlanetType, PlanetEnvironment>
{
    member1 this_;
    member2 type;
    member3 env;
};

struct TechInfoClosure : boost::spirit::classic::closure<TechInfoClosure, Tech::TechInfo, std::string, std::string, std::string,
                                                         std::string, TechType, double, int, bool>
{
    member1 this_;
    member2 name;
    member3 description;
    member4 short_description;
    member5 category;
    member6 tech_type;
    member7 research_cost;
    member8 research_turns;
    member9 researchable;
};

namespace {
    rule<Scanner, EffectsGroupClosure::context_t>               effects_group_p;
    rule<Scanner, SlotClosure::context_t>                       slot_p;
    rule<Scanner, SlotVecClosure::context_t>                    slot_vec_p;
    rule<Scanner, ShipSlotTypeVecClosure::context_t>            ship_slot_type_vec_p;
    rule<Scanner, FocusTypeClosure::context_t>                  focus_type_p;
    rule<Scanner, FocusTypeVecClosure::context_t>               focus_type_vec_p;
    rule<Scanner, PlanetTypeEnvironmentMapClosure::context_t>   planet_type_environment_map_p;
    rule<Scanner, TechInfoClosure::context_t>                   tech_info_p;

    ParamLabel scope_label("scope");
    ParamLabel activation_label("activation");
    ParamLabel stackinggroup_label("stackinggroup");
    ParamLabel effects_label("effects");
    ParamLabel name_label("name");
    ParamLabel description_label("description");
    ParamLabel shortdescription_label("short_description");
    ParamLabel buildcost_label("buildcost");
    ParamLabel buildtime_label("buildtime");
    ParamLabel captureresult_label("captureresult");
    ParamLabel effectsgroups_label("effectsgroups");
    ParamLabel graphic_label("graphic");
    ParamLabel techtype_label("techtype");
    ParamLabel category_label("category");
    ParamLabel researchcost_label("researchcost");
    ParamLabel researchturns_label("researchturns");
    ParamLabel prerequisites_label("prerequisites");
    ParamLabel unlock_label("unlock");
    ParamLabel type_label("type");
    ParamLabel environment_label("environment");
    ParamLabel environments_label("environments");
    ParamLabel foci_label("foci");
    ParamLabel location_label("location");
    ParamLabel partclass_label("class");
    ParamLabel speed_label("speed");
    ParamLabel starlane_speed_label("starlanespeed");
    ParamLabel slots_label("slots");
    ParamLabel mountableslottypes_label("mountableslottypes");
    ParamLabel colour_label("colour");
    ParamLabel position_label("position");
    ParamLabel damage_label("damage");
    ParamLabel anti_ship_damage_label("antishipdamage");
    ParamLabel anti_fighter_damage_label("antifighterdamage");
    ParamLabel ROF_label("rof");
    ParamLabel range_label("range");
    ParamLabel fighter_weapon_range_label("fighterweaponrange");
    ParamLabel stealth_label("stealth");
    ParamLabel health_label("health");
    ParamLabel structure_label("structure");
    ParamLabel fighter_type_label("fightertype");
    ParamLabel launch_rate_label("launchrate");
    ParamLabel detection_label("detection");
    ParamLabel capacity_label("capacity");
    ParamLabel fuel_label("fuel");
    ParamLabel hull_label("hull");
    ParamLabel parts_label("parts");
    ParamLabel ships_label("ships");
    ParamLabel model_label("model");
    ParamLabel spawn_rate_label("spawnrate");
    ParamLabel spawn_limit_label("spawnlimit");


    Effect::EffectsGroup* const NULL_EFF = 0;
    Condition::ConditionBase* const NULL_COND = 0;

    bool Init()
    {
        effects_group_p =
            (str_p("effectsgroup")
             >> scope_label >>              condition_p[effects_group_p.scope = arg1]
             >> !(activation_label >>       condition_p[effects_group_p.activation = arg1])
             >> !(stackinggroup_label >>    name_p[effects_group_p.stacking_group = arg1])
             >> effects_label
             >> (effect_p[push_back_(effects_group_p.effects, arg1)]
                 | ('[' >> +(effect_p[push_back_(effects_group_p.effects, arg1)]) >> ']')))
            [effects_group_p.this_ = new_<Effect::EffectsGroup>(effects_group_p.scope, effects_group_p.activation,
                                                                effects_group_p.effects, effects_group_p.stacking_group)];

        effects_group_vec_p =
            effects_group_p(NULL_EFF, NULL_COND, NULL_COND)[push_back_(effects_group_vec_p.this_, construct_<boost::shared_ptr<const Effect::EffectsGroup> >(arg1))]
            | ('[' >> +(effects_group_p(NULL_EFF, NULL_COND, NULL_COND)[push_back_(effects_group_vec_p.this_, construct_<boost::shared_ptr<const Effect::EffectsGroup> >(arg1))]) >> ']');

        building_type_p =
            (str_p("buildingtype")
             >> name_label >>               name_p[building_type_p.name = arg1]
             >> description_label >>        name_p[building_type_p.description = arg1]
             >> buildcost_label >>          real_p[building_type_p.production_cost = arg1]
             >> buildtime_label >>          int_p[building_type_p.production_time = arg1]
             >> (str_p("unproducible")[             building_type_p.producible = val(false)]
                 | (str_p("producible") | eps_p)[   building_type_p.producible = val(true)])
             >> location_label >>           condition_p[building_type_p.location = arg1]
             >> (captureresult_label >>     capture_result_p[building_type_p.capture_result = arg1]
                 |                          eps_p[building_type_p.capture_result = val(CR_CAPTURE)])
             >> !(effectsgroups_label >>    effects_group_vec_p[building_type_p.effects_groups = arg1])
             >> graphic_label >>            file_name_p[building_type_p.graphic = arg1])
            [building_type_p.this_ = new_<BuildingType>(building_type_p.name, building_type_p.description,
                                                        building_type_p.production_cost, building_type_p.production_time,
                                                        building_type_p.producible,
                                                        building_type_p.capture_result, building_type_p.location,
                                                        building_type_p.effects_groups, building_type_p.graphic)];

        special_p =
            (str_p("special")
             >> name_label >>               name_p[special_p.name = arg1]
             >> description_label >>        name_p[special_p.description = arg1]
             >> ((spawn_rate_label >>       real_p[special_p.spawn_rate = arg1]) |
                                            eps_p[special_p.spawn_rate = val(1.0)])
             >> ((spawn_limit_label >>      int_p[special_p.spawn_limit = arg1]) |
                                            eps_p[special_p.spawn_limit = val(9999)])
             >> (location_label >>          condition_p[special_p.location = arg1]
                 |                          eps_p[special_p.location = val(NULL_COND)])
             >> !(effectsgroups_label >>    effects_group_vec_p[special_p.effects_groups = arg1])
             >> graphic_label >>            file_name_p[special_p.graphic = arg1])
            [special_p.this_ = new_<Special>(special_p.name, special_p.description, special_p.effects_groups,
                                             special_p.spawn_rate, special_p.spawn_limit,
                                             special_p.location, special_p.graphic)];

        focus_type_p =
            (str_p("focus")
             >> name_label >>               name_p[focus_type_p.name = arg1]
             >> description_label >>        name_p[focus_type_p.description = arg1]
             >> location_label >>           condition_p[focus_type_p.location = arg1]
             >> graphic_label >>            file_name_p[focus_type_p.graphic = arg1])
            [focus_type_p.this_ = construct_<FocusType>(focus_type_p.name, focus_type_p.description, focus_type_p.location,
                                                        focus_type_p.graphic)];

        focus_type_vec_p =
            focus_type_p[push_back_(focus_type_vec_p.this_, arg1)]
            | ('[' >> +(focus_type_p[push_back_(focus_type_vec_p.this_, arg1)]) >> ']');

        planet_type_environment_map_p =
            (type_label >>                  planet_type_p[planet_type_environment_map_p.type = arg1]
             >> environment_label >>        planet_environment_type_p[planet_type_environment_map_p.env = arg1])
            [insert_(planet_type_environment_map_p.this_,
                     make_pair_(planet_type_environment_map_p.type,
                                planet_type_environment_map_p.env))]
            | ('[' >> +((type_label >>                  planet_type_p[planet_type_environment_map_p.type = arg1]
                         >> environment_label >>        planet_environment_type_p[planet_type_environment_map_p.env = arg1])
                        [insert_(planet_type_environment_map_p.this_,
                                 make_pair_(planet_type_environment_map_p.type,
                                            planet_type_environment_map_p.env))])
               >> ']');

        species_p =
            (str_p("species")
             >> name_label >>               name_p[species_p.name = arg1]
             >> description_label >>        name_p[species_p.description = arg1]
             >> (str_p("canproduceships")[species_p.can_produce_ships = val(true)]
               | eps_p                   [species_p.can_produce_ships = val(false)])
             >> (str_p("cancolonize")[species_p.can_colonize = val(true)]
               | eps_p               [species_p.can_colonize = val(false)])
             >> !(foci_label >>             focus_type_vec_p[species_p.foci = arg1])
             >> !(effectsgroups_label >>    effects_group_vec_p[species_p.effects_groups = arg1])
             >> !(environments_label >>     planet_type_environment_map_p[species_p.environments = arg1])
             >> graphic_label >>            file_name_p[species_p.graphic = arg1])
            [species_p.this_ = new_<Species>(species_p.name, species_p.description, species_p.foci,
                                             species_p.environments, species_p.effects_groups,
                                             species_p.can_produce_ships, species_p.can_colonize,
                                             species_p.graphic)];

        item_spec_p =
            (str_p("item")
             >> type_label >>   unlockable_item_type_p[item_spec_p.type = arg1]
             >> name_label >>   name_p[item_spec_p.name = arg1])
            [item_spec_p.this_ = construct_<ItemSpec>(item_spec_p.type, item_spec_p.name)];

        category_p =
            (str_p("category")
             >> name_label >>       name_p[category_p.name = arg1]
             >> graphic_label >>    file_name_p[category_p.graphic = arg1]
             >> colour_label >>     colour_p[category_p.colour = arg1])
            [category_p.this_ = new_<TechCategory>(category_p.name, category_p.graphic, category_p.colour)];

        tech_info_p = (
                name_label >>               name_p[tech_info_p.name = arg1]
             >> description_label >>        name_p[tech_info_p.description = arg1]
             >> shortdescription_label >>   name_p[tech_info_p.short_description = arg1]
             >> ((techtype_label >>         tech_type_p[tech_info_p.tech_type = arg1])
                 |                          eps_p[tech_info_p.tech_type = val(TT_THEORY)])
             >> category_label >>           name_p[tech_info_p.category = arg1]
             >> ((researchcost_label >>     real_p[tech_info_p.research_cost = arg1])
                 |                          eps_p[tech_info_p.research_cost = val(1.0)])
             >> ((researchturns_label >>    int_p[tech_info_p.research_turns = arg1])
                 |                          eps_p[tech_info_p.research_turns = val(1)])
             >> (str_p("unresearchable")[           tech_info_p.researchable = val(false)]
                 | (str_p("researchable") | eps_p)[ tech_info_p.researchable = val(true)]))
            [tech_info_p.this_ = construct_<Tech::TechInfo>(tech_info_p.name, tech_info_p.description, tech_info_p.short_description,
                                                            tech_info_p.category, tech_info_p.tech_type, tech_info_p.research_cost,
                                                            tech_info_p.research_turns, tech_info_p.researchable)];

        tech_p =
            (str_p("tech")
             >> tech_info_p[tech_p.tech_info = arg1]
             >> prerequisites_label
             >> (name_p[insert_(tech_p.prerequisites, arg1)] |
                 ('[' >> *(name_p[insert_(tech_p.prerequisites, arg1)]) >> ']'))
             >> unlock_label
             >> (item_spec_p[push_back_(tech_p.unlocked_items, arg1)]
                 | ('[' >> *(item_spec_p[push_back_(tech_p.unlocked_items, arg1)]) >> ']'))
             >> !(effectsgroups_label >>    effects_group_vec_p[tech_p.effects_groups = arg1])
             >> graphic_label >> file_name_p[tech_p.graphic = arg1])
            [tech_p.this_ = new_<Tech>(tech_p.tech_info,
                                       tech_p.effects_groups, tech_p.prerequisites, tech_p.unlocked_items,
                                       tech_p.graphic)];

        ship_slot_type_vec_p =
            slot_type_p[push_back_(ship_slot_type_vec_p.this_, arg1)]
            | ('[' >> +(slot_type_p[push_back_(ship_slot_type_vec_p.this_, arg1)]) >> ']');

        part_stats_p =
            // FighterStats
            (type_label >>                      combat_fighter_type_p[part_stats_p.fighter_type = arg1]
             >> anti_ship_damage_label >>       real_p[part_stats_p.anti_ship_damage = arg1]
             >> anti_fighter_damage_label >>    real_p[part_stats_p.anti_fighter_damage = arg1]
             >> launch_rate_label >>            real_p[part_stats_p.rate = arg1]
             >> fighter_weapon_range_label >>   real_p[part_stats_p.range = arg1]
             >> speed_label >>                  real_p[part_stats_p.speed = arg1]
             >> stealth_label >>                real_p[part_stats_p.stealth = arg1]
             >> structure_label >>              real_p[part_stats_p.structure = arg1]
             >> detection_label >>              real_p[part_stats_p.detection = arg1]
             >> capacity_label >>               int_p[part_stats_p.capacity = arg1])
            [part_stats_p.this_ =
             construct_<FighterStats>(part_stats_p.fighter_type, part_stats_p.anti_ship_damage,
                                      part_stats_p.anti_fighter_damage, part_stats_p.rate,
                                      part_stats_p.range, part_stats_p.speed,
                                      part_stats_p.stealth, part_stats_p.structure,
                                      part_stats_p.detection, part_stats_p.capacity)]

            // a single double stat
            | (capacity_label >> real_p[part_stats_p.this_ = arg1])

            // LRStats
            | (damage_label >>      real_p[part_stats_p.damage = arg1]
               >> ROF_label >>      real_p[part_stats_p.rate = arg1]
               >> range_label >>    real_p[part_stats_p.range = arg1]
               >> speed_label >>    real_p[part_stats_p.speed = arg1]
               >> stealth_label >>  real_p[part_stats_p.stealth = arg1]
               >> structure_label >>real_p[part_stats_p.structure = arg1]
               >> capacity_label >> int_p[part_stats_p.capacity = arg1])
            [part_stats_p.this_ =
             construct_<LRStats>(part_stats_p.damage, part_stats_p.rate, part_stats_p.range,
                                 part_stats_p.speed, part_stats_p.stealth, part_stats_p.structure,
                                 part_stats_p.capacity)]

            // DirectFireStats
            | (damage_label >>      real_p[part_stats_p.damage = arg1]
               >> ROF_label >>      real_p[part_stats_p.rate = arg1]
               >> range_label >>    real_p[part_stats_p.range = arg1])
            [part_stats_p.this_ =
             construct_<DirectFireStats>(part_stats_p.damage, part_stats_p.rate, part_stats_p.range)];

        part_p =
            (str_p("part")
             >> name_label >>               name_p[part_p.name = arg1]
             >> description_label >>        name_p[part_p.description = arg1]
             >> partclass_label >>          part_class_p[part_p.part_class = arg1]
             >> part_stats_p[part_p.stats = arg1]
             >> buildcost_label >>          real_p[part_p.cost = arg1]
             >> buildtime_label >>          int_p[part_p.production_time = arg1]
             >> (str_p("unproducible")[             part_p.producible = val(false)]
                 | (str_p("producible") | eps_p)[   part_p.producible = val(true)])
             >> mountableslottypes_label >> ship_slot_type_vec_p[part_p.mountable_slot_types = arg1]
             >> location_label >>           condition_p[part_p.location = arg1]
             >> !(effectsgroups_label >>    effects_group_vec_p[part_p.effects_groups = arg1])
             >> graphic_label >>            file_name_p[part_p.graphic = arg1])
            [part_p.this_ = new_<PartType>(part_p.name, part_p.description, part_p.part_class,
                                           part_p.stats, part_p.cost, part_p.production_time, part_p.producible,
                                           part_p.mountable_slot_types, part_p.location,
                                           part_p.effects_groups, part_p.graphic)];

        slot_p =
            (str_p("slot")
             >> type_label >>               slot_type_p[slot_p.slot_type = arg1]
             >> position_label >> '(' >>    real_p[slot_p.x = arg1]
             >> ',' >>                      real_p[slot_p.y = arg1] >> ')')
            [slot_p.this_ = construct_<HullType::Slot>(slot_p.slot_type, slot_p.x, slot_p.y)];

        slot_vec_p =
            slot_p[push_back_(slot_vec_p.this_, arg1)]
            | ('[' >> +(slot_p[push_back_(slot_vec_p.this_, arg1)]) >> ']');

        hull_stats_p =
            (speed_label >>             real_p[hull_stats_p.battle_speed = arg1]
            >> starlane_speed_label >>  real_p[hull_stats_p.starlane_speed = arg1]
            >> fuel_label >>            real_p[hull_stats_p.fuel = arg1]
            >> stealth_label >>         real_p[hull_stats_p.stealth = arg1]
            >> structure_label >>       real_p[hull_stats_p.structure = arg1])
            [hull_stats_p.this_ =
             construct_<HullTypeStats>(hull_stats_p.fuel, hull_stats_p.battle_speed, hull_stats_p.starlane_speed,
                                       hull_stats_p.stealth, hull_stats_p.structure)];

        hull_p =
            (str_p("hull")
             >> name_label >>               name_p[hull_p.name = arg1]
             >> description_label >>        name_p[hull_p.description = arg1]
             >> hull_stats_p[hull_p.stats = arg1]
             >> buildcost_label >>          real_p[hull_p.cost = arg1]
             >> buildtime_label >>          int_p[hull_p.production_time = arg1]
             >> (str_p("unproducible")[             hull_p.producible = val(false)]
                 | (str_p("producible") | eps_p)[   hull_p.producible = val(true)])
             >> !(slots_label >>            slot_vec_p[hull_p.slots = arg1])
             >> ((location_label >>         condition_p[hull_p.location = arg1]) |
                                            eps_p[hull_p.location = new_<Condition::All>()])
             >> !(effectsgroups_label >>    effects_group_vec_p[hull_p.effects_groups = arg1])
             >> graphic_label >>            file_name_p[hull_p.graphic = arg1])
            [hull_p.this_ = new_<HullType>(hull_p.name, hull_p.description,
                                           hull_p.stats, hull_p.cost, hull_p.production_time, hull_p.producible,
                                           hull_p.slots, hull_p.location,
                                           hull_p.effects_groups, hull_p.graphic)];

        ship_design_p =
            (str_p("shipdesign")
             >> name_label >>           name_p[ship_design_p.name = arg1]
             >> description_label >>    name_p[ship_design_p.description = arg1]
             >> hull_label >>           name_p[ship_design_p.hull = arg1]
             >> parts_label
             >> (name_p[push_back_(ship_design_p.parts, arg1)] |
                 ('[' >> *(name_p[push_back_(ship_design_p.parts, arg1)]) >> ']'))
             >> graphic_label >>        file_name_p[ship_design_p.graphic = arg1]
             >> model_label >>          file_name_p[ship_design_p.model = arg1])
            [ship_design_p.this_ = new_<ShipDesign>(ship_design_p.name, ship_design_p.description,
                                                    val(ALL_EMPIRES),   // created by empire id - to be reset later
                                                    val(0),             // creation turn
                                                    ship_design_p.hull, ship_design_p.parts,
                                                    ship_design_p.graphic, ship_design_p.model,
                                                    val(true))];

        fleet_plan_p =
            (str_p("fleet")
             >> name_label >>           name_p[fleet_plan_p.name = arg1]
             >> ships_label
             >> (name_p[push_back_(fleet_plan_p.ship_designs, arg1)] |
                 ('[' >> *(name_p[push_back_(fleet_plan_p.ship_designs, arg1)]) >> ']'))
            [fleet_plan_p.this_ = new_<FleetPlan>(fleet_plan_p.name, fleet_plan_p.ship_designs, val(true))]);

        monster_fleet_plan_p =
            (str_p("monsterfleet")
             >> name_label >>           name_p[monster_fleet_plan_p.name = arg1]
             >> ships_label
             >> (name_p[push_back_(monster_fleet_plan_p.ship_designs, arg1)] |
                 ('[' >> *(name_p[push_back_(monster_fleet_plan_p.ship_designs, arg1)]) >> ']'))
             >> ((spawn_rate_label >>   real_p[monster_fleet_plan_p.spawn_rate = arg1]) |
                                        eps_p[monster_fleet_plan_p.spawn_rate = val(1.0)])
             >> ((spawn_limit_label >>  int_p[monster_fleet_plan_p.spawn_limit = arg1]) |
                                        eps_p[monster_fleet_plan_p.spawn_limit = val(9999)])
             >> ((location_label >>     condition_p[monster_fleet_plan_p.location = arg1]) |
                                        eps_p[monster_fleet_plan_p.location = val(NULL_COND)])
            [monster_fleet_plan_p.this_ = new_<MonsterFleetPlan>(monster_fleet_plan_p.name,
                                                                 monster_fleet_plan_p.ship_designs,
                                                                 monster_fleet_plan_p.spawn_rate,
                                                                 monster_fleet_plan_p.spawn_limit,
                                                                 monster_fleet_plan_p.location)]);

        alignment_p =
            (str_p("alignment")
             >> name_label >>           name_p[alignment_p.name = arg1]
             >> description_label >>    name_p[alignment_p.description = arg1]
             >> graphic_label >>        file_name_p[alignment_p.graphic = arg1])
            [alignment_p.this_ = construct_<Alignment>(alignment_p.name, alignment_p.description, alignment_p.graphic)];

        return true;
    }
    bool dummy = Init();
}
