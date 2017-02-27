#include "ValueRefParserImpl.h"

#include "EnumParser.h"

#include "../universe/Enums.h"


namespace {
    struct universe_object_type_parser_rules :
        public parse::detail::enum_value_ref_rules<UniverseObjectType>
    {
        universe_object_type_parser_rules() :
           enum_value_ref_rules("ObjectType")
        {
            boost::spirit::qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                %=   tok.ObjectType_
                ;

            enum_expr
                =   tok.Building_           [ _val = OBJ_BUILDING ]
                |   tok.Ship_               [ _val = OBJ_SHIP ]
                |   tok.Fleet_              [ _val = OBJ_FLEET ]
                |   tok.Planet_             [ _val = OBJ_PLANET ]
                |   tok.PopulationCenter_   [ _val = OBJ_POP_CENTER ]
                |   tok.ProductionCenter_   [ _val = OBJ_PROD_CENTER ]
                |   tok.System_             [ _val = OBJ_SYSTEM ]
                |   tok.Field_              [ _val = OBJ_FIELD ]
                ;
        }
    };
}


namespace parse { namespace detail {

enum_value_ref_rules<UniverseObjectType>& universe_object_type_rules()
{
    static universe_object_type_parser_rules retval;
    return retval;
}

} }
