#ifndef _EffectParser3_h_
#define _EffectParser3_h_

#include "EffectParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse { namespace detail {
    struct effect_parser_rules_3 : public effect_parser_grammar {
        effect_parser_rules_3(const parse::lexer& tok,
                              Labeller& label,
                              const condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                value_ref_payload<double>,
                value_ref_payload<double>,
                value_ref_payload<std::string>
            >
        > doubles_string_rule;

        parse::double_parser_rules double_rules;
        effect_parser_rule         move_to;
        doubles_string_rule        move_in_orbit;
        doubles_string_rule        move_towards;
        effect_parser_rule         set_destination;
        effect_parser_rule         set_aggression;
        effect_parser_rule         destroy;
        effect_parser_rule         noop;
        effect_parser_rule         victory;
        effect_parser_rule         add_special_1;
        effect_parser_rule         add_special_2;
        effect_parser_rule         remove_special;
        effect_parser_rule         add_starlanes;
        effect_parser_rule         remove_starlanes;
        effect_parser_rule         set_star_type;
        effect_parser_rule         set_texture;
        effect_parser_rule         start;
        star_type_parser_rules     star_type_rules;
    };

    }
}
#endif // _EffectParser3_h_
