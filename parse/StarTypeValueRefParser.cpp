#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct star_type_parser_rules :
        public parse::detail::enum_value_ref_rules<StarType>
    {
        star_type_parser_rules() :
            enum_value_ref_rules("StarType")
        {
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.StarType_
                |    tok.NextOlderStarType_
                |    tok.NextYoungerStarType_
                ;

            enum_expr
                =   tok.Blue_           [ _val = STAR_BLUE ]
                |   tok.White_          [ _val = STAR_WHITE ]
                |   tok.Yellow_         [ _val = STAR_YELLOW ]
                |   tok.Orange_         [ _val = STAR_ORANGE ]
                |   tok.Red_            [ _val = STAR_RED ]
                |   tok.Neutron_        [ _val = STAR_NEUTRON ]
                |   tok.BlackHole_      [ _val = STAR_BLACK ]
                |   tok.NoStar_         [ _val = STAR_NONE ]
                ;

            // complex_expr left empty, as no direct complex variable
            // expressions are available available that return a StarType
        }
    };
}

namespace parse { namespace detail {

enum_value_ref_rules<StarType>& star_type_rules()
{
    static star_type_parser_rules retval;
    return retval;
}

} }
