#include "ConditionParser2.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../universe/Enums.h"

#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    condition_parser_rules_2::condition_parser_rules_2(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        condition_parser_rules_2::base_type(start, "condition_parser_rules_2"),
        int_rules(tok, label, condition_parser, string_grammar),
        castable_int_rules(tok, label, condition_parser, string_grammar),
        ship_part_class_enum(tok)
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

        has_special_since_turn
            =   (       omit_[tok.HasSpecialSinceTurn_]
                        >   label(tok.Name_) >  string_grammar
                        > -(label(tok.Low_)  >  castable_int_rules.flexible_int )
                        > -(label(tok.High_) >  castable_int_rules.flexible_int )
                ) [ _val = construct_movable_(new_<Condition::HasSpecial>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        enqueued
            %=  enqueued1
            |   enqueued3
            |   enqueued2 /* enqueued2 must come after enqueued3 or enqueued2 would always dominate because of its optional components*/
            |   enqueued4
            ;

        enqueued1
            =   (   (omit_[tok.Enqueued_]
                    >>  label(tok.Type_)   >>   omit_[tok.Building_])
                    > -(label(tok.Name_)   >    string_grammar)
                    > -(label(tok.Empire_) >    int_rules.expr)
                    > -(label(tok.Low_)    >    castable_int_rules.flexible_int)
                    > -(label(tok.High_)   >    castable_int_rules.flexible_int)
                ) [ _val = construct_movable_(new_<Condition::Enqueued>(
                    BT_BUILDING,
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass))) ]
            ;

        enqueued2
            =   (   (omit_[tok.Enqueued_]
                    >>  label(tok.Type_)   >>   omit_[tok.Ship_])
                    > -(label(tok.Design_) >    int_rules.expr)
                    > -(label(tok.Empire_) >    int_rules.expr)
                    > -(label(tok.Low_)    >    castable_int_rules.flexible_int)
                    > -(label(tok.High_)   >    castable_int_rules.flexible_int)
                ) [ _val = construct_movable_(new_<Condition::Enqueued>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass))) ]
            ;

        enqueued3
            =   (   (omit_[tok.Enqueued_]
                    >>  label(tok.Type_)   >>   omit_[tok.Ship_]
                    >>  label(tok.Name_) ) >    string_grammar
                    > -(label(tok.Empire_) >    int_rules.expr)
                    > -(label(tok.Low_)    >    castable_int_rules.flexible_int)
                    > -(label(tok.High_)   >    castable_int_rules.flexible_int)
                ) [ _val = construct_movable_(new_<Condition::Enqueued>(
                    BT_SHIP,
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_4, _pass))) ]
            ;

        enqueued4
            =   (   omit_[tok.Enqueued_]
                    > -(label(tok.Empire_) >    int_rules.expr)
                    > -(label(tok.Low_)    >    castable_int_rules.flexible_int)
                    > -(label(tok.High_)   >    castable_int_rules.flexible_int)
                ) [ _val = construct_movable_(new_<Condition::Enqueued>(
                    INVALID_BUILD_TYPE,
                    nullptr,
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    deconstruct_movable_(_3, _pass))) ]
            ;

        design_has_part
            =   (   omit_[tok.DesignHasPart_]
                    > -(label(tok.Low_)   > castable_int_rules.flexible_int)
                    > -(label(tok.High_)  > castable_int_rules.flexible_int)
                    >   label(tok.Name_)  > string_grammar
                ) [ _val = construct_movable_(new_<Condition::DesignHasPart>(
                    deconstruct_movable_(_3, _pass),
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        design_has_part_class
            =   (   omit_[tok.DesignHasPartClass_]
                    > -(label(tok.Low_)   > castable_int_rules.flexible_int)
                    > -(label(tok.High_)  > castable_int_rules.flexible_int)
                    >   label(tok.Class_) > ship_part_class_enum
                ) [ _val = construct_movable_(new_<Condition::DesignHasPartClass>(
                    _3,
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass))) ]
            ;

        in_system
            =   (   omit_[tok.InSystem_]
                    >  -(label(tok.ID_)  > int_rules.expr)
                ) [ _val = construct_movable_(new_<Condition::InSystem>(deconstruct_movable_(_1, _pass))) ]
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
