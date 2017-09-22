#include "ConditionParserImpl.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "../universe/Condition.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;


namespace {
    struct condition_parser_rules_2 {
        condition_parser_rules_2(const parse::lexer& tok, const parse::condition_parser_rule& condition_parser) :
            int_rules(tok, condition_parser),
            castable_int_rules(tok, condition_parser)
        {
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
                        >   parse::detail::label(Name_token) >  parse::string_value_ref() [ _e = _1 ]
                        > -(parse::detail::label(Low_token)  >  castable_int_rules.flexible_int [ _a = _1 ] )
                        > -(parse::detail::label(High_token) >  castable_int_rules.flexible_int [ _b = _1 ] )
                    ) [ _val = new_<Condition::HasSpecial>(_e, _a, _b) ]
                ;

            enqueued
                =   enqueued1
                |   enqueued3
                |   enqueued2 /* enqueued2 must come after enqueued3 or enqueued2 would always dominate because of its optional components*/
                |   enqueued4
                ;

            enqueued1
                =   (   (tok.Enqueued_
                        >>  parse::detail::label(Type_token)   >>   tok.Building_)
                        > -(parse::detail::label(Name_token)   >    parse::string_value_ref() [ _e = _1 ])
                        > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                        > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                        > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(BT_BUILDING, _e, _a, _b, _c) ]
                ;

            enqueued2
                =   (   (tok.Enqueued_
                        >>  parse::detail::label(Type_token)   >>   tok.Ship_)
                        > -(parse::detail::label(Design_token) >    int_rules.expr [ _d = _1 ])
                        > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                        > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                        > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(_d, _a, _b, _c) ]
                ;

            enqueued3
                =   (   (tok.Enqueued_
                        >>  parse::detail::label(Type_token)   >>   tok.Ship_
                        >>  parse::detail::label(Name_token)   ) >    parse::string_value_ref() [ _e = _1 ]
                        > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                        > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                        > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(BT_SHIP, _e, _a, _b, _c) ]
                ;

            enqueued4
                =   (   tok.Enqueued_
                        > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                        > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                        > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                    ) [ _val = new_<Condition::Enqueued>(INVALID_BUILD_TYPE, _e, _a, _b, _c) ]
                ;

            design_has_part
                =   (   tok.DesignHasPart_
                        > -(parse::detail::label(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                        > -(parse::detail::label(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                    )   >   parse::detail::label(Name_token)  > parse::string_value_ref()
                    [ _val = new_<Condition::DesignHasPart>(_1, _a, _b) ]
                ;

            design_has_part_class
                =   (   tok.DesignHasPartClass_
                        > -(parse::detail::label(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                        > -(parse::detail::label(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                    )   >   parse::detail::label(Class_token) > parse::ship_part_class_enum()
                    [ _val = new_<Condition::DesignHasPartClass>(_1, _a, _b) ]
                ;

            in_system
                =   (   tok.InSystem_
                    >  -(parse::detail::label(ID_token)  > int_rules.expr [ _a = _1 ])
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

        typedef parse::detail::rule<
            Condition::ConditionBase* (),
            qi::locals<
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*
            >
        > common_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::castable_as_int_parser_rules    castable_int_rules;
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
        static condition_parser_rules_2 retval(parse::lexer::instance(), parse::condition_parser());
        return retval.start;
    }

    condition_parser_rules_2::condition_parser_rules_2(
        const parse::lexer& tok,
        const condition_parser_grammar& condition_parser)
        :
        condition_parser_rules_2::base_type(start, "condition_parser_rules_2"),
        int_rules(tok, condition_parser),
        castable_int_rules(tok, condition_parser)
    {
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
                        >   parse::detail::label(Name_token) >  parse::string_value_ref() [ _e = _1 ]
                        > -(parse::detail::label(Low_token)  >  castable_int_rules.flexible_int [ _a = _1 ] )
                        > -(parse::detail::label(High_token) >  castable_int_rules.flexible_int [ _b = _1 ] )
                ) [ _val = new_<Condition::HasSpecial>(_e, _a, _b) ]
            ;

        enqueued
            =   enqueued1
            |   enqueued3
            |   enqueued2 /* enqueued2 must come after enqueued3 or enqueued2 would always dominate because of its optional components*/
            |   enqueued4
            ;

        enqueued1
            =   (   (tok.Enqueued_
                     >>  parse::detail::label(Type_token)   >>   tok.Building_)
                    > -(parse::detail::label(Name_token)   >    parse::string_value_ref() [ _e = _1 ])
                    > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(BT_BUILDING, _e, _a, _b, _c) ]
            ;

        enqueued2
            =   (   (tok.Enqueued_
                     >>  parse::detail::label(Type_token)   >>   tok.Ship_)
                    > -(parse::detail::label(Design_token) >    int_rules.expr [ _d = _1 ])
                    > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(_d, _a, _b, _c) ]
            ;

        enqueued3
            =   (   (tok.Enqueued_
                     >>  parse::detail::label(Type_token)   >>   tok.Ship_
                     >>  parse::detail::label(Name_token)   ) >    parse::string_value_ref() [ _e = _1 ]
                    > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(BT_SHIP, _e, _a, _b, _c) ]
            ;

        enqueued4
            =   (   tok.Enqueued_
                    > -(parse::detail::label(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(parse::detail::label(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(parse::detail::label(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(INVALID_BUILD_TYPE, _e, _a, _b, _c) ]
            ;

        design_has_part
            =   (   tok.DesignHasPart_
                    > -(parse::detail::label(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                    > -(parse::detail::label(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                )   >   parse::detail::label(Name_token)  > parse::string_value_ref()
            [ _val = new_<Condition::DesignHasPart>(_1, _a, _b) ]
            ;

        design_has_part_class
            =   (   tok.DesignHasPartClass_
                    > -(parse::detail::label(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                    > -(parse::detail::label(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                )   >   parse::detail::label(Class_token) > parse::ship_part_class_enum()
            [ _val = new_<Condition::DesignHasPartClass>(_1, _a, _b) ]
            ;

        in_system
            =   (   tok.InSystem_
                    >  -(parse::detail::label(ID_token)  > int_rules.expr [ _a = _1 ])
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

} }
