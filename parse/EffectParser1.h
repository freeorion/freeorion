#ifndef _EffectParser1_h_
#define _EffectParser1_h_

#include "EffectParserImpl.h"
#include "MovableEnvelope.h"

namespace parse { namespace detail {
    struct effect_parser_rules_1 : public effect_parser_grammar {
        effect_parser_rules_1(const parse::lexer& tok,
                              Labeller& label,
                              const condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::vector<std::pair<std::string, value_ref_payload<std::string>>>,
                EmpireAffiliationType,
                std::string,
                bool,
                value_ref_payload<int>,
                parse::detail::MovableEnvelope<Condition::ConditionBase>
                >
            > generate_sitrep_message_rule;

        typedef std::pair<std::string, value_ref_payload<std::string>> string_and_string_ref_pair;

        typedef rule<
            string_and_string_ref_pair (),
            boost::spirit::qi::locals<
                std::string,
                value_ref_payload<std::string>>
            > string_and_string_ref_rule;

        parse::int_arithmetic_rules                            int_rules;
        parse::double_parser_rules                             double_rules;
        parse::empire_affiliation_enum_grammar                 empire_affiliation_type_enum;
        effect_parser_rule                                     set_empire_meter_1;
        effect_parser_rule                                     set_empire_meter_2;
        effect_parser_rule                                     give_empire_tech;
        effect_parser_rule                                     set_empire_tech_progress;
        generate_sitrep_message_rule                           generate_sitrep_message;
        effect_parser_rule                                     set_overlay_texture;
        string_and_string_ref_rule                             string_and_string_ref;
        single_or_bracketed_repeat<string_and_string_ref_rule> one_or_more_string_and_string_ref_pair;
        effect_parser_rule                                     start;
    };

    }
}
#endif // _EffectParser1_h_
