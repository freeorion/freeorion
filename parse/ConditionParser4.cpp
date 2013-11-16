#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/home/phoenix.hpp>


namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


#if DEBUG_CONDITION_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<const Condition::ConditionBase*>&) { return os; }
}
#endif

namespace {
    struct condition_parser_rules_4 {
        condition_parser_rules_4() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref =
                parse::value_ref_parser<int>();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =
                parse::value_ref_parser<double>();

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;
            using phoenix::push_back;

            meter_value
                =    (
                            parse::non_ship_part_meter_type_enum() [ _a = _1 ]
                        >> -(
                                parse::label(Low_token) >> double_value_ref [ _b = _1 ]
                            )
                        >> -(
                                parse::label(High_token) >> double_value_ref [ _c = _1 ]
                            )
                     )
                     [ _val = new_<Condition::MeterValue>(_a, _b, _c) ]
                ;

            ship_part_meter_value
                =    (      tok.ShipPartMeter_
                        >>  parse::label(Part_token)      >>  tok.string [ _d = _1 ]
                        >>  parse::ship_part_meter_type_enum() [ _a = _1 ]
                        >> -(
                                parse::label(Low_token)  >>  double_value_ref [ _b = _1 ]
                            )
                        >> -(
                                parse::label(High_token) >>  double_value_ref [ _c = _1 ]
                            )
                     )
                     [ _val = new_<Condition::ShipPartMeterValue>(_d, _a, _b, _c) ]
                ;

            empire_meter_value
                =   tok.EmpireMeter_
                >>  (
                        parse::label(Empire_token)   >>  int_value_ref [ _b = _1 ]
                    >>  parse::label(Meter_token)    >>  tok.string [ _a = _1 ]
                    >> -(
                            parse::label(Low_token)  >>  double_value_ref [ _c = _1 ]
                        )
                    >> -(
                            parse::label(High_token) >>  double_value_ref [ _d = _1 ]
                        )
                        [ _val = new_<Condition::EmpireMeterValue>(_b, _a, _c, _d) ]
                    )
                |   (
                        parse::label(Meter_token)    >>  tok.string [ _a = _1 ]
                    >> -(
                            parse::label(Low_token)  >>  double_value_ref [ _c = _1 ]
                        )
                    >> -(
                            parse::label(High_token) >>  double_value_ref [ _d = _1 ]
                        )
                        [ _val = new_<Condition::EmpireMeterValue>(_a, _c, _d) ]
                    )
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                MeterType,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                std::string
            >,
            parse::skipper_type
        > meter_value_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
            >,
            parse::skipper_type
        > empire_meter_value_rule;

        meter_value_rule                meter_value;
        meter_value_rule                ship_part_meter_value;
        empire_meter_value_rule         empire_meter_value;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_4() {
        static condition_parser_rules_4 retval;
        return retval.start;
    }
} }
