#include "ValueRefParser.h"

#include "Parse.h"
#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

parse::detail::simple_int_parser_rules::simple_int_parser_rules(const parse::lexer& tok) :
    simple_variable_rules("integer", tok)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;

    using phoenix::new_;

    qi::_1_type _1;
    qi::_val_type _val;
    const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;

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
        |   tok.ContainerID_
        |   tok.FinalDestinationID_
        |   tok.NextSystemID_
        |   tok.NearestSystemID_
        |   tok.PreviousSystemID_
        |   tok.NumShips_
        |   tok.NumStarlanes_
        |   tok.LastTurnActiveInBattle_
        |   tok.LastTurnAttackedByShip_
        |   tok.LastTurnBattleHere_
        |   tok.LastTurnColonized_
        |   tok.LastTurnConquered_
        |   tok.LastTurnResupplied_
        |   tok.Orbit_
        |   tok.SpeciesID_
        |   tok.TurnsSinceFocusChange_
        |   tok.ETA_
        |   tok.LaunchedFrom_
        ;

    free_variable_name
        =   tok.CombatBout_
        |   tok.CurrentTurn_
        |   tok.GalaxyAge_
        |   tok.GalaxyMaxAIAggression_
        |   tok.GalaxyMonsterFrequency_
        |   tok.GalaxyNativeFrequency_
        |   tok.GalaxyPlanetDensity_
        |   tok.GalaxyShape_
        |   tok.GalaxySize_
        |   tok.GalaxySpecialFrequency_
        |   tok.GalaxyStarlaneFrequency_
        |   tok.UsedInDesignID_
        ;

    constant
        =   tok.int_ [ _val = construct_movable_(new_<ValueRef::Constant<int>>(_1)) ]
        ;
}

parse::castable_as_int_parser_rules::castable_as_int_parser_rules(
    const parse::lexer& tok,
    parse::detail::Labeller& label,
    const parse::detail::condition_parser_grammar& condition_parser,
    const parse::detail::value_ref_grammar<std::string>& string_grammar
) :
    int_rules(tok, label, condition_parser, string_grammar),
    double_rules(tok, label, condition_parser, string_grammar)
{
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;

    using phoenix::new_;

    qi::_1_type _1;
    qi::_val_type _val;
    qi::_pass_type _pass;
    const boost::phoenix::function<parse::detail::construct_movable> construct_movable_;
    const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

    castable_expr
        = double_rules.expr [ _val = construct_movable_(new_<ValueRef::StaticCast<double, int>>(deconstruct_movable_(_1, _pass))) ]
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

parse::int_arithmetic_rules::int_arithmetic_rules(
    const parse::lexer& tok,
    parse::detail::Labeller& label,
    const parse::detail::condition_parser_grammar& condition_parser,
    const parse::detail::value_ref_grammar<std::string>& string_grammar
) :
    arithmetic_rules("integer", tok, label, condition_parser),
    simple_int_rules(tok),
    int_complex_grammar(tok, label, *this, string_grammar)
{
    const parse::detail::value_ref_rule<int>& simple = simple_int_rules.simple;

    statistic_value_ref_expr
        =   simple
        |   int_complex_grammar
        ;

    primary_expr
        =   '(' >> expr >> ')'
        |   simple
        |   statistic_expr
        |   int_complex_grammar
        ;
}

namespace parse {
    bool int_free_variable(std::string& text) {
        const lexer tok;
        boost::spirit::qi::in_state_type in_state;
        parse::detail::simple_int_parser_rules simple_int_rules(tok);

        text_iterator first = text.begin();
        text_iterator last = text.end();
        token_iterator it = tok.begin(first, last);

        bool success = boost::spirit::qi::phrase_parse(
            it, tok.end(), simple_int_rules.free_variable_name, in_state("WS")[tok.self]);

        return success;
    }
}
