#ifndef _EffectParser4_h_
#define _EffectParser4_h_

#include "EffectParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse { namespace detail {
    struct effect_parser_rules_4 : public effect_parser_grammar {
        effect_parser_rules_4(const parse::lexer& tok,
                              const effect_parser_grammar& effect_parser,
                              Labeller& label,
                              const condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        parse::int_arithmetic_rules                       int_rules;
        parse::double_parser_rules                        double_rules;
        effect_parser_rule                                create_planet;
        effect_parser_rule                                create_building;
        effect_parser_rule                                create_ship_1;
        effect_parser_rule                                create_ship_2;
        effect_parser_rule                                create_field_1;
        effect_parser_rule                                create_field_2;
        effect_parser_rule                                create_system_1;
        effect_parser_rule                                create_system_2;
        effect_parser_rule                                start;
        star_type_parser_rules                            star_type_rules;
        planet_type_parser_rules                          planet_type_rules;
        planet_size_parser_rules                          planet_size_rules;
        single_or_bracketed_repeat<effect_parser_grammar> one_or_more_effects;
    };
    }
}

#endif // _EffectParser4_h_
