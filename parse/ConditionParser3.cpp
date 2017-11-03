#include "ConditionParser3.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    condition_parser_rules_3::condition_parser_rules_3(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_3::base_type(start, "condition_parser_rules_3"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        castable_int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_val_type _val;
        qi::lit_type lit;
        qi::_pass_type _pass;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        using phoenix::new_;
        using phoenix::construct;

        has_special_capacity
            =   (   tok.HasSpecialCapacity_
                    >       labeller.rule(Name_token) >  string_grammar [ _c = _1 ]
                    >     -(labeller.rule(Low_token)  >  double_rules.expr [ _a = _1 ] )
                    >     -(labeller.rule(High_token) >  double_rules.expr [ _b = _1 ] )
                ) [ _val = construct_movable_(new_<Condition::HasSpecial>(
                    deconstruct_movable_(_c, _pass),
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_(_b, _pass))) ]
            ;

        within_distance
            =   tok.WithinDistance_
            >   labeller.rule(Distance_token)  > double_rules.expr [ _a = _1 ]
            >   labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::WithinDistance>(
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_(_1, _pass))) ]
            ;

        within_starlane_jumps
            =   tok.WithinStarlaneJumps_
            >   labeller.rule(Jumps_token)     > castable_int_rules.flexible_int [ _a = _1 ]
            >   labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::WithinStarlaneJumps>(
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_(_1, _pass))) ]
            ;

        number
            =   tok.Number_
            > -(labeller.rule(Low_token)   >  castable_int_rules.flexible_int [ _a = _1 ])
            > -(labeller.rule(High_token)  >  castable_int_rules.flexible_int [ _b = _1 ])
            >   labeller.rule(Condition_token) > condition_parser
            [ _val = construct_movable_(new_<Condition::Number>(
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_(_b, _pass),
                    deconstruct_movable_(_1, _pass))) ]
            ;

        comparison_binary_double
            = ('('
               >> double_rules.expr
               >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _d = Condition::GREATER_THAN ]
                      | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _d = Condition::LESS_THAN ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
               > double_rules.expr // assuming the trinary form already didn't pass, can expect a (double) here, though it might be an (int) casted to (double). By matching the (int) cases first, can assume that at least one of the parameters here is not an (int) casted to double.
               > ')'
              )
            [ _val = construct_movable_(new_<Condition::ValueTest>(
                        deconstruct_movable_(_1, _pass),
                        _d,
                        deconstruct_movable_(_2, _pass))) ]
                ;

            comparison_trinary_double
                = ('('
                   >> double_rules.expr
                   >> (    lit("==")   [ _d = Condition::EQUAL ]
                           | lit('=')    [ _d = Condition::EQUAL ]
                           | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                           | lit('>')    [ _d = Condition::GREATER_THAN ]
                           | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                           | lit('<')    [ _d = Condition::LESS_THAN ]
                           | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                   >> double_rules.expr
                   >> (    lit("==")   [ _d = Condition::EQUAL ] // TODO check should _d be _e
                           | lit('=')    [ _d = Condition::EQUAL ] // TODO check should _d be _e
                           | lit(">=")   [ _e = Condition::GREATER_THAN_OR_EQUAL ]
                           | lit('>')    [ _e = Condition::GREATER_THAN ]
                           | lit("<=")   [ _e = Condition::LESS_THAN_OR_EQUAL ]
                           | lit('<')    [ _e = Condition::LESS_THAN ]
                           | lit("!=")   [ _e = Condition::NOT_EQUAL ])
                   >>  double_rules.expr    // if already seen (double) (operator) (double) (operator) can expect to see another (double). Some of these (double) may be (int) casted to double, though not all of them can be, as in that case, the (int) parser should have matched.
                   >>  ')' )
                [ _val = construct_movable_(new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _d,
                    deconstruct_movable_(_2, _pass),
                    _e,
                    deconstruct_movable_(_3, _pass))) ]
                ;

            comparison_binary_string
                = ( '('
                >> string_grammar
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                   > string_grammar // assuming the trinary (string) form already didn't parse, if already seen (string) (operator) can expect another (string)
                   > ')'
                  ) [ _val = construct_movable_(new_<Condition::ValueTest>(
                        deconstruct_movable_(_1, _pass),
                        _d,
                        deconstruct_movable_(_2, _pass))) ]
                ;

            comparison_trinary_string
                =
                ( '('
                  >> string_grammar
                  >> (    lit("==")   [ _d = Condition::EQUAL ]
                          | lit('=')    [ _d = Condition::EQUAL ]
                          | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                  >> string_grammar
                  >> (    lit("==")   [ _d = Condition::EQUAL ] // TODO check should _d be _e
                          | lit('=')    [ _d = Condition::EQUAL ] // TODO check should _d be _e
                          | lit("!=")   [ _e = Condition::NOT_EQUAL ])
                  >>  string_grammar // if already seen (string) (operator) (string) (operator) can expect to see another (string)
                  >>  ')' )
                [ _val = construct_movable_(new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _d,
                    deconstruct_movable_(_2, _pass),
                    _e,
                    deconstruct_movable_(_3, _pass))) ]
                ;

            comparison_binary_int
                = ( '('
                >> int_rules.expr
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _d = Condition::GREATER_THAN ]
                      | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _d = Condition::LESS_THAN ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> int_rules.expr   // can't expect an (int) here, as it could actually be a (double) comparision with the first (double) cased from an (int)
                    > ')'
                  ) [ _val = construct_movable_(new_<Condition::ValueTest>(
                        deconstruct_movable_(_1, _pass),
                        _d,
                        deconstruct_movable_(_2, _pass))) ]
                ;

            comparison_trinary_int
                = ('('
                >> int_rules.expr
                >> (    lit("==")   [ _d = Condition::EQUAL ]
                      | lit('=')    [ _d = Condition::EQUAL ]
                      | lit(">=")   [ _d = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _d = Condition::GREATER_THAN ]
                      | lit("<=")   [ _d = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _d = Condition::LESS_THAN ]
                      | lit("!=")   [ _d = Condition::NOT_EQUAL ])
                >> int_rules.expr
                >> (    lit("==")   [ _d = Condition::EQUAL ] // TODO check should _d be _e
                      | lit('=')    [ _d = Condition::EQUAL ] // TODO check should _d be _e
                      | lit(">=")   [ _e = Condition::GREATER_THAN_OR_EQUAL ]
                      | lit('>')    [ _e = Condition::GREATER_THAN ]
                      | lit("<=")   [ _e = Condition::LESS_THAN_OR_EQUAL ]
                      | lit('<')    [ _e = Condition::LESS_THAN ]
                      | lit("!=")   [ _e = Condition::NOT_EQUAL ])
                 >> int_rules.expr   // only treat as trinary (int) comparison if all parameters are (int). otherwise fall back to (double) comparison, which allows some of the parameters to be (int) casted to (double)
                 >>  ')' )
                [ _val = construct_movable_(new_<Condition::ValueTest>(
                         deconstruct_movable_(_1, _pass),
                         _d,
                         deconstruct_movable_(_2, _pass),
                         _e,
                         deconstruct_movable_(_3, _pass))) ]
                ;

            turn
                =  (tok.Turn_
                > -(labeller.rule(Low_token)  > (castable_int_rules.flexible_int [ _a = _1 ]))
                > -(labeller.rule(High_token) > (castable_int_rules.flexible_int [ _b = _1 ])))
                [ _val = construct_movable_(new_<Condition::Turn>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_b, _pass))) ]
                ;

            created_on_turn
                =  (tok.CreatedOnTurn_
                > -(labeller.rule(Low_token)  > castable_int_rules.flexible_int [ _a = _1 ])
                > -(labeller.rule(High_token) > castable_int_rules.flexible_int [ _b = _1 ]))
                [ _val = construct_movable_(new_<Condition::CreatedOnTurn>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_b, _pass))) ]
                ;

            number_of1
                =   tok.NumberOf_
                >   labeller.rule(Number_token)    > castable_int_rules.flexible_int [ _a = _1 ]
                >   labeller.rule(Condition_token) > condition_parser
                [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_1, _pass))) ]
                ;

            number_of2
                =   (   tok.MaximumNumberOf_ [ _b = Condition::SORT_MAX ]
                    |   tok.MinimumNumberOf_ [ _b = Condition::SORT_MIN ]
                    |   tok.ModeNumberOf_    [ _b = Condition::SORT_MODE ]
                    )
                >   labeller.rule(Number_token)    > castable_int_rules.flexible_int [ _a = _1 ]
                >   labeller.rule(SortKey_token)   > double_rules.expr [ _c = _1 ]
                >   labeller.rule(Condition_token) > condition_parser
                [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_c, _pass),
                        _b,
                        deconstruct_movable_(_1, _pass))) ]
                ;

            number_of
                =   number_of1
                |   number_of2
                ;

            random
                =   tok.Random_
                >   labeller.rule(Probability_token) > double_rules.expr
                [ _val = construct_movable_(new_<Condition::Chance>(deconstruct_movable_(_1, _pass))) ]
                ;

            owner_stockpile
                =   tok.OwnerTradeStockpile_ [ _a = RE_TRADE ]
                >   labeller.rule(Low_token)  > double_rules.expr [ _b = _1 ]
                >   labeller.rule(High_token) > double_rules.expr
                [ _val = construct_movable_(new_<Condition::EmpireStockpileValue>(
                        _a,
                        deconstruct_movable_(_b, _pass),
                        deconstruct_movable_(_1, _pass))) ]
                ;

            resource_supply_connected
                =   tok.ResourceSupplyConnected_
                >   labeller.rule(Empire_token)    > int_rules.expr [ _a = _1 ]
                >   labeller.rule(Condition_token) > condition_parser
                [ _val = construct_movable_(new_<Condition::ResourceSupplyConnectedByEmpire>(
                        deconstruct_movable_(_a, _pass),
                        deconstruct_movable_(_1, _pass))) ]
                ;

            can_add_starlane
                =   tok.CanAddStarlanesTo_
                >   labeller.rule(Condition_token) > condition_parser
                [ _val = construct_movable_(new_<Condition::CanAddStarlaneConnection>(
                        deconstruct_movable_(_1, _pass))) ]
                ;

            start
                =   has_special_capacity
                |   within_distance
                |   within_starlane_jumps
                |   number
                |   comparison_trinary_int    // more complicated format that is strict extension of comparison_binary_int format, so needs to be tested before it
                |   comparison_binary_int    //  first int ...
                |   comparison_trinary_double    // more complicated format that is strict extension of comparison_binary_double format, so needs to be tested before it
                |   comparison_binary_double    //  ... then double (which may include int(s) casted to double(s))...
                |   comparison_trinary_string    // more complicated format that is strict extension of comparison_binary_string format, so needs to be tested before it
                |   comparison_binary_string    //  ... then string
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
            comparison_binary_double.name("ValueTest Binary double");
            comparison_trinary_double.name("ValueTest Trinary double");
            comparison_binary_string.name("ValueTest Binary string");
            comparison_trinary_string.name("ValueTest Trinary string");
            comparison_binary_int.name("ValueTest Binary int");
            comparison_trinary_int.name("ValueTest Trinary int");
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

} }
