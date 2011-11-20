#include "ConditionParserImpl.h"

#include "ValueRefParser.h"
#include "Label.h"
#include "../universe/Condition.h"

#include <GG/ReportParseError.h>

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const ValueRef::ValueRefBase<StarType>*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_3
    {
        condition_parser_rules_3()
        {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =
                parse::value_ref_parser<int>();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =
                parse::value_ref_parser<double>();

            const parse::value_ref_parser_rule< ::StarType>::type& star_type_value_ref =
                parse::value_ref_parser< ::StarType>();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;

            within_distance
                =    tok.WithinDistance_
                >    parse::label(Distance_name)  > double_value_ref [ _a = _1 ]
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::WithinDistance>(_a, _1) ]
                ;

            within_starlane_jumps
                =    tok.WithinStarlaneJumps_
                >    parse::label(Jumps_name)     > int_value_ref [ _a = _1 ]
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::WithinStarlaneJumps>(_a, _1) ]
                ;

            number
                =    tok.Number_
                >>  -(
                            parse::label(Low_name) >> int_value_ref [ _a = _1 ]
                        )
                >>  -(
                            parse::label(High_name) >> int_value_ref [ _b = _1 ]
                        )
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::Number>(_a, _b, _1) ]
                ;

            turn
                =    (
                            tok.Turn_
                        >> -(
                                parse::label(Low_name) >> int_value_ref [ _a = _1 ]
                            )
                        >> -(
                                parse::label(High_name) >> int_value_ref [ _b = _1 ]
                            )
                        )
                        [ _val = new_<Condition::Turn>(_a, _b) ]
                ;

            created_on_turn
                =    (
                            tok.CreatedOnTurn_
                        >> -(
                                parse::label(Low_name) >> int_value_ref [ _a = _1 ]
                            )
                        >> -(
                                parse::label(High_name) >> int_value_ref [ _b = _1 ]
                            )
                        )
                        [ _val = new_<Condition::CreatedOnTurn>(_a, _b) ]
                ;

            number_of
                =    (
                            tok.NumberOf_
                        >   parse::label(Number_name)    > int_value_ref [ _a = _1 ]
                        >   parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::SortedNumberOf>(_a, _1) ]
                        )
                |    (
                            (
                                tok.MaximumNumberOf_ [ _b = Condition::SORT_MAX ]
                            |   tok.MinimumNumberOf_ [ _b = Condition::SORT_MIN ]
                            |   tok.ModeNumberOf_ [ _b = Condition::SORT_MODE ]
                            )
                        >   parse::label(Number_name)    > int_value_ref [ _a = _1 ]
                        >   parse::label(SortKey_name)   > double_value_ref [ _c = _1 ]
                        >   parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::SortedNumberOf>(_a, _c, _b, _1) ]
                        )
                ;

            contains
                =    tok.Contains_
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::Contains>(_1) ]
                ;

            contained_by
                =    tok.ContainedBy_
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::ContainedBy>(_1) ]
                ;

            star_type
                =    tok.Star_
                >    parse::label(Type_name)
                >>   (
                            '[' > +star_type_value_ref [ push_back(_a, _1) ] > ']'
                        |   star_type_value_ref [ push_back(_a, _1) ]
                        )
                        [ _val = new_<Condition::StarType>(_a) ]
                ;

            random
                =    tok.Random_
                >    parse::label(Probability_name) > double_value_ref [ _val = new_<Condition::Chance>(_1) ]
                ;

            owner_stockpile
                =    (
                            tok.OwnerFoodStockpile_ [ _a = RE_FOOD ]
                        |   tok.OwnerMineralStockpile_ [ _a = RE_MINERALS ]
                        |   tok.OwnerTradeStockpile_ [ _a = RE_TRADE ]
                        )
                >    parse::label(Low_name)  > double_value_ref [ _b = _1 ]
                >    parse::label(High_name) > double_value_ref [ _val = new_<Condition::EmpireStockpileValue>(_a, _b, _1) ]
                ;

            resource_supply_connected
                =    tok.ResourceSupplyConnected_
                >    parse::label(Empire_name)    > int_value_ref [ _a = _1 ]
                >    parse::label(Condition_name) > parse::detail::condition_parser [ _val = new_<Condition::ResourceSupplyConnectedByEmpire>(_a, _1) ]
                ;

            start
                %=   within_distance
                |    within_starlane_jumps
                |    number
                |    turn
                |    created_on_turn
                |    number_of
                |    contains
                |    contained_by
                |    star_type
                |    random
                |    owner_stockpile
                |    resource_supply_connected
                ;

            within_distance.name("WithinDistance");
            within_starlane_jumps.name("WithinStarlaneJumps");
            number.name("Number");
            turn.name("Turn");
            created_on_turn.name("CreatedOnTurn");
            number_of.name("NumberOf");
            contains.name("Contains");
            contained_by.name("ContainedBy");
            star_type.name("StarType");
            random.name("Random");
            owner_stockpile.name("OwnerStockpile");
            resource_supply_connected.name("ResourceSupplyConnected");

#if DEBUG_CONDITION_PARSERS
            debug(within_distance);
            debug(within_starlane_jumps);
            debug(number);
            debug(turn);
            debug(created_on_turn);
            debug(number_of);
            debug(contains);
            debug(contained_by);
            debug(star_type);
            debug(random);
            debug(owner_stockpile);
            debug(resource_supply_connected);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<ValueRef::ValueRefBase<int>*>,
            parse::skipper_type
        > int_ref_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<ValueRef::ValueRefBase<double>*>,
            parse::skipper_type
        > double_ref_rule;

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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<std::vector<const ValueRef::ValueRefBase<StarType>*> >,
            parse::skipper_type
        > star_type_vec_rule;

        double_ref_rule within_distance;
        int_ref_rule within_starlane_jumps;
        int_ref_int_ref_rule number;
        int_ref_int_ref_rule turn;
        int_ref_int_ref_rule created_on_turn;
        int_ref_sorting_method_double_ref_rule number_of;
        parse::condition_parser_rule contains;
        parse::condition_parser_rule contained_by;
        star_type_vec_rule star_type;
        parse::condition_parser_rule random;
        resource_type_double_ref_rule owner_stockpile;
        int_ref_rule resource_supply_connected;
        parse::condition_parser_rule start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_3()
    {
        static condition_parser_rules_3 retval;
        return retval.start;
    }
} }
