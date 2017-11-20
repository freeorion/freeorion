#include "ValueRefParser.h"

#include "MovableEnvelope.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>

namespace parse { namespace detail {
    string_complex_parser_grammar::string_complex_parser_grammar(
        const parse::lexer& tok,
        Labeller& label,
        const value_ref_grammar<std::string>& string_grammar
    ) :
        string_complex_parser_grammar::base_type(start, "string_complex_parser_grammar"),
        simple_int_rules(tok)
    {
        namespace phoenix = boost::phoenix;
        namespace qi = boost::spirit::qi;

        using phoenix::construct;
        using phoenix::new_;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_val_type _val;
        qi::_pass_type _pass;
        const boost::phoenix::function<detail::construct_movable> construct_movable_;
        const boost::phoenix::function<detail::deconstruct_movable> deconstruct_movable_;

        const value_ref_rule<int>& simple_int = simple_int_rules.simple;

        game_rule
            = ( tok.GameRule_
                >   label(tok.Name_) >     string_grammar
              ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_1, nullptr, nullptr, nullptr, deconstruct_movable_(_2, _pass), nullptr)) ]
            ;

        empire_ref =
            (
                (       tok.LowestCostEnqueuedTech_
                    |   tok.HighestCostEnqueuedTech_
                    |   tok.TopPriorityEnqueuedTech_
                    |   tok.MostSpentEnqueuedTech_
                    |   tok.RandomEnqueuedTech_
                    |   tok.LowestCostResearchableTech_
                    |   tok.HighestCostResearchableTech_
                    |   tok.TopPriorityResearchableTech_
                    |   tok.MostSpentResearchableTech_
                    |   tok.RandomResearchableTech_
                    |   tok.RandomCompleteTech_
                    |   tok.MostPopulousSpecies_
                    |   tok.MostHappySpecies_
                    |   tok.LeastHappySpecies_
                    |   tok.RandomColonizableSpecies_
                    |   tok.RandomControlledSpecies_
                )
                >   label(tok.Empire_) > simple_int
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_1, deconstruct_movable_(_2, _pass), nullptr, nullptr, nullptr, nullptr)) ]
            ;

        empire_empire_ref =
            (
                (       tok.LowestCostTransferrableTech_
                    |   tok.HighestCostTransferrableTech_
                    |   tok.TopPriorityTransferrableTech_
                    |   tok.MostSpentTransferrableTech_
                    |   tok.RandomTransferrableTech_
                )
                >   label(tok.Empire_) > simple_int
                >   label(tok.Empire_) > simple_int
            ) [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_1, deconstruct_movable_(_2, _pass), deconstruct_movable_(_3, _pass), nullptr, nullptr, nullptr)) ]
            ;

        start
            %=   game_rule
            |   empire_ref
            |   empire_empire_ref
            ;

        game_rule.name("GameRule");

        empire_ref.name("LowestCostEnqueuedTech, HighestCostEnqueuedTech, TopPriorityEnqueuedTech, "
                        "MostSpentEnqueuedTech, RandomEnqueuedTech, LowestCostResearchableTech, "
                        "HighestCostesearchableTech, TopPriorityResearchableTech, MostSpentResearchableTech, "
                        "RandomResearchableTech, MostPopulousSpecies, MostHappySpecies, "
                        "LeastHappySpecies, RandomColonizableSpecies, RandomControlledSpecies");
        empire_empire_ref.name("LowestCostTransferrableTech, HighestCostTransferrableTech, "
                               "TopPriorityTransferrableTech, MostSpentTransferrableTech, "
                               "RandomTransferrableTech");

#if DEBUG_DOUBLE_COMPLEX_PARSERS
        debug(game_rule);

        debug(empire_ref);
        debug(empire_empire_ref);
#endif
    }

    }}
