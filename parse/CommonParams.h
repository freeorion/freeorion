// -*- C++ -*-
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
        PartHullCommonParams (),
        qi::locals<
            ValueRef::ValueRefBase<double>*,
            ValueRef::ValueRefBase<int>*,
            bool,
            std::set<std::string>,
            Condition::ConditionBase*,
            std::vector<boost::shared_ptr<Effect::EffectsGroup> >,
            std::map<MeterType, ValueRef::ValueRefBase<double>*>,
            std::map<std::string, ValueRef::ValueRefBase<double>*>
        >,
        skipper_type
    > part_hull_common_params_rule;
    const part_hull_common_params_rule& common_params_parser();
} }

#endif
