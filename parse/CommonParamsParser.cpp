#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParamsParser.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "EffectParser.h"
#include "ValueRefParser.h"

#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    /** Open parsed envelopes of consumption pairs. Return a map of unique_ptr. */
    template <typename T>
    CommonParams::ConsumptionMap<T> OpenConsumptionEnvelopes(
        const common_params_rules::ConsumptionMapPackaged<T>& in, bool& pass)
    {
        CommonParams::ConsumptionMap<T> retval;
        for (auto&& name_and_values : in)
            retval[name_and_values.first] = {
                name_and_values.second.first.OpenEnvelope(pass),
                (name_and_values.second.second ? name_and_values.second.second->OpenEnvelope(pass) : nullptr)};
        return retval;
    }

    common_params_rules::common_params_rules(
        const parse::lexer& tok,
        Labeller& label,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar,
        const tags_grammar_type& tags_parser
    ) :
        castable_int_rules(tok, label, condition_parser, string_grammar),
        double_rules(tok, label, condition_parser, string_grammar),
        effects_group_grammar(tok, label, condition_parser, string_grammar),
        non_ship_part_meter_type_enum(tok),
        repeated_string(tok)
    {
        namespace qi = boost::spirit::qi;

        using phoenix::new_;
        using phoenix::construct;
        using phoenix::insert;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_3_type _3;
        qi::_4_type _4;
        qi::_5_type _5;
        qi::_6_type _6;
        qi::_7_type _7;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_r1_type _r1;
        qi::_r2_type _r2;
        qi::_val_type _val;
        qi::eps_type eps;
        qi::_pass_type _pass;
        const boost::phoenix::function<construct_movable> construct_movable_;
        const boost::phoenix::function<deconstruct_movable> deconstruct_movable_;
        const boost::phoenix::function<deconstruct_movable_vector> deconstruct_movable_vector_;

        producible
            =   tok.Unproducible_ [ _val = false ]
            |   tok.Producible_ [ _val = true ]
            |   eps [ _val = true ]
            ;

        location
            %=    (label(tok.Location_) > condition_parser)
            |     eps [ _val = construct_movable_(new_<Condition::All>()) ]
            ;

        enqueue_location
            %=    (label(tok.EnqueueLocation_) > condition_parser)
            |     eps [ _val = construct_movable_(new_<Condition::All>()) ]
            ;

        exclusions
            =
            -(label(tok.Exclusions_) >> repeated_string)
            ;

        more_common
            =
            (   label(tok.Name_)        > tok.string
                >   label(tok.Description_) > tok.string
                >   exclusions
            ) [ _val = construct<MoreCommonParams>(_1, _2, _3) ]
            ;

        common
            =
            (   label(tok.BuildCost_)  > double_rules.expr
                >   label(tok.BuildTime_)  > castable_int_rules.flexible_int
                >   producible
                >   tags_parser 
                >   location
                >   enqueue_location
                >  -consumption(_a, _b)
                > -(label(tok.EffectsGroups_)> effects_group_grammar )
            ) [ _val = construct_movable_(
                new_<CommonParams>(
                    deconstruct_movable_(_1, _pass),
                    deconstruct_movable_(_2, _pass),
                    _3, _4,
                    deconstruct_movable_(_5, _pass),
                    deconstruct_movable_vector_(_7, _pass),
                    phoenix::bind(&parse::detail::OpenConsumptionEnvelopes<MeterType>, _a, _pass),
                    phoenix::bind(&parse::detail::OpenConsumptionEnvelopes<std::string>, _b, _pass),
                    deconstruct_movable_(_6, _pass))) ]
            ;

        consumption
            =   label(tok.Consumption_) >
            (   consumable_meter(_r1)
                | consumable_special(_r2)
                |
                (
                    (   '[' >> *
                        (    consumable_meter(_r1)
                             | consumable_special(_r2)
                        )
                    )
                    >   ']'
                )
            )
            ;

        consumable_special
            =   tok.Special_
            > (
                label(tok.Name_)        > tok.string
                >   label(tok.Consumption_) > double_rules.expr
                > -(label(tok.Condition_)   > condition_parser )
            )
            [ insert(_r1, construct<ConsumptionMapPackaged<std::string>::value_type>(_1, construct<ConsumptionMapPackaged<std::string>::mapped_type>(_2, _3))) ]
            ;

        consumable_meter
            = (
                non_ship_part_meter_type_enum
                >   label(tok.Consumption_) > double_rules.expr
                > -(label(tok.Condition_)   > condition_parser )
            )
            [ insert(_r1, construct<ConsumptionMapPackaged<MeterType>::value_type>(_1, construct<ConsumptionMapPackaged<MeterType>::mapped_type>(_2, _3))) ]
            ;

        producible.name("Producible or Unproducible");
        location.name("Location");
        enqueue_location.name("Enqueue Location");
        exclusions.name("Exclusions");
        more_common.name("More Common Parameters");
        common.name("Common Paramaters");
        consumption.name("Consumption");
        consumable_special.name("Consumable Special");
        consumable_meter.name("Consumable Meter");

#if DEBUG_PARSERS
        debug(producible);
        debug(location);
        debug(enqueue_location);
        debug(exclusions);
        debug(more_common);
        debug(common);
        debug(consumption);
        debug(con_special);
        debug(consumable_meter);
#endif
    }
} }
