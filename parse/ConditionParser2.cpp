#include "ConditionParser2.h"

#include "../universe/Condition.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    condition_parser_rules_2::condition_parser_rules_2(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_2::base_type(start, "condition_parser_rules_2"),
        int_rules(tok, labeller, condition_parser, string_grammar),
        castable_int_rules(tok, labeller, condition_parser, string_grammar),
        ship_part_class_enum(tok)
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
                        >   labeller.rule(Name_token) >  string_grammar [ _e = _1 ]
                        > -(labeller.rule(Low_token)  >  castable_int_rules.flexible_int [ _a = _1 ] )
                        > -(labeller.rule(High_token) >  castable_int_rules.flexible_int [ _b = _1 ] )
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
                     >>  labeller.rule(Type_token)   >>   tok.Building_)
                    > -(labeller.rule(Name_token)   >    string_grammar [ _e = _1 ])
                    > -(labeller.rule(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(labeller.rule(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(labeller.rule(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(BT_BUILDING, _e, _a, _b, _c) ]
            ;

        enqueued2
            =   (   (tok.Enqueued_
                     >>  labeller.rule(Type_token)   >>   tok.Ship_)
                    > -(labeller.rule(Design_token) >    int_rules.expr [ _d = _1 ])
                    > -(labeller.rule(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(labeller.rule(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(labeller.rule(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(_d, _a, _b, _c) ]
            ;

        enqueued3
            =   (   (tok.Enqueued_
                     >>  labeller.rule(Type_token)   >>   tok.Ship_
                     >>  labeller.rule(Name_token)   ) >    string_grammar [ _e = _1 ]
                    > -(labeller.rule(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(labeller.rule(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(labeller.rule(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(BT_SHIP, _e, _a, _b, _c) ]
            ;

        enqueued4
            =   (   tok.Enqueued_
                    > -(labeller.rule(Empire_token) >    int_rules.expr [ _a = _1 ])
                    > -(labeller.rule(Low_token)    >    castable_int_rules.flexible_int [ _b = _1 ])
                    > -(labeller.rule(High_token)   >    castable_int_rules.flexible_int [ _c = _1 ])
                ) [ _val = new_<Condition::Enqueued>(INVALID_BUILD_TYPE, _e, _a, _b, _c) ]
            ;

        design_has_part
            =   (   tok.DesignHasPart_
                    > -(labeller.rule(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                    > -(labeller.rule(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                )   >   labeller.rule(Name_token)  > string_grammar
            [ _val = new_<Condition::DesignHasPart>(_1, _a, _b) ]
            ;

        design_has_part_class
            =   (   tok.DesignHasPartClass_
                    > -(labeller.rule(Low_token)   > castable_int_rules.flexible_int [ _a = _1 ])
                    > -(labeller.rule(High_token)  > castable_int_rules.flexible_int [ _b = _1 ])
                )   >   labeller.rule(Class_token) > ship_part_class_enum
            [ _val = new_<Condition::DesignHasPartClass>(_1, _a, _b) ]
            ;

        in_system
            =   (   tok.InSystem_
                    >  -(labeller.rule(ID_token)  > int_rules.expr [ _a = _1 ])
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
