#include "ConditionParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
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

namespace {
    struct condition_parser_rules_4 {
        condition_parser_rules_4(const parse::lexer& tok, const parse::condition_parser_rule& condition_parser) :
            int_rules(tok, condition_parser),
            double_rules(tok, condition_parser)
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
                        parse::non_ship_part_meter_type_enum() [ _a = _1 ]
                        >  -(parse::detail::label(Low_token)  > double_rules.expr [ _b = _1 ])
                        >  -(parse::detail::label(High_token) > double_rules.expr [ _c = _1 ])
                    ) [ _val = new_<Condition::MeterValue>(_a, _b, _c) ]
                ;

            ship_part_meter_value
                =   (
                        tok.ShipPartMeter_
                        >   parse::detail::label(Part_token)    >   parse::string_value_ref() [ _e = _1 ]
                        >   parse::ship_part_meter_type_enum() [ _a = _1 ]
                        >  -(parse::detail::label(Low_token)    >   double_rules.expr [ _b = _1 ])
                        >  -(parse::detail::label(High_token)   >   double_rules.expr [ _c = _1 ])
                    ) [ _val = new_<Condition::ShipPartMeterValue>(_e, _a, _b, _c) ]
                ;

            empire_meter_value1
                =   (
                    (tok.EmpireMeter_
                    >>  parse::detail::label(Empire_token))   >   int_rules.expr [ _b = _1 ]
                    >   parse::detail::label(Meter_token)    >   tok.string [ _a = _1 ]
                    >  -(parse::detail::label(Low_token)     >   double_rules.expr [ _c = _1 ])
                    >  -(parse::detail::label(High_token)    >   double_rules.expr [ _d = _1 ])
                    ) [ _val = new_<Condition::EmpireMeterValue>(_b, _a, _c, _d) ]
                ;

            empire_meter_value2
                =   (
                    (tok.EmpireMeter_
                    >>  parse::detail::label(Meter_token))    >   tok.string [ _a = _1 ]
                    >  -(parse::detail::label(Low_token)     >   double_rules.expr [ _c = _1 ])
                    >  -(parse::detail::label(High_token)    >   double_rules.expr [ _d = _1 ])
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

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                MeterType,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                std::string,
                ValueRef::ValueRefBase<std::string>*
            >
        > meter_value_rule;

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >
        > empire_meter_value_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::double_parser_rules      double_rules;
        meter_value_rule                meter_value;
        meter_value_rule                ship_part_meter_value;
        empire_meter_value_rule         empire_meter_value;
        empire_meter_value_rule         empire_meter_value1;
        empire_meter_value_rule         empire_meter_value2;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_4() {
        static condition_parser_rules_4 retval(parse::lexer::instance(), parse::condition_parser());
        return retval.start;
    }

    condition_parser_rules_4::condition_parser_rules_4(const parse::lexer& tok,
        const condition_parser_grammar& condition_parser)
        :
        condition_parser_rules_4::base_type(start, "condition_parser_rules_4"),
        int_rules(tok, condition_parser),
        double_rules(tok, condition_parser)
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
                parse::non_ship_part_meter_type_enum() [ _a = _1 ]
                >  -(parse::detail::label(Low_token)  > double_rules.expr [ _b = _1 ])
                >  -(parse::detail::label(High_token) > double_rules.expr [ _c = _1 ])
            ) [ _val = new_<Condition::MeterValue>(_a, _b, _c) ]
            ;

        ship_part_meter_value
            =   (
                tok.ShipPartMeter_
                >   parse::detail::label(Part_token)    >   parse::string_value_ref() [ _e = _1 ]
                >   parse::ship_part_meter_type_enum() [ _a = _1 ]
                >  -(parse::detail::label(Low_token)    >   double_rules.expr [ _b = _1 ])
                >  -(parse::detail::label(High_token)   >   double_rules.expr [ _c = _1 ])
            ) [ _val = new_<Condition::ShipPartMeterValue>(_e, _a, _b, _c) ]
            ;

        empire_meter_value1
            =   (
                (tok.EmpireMeter_
                 >>  parse::detail::label(Empire_token))   >   int_rules.expr [ _b = _1 ]
                >   parse::detail::label(Meter_token)    >   tok.string [ _a = _1 ]
                >  -(parse::detail::label(Low_token)     >   double_rules.expr [ _c = _1 ])
                >  -(parse::detail::label(High_token)    >   double_rules.expr [ _d = _1 ])
            ) [ _val = new_<Condition::EmpireMeterValue>(_b, _a, _c, _d) ]
            ;

        empire_meter_value2
            =   (
                (tok.EmpireMeter_
                 >>  parse::detail::label(Meter_token))    >   tok.string [ _a = _1 ]
                >  -(parse::detail::label(Low_token)     >   double_rules.expr [ _c = _1 ])
                >  -(parse::detail::label(High_token)    >   double_rules.expr [ _d = _1 ])
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
