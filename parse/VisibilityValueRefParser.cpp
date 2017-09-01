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
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            enum_expr
                =   tok.Invisible_  [ _val = VIS_NO_VISIBILITY ]
                |   tok.Basic_      [ _val = VIS_BASIC_VISIBILITY ]
                |   tok.Partial_    [ _val = VIS_PARTIAL_VISIBILITY ]
                |   tok.Full_       [ _val = VIS_FULL_VISIBILITY ]
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
