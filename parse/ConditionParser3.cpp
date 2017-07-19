#include "ConditionParserImpl.h"

#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_3 {
        condition_parser_rules_3() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;
            qi::lit_type lit;
            using phoenix::new_;
            using phoenix::push_back;

            has_special_capacity
                =   (   tok.HasSpecialCapacity_
                >       parse::detail::label(Name_token) >  parse::string_value_ref() [ _c = _1 ]
                >     -(parse::detail::label(Low_token)  >  parse::double_value_ref() [ _a = _1 ] )
                >     -(parse::detail::label(High_token) >  parse::double_value_ref() [ _b = _1 ] )
                    ) [ _val = new_<Condition::HasSpecial>(_c, _a, _b) ]
                ;

            within_distance
                =   tok.WithinDistance_
                >   parse::detail::label(Distance_token)  > parse::double_value_ref() [ _a = _1 ]
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::WithinDistance>(_a, _1) ]
                ;

            within_starlane_jumps
                =   tok.WithinStarlaneJumps_
                >   parse::detail::label(Jumps_token)     > parse::flexible_int_value_ref() [ _a = _1 ]
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::WithinStarlaneJumps>(_a, _1) ]
                ;

            number
                =   tok.Number_
                > -(parse::detail::label(Low_token)   >  parse::flexible_int_value_ref() [ _a = _1 ])
                > -(parse::detail::label(High_token)  >  parse::flexible_int_value_ref() [ _b = _1 ])
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::Number>(_a, _b, _1) ]
                ;

            value_test_1
                = '('
                >> parse::double_value_ref() [ _a = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _d = Condition::GREATER_THAN ]
                      | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _d = Condition::LESS_THAN ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> parse::double_value_ref()
                [ _val = new_<Condition::ValueTest>(_a, _d, _1) ]
                >> ')'
                ;

            value_test_2
                = ('('
                >> parse::double_value_ref() [ _a = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _d = Condition::GREATER_THAN ]
                      | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _d = Condition::LESS_THAN ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> parse::double_value_ref() [ _b = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _e = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _e = Condition::GREATER_THAN ]
                      | lit("<=")   [ _e = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _e = Condition::LESS_THAN ]
                      | lit("!=")   [ _e = Condition::NOT_EQUAL ])
                  ) >  parse::double_value_ref()
                [ _val = new_<Condition::ValueTest>(_a, _d, _b, _e, _1) ]
                >  ')'
                ;

            value_test_3
                = '('
                >> parse::string_value_ref() [ _c = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> parse::string_value_ref()
                [ _val = new_<Condition::ValueTest>(_c, _d, _1) ]
                >> ')'
                ;

            value_test_4
                = ( '('
                >> parse::string_value_ref() [ _c = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> parse::string_value_ref() [ _f = _1 ]
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit("!=")   [ _e = Condition::NOT_EQUAL ])
                ) >  parse::string_value_ref()
                [ _val = new_<Condition::ValueTest>(_c, _d, _f, _e, _1) ]
                >  ')'
                ;

            turn
                =  (tok.Turn_
                > -(parse::detail::label(Low_token)  > (parse::flexible_int_value_ref() [ _a = _1 ]))
                > -(parse::detail::label(High_token) > (parse::flexible_int_value_ref() [ _b = _1 ])))
                [ _val = new_<Condition::Turn>(_a, _b) ]
                ;

            created_on_turn
                =  (tok.CreatedOnTurn_
                > -(parse::detail::label(Low_token)  > parse::flexible_int_value_ref() [ _a = _1 ])
                > -(parse::detail::label(High_token) > parse::flexible_int_value_ref() [ _b = _1 ]))
                [ _val = new_<Condition::CreatedOnTurn>(_a, _b) ]
                ;

            number_of1
                =   tok.NumberOf_
                >   parse::detail::label(Number_token)    > parse::flexible_int_value_ref() [ _a = _1 ]
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::SortedNumberOf>(_a, _1) ]
                ;

            number_of2
                =   (   tok.MaximumNumberOf_ [ _b = Condition::SORT_MAX ]
                    |   tok.MinimumNumberOf_ [ _b = Condition::SORT_MIN ]
                    |   tok.ModeNumberOf_    [ _b = Condition::SORT_MODE ]
                    )
                >   parse::detail::label(Number_token)    > parse::flexible_int_value_ref() [ _a = _1 ]
                >   parse::detail::label(SortKey_token)   > parse::double_value_ref() [ _c = _1 ]
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::SortedNumberOf>(_a, _c, _b, _1) ]
                ;

            number_of
                =   number_of1
                |   number_of2
                ;

            random
                =   tok.Random_
                >   parse::detail::label(Probability_token) > parse::double_value_ref()
                [ _val = new_<Condition::Chance>(_1) ]
                ;

            owner_stockpile
                =   tok.OwnerTradeStockpile_ [ _a = RE_TRADE ]
                >   parse::detail::label(Low_token)  > parse::double_value_ref() [ _b = _1 ]
                >   parse::detail::label(High_token) > parse::double_value_ref()
                [ _val = new_<Condition::EmpireStockpileValue>(_a, _b, _1) ]
                ;

            resource_supply_connected
                =   tok.ResourceSupplyConnected_
                >   parse::detail::label(Empire_token)    > parse::int_value_ref() [ _a = _1 ]
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::ResourceSupplyConnectedByEmpire>(_a, _1) ]
                ;

            can_add_starlane
                =   tok.CanAddStarlanesTo_
                >   parse::detail::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::CanAddStarlaneConnection>(_1) ]
                ;

            start
                =   has_special_capacity
                |   within_distance
                |   within_starlane_jumps
                |   number
                |   value_test_2    // more complicated format that is strict extension of value_test_1 format, so needs to be tested before it
                |   value_test_1
                |   value_test_4    // more complicated format that is strict extension of value_test_3 format, so needs to be tested before it
                |   value_test_3
                |   turn
                |   created_on_turn
                |   number_of
                |   random
                |   owner_stockpile
                |   resource_supply_connected
                |   can_add_starlane
                ;

            has_special_capacity.name("HasSpecialCapacity");
            within_distance.name("WithinDistance");
            within_starlane_jumps.name("WithinStarlaneJumps");
            number.name("Number");
            value_test_1.name("ValueTest Binary");
            value_test_2.name("ValueTest Trinary");
            turn.name("Turn");
            created_on_turn.name("CreatedOnTurn");
            number_of.name("NumberOf");
            random.name("Random");
            owner_stockpile.name("OwnerStockpile");
            resource_supply_connected.name("ResourceSupplyConnected");
            can_add_starlane.name("CanAddStarlanesTo");

