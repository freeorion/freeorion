#include "ConditionParser4.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<condition_payload>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_4::condition_parser_rules_4(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_4::base_type(start, "condition_parser_rules_4"),
        int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        non_ship_part_meter_type_enum(tok),
        ship_part_meter_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        qi::omit_type omit_;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        using phoenix::new_;
        using phoenix::construct;

        meter_value
            =   (
                non_ship_part_meter_type_enum
                >  -(label(tok.Low_)  > double_rules.expr)
                >  -(label(tok.High_) > double_rules.expr)
            ) [ _val = construct_movable_(new_<Condition::MeterValue>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass))) ]
            ;

        ship_part_meter_value
            =   (
                omit_[tok.ShipPartMeter_]
                >   label(tok.Part_)    >   string_grammar
                >   label(tok.Meter_)   >   ship_part_meter_type_enum
                >  -(label(tok.Low_)    >   double_rules.expr)
                >  -(label(tok.High_)   >   double_rules.expr)
            ) [ _val = construct_movable_(new_<Condition::ShipPartMeterValue>(
                deconstruct_movable_(_1, _pass),
                _2,
                deconstruct_movable_(_3, _pass),
                deconstruct_movable_(_4, _pass))) ]
            ;

        empire_meter_value1
            =   (
                (omit_[tok.EmpireMeter_]
                >>  label(tok.Empire_))  >   int_rules.expr
                >   label(tok.Meter_)    >   tok.string
                >  -(label(tok.Low_)     >   double_rules.expr)
                >  -(label(tok.High_)    >   double_rules.expr)
            ) [ _val = construct_movable_(new_<Condition::EmpireMeterValue>(
                deconstruct_movable_(_1, _pass),
                _2,
                deconstruct_movable_(_3, _pass),
                deconstruct_movable_(_4, _pass))) ]
            ;

        empire_meter_value2
            =   (
                (omit_[tok.EmpireMeter_]
                >>   label(tok.Meter_))  >   tok.string
                >  -(label(tok.Low_)     >   double_rules.expr)
                >  -(label(tok.High_)    >   double_rules.expr)
            ) [ _val = construct_movable_(new_<Condition::EmpireMeterValue>(
                _1,
                deconstruct_movable_(_2, _pass),
                deconstruct_movable_(_3, _pass))) ]
            ;

        empire_meter_value
            %=  empire_meter_value1
            |   empire_meter_value2
            ;

        start
            %=  meter_value
            |   ship_part_meter_value
            |   empire_meter_value
            ;

        meter_value.name("MeterValue");
        ship_part_meter_value.name("ShipPartMeterValue");
        empire_meter_value.name("EmpireMeterValue");

#if DEBUG_CONDITION_PARSERS
        debug(meter_value);
        debug(ship_part_meter_value);
        debug(empire_meter_value);
#endif
    }

} }
