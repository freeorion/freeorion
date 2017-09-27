#include "ValueRefParserImpl.h"


namespace {
    struct simple_int_parser_rules :
        public parse::detail::simple_variable_rules<int>
    {
        simple_int_parser_rules() :
            simple_variable_rules("integer")
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;

            qi::_1_type _1;
            qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();

            // TODO: Should we apply elements of this list only to certain
            // objects? For example, if one writes "Source.Planet.",
            // "NumShips" should not follow.
            bound_variable_name
                =   tok.Owner_
                |   tok.SupplyingEmpire_
                |   tok.ID_
                |   tok.CreationTurn_
                |   tok.Age_
                |   tok.ProducedByEmpireID_
                |   tok.ArrivedOnTurn_
                |   tok.DesignID_
                |   tok.FleetID_
                |   tok.PlanetID_
                |   tok.SystemID_
                |   tok.FinalDestinationID_
                |   tok.NextSystemID_
                |   tok.NearestSystemID_
                |   tok.PreviousSystemID_
                |   tok.NumShips_
                |   tok.NumStarlanes_
                |   tok.LastTurnBattleHere_
                |   tok.LastTurnActiveInBattle_
                |   tok.LastTurnResupplied_
                |   tok.Orbit_
                |   tok.SpeciesID_
                |   tok.TurnsSinceFocusChange_
                |   tok.ETA_
                ;

            free_variable_name
                =   tok.CurrentTurn_
                |   tok.GalaxyAge_
                |   tok.GalaxyMaxAIAggression_
                |   tok.GalaxyMonsterFrequency_
                |   tok.GalaxyNativeFrequency_
                |   tok.GalaxyPlanetDensity_
                |   tok.GalaxyShape_
                |   tok.GalaxySize_
                |   tok.GalaxySpecialFrequency_
                |   tok.GalaxyStarlaneFrequency_
                ;

            constant
                =   tok.int_    [ _val = new_<ValueRef::Constant<int>>(_1) ]
                ;
        }
    };


    simple_int_parser_rules& get_simple_int_parser_rules() {
        static simple_int_parser_rules retval;
        return retval;
    }


    struct int_arithmetic_rules : public arithmetic_rules<int> {
        int_arithmetic_rules() :
            arithmetic_rules("integer")
        {
            const parse::value_ref_rule<int>& simple = int_simple();

            statistic_value_ref_expr
                =   simple
                |   int_var_complex()
            ;

            primary_expr
                =   '(' >> expr >> ')'
                |   simple
                |   statistic_expr
                |   int_var_complex()
                ;
        }
    };

    int_arithmetic_rules& get_int_arithmetic_rules() {
        static int_arithmetic_rules retval;
        return retval;
    }

    struct castable_as_int_parser_rules {
        castable_as_int_parser_rules() {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;

            qi::_1_type _1;
            qi::_val_type _val;

            castable_expr
                = parse::double_value_ref() [ _val = new_<ValueRef::StaticCast<double, int>>(_1) ]
                ;

            flexible_int
                = parse::int_value_ref()
                | castable_expr
                ;

            castable_expr.name("castable as integer expression");
            flexible_int.name("integer or castable as integer");

#if DEBUG_VALUEREF_PARSERS
            debug(castable_expr);
#endif
        }

        parse::value_ref_rule<int> castable_expr;
        parse::value_ref_rule<int> flexible_int;
    };

    castable_as_int_parser_rules& get_castable_as_int_parser_rules() {
        static castable_as_int_parser_rules retval;
        return retval;
    }
}


const variable_rule<int>& int_bound_variable()
{ return get_simple_int_parser_rules().bound_variable; }

const variable_rule<int>& int_free_variable()
{ return get_simple_int_parser_rules().free_variable; }

const parse::value_ref_rule<int>& int_simple()
{ return get_simple_int_parser_rules().simple; }

const statistic_rule<int>& int_var_statistic()
{ return get_int_arithmetic_rules().statistic_expr; }


namespace parse {
    value_ref_rule<int>& int_value_ref()
    { return get_int_arithmetic_rules().expr; }

    value_ref_rule<int>& flexible_int_value_ref()
    { return get_castable_as_int_parser_rules().flexible_int; }
}
