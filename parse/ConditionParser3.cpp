#include "ConditionParserImpl.h"

#include "ValueRefParser.h"
#include "Label.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_3 {
        condition_parser_rules_3() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =
                parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<double>::type& double_value_ref =
                parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule< int >::type& flexible_int_ref = 
                parse::value_ref_parser_flexible_int();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref =
                parse::value_ref_parser<std::string>();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            has_special_capacity
                =   (   tok.HasSpecialCapacity_
                >       parse::label(Name_token) >  string_value_ref [ _c = _1 ]
                >     -(parse::label(Low_token)  >  double_value_ref [ _a = _1 ] )
                >     -(parse::label(High_token) >  double_value_ref [ _b = _1 ] )
                    ) [ _val = new_<Condition::HasSpecial>(_c, _a, _b) ]
                ;

            within_distance
                =   tok.WithinDistance_
                >   parse::label(Distance_token)  > double_value_ref [ _a = _1 ]
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::WithinDistance>(_a, _1) ]
                ;

            within_starlane_jumps
                =   tok.WithinStarlaneJumps_
                >   parse::label(Jumps_token)     > flexible_int_ref [ _a = _1 ]
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::WithinStarlaneJumps>(_a, _1) ]
                ;

            number
                =   tok.Number_
                > -(parse::label(Low_token)   >  flexible_int_ref [ _a = _1 ])
                > -(parse::label(High_token)  >  flexible_int_ref [ _b = _1 ])
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::Number>(_a, _b, _1) ]
                ;

            value_test
                =   tok.ValueTest_
                > -(parse::label(Low_token)    > double_value_ref [ _a = _1 ])
                > -(parse::label(High_token)   > double_value_ref [ _b = _1 ])
                >   parse::label(TestValue_token) > double_value_ref
                [ _val = new_<Condition::ValueTest>(_1, _a, _b) ]
                ;

            turn
                =  (tok.Turn_
                > -(parse::label(Low_token)  > (flexible_int_ref [ _a = _1 ]))
                > -(parse::label(High_token) > (flexible_int_ref [ _b = _1 ])))
                [ _val = new_<Condition::Turn>(_a, _b) ]
                ;

            created_on_turn
                =  (tok.CreatedOnTurn_
                > -(parse::label(Low_token)  > flexible_int_ref [ _a = _1 ])
                > -(parse::label(High_token) > flexible_int_ref [ _b = _1 ]))
                [ _val = new_<Condition::CreatedOnTurn>(_a, _b) ]
                ;

            number_of1
                =   tok.NumberOf_
                >   parse::label(Number_token)    > flexible_int_ref [ _a = _1 ]
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::SortedNumberOf>(_a, _1) ]
                ;

            number_of2
                =   (   tok.MaximumNumberOf_ [ _b = Condition::SORT_MAX ]
                    |   tok.MinimumNumberOf_ [ _b = Condition::SORT_MIN ]
                    |   tok.ModeNumberOf_    [ _b = Condition::SORT_MODE ]
                    )
                >   parse::label(Number_token)    > flexible_int_ref [ _a = _1 ]
                >   parse::label(SortKey_token)   > double_value_ref [ _c = _1 ]
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::SortedNumberOf>(_a, _c, _b, _1) ]
                ;

            number_of
                =   number_of1
                |   number_of2
                ;

            random
                =   tok.Random_
                >   parse::label(Probability_token) > double_value_ref
                [ _val = new_<Condition::Chance>(_1) ]
                ;

            owner_stockpile
                =   tok.OwnerTradeStockpile_ [ _a = RE_TRADE ]
                >   parse::label(Low_token)  > double_value_ref [ _b = _1 ]
                >   parse::label(High_token) > double_value_ref
                [ _val = new_<Condition::EmpireStockpileValue>(_a, _b, _1) ]
                ;

            resource_supply_connected
                =   tok.ResourceSupplyConnected_
                >   parse::label(Empire_token)    > int_value_ref [ _a = _1 ]
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::ResourceSupplyConnectedByEmpire>(_a, _1) ]
                ;

            can_add_starlane
                =   tok.CanAddStarlanesTo_
                >   parse::label(Condition_token) > parse::detail::condition_parser
                [ _val = new_<Condition::CanAddStarlaneConnection>(_1) ]
                ;

            start
                =   has_special_capacity
                |   within_distance
                |   within_starlane_jumps
                |   number
                |   value_test
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
            value_test.name("ValueTest");
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
            debug(value_test);
            debug(turn);
            debug(created_on_turn);
            debug(number_of);
            debug(random);
            debug(owner_stockpile);
            debug(resource_supply_connected);
            debug(can_add_starlane);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*
            >,
            parse::skipper_type
        > double_ref_double_ref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*
            >,
            parse::skipper_type
        > int_ref_int_ref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                Condition::SortingMethod,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > int_ref_sorting_method_double_ref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > resource_type_double_ref_rule;

        double_ref_double_ref_rule              has_special_capacity;
        double_ref_double_ref_rule              within_distance;
        int_ref_int_ref_rule                    within_starlane_jumps;
        int_ref_int_ref_rule                    number;
        double_ref_double_ref_rule              value_test;
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
