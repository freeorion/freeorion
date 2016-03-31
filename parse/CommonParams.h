#ifndef _Common_Params_h_
#define _Common_Params_h_

#include "Lexer.h"
#include "../universe/Enums.h"
#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"
#include "../universe/Effect.h"


#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

namespace parse { namespace detail {

    typedef qi::rule<
        token_iterator,
        bool (),
        skipper_type
    > producible_rule;
    const producible_rule& producible_parser();

    typedef qi::rule<
        token_iterator,
        void (Condition::ConditionBase*&),
        skipper_type
    > location_rule;
    const location_rule& location_parser();

    typedef qi::rule<
        token_iterator,
        CommonParams (),
        qi::locals<
            ValueRef::ValueRefBase<double>*,
            ValueRef::ValueRefBase<int>*,
            bool,
            std::set<std::string>,
            Condition::ConditionBase*,
            std::vector<boost::shared_ptr<Effect::EffectsGroup> >,
            std::map<MeterType, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*> >,
            std::map<std::string, std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*> >,
            Condition::ConditionBase*
        >,
        skipper_type
    > common_params_rule;
    const common_params_rule& common_params_parser();

    typedef qi::rule<
        token_iterator,
        MoreCommonParams (),
        qi::locals<
            std::string,
            std::string,
            std::set<std::string>
        >,
        skipper_type
    > more_common_params_rule;
    const more_common_params_rule& more_common_params_parser();

} }

#endif
