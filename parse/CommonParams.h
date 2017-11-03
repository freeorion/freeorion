#ifndef _Common_Params_h_
#define _Common_Params_h_

#include "Lexer.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "EffectParser.h"
#include "EnumParser.h"
#include "MovableEnvelope.h"
#include "../universe/EnumsFwd.h"
#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/qi.hpp>


namespace parse { namespace detail {

    struct common_params_rules {
        template <typename T>
        using ConsumptionMapPackaged = std::map<T, std::pair<value_ref_payload<double>, condition_payload>>;
        typedef rule<
            bool ()
            > producible_rule;

        typedef rule<
            MovableEnvelope<CommonParams> (),
            boost::spirit::qi::locals<
                value_ref_payload<double>,
                value_ref_payload<int>,
                bool,
                std::set<std::string>,
                condition_payload,
                std::vector<MovableEnvelope<Effect::EffectsGroup>>,
                ConsumptionMapPackaged<MeterType>,
                ConsumptionMapPackaged<std::string>,
                condition_payload
                >
            > common_params_rule;

        typedef rule<
            MoreCommonParams (),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                std::set<std::string>
                >
            > more_common_params_rule;

        typedef rule<
            void (ConsumptionMapPackaged<MeterType>&, ConsumptionMapPackaged<std::string>&)
            > consumption_rule;

        template <typename T>
        using consumable_rule = rule<
            void (ConsumptionMapPackaged<T>&),
            boost::spirit::qi::locals<
                T,
                value_ref_payload<double>,
                condition_payload
                >
            >;

        typedef rule<
            void (std::set<std::string>&)
        > exclusions_rule;

        common_params_rules(const parse::lexer& tok,
                            Labeller& labeller,
                            const condition_parser_grammar& condition_parser,
                            const value_ref_grammar<std::string>& string_grammar,
                            const tags_grammar_type& tags_parser);

        parse::castable_as_int_parser_rules     castable_int_rules;
        parse::double_parser_rules              double_rules;
        parse::effects_group_grammar            effects_group_grammar;
        parse::non_ship_part_meter_enum_grammar non_ship_part_meter_type_enum;
        producible_rule                         producible;
        condition_parser_rule                   location;
        condition_parser_rule                   enqueue_location;
        exclusions_rule                         exclusions;
        more_common_params_rule                 more_common;
        common_params_rule                      common;
        consumption_rule                        consumption;
        consumable_rule<std::string>            consumable_special;
        consumable_rule<MeterType>              consumable_meter;
    };

} }

#endif
