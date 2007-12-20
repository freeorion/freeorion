#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Tech.h"       // for struct ItemSpec;
#include "Effect.h"     // for Effect::EffectsGroup constructor
#include "Building.h"   // for Building and constructor
#include "Special.h"    // for Special and constructor
#include "../Empire/Empire.h"
#include "Condition.h"
#include "ShipDesign.h"

using namespace boost::spirit;
using namespace phoenix;

rule<Scanner, BuildingTypeClosure::context_t> building_type_p;
rule<Scanner, SpecialClosure::context_t> special_p;
rule<Scanner, NameClosure::context_t> tech_category_p;
rule<Scanner, TechClosure::context_t> tech_p;
rule<Scanner, PartClosure::context_t> part_p;

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

namespace {
    rule<Scanner, EffectsGroupClosure::context_t> effects_group_p;
    rule<Scanner, TechItemSpecClosure::context_t> tech_item_spec_p;
    rule<Scanner, EffectsGroupVecClosure::context_t> effects_group_vec_p;

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
    ParamLabel partclass_label("partclass");
    ParamLabel upgrade_label("upgrade");
    ParamLabel power_label("power");
    ParamLabel range_label("range");
    ParamLabel mass_label("mass");

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

        tech_category_p =
            (str_p("techcategory")
             >> name_p[tech_category_p.this_ = arg1]);

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

        part_p =
            (str_p("part")
             >> name_label >> name_p[part_p.name = arg1]
             >> description_label >> name_p[part_p.description = arg1]
             >> partclass_label >> part_class_p[part_p.part_class = arg1]
             >> power_label >> real_p[part_p.power = arg1]
             >> range_label >> real_p[part_p.range = arg1]
             >> mass_label >> real_p[part_p.mass = arg1]
             >> upgrade_label >> name_p[part_p.upgrade = arg1]
             >> graphic_label >> file_name_p[part_p.graphic = arg1])
            [part_p.this_ = new_<PartType>(part_p.name, part_p.description, part_p.part_class, part_p.upgrade,
                                           part_p.mass, part_p.power, part_p.range, part_p.graphic)];

        return true;
    }
    bool dumy = Init();
}
