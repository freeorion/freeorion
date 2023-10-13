#ifndef _EffectParser2_h_
#define _EffectParser2_h_

#include "EffectParserImpl.h"
#include "EnumValueRefRules.h"

namespace parse::detail {
    struct effect_parser_rules_2 : public effect_parser_grammar {
        effect_parser_rules_2(const parse::lexer& tok,
                              Labeller& label,
                              const condition_parser_grammar& condition_parser,
                              const value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                MeterType,
                value_ref_payload<double>,
                boost::optional<std::string>
            >
        > set_meter_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ResourceType,
                value_ref_payload<int>,
                value_ref_payload<Visibility>,
                EmpireAffiliationType,
                condition_payload
            >
        > set_stockpile_or_vis_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                value_ref_payload<std::string>,
                value_ref_payload<std::string>,
                value_ref_payload<int>,
                bool
            >
        > string_string_int_rule;

        parse::int_arithmetic_rules                 int_rules;
        parse::double_parser_rules                  double_rules;
        visibility_parser_rules                     visibility_rules;
        set_meter_rule                              set_meter;
        effect_parser_rule                          set_ship_part_meter;
        set_stockpile_or_vis_rule                   set_empire_stockpile;
        effect_parser_rule                          set_empire_capital;
        effect_parser_rule                          set_planet_type;
        effect_parser_rule                          set_original_type;
        effect_parser_rule                          set_planet_size;
        effect_parser_rule                          set_focus;
        effect_parser_rule                          set_species;
        string_string_int_rule                      set_species_opinion;
        effect_parser_rule                          set_owner;
        set_stockpile_or_vis_rule                   set_visibility;
        effect_parser_rule                          start;
        planet_type_parser_rules                    planet_type_rules;
        planet_size_parser_rules                    planet_size_rules;
        parse::empire_affiliation_enum_grammar      empire_affiliation_type_enum;
        parse::set_non_ship_part_meter_enum_grammar set_non_ship_part_meter_type_enum;
        parse::set_ship_part_meter_enum_grammar     set_ship_part_meter_type_enum;
        parse::resource_type_grammar                resource_type_enum;
    };
}

#endif
