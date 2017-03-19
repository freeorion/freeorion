#ifndef _Common_Params_h_
#define _Common_Params_h_

#include "Lexer.h"
#include "ParseImpl.h"
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

} }

#endif
