#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Tech.h"       // for Tech, ItemSpec and TechCategory;
#include "Effect.h"     // for Effect::EffectsGroup
#include "Building.h"   // for Building
#include "Special.h"    // for Special
#include "../Empire/Empire.h"
#include "Condition.h"
#include "ShipDesign.h"

using namespace boost::spirit;
using namespace phoenix;

rule<Scanner, BuildingTypeClosure::context_t>   building_type_p;
rule<Scanner, SpecialClosure::context_t>        special_p;
rule<Scanner, TechClosure::context_t>           tech_p;
rule<Scanner, CategoryClosure::context_t>       category_p;
rule<Scanner, PartStatsClosure::context_t>      part_stats_p;
rule<Scanner, PartClosure::context_t>           part_p;
rule<Scanner, HullClosure::context_t>           hull_p;

struct EffectsGroupClosure : boost::spirit::closure<EffectsGroupClosure, Effect::EffectsGroup*,
                                                    Condition::ConditionBase*, Condition::ConditionBase*,
                                                    std::string, std::vector<Effect::EffectBase*> >
{
    member1 this_;
    member2 scope;
    member3 activation;
    member4 stacking_group;
    member5 effects;
};

struct EffectsGroupVecClosure : boost::spirit::closure<EffectsGroupVecClosure, std::vector<boost::shared_ptr<const Effect::EffectsGroup> > >
{
    member1 this_;
};

struct TechItemSpecClosure : boost::spirit::closure<TechItemSpecClosure, ItemSpec, UnlockableItemType, std::string>
{
    member1 this_;
    member2 type;
    member3 name;
};

struct SlotClosure : boost::spirit::closure<SlotClosure, HullType::Slot, ShipSlotType, double, double>
{
    member1 this_;
    member2 slot_type;
    member3 x;
    member4 y;
};

struct SlotVecClosure : boost::spirit::closure<SlotVecClosure, std::vector<HullType::Slot> >
{
    member1 this_;
};

struct ShipSlotTypeVecClosure : boost::spirit::closure<ShipSlotTypeVecClosure, std::vector<ShipSlotType> >
{
    member1 this_;
};

namespace {
    rule<Scanner, EffectsGroupClosure::context_t>       effects_group_p;
    rule<Scanner, EffectsGroupVecClosure::context_t>    effects_group_vec_p;
    rule<Scanner, TechItemSpecClosure::context_t>       tech_item_spec_p;
    rule<Scanner, SlotClosure::context_t>               slot_p;
    rule<Scanner, SlotVecClosure::context_t>            slot_vec_p;
    rule<Scanner, ShipSlotTypeVecClosure::context_t>    ship_slot_type_vec_p;

    ParamLabel scope_label("scope");
    ParamLabel activation_label("activation");
    ParamLabel stackinggroup_label("stackinggroup");
    ParamLabel effects_label("effects");
    ParamLabel name_label("name");
    ParamLabel description_label("description");
    ParamLabel shortdescription_label("short_description");
    ParamLabel buildcost_label("buildcost");
    ParamLabel buildtime_label("buildtime");
    ParamLabel maintenancecost_label("maintenancecost");
    ParamLabel effectsgroups_label("effectsgroups");
    ParamLabel graphic_label("graphic");
    ParamLabel techtype_label("techtype");
    ParamLabel category_label("category");
    ParamLabel researchcost_label("researchcost");
    ParamLabel researchturns_label("researchturns");
    ParamLabel prerequisites_label("prerequisites");
    ParamLabel unlock_label("unlock");
    ParamLabel type_label("type");
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
    ParamLabel stealth_label("stealth");
    ParamLabel health_label("health");
    ParamLabel fighter_type_label("fightertype");
    ParamLabel launch_rate_label("launchrate");
    ParamLabel detection_label("detection");
    ParamLabel capacity_label("capacity");
    ParamLabel fuel_label("fuel");

    Effect::EffectsGroup* const NULL_EFF = 0;
    Condition::ConditionBase* const NULL_COND = 0;

