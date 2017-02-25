#include "ValueRefParserImpl.h"


namespace {
    struct simple_double_parser_rules {
        simple_double_parser_rules() {
            using phoenix::new_;
            using phoenix::static_cast_;

            qi::_1_type _1;
            qi::_val_type _val;

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
                |   tok.Attack_
                |   tok.PropagatedSupplyRange_
                ;

            free_variable_name
                =   tok.UniverseCentreX_
                |   tok.UniverseCentreY_
                |   tok.UniverseWidth_
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

            simple
                =   constant
                |   free_variable
                |   bound_variable
                ;

            initialize_bound_variable_parser<double>(bound_variable, bound_variable_name);

            bound_variable_name.name("real number bound variable name (e.g., Population)");
            free_variable_name.name("real number free variable name (e.g., UniverseCentreX)");
            constant.name("real number constant");
            free_variable.name("free real number variable");
            bound_variable.name("real number bound variable");
            simple.name("simple read number expression (constant, free or bound variable)");

#if DEBUG_VALUEREF_PARSERS
            debug(bound_variable_name);
            debug(free_variable_name);
            debug(constant);
            debug(free_variable);
            debug(bound_variable);
            debug(simple);
#endif
        }

        name_token_rule bound_variable_name;
        name_token_rule free_variable_name;
        parse::value_ref_rule<double> constant;
        variable_rule<double> free_variable;
        variable_rule<double> bound_variable;
        parse::value_ref_rule<double> simple;
    };


    simple_double_parser_rules& get_simple_double_parser_rules() {
        static simple_double_parser_rules retval;
        return retval;
    }


    struct double_parser_rules : public arithmetic_rules<double> {
        double_parser_rules() :
            arithmetic_rules("real number")
        {
            using phoenix::new_;

            qi::_1_type _1;
            qi::_val_type _val;

            const parse::value_ref_rule<double>& simple = double_simple();

            int_bound_variable_cast
                =   int_bound_variable() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            int_statistic_cast
                =   int_var_statistic() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            int_complex_variable_cast
                =   int_var_complex() [ _val = new_<ValueRef::StaticCast<int, double> >(_1) ]
                ;

            statistic_value_ref_expr
                = primary_expr.alias();

            primary_expr
                =   ('(' > expr > ')')
                |    simple
                |    int_bound_variable_cast
                |    statistic_expr
                |    int_statistic_cast
                |    double_var_complex()
                |    int_complex_variable_cast
                ;

            int_bound_variable_cast.name("integer bound variable");
            int_statistic_cast.name("integer statistic");
            int_complex_variable_cast.name("integer complex variable");

#if DEBUG_VALUEREF_PARSERS
            debug(int_statistic_cast);
            debug(int_complex_variable_cast);
            debug(double_complex_variable);
#endif
        }

        parse::value_ref_rule<double> int_bound_variable_cast;
        parse::value_ref_rule<double> int_statistic_cast;
        parse::value_ref_rule<double> int_complex_variable_cast;
    };

    double_parser_rules& get_double_parser_rules() {
        static double_parser_rules retval;
        return retval;
    }
}



const parse::value_ref_rule<double>& double_simple()
{ return get_simple_double_parser_rules().simple; }


namespace parse {
    value_ref_rule<double>& double_value_ref()
    { return get_double_parser_rules().expr; }
}
