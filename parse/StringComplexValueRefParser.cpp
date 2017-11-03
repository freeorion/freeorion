#include "ValueRefParserImpl.h"


namespace parse { namespace detail {
    string_complex_parser_grammar::string_complex_parser_grammar(
        const parse::lexer& tok,
        Labeller& labeller,
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
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_val_type _val;
            qi::_pass_type _pass;
            const boost::phoenix::function<detail::construct_movable> construct_movable_;
            const boost::phoenix::function<detail::deconstruct_movable> deconstruct_movable_;

            const value_ref_rule<int>& simple_int = simple_int_rules.simple;

            game_rule
                =   tok.GameRule_ [ _a = construct<std::string>(_1) ]
                >   labeller.rule(Name_token) >     string_grammar [ _d = _1 ]
                    [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_a, deconstruct_movable_(_b, _pass), deconstruct_movable_(_c, _pass), deconstruct_movable_(_f, _pass), deconstruct_movable_(_d, _pass), deconstruct_movable_(_e, _pass))) ]
                ;

            empire_ref
                =   (
                        (   tok.LowestCostEnqueuedTech_         [ _a = construct<std::string>(_1) ]
                        |   tok.HighestCostEnqueuedTech_        [ _a = construct<std::string>(_1) ]
                        |   tok.TopPriorityEnqueuedTech_        [ _a = construct<std::string>(_1) ]
                        |   tok.MostSpentEnqueuedTech_          [ _a = construct<std::string>(_1) ]
                        |   tok.RandomEnqueuedTech_             [ _a = construct<std::string>(_1) ]
                        |   tok.LowestCostResearchableTech_     [ _a = construct<std::string>(_1) ]
                        |   tok.HighestCostResearchableTech_    [ _a = construct<std::string>(_1) ]
                        |   tok.TopPriorityResearchableTech_    [ _a = construct<std::string>(_1) ]
                        |   tok.MostSpentResearchableTech_      [ _a = construct<std::string>(_1) ]
                        |   tok.RandomResearchableTech_         [ _a = construct<std::string>(_1) ]
                        |   tok.RandomCompleteTech_             [ _a = construct<std::string>(_1) ]
                        |   tok.MostPopulousSpecies_            [ _a = construct<std::string>(_1) ]
                        |   tok.MostHappySpecies_               [ _a = construct<std::string>(_1) ]
                        |   tok.LeastHappySpecies_              [ _a = construct<std::string>(_1) ]
                        |   tok.RandomColonizableSpecies_       [ _a = construct<std::string>(_1) ]
                        |   tok.RandomControlledSpecies_        [ _a = construct<std::string>(_1) ]
                        )
                    )
                >   labeller.rule(Empire_token) > simple_int [ _b = _1 ]
                    [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_a, deconstruct_movable_(_b, _pass), deconstruct_movable_(_c, _pass), deconstruct_movable_(_f, _pass), deconstruct_movable_(_d, _pass), deconstruct_movable_(_e, _pass))) ]
                ;

            empire_empire_ref
                =   (
                        (   tok.LowestCostTransferrableTech_    [ _a = construct<std::string>(_1) ]
                        |   tok.HighestCostTransferrableTech_   [ _a = construct<std::string>(_1) ]
                        |   tok.TopPriorityTransferrableTech_   [ _a = construct<std::string>(_1) ]
                        |   tok.MostSpentTransferrableTech_     [ _a = construct<std::string>(_1) ]
                        |   tok.RandomTransferrableTech_        [ _a = construct<std::string>(_1) ]
                        )
                    )
                >   labeller.rule(Empire_token) > simple_int [ _b = _1 ]
                >   labeller.rule(Empire_token) > simple_int [ _c = _1 ]
                    [ _val = construct_movable_(new_<ValueRef::ComplexVariable<std::string>>(_a, deconstruct_movable_(_b, _pass), deconstruct_movable_(_c, _pass), deconstruct_movable_(_f, _pass), deconstruct_movable_(_d, _pass), deconstruct_movable_(_e, _pass))) ]
                ;

            start
                =   game_rule
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
