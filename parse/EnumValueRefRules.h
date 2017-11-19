#ifndef _EnumValueRefRules_h_
#define _EnumValueRefRules_h_

#include "ValueRefParser.h"

namespace parse {
    namespace detail {

    template <typename T>
    void initialize_nonnumeric_statistic_parser(
        parse::detail::statistic_rule<T>& statistic,
        const parse::lexer& tok,
        parse::detail::Labeller& labeller,
        const parse::detail::condition_parser_grammar& condition_parser,
        const typename parse::detail::value_ref_rule<T>& value_ref);

    template <typename T>
    struct enum_value_ref_rules {
        enum_value_ref_rules(const std::string& type_name,
                             const lexer& tok,
                             Labeller& labeller,
                             const condition_parser_grammar& condition_parser);

        name_token_rule variable_name;
        enum_rule<T> enum_expr;
        value_ref_rule<T> constant_expr;
        value_ref_rule<T> free_variable_expr;
        variable_rule<T> bound_variable_expr;
        expression_rule<T> functional_expr;
        value_ref_rule<T> primary_expr;
        value_ref_rule<T> statistic_sub_value_ref;
        statistic_rule<T> statistic_expr;
        complex_variable_rule<T> complex_expr;
        value_ref_rule<T> expr;
        reference_token_rule variable_scope_rule;
        name_token_rule container_type_rule;
    };

    struct planet_environment_parser_rules :
        public enum_value_ref_rules<PlanetEnvironment>
    {
        planet_environment_parser_rules(const lexer& tok,
                                        Labeller& labeller,
                                        const condition_parser_grammar& condition_parser);
    };

    struct planet_size_parser_rules :
        public enum_value_ref_rules<PlanetSize>
    {
        planet_size_parser_rules(const lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser);
    };

    struct planet_type_parser_rules :
        public enum_value_ref_rules<PlanetType>
    {
        planet_type_parser_rules(const lexer& tok,
                                 Labeller& labeller,
                                 const condition_parser_grammar& condition_parser);
    };

    struct star_type_parser_rules :
        public enum_value_ref_rules<StarType>
    {
        star_type_parser_rules(const lexer& tok,
                               Labeller& labeller,
                               const condition_parser_grammar& condition_parser);
    };

    struct visibility_complex_parser_grammar : public complex_variable_grammar<Visibility> {
        visibility_complex_parser_grammar(const lexer& tok, Labeller& labeller);

        simple_int_parser_rules  simple_int_rules;
        complex_variable_rule<Visibility> empire_object_visibility;
        complex_variable_rule<Visibility> start;
    };

    struct visibility_parser_rules :
        public enum_value_ref_rules<Visibility>
    {
        visibility_parser_rules(const lexer& tok,
                                Labeller& labeller,
                                const condition_parser_grammar& condition_parser);

        visibility_complex_parser_grammar visibility_var_complex_grammar;
    };

    struct universe_object_type_parser_rules :
        public enum_value_ref_rules<UniverseObjectType>
    {
        universe_object_type_parser_rules(const lexer& tok,
                                          Labeller& labeller,
                                          const condition_parser_grammar& condition_parser);
    };

}}

#endif // _EnumValueRefRules_h_