#if DEBUG_CONDITION_PARSERS
            debug(has_special_capacity);
            debug(within_distance);
            debug(within_starlane_jumps);
            debug(number);
            debug(value_test_1);
            debug(value_test_2);
            debug(turn);
            debug(created_on_turn);
            debug(number_of);
            debug(random);
            debug(owner_stockpile);
            debug(resource_supply_connected);
            debug(can_add_starlane);
#endif
        }

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                Condition::ComparisonType,
                Condition::ComparisonType,
                ValueRef::ValueRefBase<std::string>*
            >
        > double_ref_double_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >
        > int_ref_int_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                Condition::SortingMethod,
                ValueRef::ValueRefBase<double>*
            >
        > int_ref_sorting_method_double_ref_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<double>*
            >
        > resource_type_double_ref_rule;

        double_ref_double_ref_rule              has_special_capacity;
        double_ref_double_ref_rule              within_distance;
        int_ref_int_ref_rule                    within_starlane_jumps;
        int_ref_int_ref_rule                    number;
        double_ref_double_ref_rule              value_test_1;
        double_ref_double_ref_rule              value_test_2;
        double_ref_double_ref_rule              value_test_3;
        double_ref_double_ref_rule              value_test_4;
        int_ref_int_ref_rule                    turn;
        int_ref_int_ref_rule                    created_on_turn;
        int_ref_sorting_method_double_ref_rule  number_of;
        int_ref_sorting_method_double_ref_rule  number_of1;
        int_ref_sorting_method_double_ref_rule  number_of2;
        parse::condition_parser_rule            random;
        resource_type_double_ref_rule           owner_stockpile;
        int_ref_int_ref_rule                    resource_supply_connected;
        parse::condition_parser_rule            can_add_starlane;
        parse::condition_parser_rule            start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_3() {
        static condition_parser_rules_3 retval;
        return retval.start;
    }
} }
