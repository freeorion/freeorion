#ifndef _EffectParser1_h_
#define _EffectParser1_h_

#include "EffectParserImpl.h"
#include "MovableEnvelope.h"

namespace parse { namespace detail {
    struct effect_parser_rules_1 : public effect_parser_grammar {
        effect_parser_rules_1(const parse::lexer& tok,
                              Labeller& labeller,
                              const condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                std::string,
                value_ref_payload<int>,
                value_ref_payload<int>,
                value_ref_payload<std::string>
                >
            > string_and_intref_and_intref_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                value_ref_payload<std::string>,
                value_ref_payload<double>,
                value_ref_payload<double>
                >
            > stringref_and_doubleref_rule;

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

        typedef rule<
            std::vector<string_and_string_ref_pair> ()
            > string_and_string_ref_vector_rule;

        parse::int_arithmetic_rules         int_rules;
        parse::double_parser_rules          double_rules;
        parse::empire_affiliation_enum_grammar empire_affiliation_type_enum;
        string_and_intref_and_intref_rule   set_empire_meter_1;
        string_and_intref_and_intref_rule   set_empire_meter_2;
        string_and_intref_and_intref_rule   give_empire_tech;
        stringref_and_doubleref_rule        set_empire_tech_progress;
        generate_sitrep_message_rule        generate_sitrep_message;
        string_and_intref_and_intref_rule   set_overlay_texture;
        string_and_string_ref_rule          string_and_string_ref;
        string_and_string_ref_vector_rule   string_and_string_ref_vector;
        effect_parser_rule   start;
    };

    }
}
#endif // _EffectParser1_h_
