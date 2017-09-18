#ifndef _Common_Params_h_
#define _Common_Params_h_

#include "Lexer.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"
#include "../universe/EnumsFwd.h"
#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"

#include <boost/spirit/include/qi.hpp>


namespace parse { namespace detail {

    typedef rule<
        bool ()
    > producible_rule;
    const producible_rule& producible_parser();

    typedef rule<
        void (Condition::ConditionBase*&)
    > location_rule;
    const location_rule& location_parser();

    typedef rule<
        CommonParams (),
        boost::spirit::qi::locals<
            ValueRef::ValueRefBase<double>*,
            ValueRef::ValueRefBase<int>*,
            bool,
            std::set<std::string>,
            Condition::ConditionBase*,
            std::vector<std::shared_ptr<Effect::EffectsGroup>>,
            std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>,
            std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*>>,
            Condition::ConditionBase*
        >
    > common_params_rule;
    const common_params_rule& common_params_parser();

    typedef rule<
        MoreCommonParams (),
        boost::spirit::qi::locals<
            std::string,
            std::string,
            std::set<std::string>
        >
    > more_common_params_rule;
    const more_common_params_rule& more_common_params_parser();

    struct common_params_rules {
        typedef std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*> val_cond_pair;

        common_params_rules(const parse::lexer& tok, parse::detail::Labeller& labeller);

        typedef rule<
            void (std::map<MeterType, val_cond_pair>&,
                  std::map<std::string, val_cond_pair>&)
        > consumption_rule;

        typedef rule<
            void (std::map<MeterType, val_cond_pair>&, std::map<std::string, val_cond_pair>&),
            boost::spirit::qi::locals<
                MeterType,
                std::string,
                ValueRef::ValueRefBase<double>*,
                Condition::ConditionBase*
            >
        > consumable_rule;

        typedef rule<
            void (std::set<std::string>&)
        > exclusions_rule;

        parse::castable_as_int_parser_rules    castable_int_rules;
        parse::double_parser_rules     double_rules;
        producible_rule         producible;
        location_rule           location;
        location_rule           enqueue_location;
        exclusions_rule         exclusions;
        more_common_params_rule more_common;
        common_params_rule      common;
        consumption_rule        consumption;
        consumable_rule         consumable_special;
        consumable_rule         consumable_meter;
    };

} }

#endif
