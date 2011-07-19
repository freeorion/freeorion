#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Condition.h"

using namespace boost::spirit::classic;
using namespace phoenix;

extern ParamLabel low_label;
extern ParamLabel high_label;
extern ParamLabel number_label;
extern ParamLabel condition_label;
extern ParamLabel empire_label;
extern ParamLabel affiliation_label;
extern ParamLabel name_label;
extern ParamLabel type_label;
extern ParamLabel class_label;
extern ParamLabel planetsize_label;
extern ParamLabel environment_label;
extern ParamLabel probability_label;
extern ParamLabel distance_label;
extern ParamLabel jumps_label;
extern ParamLabel sort_key_label;

rule<Scanner, ConditionClosure::context_t> condition2_p;

namespace {
    class ConditionParser2Definition
    {
    public:
        typedef rule<Scanner, ConditionClosure::context_t> Rule;

        ConditionParser2Definition();

    private:
        struct DoubleRefConditionClosure : boost::spirit::classic::closure<DoubleRefConditionClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 double_ref_vec;
            member3 condition;
        };

        struct IntRefConditionClosure : boost::spirit::classic::closure<IntRefConditionClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 int_ref;
            member3 condition;
        };

        struct IntRefIntRefConditionClosure : boost::spirit::classic::closure<IntRefIntRefConditionClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
            member4 condition;
        };

        struct IntRefIntRefClosure : boost::spirit::classic::closure<IntRefIntRefClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
        };

        struct SortedNumberOfClosure : boost::spirit::classic::closure<SortedNumberOfClosure, Condition::ConditionBase*,
                                                                       ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<double>*,
                                                                       Condition::SortingMethod, Condition::ConditionBase*>
        {
            member1 this_;
            member2 number;
            member3 sort_key;
            member4 sorting_method;
            member5 condition;
        };

        struct StringClosure : boost::spirit::classic::closure<StringClosure, Condition::ConditionBase*, std::string>
        {
            member1 this_;
            member2 name;
        };

        struct ConditionParamClosure : boost::spirit::classic::closure<ConditionParamClosure, Condition::ConditionBase*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 condition;
        };

        struct StarTypeClosure : boost::spirit::classic::closure<StarTypeClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase< ::StarType>*> >
        {
            member1 this_;
            member2 types;
        };

        struct DesignHasPartClosure : boost::spirit::classic::closure<DesignHasPartClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*, std::string>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
            member4 name;
        };

        struct DesignHasPartClassClosure : boost::spirit::classic::closure<DesignHasPartClassClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*, ShipPartClass>
        {
            member1 this_;
            member2 int_ref_1;
            member3 int_ref_2;
            member4 part_class;
        };

        struct DoubleRefClosure : boost::spirit::classic::closure<DoubleRefClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 double_ref;
        };

        struct StockpileClosure : boost::spirit::classic::closure<StockpileClosure, Condition::ConditionBase*, ResourceType, ValueRef::ValueRefBase<double>*, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 stockpile_type;
            member3 int_ref_2;
            member4 int_ref_1;
        };

        struct IntRefVecClosure : boost::spirit::classic::closure<IntRefVecClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase<int>*> >
        {
            member1 this_;
            member2 int_ref_vec;
        };

        struct IntRefClosure : boost::spirit::classic::closure<IntRefClosure, Condition::ConditionBase*, const ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 int_ref;
        };

        typedef rule<Scanner, DoubleRefConditionClosure::context_t>     DoubleRefConditionRule;
        typedef rule<Scanner, IntRefConditionClosure::context_t>        IntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefConditionClosure::context_t>  IntRefIntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefClosure::context_t>           IntRefIntRefRule;
        typedef rule<Scanner, SortedNumberOfClosure::context_t>         SortedNumberOfRule;
        typedef rule<Scanner, StringClosure::context_t>                 StringRule;
        typedef rule<Scanner, ConditionParamClosure::context_t>         ConditionParamRule;
        typedef rule<Scanner, StarTypeClosure::context_t>               StarTypeRule;
        typedef rule<Scanner, DesignHasPartClosure::context_t>          DesignHasPartRule;
        typedef rule<Scanner, DesignHasPartClassClosure::context_t>     DesignHasPartClassRule;
        typedef rule<Scanner, DoubleRefClosure::context_t>              DoubleRefRule;
        typedef rule<Scanner, StockpileClosure::context_t>              StockpileRule;
        typedef rule<Scanner, IntRefVecClosure::context_t>              IntRefVecRule;
        typedef rule<Scanner, IntRefClosure::context_t>                 IntRefRule;

        StringRule                  owner_has_tech;
        DoubleRefConditionRule      within_distance;
        IntRefConditionRule         within_starlane_jumps;
        IntRefIntRefConditionRule   number;
        IntRefIntRefRule            turn;
        SortedNumberOfRule          number_of;
        StringRule                  has_special;
        ConditionParamRule          contains;
        ConditionParamRule          contained_by;
        StarTypeRule                star_type;
        StringRule                  design_has_hull;
        DesignHasPartRule           design_has_part;
        DesignHasPartClassRule      design_has_part_class;
        IntRefRule                  produced_by_empire;
        DoubleRefRule               random;
        StockpileRule               owner_stockpile;
        IntRefVecRule               visible_to_empire;
        Rule                        stationary;
        IntRefRule                  fleet_supplyable;
        IntRefConditionRule         resource_supply_connected;
    };

    ConditionParser2Definition::ConditionParser2Definition()
    {
        owner_has_tech =
            (str_p("ownerhastech")
             >> name_label >> name_p[owner_has_tech.name = arg1])
            [owner_has_tech.this_ = new_<Condition::OwnerHasTech>(owner_has_tech.name)];

        within_distance =
            (str_p("withindistance")
             >> distance_label >> double_expr_p[within_distance.double_ref_vec = arg1]
             >> condition_label >> condition_p[within_distance.condition = arg1])
            [within_distance.this_ = new_<Condition::WithinDistance>(within_distance.double_ref_vec, within_distance.condition)];

        within_starlane_jumps =
            (str_p("withinstarlanejumps")
             >> jumps_label >> int_expr_p[within_starlane_jumps.int_ref = arg1]
             >> condition_label >> condition_p[within_starlane_jumps.condition = arg1])
            [within_starlane_jumps.this_ = new_<Condition::WithinStarlaneJumps>(within_starlane_jumps.int_ref, within_starlane_jumps.condition)];

        number =
            (str_p("number")
             >> low_label >> int_expr_p[number.int_ref_2 = arg1]
             >> high_label >> int_expr_p[number.int_ref_1 = arg1]
             >> condition_label >> condition_p[number.condition = arg1])
            [number.this_ = new_<Condition::Number>(number.int_ref_2, number.int_ref_1, number.condition)];

        turn =
            (str_p("turn")
             >> low_label >> int_expr_p[turn.int_ref_2 = arg1]
             >> high_label >> int_expr_p[turn.int_ref_1 = arg1])
            [turn.this_ = new_<Condition::Turn>(turn.int_ref_2, turn.int_ref_1)];

        number_of =
            ((str_p("numberof")
              >> number_label >> int_expr_p[number_of.number = arg1]
              >> condition_label >> condition_p[number_of.condition = arg1])
             [number_of.this_ = new_<Condition::SortedNumberOf>(number_of.number, number_of.condition)])

            | ( ((str_p("maximumnumberof")[number_of.sorting_method =    val(Condition::SORT_MAX)]
                 | str_p("minimumnumberof")[number_of.sorting_method =  val(Condition::SORT_MIN)]
                 | str_p("modenumberof")[number_of.sorting_method =     val(Condition::SORT_MODE)])
                 >> number_label >> int_expr_p[number_of.number = arg1]
                 >> sort_key_label >> double_expr_p[number_of.sort_key = arg1]
                 >> condition_label >> condition_p[number_of.condition = arg1])
                [number_of.this_ = new_<Condition::SortedNumberOf>(number_of.number, number_of.sort_key,
                                                                  number_of.sorting_method, number_of.condition)]);

        has_special =
            (str_p("hasspecial")
             >> name_label >> name_p[has_special.name = arg1])
            [has_special.this_ = new_<Condition::HasSpecial>(has_special.name)];

        contains =
            (str_p("contains")
             >> condition_label >> condition_p[contains.condition = arg1])
            [contains.this_ = new_<Condition::Contains>(contains.condition)];

        contained_by =
            (str_p("containedby")
             >> condition_label >> condition_p[contained_by.condition = arg1])
            [contained_by.this_ = new_<Condition::ContainedBy>(contained_by.condition)];

        star_type =
            (str_p("star")
             >> type_label
             >> (startype_expr_p[push_back_(star_type.types, arg1)]
                 | ('[' >> +(startype_expr_p[push_back_(star_type.types, arg1)]) >> ']')))
            [star_type.this_ = new_<Condition::StarType>(star_type.types)];

        design_has_hull =
            (str_p("designhashull")
             >> name_label >> name_p[design_has_hull.name = arg1])
            [design_has_hull.this_ = new_<Condition::DesignHasHull>(design_has_hull.name)];

        design_has_part =
            (str_p("designhaspart")
             >> low_label >>    int_expr_p[ design_has_part.int_ref_2 = arg1]
             >> high_label >>   int_expr_p[ design_has_part.int_ref_1 = arg1]
             >> name_label >>   name_p[     design_has_part.name = arg1])
            [design_has_part.this_ = new_<Condition::DesignHasPart>(design_has_part.int_ref_2,
                                                                    design_has_part.int_ref_1,
                                                                    design_has_part.name)];

        design_has_part_class =
            (str_p("designhaspartclass")
             >> low_label >>    int_expr_p[     design_has_part_class.int_ref_2 = arg1]
             >> high_label >>   int_expr_p[     design_has_part_class.int_ref_1 = arg1]
             >> class_label >>  part_class_p[   design_has_part_class.part_class = arg1])
            [design_has_part_class.this_ = new_<Condition::DesignHasPartClass>(design_has_part_class.int_ref_2,
                                                                               design_has_part_class.int_ref_1,
                                                                               design_has_part_class.part_class)];

        produced_by_empire =
            (str_p("producedbyempire")
             >> empire_label >> int_expr_p[fleet_supplyable.int_ref = arg1])
            [produced_by_empire.this_ = new_<Condition::ProducedByEmpire>(produced_by_empire.int_ref)];

        random =
            (str_p("random")
             >> probability_label
             >> double_expr_p[random.double_ref = arg1])
            [random.this_ = new_<Condition::Chance>(random.double_ref)];

        owner_stockpile =
            ((str_p("ownerfoodstockpile")[owner_stockpile.stockpile_type = val(RE_FOOD)]
              | str_p("ownermineralstockpile")[owner_stockpile.stockpile_type = val(RE_MINERALS)]
              | str_p("ownertradestockpile")[owner_stockpile.stockpile_type = val(RE_TRADE)])
             >> low_label >> double_expr_p[owner_stockpile.int_ref_2 = arg1]
             >> high_label >> double_expr_p[owner_stockpile.int_ref_1 = arg1])
            [owner_stockpile.this_ = new_<Condition::EmpireStockpileValue>(owner_stockpile.stockpile_type, owner_stockpile.int_ref_2, owner_stockpile.int_ref_1)];

        visible_to_empire =
            (str_p("visibletoempire")
             >> empire_label
             >> (int_expr_p[push_back_(visible_to_empire.int_ref_vec, arg1)]
                 | ('[' >> +(int_expr_p[push_back_(visible_to_empire.int_ref_vec, arg1)]) >> ']')))
            [visible_to_empire.this_ = new_<Condition::VisibleToEmpire>(visible_to_empire.int_ref_vec)];

        stationary =
            str_p("stationary")
            [stationary.this_ = new_<Condition::Stationary>()];

        fleet_supplyable =
            (str_p("fleetsupplyablebyempire")
             >> empire_label >> int_expr_p[fleet_supplyable.int_ref = arg1])
            [fleet_supplyable.this_ = new_<Condition::FleetSupplyableByEmpire>(fleet_supplyable.int_ref)];

        resource_supply_connected =
            (str_p("resourcesupplyconnectedbyempire")
             >> empire_label >> int_expr_p[resource_supply_connected.int_ref = arg1]
             >> condition_label >> condition_p[resource_supply_connected.condition = arg1])
            [resource_supply_connected.this_ = new_<Condition::ResourceSupplyConnectedByEmpire>(resource_supply_connected.int_ref,
                                                                                                resource_supply_connected.condition)];

        condition2_p =
            owner_has_tech[condition2_p.this_ = arg1]
            | within_distance[condition2_p.this_ = arg1]
            | within_starlane_jumps[condition2_p.this_ = arg1]
            | number[condition2_p.this_ = arg1]
            | turn[condition2_p.this_ = arg1]
            | number_of[condition2_p.this_ = arg1]
            | has_special[condition2_p.this_ = arg1]
            | contains[condition2_p.this_ = arg1]
            | contained_by[condition2_p.this_ = arg1]
            | star_type[condition2_p.this_ = arg1]
            | design_has_hull[condition2_p.this_ = arg1]
            | design_has_part[condition2_p.this_ = arg1]
            | design_has_part_class[condition2_p.this_ = arg1]
            | produced_by_empire[condition2_p.this_ = arg1]
            | random[condition2_p.this_ = arg1]
            | owner_stockpile[condition2_p.this_ = arg1]
            | visible_to_empire[condition2_p.this_ = arg1]
            | stationary[condition2_p.this_ = arg1]
            | fleet_supplyable[condition2_p.this_ = arg1]
            | resource_supply_connected[condition2_p.this_ = arg1];
    }
    ConditionParser2Definition condition2_def;
}
