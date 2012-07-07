#include "ValueRefParserImpl.h"

#include "Double.h"

#include <GG/ReportParseError.h>


namespace {
    struct double_parser_rules {
        double_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_val_type _val;
            using phoenix::new_;
            using phoenix::push_back;
            using phoenix::static_cast_;

            const parse::lexer& tok = parse::lexer::instance();

            final_token
                %=   tok.Industry_
                |    tok.TargetIndustry_
                |    tok.Research_
                |    tok.TargetResearch_
                |    tok.Trade_
                |    tok.TargetTrade_
                |    tok.Construction_
                |    tok.TargetConstruction_
                |    tok.Population_
                |    tok.TargetPopulation_
                |    tok.MaxFuel_
                |    tok.Fuel_
                |    tok.MaxShield_
                |    tok.Shield_
                |    tok.MaxDefense_
                |    tok.Defense_
                |    tok.MaxTroops_
                |    tok.Troops_
                |    tok.MaxStructure_
                |    tok.Structure_
                |    tok.Supply_
                |    tok.Stealth_
                |    tok.Detection_
                |    tok.BattleSpeed_
                |    tok.StarlaneSpeed_
                |    tok.TradeStockpile_
                |    tok.DistanceToSource_
                |    tok.SizeAsDouble_
                ;

            constant
                =    parse::double_ [ _val = new_<ValueRef::Constant<double> >(_1) ]
                ;

            variable
                = (
                        first_token [ push_back(_a, _1) ] > '.'
                    >  -(container_token [ push_back(_a, _1) ] > '.')
                    >   (
                            final_token
                            [ push_back(_a, _1), _val = new_<ValueRef::Variable<double> >(_a) ]
                        |   int_var_final_token()
                            [ push_back(_a, _1), _val = new_<ValueRef::StaticCast<int, double> >(new_<ValueRef::Variable<int> >(_a)) ]
                        )
                  )
                | (
                        tok.CurrentTurn_ [ push_back(_a, _1), _val = new_<ValueRef::StaticCast<int, double> >(new_<ValueRef::Variable<int> >(_a)) ]
                    |  tok.Value_ [ push_back(_a, _1), _val = new_<ValueRef::Variable<double> >(_a) ]
                  )
                ;

            initialize_numeric_statistic_parser<double>(statistic, final_token);

            initialize_expression_parsers<double>(negate_expr,
                                                  multiplicative_expr,
                                                  additive_expr,
                                                  expr,
                                                  primary_expr);

            int_statistic
                =    int_var_statistic() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            primary_expr
                %=   '(' > expr > ')'
                |    constant
                |    variable
                |    int_statistic
                |    statistic
                ;

            final_token.name("real number variable name (e.g., Growth)");
            constant.name("real number");
            variable.name("real number variable");
            statistic.name("real number statistic");
            int_statistic.name("integer statistic");
            negate_expr.name("real number or real number expression");
            multiplicative_expr.name("real number or real number expression");
            additive_expr.name("real number or real number expression");
            expr.name("real number expression");
            primary_expr.name("real number expression");

#if DEBUG_VALUEREF_PARSERS
            debug(final_token);
            debug(constant);
            debug(variable);
            debug(statistic);
            debug(int_statistic);
            debug(negate_expr);
            debug(multiplicative_expr);
            debug(additive_expr);
            debug(expr);
            debug(primary_expr);
#endif
        }

        typedef parse::value_ref_parser_rule<double>::type  rule;
        typedef variable_rule<double>::type                 variable_rule;
        typedef statistic_rule<double>::type                statistic_rule;
        typedef multiplicative_expr_rule<double>::type      multiplicative_expression_rule;
        typedef additive_expr_rule<double>::type            additive_expression_rule;

        name_token_rule                 final_token;
        rule                            constant;
        variable_rule                   variable;
        statistic_rule                  statistic;
        rule                            int_statistic;
        rule                            negate_expr;
        multiplicative_expression_rule  multiplicative_expr;
        additive_expression_rule        additive_expr;
        rule                            expr;
        rule                            primary_expr;
    };

    double_parser_rules& get_double_parser_rules() {
        static double_parser_rules retval;
        return retval;
    }
}

const name_token_rule& double_var_final_token()
{ return get_double_parser_rules().final_token; }

const statistic_rule<double>::type& double_var_statistic()
{ return get_double_parser_rules().statistic; }

namespace parse {
    template <>
    value_ref_parser_rule<double>::type& value_ref_parser<double>()
    { return get_double_parser_rules().expr; }
}