    bool Init()
    {
        effects_group_p =
            (str_p("effectsgroup")
             >> scope_label >> condition_p[effects_group_p.scope = arg1]
             >> !(activation_label >> condition_p[effects_group_p.activation = arg1])
             >> !(stackinggroup_label >> name_p[effects_group_p.stacking_group = arg1])
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
             >> name_label >> name_p[building_type_p.name = arg1]
             >> description_label >> name_p[building_type_p.description = arg1]
             >> buildcost_label >> real_p[building_type_p.build_cost = arg1]
             >> buildtime_label >> int_p[building_type_p.build_time = arg1]
             >> maintenancecost_label >> real_p[building_type_p.maintenance_cost = arg1]
             >> location_label >> condition_p[building_type_p.location = arg1]
             >> effectsgroups_label >> effects_group_vec_p[building_type_p.effects_groups = arg1]
             >> graphic_label >> file_name_p[building_type_p.graphic = arg1])
            [building_type_p.this_ = new_<BuildingType>(building_type_p.name, building_type_p.description, building_type_p.build_cost,
                                                        building_type_p.build_time, building_type_p.maintenance_cost, building_type_p.location,
                                                        building_type_p.effects_groups, building_type_p.graphic)];

        special_p =
            (str_p("special")
             >> name_label >> name_p[special_p.name = arg1]
             >> description_label >> name_p[special_p.description = arg1]
             >> effectsgroups_label >> effects_group_vec_p[special_p.effects_groups = arg1]
             >> graphic_label >> file_name_p[special_p.graphic = arg1])
            [special_p.this_ = new_<Special>(special_p.name, special_p.description, special_p.effects_groups,
                                             special_p.graphic)];

        tech_item_spec_p =
            (str_p("item")
             >> type_label >> unlockable_item_type_p[tech_item_spec_p.type = arg1]
             >> name_label >> name_p[tech_item_spec_p.name = arg1])
            [tech_item_spec_p.this_ = construct_<ItemSpec>(tech_item_spec_p.type, tech_item_spec_p.name)];

        category_p =
            (str_p("category")
             >> name_label >> name_p[category_p.name = arg1]
             >> graphic_label >> file_name_p[category_p.graphic = arg1]
             >> colour_label >> colour_p[category_p.colour = arg1])
            [category_p.this_ = new_<TechCategory>(category_p.name, category_p.graphic, category_p.colour)];

        tech_p =
            (str_p("tech")
             >> name_label >> name_p[tech_p.name = arg1]
             >> description_label >> name_p[tech_p.description = arg1]
             >> shortdescription_label >> name_p[tech_p.short_description = arg1]
             >> techtype_label >> tech_type_p[tech_p.tech_type = arg1]
             >> category_label >> name_p[tech_p.category = arg1]
             >> researchcost_label >> real_p[tech_p.research_cost = arg1]
             >> researchturns_label >> int_p[tech_p.research_turns = arg1]
             >> prerequisites_label
             >> (name_p[insert_(tech_p.prerequisites, arg1)] |
                 ('[' >> *(name_p[insert_(tech_p.prerequisites, arg1)]) >> ']'))
             >> unlock_label
             >> (tech_item_spec_p[push_back_(tech_p.unlocked_items, arg1)]
                 | ('[' >> *(tech_item_spec_p[push_back_(tech_p.unlocked_items, arg1)]) >> ']'))
             >> !(effectsgroups_label >> effects_group_vec_p[tech_p.effects_groups = arg1])
             >> graphic_label >> file_name_p[tech_p.graphic = arg1])
            [tech_p.this_ = new_<Tech>(tech_p.name, tech_p.description, tech_p.short_description, tech_p.category,
                                       tech_p.tech_type, tech_p.research_cost, tech_p.research_turns,
                                       tech_p.effects_groups, tech_p.prerequisites, tech_p.unlocked_items,
                                       tech_p.graphic)];

        ship_slot_type_vec_p =
            slot_type_p[push_back_(ship_slot_type_vec_p.this_, arg1)]
            | ('[' >> +(slot_type_p[push_back_(ship_slot_type_vec_p.this_, arg1)]) >> ']');

        part_stats_p =
            // FighterStats
            (type_label >> combat_fighter_type_p[part_stats_p.fighter_type = arg1]
             >> anti_ship_damage_label >> real_p[part_stats_p.anti_ship_damage = arg1]
             >> anti_fighter_damage_label >> real_p[part_stats_p.anti_fighter_damage = arg1]
             >> launch_rate_label >> real_p[part_stats_p.rate = arg1]
             >> speed_label >> real_p[part_stats_p.speed = arg1]
             >> stealth_label >> real_p[part_stats_p.stealth = arg1]
             >> health_label >> real_p[part_stats_p.health = arg1]
             >> detection_label >> real_p[part_stats_p.detection = arg1]
             >> capacity_label >> int_p[part_stats_p.capacity = arg1])
            [part_stats_p.this_ =
             construct_<FighterStats>(part_stats_p.fighter_type, part_stats_p.anti_ship_damage,
                                      part_stats_p.anti_fighter_damage, part_stats_p.rate,
                                      part_stats_p.speed, part_stats_p.stealth,
                                      part_stats_p.health, part_stats_p.detection, part_stats_p.capacity)]

            // a single double stat
            | (capacity_label >> real_p[part_stats_p.this_ = arg1])

            // LRStats
            | (damage_label >> real_p[part_stats_p.damage = arg1]
               >> ROF_label >> real_p[part_stats_p.rate = arg1]
               >> range_label >> real_p[part_stats_p.range = arg1]
               >> speed_label >> real_p[part_stats_p.speed = arg1]
               >> stealth_label >> real_p[part_stats_p.stealth = arg1]
               >> health_label >> real_p[part_stats_p.health = arg1]
               >> capacity_label >> int_p[part_stats_p.capacity = arg1])
            [part_stats_p.this_ =
             construct_<LRStats>(part_stats_p.damage, part_stats_p.rate, part_stats_p.range,
                                 part_stats_p.speed, part_stats_p.stealth, part_stats_p.health,
                                 part_stats_p.capacity)]

            // DirectFireStats
            | (damage_label >> real_p[part_stats_p.damage = arg1]
               >> ROF_label >> real_p[part_stats_p.rate = arg1]
               >> range_label >> real_p[part_stats_p.range = arg1])
            [part_stats_p.this_ =
             construct_<DirectFireStats>(part_stats_p.damage, part_stats_p.rate, part_stats_p.range)];

        part_p =
            (str_p("part")
             >> name_label >> name_p[part_p.name = arg1]
             >> description_label >> name_p[part_p.description = arg1]
             >> partclass_label >> part_class_p[part_p.part_class = arg1]
             >> part_stats_p[part_p.stats = arg1]
             >> buildcost_label >> real_p[part_p.cost = arg1]
             >> buildtime_label >> int_p[part_p.build_time = arg1]
             >> mountableslottypes_label >> ship_slot_type_vec_p[part_p.mountable_slot_types = arg1]
             >> location_label >> condition_p[part_p.location = arg1]
             >> graphic_label >> file_name_p[part_p.graphic = arg1])
            [part_p.this_ = new_<PartType>(part_p.name, part_p.description, part_p.part_class,
                                           part_p.stats, part_p.cost, part_p.build_time,
                                           part_p.mountable_slot_types, part_p.location,
                                           part_p.graphic)];

        slot_p =
            (str_p("slot")
             >> type_label >> slot_type_p[slot_p.slot_type = arg1]
             >> position_label >> '(' >> real_p[slot_p.x = arg1]
             >> ',' >> real_p[slot_p.y = arg1] >> ')')
            [slot_p.this_ = construct_<HullType::Slot>(slot_p.slot_type, slot_p.x, slot_p.y)];

        slot_vec_p =
            slot_p[push_back_(slot_vec_p.this_, arg1)]
            | ('[' >> +(slot_p[push_back_(slot_vec_p.this_, arg1)]) >> ']');

        hull_p =
            (str_p("hull")
             >> name_label >> name_p[hull_p.name = arg1]
             >> description_label >> name_p[hull_p.description = arg1]
             >> speed_label >> real_p[hull_p.speed = arg1]
             >> starlane_speed_label >> real_p[hull_p.starlane_speed = arg1]
             >> fuel_label >> real_p[hull_p.fuel = arg1]
             >> health_label >> real_p[hull_p.health = arg1]
             >> buildcost_label >> real_p[hull_p.cost = arg1]
             >> buildtime_label >> int_p[hull_p.build_time = arg1]
             >> !(slots_label >> slot_vec_p[hull_p.slots = arg1])
             >> location_label >> condition_p[hull_p.location = arg1]
             >> graphic_label >> file_name_p[hull_p.graphic = arg1])
            [hull_p.this_ = new_<HullType>(hull_p.name, hull_p.description, hull_p.speed,
                                           hull_p.starlane_speed, hull_p.fuel, hull_p.health,
                                           hull_p.cost, hull_p.build_time, hull_p.slots,
                                           hull_p.location, hull_p.graphic)];

        return true;
    }
    bool dummy = Init();
}
