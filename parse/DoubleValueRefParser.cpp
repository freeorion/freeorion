#include "ValueRefParserImpl.h"

namespace {
    struct double_parser_rules {
        double_parser_rules() {
            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_val_type _val;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;
            using phoenix::static_cast_;

            const parse::lexer& tok = parse::lexer::instance();

            variable_name
                =    tok.Industry_
                |    tok.TargetIndustry_
                |    tok.Research_
                |    tok.TargetResearch_
                |    tok.Trade_
                |    tok.TargetTrade_
                |    tok.Construction_
                |    tok.TargetConstruction_
                |    tok.Population_
                |    tok.TargetPopulation_
                |    tok.TargetHappiness_
                |    tok.Happiness_
                |    tok.MaxFuel_
                |    tok.Fuel_
                |    tok.MaxShield_
                |    tok.Shield_
                |    tok.MaxDefense_
                |    tok.Defense_
                |    tok.MaxTroops_
                |    tok.Troops_
                |    tok.RebelTroops_
                |    tok.MaxStructure_
                |    tok.Structure_
                |    tok.Supply_
                |    tok.Stealth_
                |    tok.Detection_
                |    tok.BattleSpeed_
                |    tok.StarlaneSpeed_
                |    tok.TradeStockpile_
                //|    tok.DistanceToSource_    // Note: DistanceToSource will be a Source-variant property, but without an explicit Source reference, so will be treated as Source-invariant by ValueRef and parsing code. This is bad.
                |    tok.X_
                |    tok.Y_
                |    tok.SizeAsDouble_
                |    tok.NextTurnPopGrowth_
                |    tok.Size_
                |    tok.DistanceFromOriginalType_
                ;

            constant
                =    tok.int_ [ _val = new_<ValueRef::Constant<double> >(static_cast_<double>(_1)) ]
                |    tok.double_ [ _val = new_<ValueRef::Constant<double> >(_1) ]
                ;

            free_variable
                =
                    tok.Value_ [ _val = new_<ValueRef::Variable<double> >(ValueRef::EFFECT_TARGET_VALUE_REFERENCE, _a) ]
                |
                    (
                        tok.UniverseCentreX_
                    |   tok.UniverseCentreY_
                        // add more object-independent ValueRef int functions here
                    ) [ push_back(_a, construct<std::string>(phoenix::bind(&adobe::name_t::c_str, _1))),
                        _val = new_<ValueRef::Variable<double> >(ValueRef::NON_OBJECT_REFERENCE, _a) ]
                ;

            variable
                = (
                        variable_scope() [ _b = _1 ] > '.'  // determines reference type from explicit use of Source, Target, LocalCandiate, or RootCandidate in expression
                    >  -(container_type() [ push_back(_a, construct<std::string>(phoenix::bind(&adobe::name_t::c_str, _1))) ] > '.')
                    >   (
                            variable_name [ push_back(_a, construct<std::string>(phoenix::bind(&adobe::name_t::c_str, _1))),
                                            _val = new_<ValueRef::Variable<double> >(_b, _a) ]
                        |   int_var_variable_name() [ push_back(_a, construct<std::string>(phoenix::bind(&adobe::name_t::c_str, _1))),
                                                      _val = new_<ValueRef::StaticCast<int, double> >(new_<ValueRef::Variable<int> >(_b, _a)) ]
                        )
                  )
                | (
                        tok.CurrentTurn_
                        [ push_back(_a, construct<std::string>(phoenix::bind(&adobe::name_t::c_str, _1))),
                          _val = new_<ValueRef::StaticCast<int, double> >(new_<ValueRef::Variable<int> >(ValueRef::NON_OBJECT_REFERENCE, _a)) ]
                  )
                ;

            initialize_numeric_statistic_parser<double>(statistic, variable_name);
            initialize_expression_parsers<double>(function_expr,
                                                  exponential_expr,
                                                  multiplicative_expr,
                                                  additive_expr,
                                                  expr,
                                                  primary_expr);

            int_statistic
                =    int_var_statistic() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            primary_expr
                =   '(' > expr > ')'
                |   constant
                |   free_variable
                |   variable
                |   statistic
                |   int_statistic
                ;

            variable_name.name("real number variable name (e.g., Growth)");
            constant.name("real number constant");


            free_variable.name("free integer variable");
            variable.name("real number variable");
            statistic.name("real number statistic");
            int_statistic.name("integer statistic");
            function_expr.name("real number function expression");
            exponential_expr.name("real number exponential expression");
            multiplicative_expr.name("real number multiplication expression");
            additive_expr.name("real number additive expression");
            expr.name("real number expression");
            primary_expr.name("real number expression");

#if DEBUG_VALUEREF_PARSERS
            debug(variable_name);
            debug(constant);
            debug(free_variable);
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
        typedef expression_rule<double>::type               expression_rule;

        name_token_rule     variable_name;
        rule                constant;
        variable_rule       free_variable;
        variable_rule       variable;
        statistic_rule      statistic;
        rule                int_statistic;
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

const name_token_rule& double_var_variable_name()
{ return get_double_parser_rules().variable_name; }

const statistic_rule<double>::type& double_var_statistic()
{ return get_double_parser_rules().statistic; }

namespace parse {
    template <>
    value_ref_parser_rule<double>::type& value_ref_parser<double>()
    { return get_double_parser_rules().expr; }
}
