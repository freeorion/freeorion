#include "ValueRefParserImpl.h"


namespace parse {
    struct string_complex_parser_rules {
        string_complex_parser_rules() {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
            using phoenix::new_;

            qi::_1_type _1;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;

            const parse::lexer& tok = parse::lexer::instance();
            const parse::value_ref_rule<int>& simple_int = int_simple();


            game_rule
                =   tok.GameRule_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Name_token) >     parse::string_value_ref() [ _d = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            lowest_cost_enqueued_tech
                =   tok.LowestCostEnqueuedTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            highest_cost_enqueued_tech
                = tok.HighestCostEnqueuedTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            top_priority_enqueued_tech
                = tok.TopPriorityEnqueuedTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            most_spent_enqueued_tech
                = tok.MostSpentEnqueuedTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            random_enqueued_tech
                = tok.RandomEnqueuedTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            lowest_cost_researchable_tech
                = tok.LowestCostResearchableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            highest_cost_researchable_tech
                = tok.HighestCostResearchableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            top_priority_researchable_tech
                = tok.TopPriorityResearchableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            most_spent_researchable_tech
                = tok.MostSpentResearchableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            random_researchable_tech
                = tok.RandomResearchableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            random_complete_tech
                =   tok.RandomCompleteTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            lowest_cost_transferrable_tech
                =   tok.LowestCostTransferrableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   detail::label(Empire_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            highest_cost_transferrable_tech
                =   tok.HighestCostTransferrableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   detail::label(Empire_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            top_priority_transferrable_tech
                =   tok.TopPriorityTransferrableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   detail::label(Empire_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            most_spent_transferrable_tech
                =   tok.MostSpentTransferrableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   detail::label(Empire_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            random_transferrable_tech
                =   tok.RandomTransferrableTech_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                >   detail::label(Empire_token)  > simple_int [ _c = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            most_populous_species
                =   tok.MostPopulousSpecies_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            most_happy_species
                =   tok.MostHappySpecies_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            least_happy_species
                =   tok.LeastHappySpecies_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            ramdom_colonizable_species
                =   tok.RandomColonizableSpecies_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            random_controlled_species
                =   tok.RandomControlledSpecies_ [ _a = construct<std::string>(_1) ]
                >   detail::label(Empire_token)  > simple_int [ _b = _1 ]
                    [ _val = new_<ValueRef::ComplexVariable<std::string>>(_a, _b, _c, _f, _d, _e) ]
                ;

            start
                =   game_rule

                |   lowest_cost_enqueued_tech
                |   highest_cost_enqueued_tech
                |   top_priority_enqueued_tech
                |   most_spent_enqueued_tech
                |   random_enqueued_tech

                |   lowest_cost_researchable_tech
                |   highest_cost_researchable_tech
                |   top_priority_researchable_tech
                |   most_spent_researchable_tech
                |   random_researchable_tech

                |   random_complete_tech

                |   lowest_cost_transferrable_tech
                |   highest_cost_transferrable_tech
                |   top_priority_transferrable_tech
                |   most_spent_transferrable_tech
                |   random_transferrable_tech

                |   most_populous_species
                |   most_happy_species
                |   least_happy_species
                |   ramdom_colonizable_species
                |   random_controlled_species
                ;

            game_rule.name("GameRule");

            lowest_cost_enqueued_tech.name("LowestCostEnqueuedTech");
            highest_cost_enqueued_tech.name("HighestCostEnqueuedTech");
            top_priority_enqueued_tech.name("TopPriorityEnqueuedTech");
            most_spent_enqueued_tech.name("MostSpentEnqueuedTech");
            random_enqueued_tech.name("RandomEnqueuedTech");

            lowest_cost_researchable_tech.name("LowestCostResearchableTech");
            highest_cost_researchable_tech.name("HighestCostesearchableTech");
            top_priority_researchable_tech.name("TopPriorityResearchableTech");
            most_spent_researchable_tech.name("MostSpentResearchableTech");
            random_researchable_tech.name("RandomResearchableTech");

            random_complete_tech.name("RandomCompleteTech");

            lowest_cost_transferrable_tech.name("LowestCostTransferrableTech");
            highest_cost_transferrable_tech.name("HighestCostTransferrableTech");
            top_priority_transferrable_tech.name("TopPriorityTransferrableTech");
            most_spent_transferrable_tech.name("MostSpentTransferrableTech");
            random_transferrable_tech.name("RandomTransferrableTech");

            most_populous_species.name("MostPopulousSpecies");
            most_happy_species.name("MostHappySpecies");
            least_happy_species.name("LeastHappySpecies");
            ramdom_colonizable_species.name("RandomColonizableSpecies");
            random_controlled_species.name("RandomControlledSpecies");

#if DEBUG_DOUBLE_COMPLEX_PARSERS
            debug(game_rule);

            debug(lowest_cost_enqueued_tech);
            debug(highest_cost_enqueued_tech);
            debug(top_priority_enqueued_tech);
            debug(most_spent_enqueued_tech);
            debug(random_enqueued_tech);

            debug(lowest_cost_researchable_tech);
            debug(highest_cost_researchable_tech);
            debug(top_priority_researchable_tech);
            debug(most_spent_researchable_tech);
            debug(random_researchable_tech);

            debug(random_complete_tech);

            debug(lowest_cost_transferrable_tech);
            debug(highest_cost_transferrable_tech);
            debug(top_priority_transferrable_tech);
            debug(most_spent_transferrable_tech);
            debug(random_transferrable_tech);

            debug(most_populous_species);
            debug(most_happy_species);
            debug(least_happy_species);
            debug(ramdom_colonizable_species);
            debug(random_controlled_species);
#endif
        }

        complex_variable_rule<std::string> game_rule;

        complex_variable_rule<std::string> lowest_cost_enqueued_tech;
        complex_variable_rule<std::string> highest_cost_enqueued_tech;
        complex_variable_rule<std::string> top_priority_enqueued_tech;
        complex_variable_rule<std::string> most_spent_enqueued_tech;
        complex_variable_rule<std::string> random_enqueued_tech;

        complex_variable_rule<std::string> lowest_cost_researchable_tech;
        complex_variable_rule<std::string> highest_cost_researchable_tech;
        complex_variable_rule<std::string> top_priority_researchable_tech;
        complex_variable_rule<std::string> most_spent_researchable_tech;
        complex_variable_rule<std::string> random_researchable_tech;

        complex_variable_rule<std::string> random_complete_tech;

        complex_variable_rule<std::string> lowest_cost_transferrable_tech;
        complex_variable_rule<std::string> highest_cost_transferrable_tech;
        complex_variable_rule<std::string> top_priority_transferrable_tech;
        complex_variable_rule<std::string> most_spent_transferrable_tech;
        complex_variable_rule<std::string> random_transferrable_tech;

        complex_variable_rule<std::string> most_populous_species;
        complex_variable_rule<std::string> most_happy_species;
        complex_variable_rule<std::string> least_happy_species;
        complex_variable_rule<std::string> ramdom_colonizable_species;
        complex_variable_rule<std::string> random_controlled_species;

        complex_variable_rule<std::string> start;
    };

    namespace detail {
        string_complex_parser_rules string_complex_parser;
    }
}

const complex_variable_rule<std::string>& string_var_complex()
{ return parse::detail::string_complex_parser.start; }
