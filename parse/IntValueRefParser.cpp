#include "ValueRefParserImpl.h"

#include <GG/ReportParseError.h>


name_token_rule first_token;
name_token_rule container_token;

namespace {

    struct int_parser_rules
    {
        int_parser_rules()
            {
                qi::_1_type _1;
                qi::_2_type _2;
                qi::_3_type _3;
                qi::_4_type _4;
                qi::_a_type _a;
                qi::_val_type _val;
                qi::lit_type lit;
                using phoenix::new_;
                using phoenix::push_back;
                using phoenix::static_cast_;

                const parse::lexer& tok = parse::lexer::instance();

                first_token
                    %=   tok.Source_
                    |    tok.Target_
                    |    tok.LocalCandidate_
                    |    tok.RootCandidate_
                    ;

                container_token
                    %=   tok.Planet_
                    |    tok.System_
                    |    tok.Fleet_
                    ;

                // TODO: Should we apply elements of this list only to certain
                // containers?  For example, if one writes "Source.Planet.",
                // "NumShips" should not follow.
                final_token
                    %=   tok.Owner_
                    |    tok.ID_
                    |    tok.CreationTurn_
                    |    tok.Age_
                    |    tok.ProducedByEmpireID_
                    |    tok.DesignID_
                    |    tok.FleetID_
                    |    tok.PlanetID_
                    |    tok.SystemID_
                    |    tok.FinalDestinationID_
                    |    tok.NextSystemID_
                    |    tok.PreviousSystemID_
                    |    tok.NumShips_
                    |    tok.LastTurnBattleHere_
                    ;

                constant
                    =    tok.double_ [ _val = new_<ValueRef::Constant<int> >(static_cast_<int>(_1)) ]
                    |    tok.int_ [ _val = new_<ValueRef::Constant<int> >(_1) ]
                    ;

                variable
                    = (
                           (
                                first_token [ push_back(_a, _1) ] > '.'
                            >  -(container_token [ push_back(_a, _1) ] > '.')
                            >   final_token [ push_back(_a, _1) ]
                           )
                       |   (
                               tok.CurrentTurn_
                            |  tok.Value_
                           )
                           [ push_back(_a, _1) ]
                      )
                      [ _val = new_<ValueRef::Variable<int> >(_a) ]
                    ;

                initialize_numeric_statistic_parser<int>(statistic, final_token);

                initialize_expression_parsers<int>(negate_expr,
                                                   multiplicative_expr,
                                                   additive_expr,
                                                   expr,
                                                   primary_expr);

                primary_expr
                    %=   '(' > expr > ')'
                    |    constant
                    |    variable
                    |    statistic
                    ;

                first_token.name("Source, Target, LocalCandidate, or RootCandidate");
                container_token.name("Planet, System, or Fleet");
                final_token.name("integer variable name (e.g., FleetID)");
                constant.name("integer");
                variable.name("integer variable");
                statistic.name("integer statistic");
                negate_expr.name("integer or integer expression");
                multiplicative_expr.name("integer or integer expression");
                additive_expr.name("integer or integer expression");
                expr.name("integer expression");
                primary_expr.name("integer expression");

#if DEBUG_VALUEREF_PARSERS
                debug(first_token);
                debug(container_token);
                debug(final_token);
                debug(constant);
                debug(variable);
                debug(statistic);
                debug(negate_expr);
                debug(multiplicative_expr);
                debug(additive_expr);
                debug(expr);
                debug(primary_expr);
#endif
            }

        typedef parse::value_ref_parser_rule<int>::type rule;
        typedef variable_rule<int>::type variable_rule;
        typedef statistic_rule<int>::type statistic_rule;
        typedef multiplicative_expr_rule<int>::type multiplicative_expression_rule;
        typedef additive_expr_rule<int>::type additive_expression_rule;

        name_token_rule final_token;
        rule constant;
        variable_rule variable;
        statistic_rule statistic;
        rule negate_expr;
        multiplicative_expression_rule multiplicative_expr;
        additive_expression_rule additive_expr;
        rule expr;
        rule primary_expr;
    };

    int_parser_rules& get_int_parser_rules()
    {
        static int_parser_rules retval;
        return retval;
    }

}

const name_token_rule& int_var_final_token()
{ return get_int_parser_rules().final_token; }

const statistic_rule<int>::type& int_var_statistic()
{ return get_int_parser_rules().statistic; }

namespace parse {

    template <>
    value_ref_parser_rule<int>::type& value_ref_parser<int>()
    { return get_int_parser_rules().expr; }

}
