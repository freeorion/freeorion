#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParams.h"

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
            retval[name_and_values.first] = {name_and_values.second.first.OpenEnvelope(pass),
                                             name_and_values.second.second.OpenEnvelope(pass)};
        return retval;
    }

    common_params_rules::common_params_rules(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const value_ref_grammar<std::string>& string_grammar,
        const tags_grammar_type& tags_parser
    ) :
        castable_int_rules(tok, labeller, condition_parser, string_grammar),
        double_rules(tok, labeller, condition_parser, string_grammar),
        effects_group_grammar(tok, labeller, condition_parser, string_grammar),
        non_ship_part_meter_type_enum(tok)
    {
        namespace qi = boost::spirit::qi;

        using phoenix::new_;
        using phoenix::construct;
        using phoenix::insert;

        qi::_1_type _1;
        qi::_a_type _a;
        qi::_b_type _b;
        qi::_c_type _c;
        qi::_d_type _d;
        qi::_e_type _e;
        qi::_f_type _f;
        qi::_g_type _g;
        qi::_h_type _h;
        qi::_i_type _i;
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
            =    (labeller.rule(Location_token) > condition_parser [ _val = _1 ])
            |     eps [ _val = construct_movable_(new_<Condition::All>()) ]
            ;

        enqueue_location
            =    (labeller.rule(EnqueueLocation_token) > condition_parser [ _val = _1 ])
            |     eps [ _val = construct_movable_(new_<Condition::All>()) ]
            ;

        exclusions
            =  -(
                labeller.rule(Exclusions_token)
                >>  (
                    ('[' > +tok.string [ insert(_r1, _1) ] > ']')
                    |   tok.string [ insert(_r1, _1) ]
                )
            )
            ;

        more_common
            =
            (   labeller.rule(Name_token)        > tok.string [ _a = _1 ]
                >   labeller.rule(Description_token) > tok.string [ _b = _1 ]
                >   exclusions(_c)
            ) [ _val = construct<MoreCommonParams>(_a, _b, _c) ]
            ;

        common
            =
            (   labeller.rule(BuildCost_token)  > double_rules.expr [ _a = _1 ]
                >   labeller.rule(BuildTime_token)  > castable_int_rules.flexible_int [ _b = _1 ]
                >   producible                                          [ _c = _1 ]
                >   tags_parser(_d)
                >   location [_e = _1]
                >   enqueue_location [_i = _1]
                >  -consumption(_g, _h)
                > -(labeller.rule(EffectsGroups_token)> effects_group_grammar [ _f = _1 ])
            ) [ _val = construct_movable_(
                new_<CommonParams>(
                    deconstruct_movable_(_a, _pass),
                    deconstruct_movable_(_b, _pass),
                    _c, _d,
                    deconstruct_movable_(_e, _pass),
                    deconstruct_movable_vector_(_f, _pass),
                    phoenix::bind(&parse::detail::OpenConsumptionEnvelopes<MeterType>, _g, _pass),
                    phoenix::bind(&parse::detail::OpenConsumptionEnvelopes<std::string>, _h, _pass),
                    deconstruct_movable_(_i, _pass))) ]
            ;

        consumption
            =   labeller.rule(Consumption_token) >
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
                labeller.rule(Name_token)        > tok.string [ _a = _1 ]
                >   labeller.rule(Consumption_token) > double_rules.expr [ _b = _1 ]
                > -(labeller.rule(Condition_token)   > condition_parser [ _c = _1 ])
            )
            [ insert(_r1, construct<ConsumptionMapPackaged<std::string>::value_type>(_a, construct<ConsumptionMapPackaged<std::string>::mapped_type>(_b, _c))) ]
            ;

        consumable_meter
            = (
                non_ship_part_meter_type_enum [ _a = _1 ]
                >   labeller.rule(Consumption_token) > double_rules.expr [ _b = _1 ]
                > -(labeller.rule(Condition_token)   > condition_parser [ _c = _1 ])
            )
            [ insert(_r1, construct<ConsumptionMapPackaged<MeterType>::value_type>(_a, construct<ConsumptionMapPackaged<MeterType>::mapped_type>(_b, _c))) ]
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
