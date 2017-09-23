#ifndef _EffectParser_h_
#define _EffectParser_h_

#include "Lexer.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "ConditionParser.h"

#include <boost/spirit/include/qi.hpp>

namespace Condition {
    struct ConditionBase;
}

namespace Effect {
    class EffectBase;
}

namespace parse { namespace detail {
    using effect_signature      = Effect::EffectBase* ();
    using effect_parser_rule    = rule<effect_signature>;
    using effect_parser_grammar = grammar<effect_signature>;

    struct effect_parser_rules_1 : public effect_parser_grammar {
        effect_parser_rules_1(const parse::lexer& tok,
                              Labeller& labeller,
                              const condition_parser_grammar& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                std::string,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*
                >
            > string_and_intref_and_intref_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*
                >
            > stringref_and_doubleref_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*>>,
                EmpireAffiliationType,
                std::string,
                bool
                >
            > generate_sitrep_message_rule;

        typedef std::pair<std::string, ValueRef::ValueRefBase<std::string>*> string_and_string_ref_pair;

        typedef parse::detail::rule<
            string_and_string_ref_pair (),
            boost::spirit::qi::locals<std::string>
            > string_and_string_ref_rule;

        typedef parse::detail::rule<
            std::vector<string_and_string_ref_pair> ()
            > string_and_string_ref_vector_rule;

        parse::int_arithmetic_rules         int_rules;
        parse::double_parser_rules          double_rules;
        string_and_intref_and_intref_rule   set_empire_meter_1;
        string_and_intref_and_intref_rule   set_empire_meter_2;
        string_and_intref_and_intref_rule   give_empire_tech;
        stringref_and_doubleref_rule        set_empire_tech_progress;
        generate_sitrep_message_rule        generate_sitrep_message;
        string_and_intref_and_intref_rule   set_overlay_texture;
        string_and_string_ref_rule          string_and_string_ref;
        string_and_string_ref_vector_rule   string_and_string_ref_vector;
        parse::detail::effect_parser_rule   start;
    };

    struct effect_parser_rules_2 : public effect_parser_grammar {
        effect_parser_rules_2(const parse::lexer& tok,
                              Labeller& labeller,
                              const parse::condition_parser_rule& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                MeterType,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<double>*,
                std::string
            >
        > set_meter_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ResourceType,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<Visibility>*,
                EmpireAffiliationType,
                Condition::ConditionBase*
            >
        > set_stockpile_or_vis_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*
            >
        > string_string_int_rule;

        parse::int_arithmetic_rules       int_rules;
        parse::double_parser_rules        double_rules;
        visibility_parser_rules           visibility_rules;
        set_meter_rule                    set_meter;
        set_meter_rule                    set_ship_part_meter;
        set_stockpile_or_vis_rule         set_empire_stockpile;
        parse::detail::effect_parser_rule set_empire_capital;
        parse::detail::effect_parser_rule set_planet_type;
        parse::detail::effect_parser_rule set_planet_size;
        parse::detail::effect_parser_rule set_species;
        string_string_int_rule            set_species_opinion;
        parse::detail::effect_parser_rule set_owner;
        set_stockpile_or_vis_rule         set_visibility;
        parse::detail::effect_parser_rule start;
        planet_type_parser_rules planet_type_rules;
    };

    struct effect_parser_rules_3 : public effect_parser_grammar {
        effect_parser_rules_3(const parse::lexer& tok,
                              Labeller& labeller,
                              const parse::condition_parser_rule& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*
            >
        > doubles_string_rule;

        parse::double_parser_rules          double_rules;
        parse::detail::effect_parser_rule           move_to;
        doubles_string_rule                 move_in_orbit;
        doubles_string_rule                 move_towards;
        parse::detail::effect_parser_rule           set_destination;
        parse::detail::effect_parser_rule           set_aggression;
        parse::detail::effect_parser_rule           destroy;
        parse::detail::effect_parser_rule           noop;
        parse::detail::effect_parser_rule           victory;
        parse::detail::effect_parser_rule           add_special_1;
        doubles_string_rule                 add_special_2;
        parse::detail::effect_parser_rule           remove_special;
        parse::detail::effect_parser_rule           add_starlanes;
        parse::detail::effect_parser_rule           remove_starlanes;
        parse::detail::effect_parser_rule           set_star_type;
        parse::detail::effect_parser_rule           set_texture;
        parse::detail::effect_parser_rule           start;
        parse::detail::star_type_parser_rules   star_type_rules;
    };

    struct effect_parser_rules_4 : public effect_parser_grammar {
        effect_parser_rules_4(const parse::lexer& tok,
                              const effect_parser_grammar& effect_parser,
                              Labeller& labeller,
                              const parse::condition_parser_rule& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase< ::PlanetType>*,
                ValueRef::ValueRefBase< ::PlanetSize>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_planet_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_building_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase< ::StarType>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_system_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<int>*,
                ValueRef::ValueRefBase<std::string>*,
                ValueRef::ValueRefBase<std::string>*,
                std::vector<Effect::EffectBase*>
            >
        > create_ship_rule;

        typedef parse::detail::rule<
            Effect::EffectBase* (),
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
        parse::detail::effect_parser_rule       start;
        parse::detail::star_type_parser_rules   star_type_rules;
        parse::detail::planet_type_parser_rules planet_type_rules;
    };

    struct effect_parser_rules_5 : public effect_parser_grammar {
        effect_parser_rules_5(const effect_parser_grammar& effect_parser,
                              Labeller& labeller,
                              const parse::condition_parser_rule& condition_parser);

        typedef parse::detail::rule<
            Effect::EffectBase* (),
            boost::spirit::qi::locals<
                Condition::ConditionBase*,
                std::vector<Effect::EffectBase*>,
                std::vector<Effect::EffectBase*>
            >
        > conditional_rule;

        conditional_rule            conditional;
        parse::detail::effect_parser_rule   start;
    };


    } //end namespace detail
} //end namespace parse

namespace parse {
    struct effects_parser_grammar : public detail::effect_parser_grammar {
        effects_parser_grammar(const lexer& tok,
                               detail::Labeller& labeller,
                               const parse::condition_parser_rule& condition_parser,
                               const parse::value_ref_grammar<std::string>& string_grammar);

        detail::effect_parser_rules_1 effect_parser_1;
        detail::effect_parser_rules_2 effect_parser_2;
        detail::effect_parser_rules_3 effect_parser_3;
        detail::effect_parser_rules_4 effect_parser_4;
        detail::effect_parser_rules_5 effect_parser_5;
        detail::effect_parser_rule start;
    };

    using effects_group_signature = std::vector<std::shared_ptr<Effect::EffectsGroup>> ();
    using effects_group_rule = detail::rule<effects_group_signature>;

    struct effects_group_grammar : public detail::grammar<effects_group_signature> {
        effects_group_grammar(const lexer& tok,
                              detail::Labeller& labeller,
                              const parse::condition_parser_rule& condition_parser,
                              const parse::value_ref_grammar<std::string>& string_grammar);

        typedef parse::detail::rule<
            Effect::EffectsGroup* (),
            boost::spirit::qi::locals<
                Condition::ConditionBase*,
                Condition::ConditionBase*,
                std::string,
                std::vector<Effect::EffectBase*>,
                std::string,
                int,
                std::string
            >
        > effects_group_rule;

        effects_parser_grammar effects_grammar;
        effects_group_rule effects_group;
        parse::effects_group_rule start;
    };

}

#endif
