#ifndef _EffectParser4_h_
#define _EffectParser4_h_

#include "EffectParserImpl.h"

namespace parse { namespace detail {
    struct effect_parser_rules_4 : public effect_parser_grammar {
        effect_parser_rules_4(const parse::lexer& tok,
                              const effect_parser_grammar& effect_parser,
                              Labeller& labeller,
                              const condition_parser_grammar& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase< ::PlanetType>*,
                ValueRef::ValueRefBase< ::PlanetSize>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_planet_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_building_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase< ::StarType>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_system_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_ship_rule;

        typedef rule<
            effect_signature,
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                std::vector<Effect::EffectBase*>
            >
        > create_field_rule;

        parse::int_arithmetic_rules     int_rules;
        parse::double_parser_rules      double_rules;
        create_planet_rule              create_planet;
        create_building_rule            create_building;
        create_ship_rule                create_ship_1;
        create_ship_rule                create_ship_2;
        create_field_rule               create_field_1;
        create_field_rule               create_field_2;
        create_system_rule              create_system_1;
        create_system_rule              create_system_2;
        effect_parser_rule       start;
        star_type_parser_rules   star_type_rules;
        planet_type_parser_rules planet_type_rules;
        planet_size_parser_rules planet_size_rules;
    };
    }
}

#endif // _EffectParser4_h_
