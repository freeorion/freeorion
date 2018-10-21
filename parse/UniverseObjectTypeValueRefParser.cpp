#include "ValueRefParser.h"

#include "EnumParser.h"
#include "EnumValueRefRules.h"

#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

namespace parse { namespace detail {
    universe_object_type_parser_rules::universe_object_type_parser_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser
) :
        enum_value_ref_rules("ObjectType", tok, label, condition_parser)
    {
        boost::spirit::qi::_val_type _val;

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
            |   tok.Fighter_            [ _val = OBJ_FIGHTER ]
            ;
    }
}
}
