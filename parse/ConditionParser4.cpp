#include "ConditionParser4.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<Condition::ConditionBase*>&) { return os; }
}
#endif

namespace parse { namespace detail {
    condition_parser_rules_4::condition_parser_rules_4(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_4::base_type(start, "condition_parser_rules_4"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar),
        non_ship_part_meter_type_enum(tok),
        ship_part_meter_type_enum(tok)
    {
        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_val_type _val;
        qi::eps_type eps;
        using phoenix::new_;
        using phoenix::push_back;

        meter_value
            =   (
                non_ship_part_meter_type_enum [ _a = _1 ]
                >  -(labeller.rule(Low_token)  > double_rules.expr [ _b = _1 ])
                >  -(labeller.rule(High_token) > double_rules.expr [ _c = _1 ])
            ) [ _val = new_<Condition::MeterValue>(_a, _b, _c) ]
            ;

        ship_part_meter_value
            =   (
                tok.ShipPartMeter_
                >   labeller.rule(Part_token)    >   string_grammar [ _e = _1 ]
                >   ship_part_meter_type_enum [ _a = _1 ]
                >  -(labeller.rule(Low_token)    >   double_rules.expr [ _b = _1 ])
                >  -(labeller.rule(High_token)   >   double_rules.expr [ _c = _1 ])
            ) [ _val = new_<Condition::ShipPartMeterValue>(_e, _a, _b, _c) ]
            ;

        empire_meter_value1
            =   (
                (tok.EmpireMeter_
                 >>  labeller.rule(Empire_token))   >   int_rules.expr [ _b = _1 ]
                >   labeller.rule(Meter_token)    >   tok.string [ _a = _1 ]
                >  -(labeller.rule(Low_token)     >   double_rules.expr [ _c = _1 ])
                >  -(labeller.rule(High_token)    >   double_rules.expr [ _d = _1 ])
            ) [ _val = new_<Condition::EmpireMeterValue>(_b, _a, _c, _d) ]
            ;

        empire_meter_value2
            =   (
                (tok.EmpireMeter_
                 >>  labeller.rule(Meter_token))    >   tok.string [ _a = _1 ]
                >  -(labeller.rule(Low_token)     >   double_rules.expr [ _c = _1 ])
                >  -(labeller.rule(High_token)    >   double_rules.expr [ _d = _1 ])
            ) [ _val = new_<Condition::EmpireMeterValue>(_a, _c, _d) ]
            ;

        empire_meter_value
            =   empire_meter_value1
            |   empire_meter_value2
            ;

        start
            %=   meter_value
            |    ship_part_meter_value
            |    empire_meter_value
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
