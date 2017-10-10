#include "ValueRefParserImpl.h"


parse::detail::simple_int_parser_rules::simple_int_parser_rules(const parse::lexer& tok) :
    simple_variable_rules("integer")
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;

    using phoenix::new_;

    qi::_1_type _1;
    qi::_val_type _val;

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

parse::castable_as_int_parser_rules::castable_as_int_parser_rules(const parse::lexer& tok) :
    int_rules(tok),
    double_rules(tok)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;

    using phoenix::new_;

    qi::_1_type _1;
    qi::_val_type _val;

    castable_expr
        = double_rules.expr [ _val = new_<ValueRef::StaticCast<double, int>>(_1) ]
        ;

    flexible_int
        = int_rules.expr
        | castable_expr
        ;

    castable_expr.name("castable as integer expression");
    flexible_int.name("integer or castable as integer");

#if DEBUG_VALUEREF_PARSERS
    debug(castable_expr);
#endif
}

parse::int_arithmetic_rules::int_arithmetic_rules(const parse::lexer& tok) :
    arithmetic_rules("integer"),
    simple_int_rules(tok)
{
    const parse::value_ref_rule<int>& simple = simple_int_rules.simple;

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
