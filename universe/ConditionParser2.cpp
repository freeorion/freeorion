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
extern ParamLabel planetsize_label;
extern ParamLabel environment_label;
extern ParamLabel probability_label;
extern ParamLabel distance_label;
extern ParamLabel jumps_label;

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
            member2 distance;
            member3 condition;
        };

        struct IntRefConditionClosure : boost::spirit::classic::closure<IntRefConditionClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 jumps;
            member3 condition;
        };

        struct IntRefIntRefConditionClosure : boost::spirit::classic::closure<IntRefIntRefConditionClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 high;
            member3 low;
            member4 condition;
        };

        struct IntRefIntRefClosure : boost::spirit::classic::closure<IntRefIntRefClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, ValueRef::ValueRefBase<int>*>
        {
            member1 this_;
            member2 high;
            member3 low;
        };

        struct NumberOfClosure : boost::spirit::classic::closure<NumberOfClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<int>*, Condition::ConditionBase*>
        {
            member1 this_;
            member2 number;
            member3 condition;
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
            member2 high;
            member3 low;
            member4 name;
        };

        struct DoubleRefClosure : boost::spirit::classic::closure<DoubleRefClosure, Condition::ConditionBase*, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 probability;
        };

        struct StockpileClosure : boost::spirit::classic::closure<StockpileClosure, Condition::ConditionBase*, ResourceType, ValueRef::ValueRefBase<double>*, ValueRef::ValueRefBase<double>*>
        {
            member1 this_;
            member2 stockpile_type;
            member3 low;
            member4 high;
        };

        struct IntRefVecClosure : boost::spirit::classic::closure<IntRefVecClosure, Condition::ConditionBase*, std::vector<const ValueRef::ValueRefBase<int>*> >
        {
            member1 this_;
            member2 empires;
        };

        typedef rule<Scanner, DoubleRefConditionClosure::context_t>     DoubleRefConditionRule;
        typedef rule<Scanner, IntRefConditionClosure::context_t>        IntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefConditionClosure::context_t>  IntRefIntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefClosure::context_t>           IntRefIntRefRule;
        typedef rule<Scanner, NumberOfClosure::context_t>               NumberOfRule;
        typedef rule<Scanner, StringClosure::context_t>                 StringRule;
        typedef rule<Scanner, ConditionParamClosure::context_t>         ConditionParamRule;
        typedef rule<Scanner, StarTypeClosure::context_t>               StarTypeRule;
        typedef rule<Scanner, DesignHasPartClosure::context_t>          DesignHasPartRule;
        typedef rule<Scanner, DoubleRefClosure::context_t>              DoubleRefRule;
        typedef rule<Scanner, StockpileClosure::context_t>              StockpileRule;
        typedef rule<Scanner, IntRefVecClosure::context_t>              IntRefVecRule;

        StringRule                  owner_has_tech;
        DoubleRefConditionRule      within_distance;
        IntRefConditionRule         within_starlane_jumps;
        IntRefIntRefConditionRule   number;
        IntRefIntRefRule            turn;
        NumberOfRule                number_of;
        StringRule                  has_special;
        ConditionParamRule          contains;
        ConditionParamRule          contained_by;
        StarTypeRule                star_type;
        StringRule                  design_has_hull;
        DesignHasPartRule           design_has_part;
        DoubleRefRule               random;
        StockpileRule               owner_stockpile;
        IntRefVecRule               visible_to_empire;
        Rule                        stationary;
    };

    ConditionParser2Definition::ConditionParser2Definition()
    {
        owner_has_tech =
            (str_p("ownerhastech")
             >> name_label >> name_p[owner_has_tech.name = arg1])
            [owner_has_tech.this_ = new_<Condition::OwnerHasTech>(owner_has_tech.name)];

        within_distance =
            (str_p("withindistance")
             >> distance_label >> double_expr_p[within_distance.distance = arg1]
             >> condition_label >> condition_p[within_distance.condition = arg1])
            [within_distance.this_ = new_<Condition::WithinDistance>(within_distance.distance, within_distance.condition)];

        within_starlane_jumps =
            (str_p("withinstarlanejumps")
             >> jumps_label >> int_expr_p[within_starlane_jumps.jumps = arg1]
             >> condition_label >> condition_p[within_starlane_jumps.condition = arg1])
            [within_starlane_jumps.this_ = new_<Condition::WithinStarlaneJumps>(within_starlane_jumps.jumps, within_starlane_jumps.condition)];

        number =
            (str_p("number")
             >> low_label >> int_expr_p[number.low = arg1]
             >> high_label >> int_expr_p[number.high = arg1]
             >> condition_label >> condition_p[number.condition = arg1])
            [number.this_ = new_<Condition::Number>(number.low, number.high, number.condition)];

        turn =
            (str_p("turn")
             >> low_label >> int_expr_p[turn.low = arg1]
             >> high_label >> int_expr_p[turn.high = arg1])
            [turn.this_ = new_<Condition::Turn>(turn.low, turn.high)];

        number_of =
            (str_p("numberof")
             >> number_label >> int_expr_p[number_of.number = arg1]
             >> condition_label >> condition_p[number_of.condition = arg1])
            [number_of.this_ = new_<Condition::NumberOf>(number_of.number, number_of.condition)];

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
             >> low_label >> int_expr_p[design_has_part.low = arg1]
             >> high_label >> int_expr_p[design_has_part.high = arg1]
             >> name_label >> name_p[design_has_part.name = arg1])
            [design_has_part.this_ = new_<Condition::DesignHasPart>(design_has_part.low, design_has_part.high, design_has_part.name)];

        random =
            (str_p("random")
             >> probability_label
             >> double_expr_p[random.probability = arg1])
            [random.this_ = new_<Condition::Chance>(random.probability)];

        owner_stockpile =
            ((str_p("ownerfoodstockpile")[owner_stockpile.stockpile_type = val(RE_FOOD)]
              | str_p("ownermineralstockpile")[owner_stockpile.stockpile_type = val(RE_MINERALS)]
              | str_p("ownertradestockpile")[owner_stockpile.stockpile_type = val(RE_TRADE)])
             >> low_label >> double_expr_p[owner_stockpile.low = arg1]
             >> high_label >> double_expr_p[owner_stockpile.high = arg1])
            [owner_stockpile.this_ = new_<Condition::EmpireStockpileValue>(owner_stockpile.stockpile_type, owner_stockpile.low, owner_stockpile.high)];

        visible_to_empire =
            (str_p("visibletoempire")
             >> empire_label
             >> (int_expr_p[push_back_(visible_to_empire.empires, arg1)]
                 | ('[' >> +(int_expr_p[push_back_(visible_to_empire.empires, arg1)]) >> ']')))
            [visible_to_empire.this_ = new_<Condition::VisibleToEmpire>(visible_to_empire.empires)];

        stationary =
            str_p("stationary")
            [stationary.this_ = new_<Condition::Stationary>()];


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
            | random[condition2_p.this_ = arg1]
            | owner_stockpile[condition2_p.this_ = arg1]
            | visible_to_empire[condition2_p.this_ = arg1]
            | stationary[condition2_p.this_ = arg1];
    }
    ConditionParser2Definition condition2_def;
}
