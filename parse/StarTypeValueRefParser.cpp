#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct star_type_parser_rules : public enum_value_ref_rules<StarType> {
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
        }
    };

    star_type_parser_rules& get_star_type_parser_rules()
    {
        static star_type_parser_rules retval;
        return retval;
    }
}

namespace parse {
    template <>
    enum_rule<StarType>& enum_expr<StarType>()
    {
        return get_star_type_parser_rules().enum_expr;
    }

    value_ref_rule<StarType>& star_type_value_ref()
    {
        return get_star_type_parser_rules().expr;
    }
}
