#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct visibility_parser_rules :
        public parse::detail::enum_value_ref_rules<Visibility>
    {
        visibility_parser_rules() :
            enum_value_ref_rules("Visibility")
        {
            using boost::phoenix::construct;
            using boost::phoenix::new_;
            using boost::phoenix::push_back;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();
            const parse::value_ref_rule<int>& simple_int = int_simple();

            // variable_name left empty, as no direct object properties are
            // available that return a visibility

            enum_expr
                =   tok.Invisible_  [ _val = VIS_NO_VISIBILITY ]
                |   tok.Basic_      [ _val = VIS_BASIC_VISIBILITY ]
                |   tok.Partial_    [ _val = VIS_PARTIAL_VISIBILITY ]
                |   tok.Full_       [ _val = VIS_FULL_VISIBILITY ]
                ;

            complex_expr
                =   tok.EmpireObjectVisiblity_ [ _a = construct<std::string>(_1) ]
                >   parse::detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   parse::detail::label(Object_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<Visibility>>(_a, _b, _c, _f, _d, _e) ]
                ;
        }
    };
}

namespace parse { namespace detail {

enum_value_ref_rules<Visibility>& visibility_rules()
{
    static visibility_parser_rules retval;
    return retval;
}

} }
