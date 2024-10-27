#include "ConditionParser3.h"

#include "../Empire/ResourcePool.h"
#include "../universe/Conditions.h"
#include "../universe/ValueRef.h"

#include <boost/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse::detail {
    condition_parser_rules_3::condition_parser_rules_3(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_3::base_type(start, "condition_parser_rules_3"),
        int_rules(tok, label, condition_parser, string_grammar),
        castable_int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        resource_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_5_type _5;
        qi::_val_type _val;
        qi::lit_type lit;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        using phoenix::new_;
        using phoenix::construct;

        has_special_capacity
            = ( omit_[tok.HasSpecialCapacity_]
            >   label(tok.name_) >  string_grammar
            > -(label(tok.low_)  >  double_rules.expr)
            > -(label(tok.high_) >  double_rules.expr)
              ) [ _val = construct_movable_(new_<Condition::HasSpecial>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        within_distance
            = (omit_[tok.WithinDistance_]
               > label(tok.distance_)  > double_rules.expr
               > label(tok.condition_) > condition_parser
              ) [ _val = construct_movable_(new_<Condition::WithinDistance>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        within_starlane_jumps
            = (omit_[tok.WithinStarlaneJumps_]
               > label(tok.jumps_)     > castable_int_rules.flexible_int
               > label(tok.condition_) > condition_parser
              ) [ _val = construct_movable_(new_<Condition::WithinStarlaneJumps>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        number
            = (omit_[tok.Number_]
               > -(label(tok.low_)   >  castable_int_rules.flexible_int)
               > -(label(tok.high_)  >  castable_int_rules.flexible_int)
               >   label(tok.condition_) > condition_parser
              ) [ _val = construct_movable_(new_<Condition::Number>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        comparison_operator =
              lit("==")   [ _val = Condition::ComparisonType::EQUAL ]
            | lit('=')    [ _val = Condition::ComparisonType::EQUAL ]
            | lit(">=")   [ _val = Condition::ComparisonType::GREATER_THAN_OR_EQUAL ]
            | lit('>')    [ _val = Condition::ComparisonType::GREATER_THAN ]
            | lit("<=")   [ _val = Condition::ComparisonType::LESS_THAN_OR_EQUAL ]
            | lit('<')    [ _val = Condition::ComparisonType::LESS_THAN ]
            | lit("!=")   [ _val = Condition::ComparisonType::NOT_EQUAL ]
            ;

        string_comparison_operator =
              lit("==")   [ _val = Condition::ComparisonType::EQUAL ]
            | lit('=')    [ _val = Condition::ComparisonType::EQUAL ]
            | lit("!=")   [ _val = Condition::ComparisonType::NOT_EQUAL ]
            ;

        comparison_binary_double
            = (('('
                >> double_rules.expr
                >> comparison_operator
                >> double_rules.expr // assuming the trinary form already didn't pass, can expect a (double) here, though it might be an (int) casted to (double). By matching the (int) cases first, can assume that at least one of the parameters here is not an (int) casted to double.
               ) > ')'
              ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass))) ]
            ;

        comparison_trinary_double
            = (( '('
               >> double_rules.expr
               >> comparison_operator
               >> double_rules.expr
               >> comparison_operator
               >> double_rules.expr    // if already seen (double) (operator) (double) (operator) can expect to see another (double). Some of these (double) may be (int) casted to double, though not all of them can be, as in that case, the (int) parser should have matched.
               ) >  ')'
              ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass),
                    _4,
                    deconstruct_movable_(_5, _pass))) ]
            ;

        comparison_binary_string
            = (( '('
                >> string_grammar
                >> string_comparison_operator
                >> string_grammar // assuming the trinary (string) form already didn't parse, if already seen (string) (operator) can expect another (string)
               ) > ')'
              ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass))) ]
            ;

        comparison_trinary_string
            =
            (( '('
              >> string_grammar
              >> string_comparison_operator
              >> string_grammar
              >> string_comparison_operator
              >> string_grammar // if already seen (string) (operator) (string) (operator) can expect to see another (string)
             ) >  ')'
            ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass),
                    _4,
                    deconstruct_movable_(_5, _pass))) ]
            ;

        comparison_binary_int
            = (( '('
                >> castable_int_rules.enum_or_int
                >> comparison_operator
                >> castable_int_rules.enum_or_int // can't expect an (int) here, as it could actually be a (double) comparision with the first (double) casted from an (int)
               ) > ')'
              ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass))) ]
            ;

        comparison_trinary_int
            = (( '('
               >> castable_int_rules.enum_or_int
               >> comparison_operator
               >> castable_int_rules.enum_or_int
               >> comparison_operator
               >> castable_int_rules.enum_or_int // only treat as trinary (int) comparison if all parameters are (int) or an enum. otherwise fall back to (double) comparison, which allows some of the parameters to be (int) casted to (double)
               ) > ')'
              ) [ _val = construct_movable_(
                new_<Condition::ValueTest>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass),
                    _4,
                    deconstruct_movable_(_5, _pass))) ]
            ;

        turn
            = ( omit_[tok.Turn_]
            > -(label(tok.low_)  > (castable_int_rules.flexible_int ))
            > -(label(tok.high_) > (castable_int_rules.flexible_int )))
            [ _val = construct_movable_(new_<Condition::Turn>(
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass))) ]
            ;

        created_on_turn
            = ( omit_[tok.CreatedOnTurn_]
            > -(label(tok.low_)  > castable_int_rules.flexible_int )
            > -(label(tok.high_) > castable_int_rules.flexible_int ))
            [ _val = construct_movable_(new_<Condition::CreatedOnTurn>(
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass))) ]
            ;

        number_of1
            = ( omit_[tok.NumberOf_]
            >   label(tok.number_)    > castable_int_rules.flexible_int
            >   label(tok.condition_) > condition_parser)
            [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass))) ]
            ;

        unique_of1
            = ((omit_[tok.Unique_]
            >>  label(tok.sortkey_)  >> double_rules.expr)
            >   label(tok.condition_) > condition_parser)
            [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<int>>(std::numeric_limits<int>::max())), _pass),
                deconstruct_movable_(_1, _pass),
                Condition::SortingMethod::SORT_UNIQUE,
                deconstruct_movable_(_2, _pass))) ]
            ;

        unique_of2
            = ((omit_[tok.Unique_]
            >> label(tok.sortkey_) >> string_grammar)
            >  label(tok.condition_) > condition_parser)
            [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                deconstruct_movable_(construct_movable_(new_<ValueRef::Constant<int>>(std::numeric_limits<int>::max())), _pass),
                deconstruct_movable_(_1, _pass),
                Condition::SortingMethod::SORT_UNIQUE,
                deconstruct_movable_(_2, _pass))) ]
            ;

        sorting_operator =
                tok.MaximumNumberOf_ [ _val = Condition::SortingMethod::SORT_MAX ]
            |   tok.MinimumNumberOf_ [ _val = Condition::SortingMethod::SORT_MIN ]
            |   tok.ModeNumberOf_    [ _val = Condition::SortingMethod::SORT_MODE ]
            |   tok.UniqueNumberOf_  [ _val = Condition::SortingMethod::SORT_UNIQUE ];

        number_of2
            =  (sorting_operator
            >   label(tok.number_)    > castable_int_rules.flexible_int
            >   label(tok.sortkey_)   > double_rules.expr
            >   label(tok.condition_) > condition_parser)
            [ _val = construct_movable_(new_<Condition::SortedNumberOf>(
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                _1,
                deconstruct_movable_(_4, _pass))) ]
            ;

        number_of
            =   number_of1
            |   unique_of1
            |   unique_of2
            |   number_of2
            ;

        random
            =   tok.Random_
            >   label(tok.probability_) > double_rules.expr
            [ _val = construct_movable_(new_<Condition::Chance>(deconstruct_movable_(_1, _pass))) ]
            ;

        stockpile
            = ( omit_[tok.EmpireStockpile_]
            >   label(tok.empire_)   >  int_rules.expr
            >   label(tok.resource_) >  resource_type_enum
            > -(label(tok.low_)      >  double_rules.expr)
            > -(label(tok.high_)     >  double_rules.expr)
              ) [ _val = construct_movable_(new_<Condition::EmpireStockpileValue>(
                    deconstruct_movable_(_1, _pass),
                    _2,
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass))) ]
            ;

        resource_supply_connected
            = ( omit_[tok.ResourceSupplyConnected_]
            >   label(tok.empire_)    > int_rules.expr
            >   label(tok.condition_) > condition_parser)
            [ _val = construct_movable_(new_<Condition::ResourceSupplyConnectedByEmpire>(
                deconstruct_movable_(_1, _pass),
                deconstruct_movable_(_2, _pass))) ]
            ;

        has_starlane_to
            = ( omit_[tok.HasStarlane_]
            >   label(tok.from_) > condition_parser)
            [_val = construct_movable_(new_<Condition::HasStarlaneTo>(
                deconstruct_movable_(_1, _pass)))]
        ;

        starlane_to_would_cross_existing_starlane
            = ( omit_[tok.StarlaneToWouldCrossExistingStarlane_]
            >   label(tok.from_) > condition_parser)
            [_val = construct_movable_(new_<Condition::StarlaneToWouldCrossExistingStarlane>(
                deconstruct_movable_(_1, _pass)))]
        ;

        starlane_to_would_be_angularly_close_to_existing_starlane
            = ( omit_[tok.StarlaneToWouldBeAngularlyCloseToExistingStarlane_]
            >   label(tok.from_) > condition_parser
            >   label(tok.maxdotprod_) > tok.double_)
            [_val = construct_movable_(new_<Condition::StarlaneToWouldBeAngularlyCloseToExistingStarlane>(
                deconstruct_movable_(_1, _pass),
                _2))]
        ;

        starlane_to_would_be_close_to_object
            = ( omit_[tok.StarlaneToWouldBeCloseToObject_]
            >   label(tok.distance_) > tok.double_
            >   label(tok.from_) > condition_parser
            >   label(tok.closeto_) > condition_parser)
            [_val = construct_movable_(new_<Condition::StarlaneToWouldBeCloseToObject>(
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass),
                _1))]
        ;

        start
            %=  has_special_capacity
            |   within_distance
            |   within_starlane_jumps
            |   number
            |   comparison_trinary_int      // more complicated format that is strict extension of comparison_binary_int format, so needs to be tested before it
            |   comparison_binary_int       //  first int ...
            |   comparison_trinary_double   // more complicated format that is strict extension of comparison_binary_double format, so needs to be tested before it
            |   comparison_binary_double    //  ... then double (which may include int(s) casted to double(s))...
            |   comparison_trinary_string   // more complicated format that is strict extension of comparison_binary_string format, so needs to be tested before it
            |   comparison_binary_string    //  ... then string
            |   turn
            |   created_on_turn
            |   number_of
            |   random
            |   stockpile
            |   resource_supply_connected
            |   has_starlane_to
            |   starlane_to_would_cross_existing_starlane
            |   starlane_to_would_be_angularly_close_to_existing_starlane
            |   starlane_to_would_be_close_to_object
            ;

        has_special_capacity.name("HasSpecialCapacity");
        within_distance.name("WithinDistance");
        within_starlane_jumps.name("WithinStarlaneJumps");
        number.name("Number");
        comparison_operator.name("comparison operator");
        string_comparison_operator.name("string comparison operator");
        comparison_operator.name("comparison operator");
        comparison_binary_double.name("ValueTest Binary double");
        comparison_trinary_double.name("ValueTest Trinary double");
        comparison_binary_string.name("ValueTest Binary string");
        comparison_trinary_string.name("ValueTest Trinary string");
        comparison_binary_int.name("ValueTest Binary int");
        comparison_trinary_int.name("ValueTest Trinary int");
        turn.name("Turn");
        created_on_turn.name("CreatedOnTurn");
        sorting_operator.name("sorting operator");
        number_of.name("NumberOf");
        random.name("Random");
        stockpile.name("EmpireStockpile");
        resource_supply_connected.name("ResourceSupplyConnected");
        has_starlane_to.name("HasStarlaneTo");
        starlane_to_would_cross_existing_starlane.name("StarlaneToWouldCrossExistingStarlane");
        starlane_to_would_be_angularly_close_to_existing_starlane.name("StarlaneToWouldBeAngularlyCloseToExistingStarlane");
        starlane_to_would_be_close_to_object.name("StarlaneToWouldBeCloseToObject");

#if DEBUG_CONDITION_PARSERS
        debug(has_special_capacity);
        debug(within_distance);
        debug(within_starlane_jumps);
        debug(number);
        debug(turn);
        debug(created_on_turn);
        debug(number_of);
        debug(random);
        debug(stockpile);
        debug(resource_supply_connected);
        debug(has_starlane_to);
        debug(starlane_to_would_cross_existing_starlane);
        debug(starlane_to_would_be_angularly_close_to_existing_starlane);
        debug(starlane_to_would_be_close_to_object);
#endif
    }
}
