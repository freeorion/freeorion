#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/System.h"

namespace parse { namespace detail {
    star_type_parser_rules::star_type_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
    ) :
        enum_value_ref_rules("StarType", tok, label, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

        variable_name
            %=   tok.StarType_
            |    tok.NextOlderStarType_
            |    tok.NextYoungerStarType_
            ;

        enum_expr
            =   tok.Blue_      [ _val = StarType::STAR_BLUE ]
            |   tok.White_     [ _val = StarType::STAR_WHITE ]
            |   tok.Yellow_    [ _val = StarType::STAR_YELLOW ]
            |   tok.Orange_    [ _val = StarType::STAR_ORANGE ]
            |   tok.Red_       [ _val = StarType::STAR_RED ]
            |   tok.Neutron_   [ _val = StarType::STAR_NEUTRON ]
            |   tok.BlackHole_ [ _val = StarType::STAR_BLACK ]
            |   tok.NoStar_    [ _val = StarType::STAR_NONE ]
            ;

        // complex_expr left empty, as no direct complex variable
        // expressions are available available that return a StarType
    }
} }
