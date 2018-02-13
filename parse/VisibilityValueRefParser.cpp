#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse { namespace detail {
    visibility_complex_parser_grammar::visibility_complex_parser_grammar(
        const parse::lexer& tok, Labeller& labeller
    ) :
        visibility_complex_parser_grammar::base_type(start, "EmpireObjectVisibility"),
        simple_int_rules(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_val_type _val;
        qi::_pass_type _pass;
        const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
        const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

        const value_ref_rule<int>& simple_int = simple_int_rules.simple;

        empire_object_visibility
            =   tok.EmpireObjectVisibility_ [ _a = construct<std::string>(_1) ]
            >   labeller.rule(Empire_token) >   simple_int [ _b = _1 ]
            >   labeller.rule(Object_token) >   simple_int [ _c = _1 ]
            [ _val = construct_movable_(new_<ValueRef::ComplexVariable<Visibility>>(_a, deconstruct_movable_(_b, _pass), deconstruct_movable_(_c, _pass), deconstruct_movable_(_f, _pass), deconstruct_movable_(_d, _pass), deconstruct_movable_(_e, _pass))) ]
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
        Labeller& labeller,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("Visibility", tok, labeller, condition_parser),
        visibility_var_complex_grammar(tok, labeller)
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
