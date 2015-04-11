#include "ConditionParserImpl.h"

#include "EnumParser.h"
#include "Label.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_2 {
        condition_parser_rules_2() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<int>::type& int_value_ref = parse::value_ref_parser<int>();
            const parse::value_ref_parser_rule<int>::type& flexible_int_ref = parse::value_ref_parser_flexible_int();
            const parse::value_ref_parser_rule<std::string>::type& string_value_ref = parse::value_ref_parser<std::string>();

            qi::_1_type _1;
            qi::_a_type _a; // intref
            qi::_b_type _b; // intref
            qi::_c_type _c; // intref
            qi::_d_type _d; // intref
            qi::_e_type _e; // string
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;

            has_special_since_turn
                =   (       tok.HasSpecialSinceTurn_
                        >   parse::label(Name_token) >  string_value_ref [ _e = _1 ]
                        > -(parse::label(Low_token)  >  flexible_int_ref [ _a = _1 ] )
                        > -(parse::label(High_token) >  flexible_int_ref [ _b = _1 ] )
                    ) [ _val = new_<Condition::HasSpecial>(_e, _a, _b) ]
                ;

            enqueued
                =   enqueued1
                |   enqueued3
                |   enqueued2 /* enqueued2 must come after enqueued3 or enqueued2 would always dominate because of its optional components*/
                |   enqueued4
                ;

            enqueued1
                =   (   tok.Enqueued_
                        >>  parse::label(Type_token)   >>   tok.Building_
                        > -(parse::label(Name_token)   >    string_value_ref[ _e = _1 ])
                        > -(parse::label(Empire_token) >    int_value_ref   [ _a = _1 ])
                        > -(parse::label(Low_token)    >    flexible_int_ref[ _b = _1 ])
                        > -(parse::label(High_token)   >    flexible_int_ref[ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(BT_BUILDING, _e, _a, _b, _c) ]
                ;

            enqueued2
                =   (   tok.Enqueued_
                        >>  parse::label(Type_token)   >>   tok.Ship_
                        > -(parse::label(Design_token) >    int_value_ref   [ _d = _1 ])
                        > -(parse::label(Empire_token) >    int_value_ref   [ _a = _1 ])
                        > -(parse::label(Low_token)    >    flexible_int_ref[ _b = _1 ])
                        > -(parse::label(High_token)   >    flexible_int_ref[ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(_d, _a, _b, _c) ]
                ;

            enqueued3
                =   (   tok.Enqueued_
                        >>  parse::label(Type_token)   >>   tok.Ship_
                        >>  parse::label(Name_token)   >    string_value_ref[ _e = _1 ]
                        > -(parse::label(Empire_token) >    int_value_ref   [ _a = _1 ])
                        > -(parse::label(Low_token)    >    flexible_int_ref[ _b = _1 ])
                        > -(parse::label(High_token)   >    flexible_int_ref[ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(BT_SHIP, _e, _a, _b, _c) ]
                ;

            enqueued4
                =   (   tok.Enqueued_
                        > -(parse::label(Empire_token) >    int_value_ref   [ _a = _1 ])
                        > -(parse::label(Low_token)    >    flexible_int_ref[ _b = _1 ])
                        > -(parse::label(High_token)   >    flexible_int_ref[ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(INVALID_BUILD_TYPE, _e, _a, _b, _c) ]
                ;

            design_has_part
                =    tok.DesignHasPart_
                >    parse::label(Low_token)   > flexible_int_ref [ _a = _1 ]
                >    parse::label(High_token)  > flexible_int_ref [ _b = _1 ]
                >    parse::label(Name_token)  > string_value_ref
                [ _val = new_<Condition::DesignHasPart>(_a, _b, _1) ]
                ;

            design_has_part_class
                =    tok.DesignHasPartClass_
                >    parse::label(Low_token)   > flexible_int_ref [ _a = _1 ]
                >    parse::label(High_token)  > flexible_int_ref [ _b = _1 ]
                >    parse::label(Class_token) > parse::enum_parser<ShipPartClass>()
                [ _val = new_<Condition::DesignHasPartClass>(_a, _b, _1) ]
                ;

            in_system
                =   (   tok.InSystem_
                    >  -(parse::label(ID_token)  > int_value_ref [ _a = _1 ])
                    )
                    [ _val = new_<Condition::InSystem>(_a) ]
                ;

            start
                %=   has_special_since_turn
                |    enqueued
                |    design_has_part
                |    design_has_part_class
                |    in_system
                ;

            has_special_since_turn.name("HasSpecialSinceTurn");
            enqueued.name("Enqueued");
            design_has_part.name("DesignHasPart");
            design_has_part_class.name("DesignHasPartClass");
            in_system.name("InSystem");

#if DEBUG_CONDITION_PARSERS
            debug(has_special_since_turn);
            debug(enqueued);
            debug(design_has_part);
            debug(design_has_part_class);
            debug(in_system);
#endif
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*
            >,
            parse::skipper_type
        > common_rule;

        common_rule                     has_special_since_turn;
        common_rule                     enqueued;
        common_rule                     enqueued1;
        common_rule                     enqueued2;
        common_rule                     enqueued3;
        common_rule                     enqueued4;
        common_rule                     design_has_part;
        common_rule                     design_has_part_class;
        common_rule                     in_system;
        parse::condition_parser_rule    start;
    };
}

namespace parse { namespace detail {
    const condition_parser_rule& condition_parser_2() {
        static condition_parser_rules_2 retval;
        return retval.start;
    }
} }
