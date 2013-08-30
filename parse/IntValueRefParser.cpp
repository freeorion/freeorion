#include "ValueRefParserImpl.h"


name_token_rule variable_scope;
name_token_rule container_type;

namespace {
    struct int_parser_rules {
        int_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;
            using phoenix::static_cast_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_scope
                =   tok.Source_
                |   tok.Target_
                |   tok.LocalCandidate_
                |   tok.RootCandidate_
                ;

            container_type
                =   tok.Planet_
                |   tok.System_
                |   tok.Fleet_
                ;

            // TODO: Should we apply elements of this list only to certain
            // objects? For example, if one writes "Source.Planet.",
            // "NumShips" should not follow.
            variable_name
                =   tok.Owner_
                |   tok.ID_
                |   tok.CreationTurn_
                |   tok.Age_
                |   tok.ProducedByEmpireID_
                |   tok.DesignID_
                |   tok.FleetID_
                |   tok.PlanetID_
                |   tok.SystemID_
                |   tok.FinalDestinationID_
                |   tok.NextSystemID_
                |   tok.PreviousSystemID_
                |   tok.NumShips_
                |   tok.LastTurnBattleHere_
                |   tok.Orbit_
                |   tok.Species_
                ;

            constant
                =   tok.double_ [ _val = new_<ValueRef::Constant<int> >(static_cast_<int>(_1)) ]
                |   tok.int_ [ _val = new_<ValueRef::Constant<int> >(_1) ]
                ;

            free_variable
                =   (
                        tok.CurrentTurn_
                    |   tok.Value_
                        // add more object-independent ValueRef int functions here
                    ) [ push_back(_a, _1), _val = new_<ValueRef::Variable<int> >(_a) ]
                ;

            initialize_bound_variable_parser<int>(bound_variable, variable_name);
            initialize_numeric_statistic_parser<int>(statistic, variable_name);
            initialize_expression_parsers<int>(function_expr,
                                               exponential_expr,
                                               multiplicative_expr,
                                               additive_expr,
                                               expr,
                                               primary_expr);

            primary_expr
                =   '(' > expr > ')'
                |   constant
                |   free_variable
                |   bound_variable
                |   statistic
                ;

            variable_scope.name("Source, Target, LocalCandidate, or RootCandidate");
            container_type.name("Planet, System, or Fleet");
            variable_name.name("integer variable name (e.g., FleetID)");
            constant.name("integer constant");
            free_variable.name("free integer variable");
            bound_variable.name("bound integer variable");
            statistic.name("integer statistic");
            function_expr.name("integer function expression");
            exponential_expr.name("integer exponential expression");
            multiplicative_expr.name("integer multiplication expression");
            additive_expr.name("integer additive expression");
            expr.name("integer expression");
            primary_expr.name("integer expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_scope);
            debug(container_type);
            debug(variable_name);
            debug(constant);
            debug(free_variable);
            debug(bound_variable);
            debug(statistic);
            debug(negate_expr);
            debug(multiplicative_expr);
            debug(additive_expr);
            debug(expr);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<int>::type rule;
        typedef variable_rule<int>::type                variable_rule;
        typedef statistic_rule<int>::type               statistic_rule;
        typedef expression_rule<int>::type              expression_rule;

        name_token_rule     variable_name;
        rule                constant;
        variable_rule       free_variable;
        variable_rule       bound_variable;
        statistic_rule      statistic;
        expression_rule     function_expr;
        expression_rule     exponential_expr;
        expression_rule     multiplicative_expr;
        expression_rule     additive_expr;
        rule                expr;
        rule                primary_expr;
    };

    int_parser_rules& get_int_parser_rules() {
        static int_parser_rules retval;
        return retval;
    }
}

const name_token_rule& int_var_variable_name()
{ return get_int_parser_rules().variable_name; }

const statistic_rule<int>::type& int_var_statistic()
{ return get_int_parser_rules().statistic; }

namespace parse {
    template <>
    value_ref_parser_rule<int>::type& value_ref_parser<int>()
    { return get_int_parser_rules().expr; }
}
