#include "Parser.h"

#include "ParserUtil.h"
#include "ValueRefParser.h"
#include "Condition.h"

using namespace boost::spirit::classic;
using namespace phoenix;

extern ParamLabel distance_label;
extern ParamLabel condition_label;
extern ParamLabel jumps_label;
extern ParamLabel low_label;
extern ParamLabel high_label;
extern ParamLabel number_label;
extern ParamLabel sort_key_label;
extern ParamLabel type_label;
extern ParamLabel probability_label;
extern ParamLabel empire_label;

ValueRef::ValueRefBase<int>* const NULL_INT_REF = 0;

rule<Scanner, ConditionClosure::context_t> condition3_p;

namespace {
    class ConditionParser3Definition
    {
    public:
        typedef rule<Scanner, ConditionClosure::context_t> Rule;

        ConditionParser3Definition();

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

        typedef rule<Scanner, DoubleRefConditionClosure::context_t>     DoubleRefConditionRule;
        typedef rule<Scanner, IntRefConditionClosure::context_t>        IntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefConditionClosure::context_t>  IntRefIntRefConditionRule;
        typedef rule<Scanner, IntRefIntRefClosure::context_t>           IntRefIntRefRule;
        typedef rule<Scanner, SortedNumberOfClosure::context_t>         SortedNumberOfRule;
        typedef rule<Scanner, ConditionParamClosure::context_t>         ConditionParamRule;
        typedef rule<Scanner, StarTypeClosure::context_t>               StarTypeRule;
        typedef rule<Scanner, DoubleRefClosure::context_t>              DoubleRefRule;
        typedef rule<Scanner, StockpileClosure::context_t>              StockpileRule;

        DoubleRefConditionRule      within_distance;
        IntRefConditionRule         within_starlane_jumps;
        IntRefIntRefConditionRule   number;
        IntRefIntRefRule            turn;
        IntRefIntRefRule            created_on_turn;
        SortedNumberOfRule          number_of;
        ConditionParamRule          contains;
        ConditionParamRule          contained_by;
        StarTypeRule                star_type;
        DoubleRefRule               random;
        StockpileRule               owner_stockpile;
        IntRefConditionRule         resource_supply_connected;
    };

    ConditionParser3Definition::ConditionParser3Definition()
    {
        within_distance =
            (str_p("withindistance")
             >> distance_label >> double_expr_p[within_distance.double_ref_vec = arg1]
             >> condition_label >> condition_p[within_distance.condition = arg1])
            [within_distance.this_ = new_<Condition::WithinDistance>(within_distance.double_ref_vec, within_distance.condition)];

        within_starlane_jumps =
            (str_p("withinstarlanejumps")
             >> jumps_label >>      int_expr_p[ within_starlane_jumps.int_ref = arg1]
             >> condition_label >>  condition_p[within_starlane_jumps.condition = arg1])
            [within_starlane_jumps.this_ = new_<Condition::WithinStarlaneJumps>(within_starlane_jumps.int_ref, within_starlane_jumps.condition)];

        number =
            (str_p("number")
             >> (low_label >>       int_expr_p[ number.int_ref_1 = arg1]
                 |                  eps_p[      number.int_ref_1 = val(NULL_INT_REF)])
             >> (high_label >>      int_expr_p[ number.int_ref_2 = arg1]
                 |                  eps_p[      number.int_ref_2 = val(NULL_INT_REF)])
             >> condition_label >>  condition_p[number.condition = arg1])
            [number.this_ = new_<Condition::Number>(number.int_ref_1, number.int_ref_2, number.condition)];

        turn =
            (str_p("turn")
             >> (low_label >>       int_expr_p[ turn.int_ref_1 = arg1]
                 |                  eps_p[      turn.int_ref_1 = val(NULL_INT_REF)])
             >> (high_label >>      int_expr_p[ turn.int_ref_2 = arg1]
                 |                  eps_p[      turn.int_ref_2 = val(NULL_INT_REF)]))
            [turn.this_ = new_<Condition::Turn>(turn.int_ref_1, turn.int_ref_2)];

        created_on_turn =
            (str_p("createdonturn")
             >> (low_label >>       int_expr_p[ created_on_turn.int_ref_1 = arg1]
                 |                  eps_p[      created_on_turn.int_ref_1 = val(NULL_INT_REF)])
             >> (high_label >>      int_expr_p[ created_on_turn.int_ref_2 = arg1]
                 |                  eps_p[      created_on_turn.int_ref_2 = val(NULL_INT_REF)]))
            [created_on_turn.this_ = new_<Condition::CreatedOnTurn>(created_on_turn.int_ref_1, created_on_turn.int_ref_2)];

        number_of =
            ((str_p("numberof")
              >> number_label >>    int_expr_p[ number_of.number = arg1]
              >> condition_label >> condition_p[number_of.condition = arg1])
             [number_of.this_ = new_<Condition::SortedNumberOf>(number_of.number, number_of.condition)])

            | ( ((str_p("maximumnumberof")[number_of.sorting_method =   val(Condition::SORT_MAX)]
                 | str_p("minimumnumberof")[number_of.sorting_method =  val(Condition::SORT_MIN)]
                 | str_p("modenumberof")[number_of.sorting_method =     val(Condition::SORT_MODE)])
                 >> number_label >>     int_expr_p[number_of.number = arg1]
                 >> sort_key_label >>   double_expr_p[number_of.sort_key = arg1]
                 >> condition_label >>  condition_p[number_of.condition = arg1])
                [number_of.this_ = new_<Condition::SortedNumberOf>(number_of.number, number_of.sort_key,
                                                                   number_of.sorting_method, number_of.condition)]);

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

        resource_supply_connected =
            (str_p("resourcesupplyconnectedbyempire")
             >> empire_label >> int_expr_p[resource_supply_connected.int_ref = arg1]
             >> condition_label >> condition_p[resource_supply_connected.condition = arg1])
            [resource_supply_connected.this_ = new_<Condition::ResourceSupplyConnectedByEmpire>(resource_supply_connected.int_ref,
                                                                                                resource_supply_connected.condition)];

        condition3_p =
            within_distance[condition3_p.this_ = arg1]
            | within_starlane_jumps[condition3_p.this_ = arg1]
            | number[condition3_p.this_ = arg1]
            | turn[condition3_p.this_ = arg1]
            | created_on_turn[condition3_p.this_ = arg1]
            | number_of[condition3_p.this_ = arg1]
            | contains[condition3_p.this_ = arg1]
            | contained_by[condition3_p.this_ = arg1]
            | star_type[condition3_p.this_ = arg1]
            | random[condition3_p.this_ = arg1]
            | owner_stockpile[condition3_p.this_ = arg1]
            | resource_supply_connected[condition3_p.this_ = arg1];
    }
    ConditionParser3Definition condition3_def;
}
