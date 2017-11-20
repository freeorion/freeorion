#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse { namespace detail {
    visibility_complex_parser_grammar::visibility_complex_parser_grammar(
        const parse::lexer& tok, Labeller& label
    ) :
        visibility_complex_parser_grammar::base_type(start, "EmpireObjectVisibility"),
        simple_int_rules(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::_pass_type _pass;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
        const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

        const value_ref_rule<int>& simple_int = simple_int_rules.simple;

        empire_object_visibility
            = ( tok.EmpireObjectVisibility_
                >   label(tok.Empire_) >   simple_int
                >   label(tok.Object_) >   simple_int
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<Visibility>>(_1, deconstruct_movable_(_2, _pass), deconstruct_movable_(_3, _pass), nullptr, nullptr, nullptr)) ]
            ;

        start
            =   empire_object_visibility
            ;

        empire_object_visibility.name("EmpireObjectVisibility");

#if DEBUG_DOUBLE_COMPLEX_PARSERS
        debug(empire_object_visibility);

#endif
    }

    visibility_parser_rules::visibility_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("Visibility", tok, label, condition_parser),
        visibility_var_complex_grammar(tok, label)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::new_;

        qi::_val_type _val;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
        const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

        // variable_name left empty, as no direct object properties are
        // available that return a visibility

        enum_expr
            =   tok.Invisible_  [ _val = VIS_NO_VISIBILITY ]
            |   tok.Basic_      [ _val = VIS_BASIC_VISIBILITY ]
            |   tok.Partial_    [ _val = VIS_PARTIAL_VISIBILITY ]
            |   tok.Full_       [ _val = VIS_FULL_VISIBILITY ]
            ;

        free_variable_expr
        =   tok.Value_      [ _val = construct_movable_(new_<ValueRef::Variable<Visibility>>(ValueRef::EFFECT_TARGET_VALUE_REFERENCE)) ]
            ;

        complex_expr = visibility_var_complex_grammar;
    }

    }
}
