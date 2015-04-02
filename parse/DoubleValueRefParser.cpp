#include "ValueRefParserImpl.h"

namespace {
    struct double_parser_rules {
        double_parser_rules() {
            qi::_1_type _1;
            qi::_val_type _val;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;
            using phoenix::static_cast_;

            const parse::lexer& tok = parse::lexer::instance();

            bound_variable_name
                =   tok.Industry_
                |   tok.TargetIndustry_
                |   tok.Research_
                |   tok.TargetResearch_
                |   tok.Trade_
                |   tok.TargetTrade_
                |   tok.Construction_
                |   tok.TargetConstruction_
                |   tok.Population_
                |   tok.TargetPopulation_
                |   tok.TargetHappiness_
                |   tok.Happiness_
                |   tok.MaxFuel_
                |   tok.Fuel_
                |   tok.MaxShield_
                |   tok.Shield_
                |   tok.MaxDefense_
                |   tok.Defense_
                |   tok.MaxTroops_
                |   tok.Troops_
                |   tok.RebelTroops_
                |   tok.MaxStructure_
                |   tok.Structure_
                |   tok.Supply_
                |   tok.Stealth_
                |   tok.Detection_
                |   tok.Speed_
                |   tok.TradeStockpile_
                |   tok.X_
                |   tok.Y_
                |   tok.SizeAsDouble_
                |   tok.NextTurnPopGrowth_
                |   tok.Size_
                |   tok.DistanceFromOriginalType_
                ;

            free_variable_name
                =   tok.UniverseCentreX_
                |   tok.UniverseCentreY_
                ;

            constant
                =   tok.int_ [ _val = new_<ValueRef::Constant<double> >(static_cast_<double>(_1)) ]
                |   tok.double_ [ _val = new_<ValueRef::Constant<double> >(_1) ]
                ;

            free_variable
                =   tok.Value_
                    [ _val = new_<ValueRef::Variable<double> >(ValueRef::EFFECT_TARGET_VALUE_REFERENCE) ]
                |   free_variable_name
                    [ _val = new_<ValueRef::Variable<double> >(ValueRef::NON_OBJECT_REFERENCE, _1) ]
                |   int_free_variable()
                    [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            initialize_bound_variable_parser<double>(bound_variable, bound_variable_name);

            statistic_sub_value_ref
                =   constant
                |   free_variable
                |   bound_variable
                |   int_bound_variable_cast
                |   double_var_complex()
                |   int_complex_variable_cast
            ;

            initialize_numeric_statistic_parser<double>(statistic, statistic_1, statistic_2,
                                                        statistic_sub_value_ref);

            initialize_expression_parsers<double>(function_expr,
                                                  exponential_expr,
                                                  multiplicative_expr,
                                                  additive_expr,
                                                  expr,
                                                  primary_expr);

            int_bound_variable_cast
                =   int_bound_variable() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            int_statistic_cast
                =    int_var_statistic() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            int_complex_variable_cast
                =    int_var_complex() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            primary_expr
                =   '(' > expr > ')'
                |   constant
                |   free_variable
                |   bound_variable
                |   int_bound_variable_cast
                |   statistic
                |   int_statistic_cast
                |   double_var_complex()
                |   int_complex_variable_cast
                ;

            bound_variable_name.name("real number bound variable name (e.g., Population)");
            free_variable_name.name("real number free variable name (e.g., UniverseCentreX)");
            constant.name("real number constant");
            free_variable.name("free real number variable");
            bound_variable.name("real number bound variable");
            statistic.name("real number statistic");
            int_statistic_cast.name("integer statistic");
            int_complex_variable_cast.name("integer complex variable");
            int_bound_variable_cast.name("integer bound variable");
            function_expr.name("real number function expression");
            exponential_expr.name("real number exponential expression");
            multiplicative_expr.name("real number multiplication expression");
            additive_expr.name("real number additive expression");
            expr.name("real number expression");
            primary_expr.name("real number expression");

#if DEBUG_VALUEREF_PARSERS
            debug(bound_variable_name);
            debug(free_variable_name);
            debug(constant);
            debug(free_variable);
            debug(bound_variable);
            debug(statistic);
            debug(int_statistic_cast);
            debug(int_complex_variable_cast);
            debug(int_complex_variable_cast);
            debug(double_complex_variable);
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
        typedef expression_rule<double>::type               expression_rule;

        name_token_rule     bound_variable_name;
        name_token_rule     free_variable_name;
        rule                constant;
        variable_rule       free_variable;
        variable_rule       bound_variable;
        statistic_rule      statistic_1;
        statistic_rule      statistic_2;
        statistic_rule      statistic;
        rule                statistic_sub_value_ref;
        rule                int_bound_variable_cast;
        rule                int_statistic_cast;
        rule                int_complex_variable_cast;
        expression_rule     function_expr;
        expression_rule     exponential_expr;
        expression_rule     multiplicative_expr;
        expression_rule     additive_expr;
        rule                expr;
        rule                primary_expr;
    };

    double_parser_rules& get_double_parser_rules() {
        static double_parser_rules retval;
        return retval;
    }
}

const double_rule& double_constant()
{ return get_double_parser_rules().constant; }

const name_token_rule& double_bound_variable_name()
{ return get_double_parser_rules().bound_variable_name; }

const variable_rule<double>::type& double_bound_variable()
{ return get_double_parser_rules().bound_variable; }

const name_token_rule& double_free_variable_name()
{ return get_double_parser_rules().free_variable_name; }

const variable_rule<double>::type& double_free_variable()
{ return get_double_parser_rules().free_variable; }

const statistic_rule<double>::type& double_var_statistic()
{ return get_double_parser_rules().statistic; }

namespace parse {
    template <>
    value_ref_parser_rule<double>::type& value_ref_parser<double>()
    { return get_double_parser_rules().expr; }
}
