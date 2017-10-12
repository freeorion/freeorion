#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParams.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "EffectParser.h"
#include "ValueRefParser.h"

#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>


namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    common_params_rules::common_params_rules(
        const parse::lexer& tok,
        Labeller& labeller,
        const condition_parser_grammar& condition_parser,
        const parse::value_ref_grammar<std::string>& string_grammar,
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

        producible
            =   tok.Unproducible_ [ _val = false ]
            |   tok.Producible_ [ _val = true ]
            |   eps [ _val = true ]
            ;

        location
            =    (labeller.rule(Location_token) > condition_parser [ _r1 = _1 ])
            |     eps [ _r1 = new_<Condition::All>() ]
            ;

        enqueue_location
            =    (labeller.rule(EnqueueLocation_token) > condition_parser [ _r1 = _1 ])
            |     eps [ _r1 = new_<Condition::All>() ]
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
                >   location(_e)
                >   enqueue_location(_i)
                >  -consumption(_g, _h)
                > -(labeller.rule(EffectsGroups_token)> effects_group_grammar [ _f = _1 ])
            ) [ _val = construct<CommonParams>(_a, _b, _c, _d, _e, _f, _g, _h, _i) ]
            ;

        consumption
            =   labeller.rule(Consumption_token) >
            (   consumable_meter(_r1, _r2)
                | consumable_special(_r1, _r2)
                |
                (
                    (   '[' >> *
                        (    consumable_meter(_r1, _r2)
                             | consumable_special(_r1, _r2)
                        )
                    )
                    >   ']'
                )
            )
            ;

        typedef std::map<std::string, val_cond_pair>::value_type special_consumable_map_value_type;
        consumable_special
            =   tok.Special_
            > (
                labeller.rule(Name_token)        > tok.string [ _b = _1 ]
                >   labeller.rule(Consumption_token) > double_rules.expr [ _c = _1 ]
                > -(labeller.rule(Condition_token)   > condition_parser [ _d = _1 ])
            )
            [ insert(_r2, construct<special_consumable_map_value_type>(_b, construct<val_cond_pair>(_c, _d))) ]
            ;

        typedef std::map<MeterType, val_cond_pair>::value_type meter_consumable_map_value_type;
        consumable_meter
            = (
                non_ship_part_meter_type_enum [ _a = _1 ]
                >   labeller.rule(Consumption_token) > double_rules.expr [ _c = _1 ]
                > -(labeller.rule(Condition_token)   > condition_parser [ _d = _1 ])
            )
            [ insert(_r1, construct<meter_consumable_map_value_type>(_a, construct<val_cond_pair>(_c, _d))) ]
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
