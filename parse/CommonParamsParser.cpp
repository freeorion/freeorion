#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParams.h"
#include "Label.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "EnumParser.h"
#include "ParseImpl.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    struct rules {
        typedef std::pair<ValueRef::ValueRefBase<double>*, Condition::ConditionBase*> val_cond_pair;


        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule< int >::type& flexible_int_ref =     parse::value_ref_parser_flexible_int();

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
            using phoenix::new_;
            using phoenix::construct;
            using phoenix::insert;

            producible
                =   tok.Unproducible_ [ _val = false ]
                |   tok.Producible_ [ _val = true ]
                |   eps [ _val = true ]
                ;

            location
                =    (parse::label(Location_token) > parse::detail::condition_parser [ _r1 = _1 ])
                |     eps [ _r1 = new_<Condition::All>() ]
                ;

            enqueue_location
                =    (parse::label(EnqueueLocation_token) > parse::detail::condition_parser [ _r1 = _1 ])
                |     eps [ _r1 = new_<Condition::All>() ]
                ;

            exclusions
                =  -(
                        parse::label(Exclusions_token)
                    >>  (
                            ('[' > +tok.string [ insert(_r1, _1) ] > ']')
                            |   tok.string [ insert(_r1, _1) ]
                        )
                    )
                ;

            more_common
                =
                (   parse::label(Name_token)        > tok.string [ _a = _1 ]
                >   parse::label(Description_token) > tok.string [ _b = _1 ]
                >   exclusions(_c)
                ) [ _val = construct<MoreCommonParams>(_a, _b, _c) ]
                ;

            common
                =
                (   parse::label(BuildCost_token)    > double_value_ref [ _a = _1 ]
                >   parse::label(BuildTime_token)    > flexible_int_ref [ _b = _1 ]
                >   producible                                          [ _c = _1 ]
                >   parse::detail::tags_parser()(_d)
                >   location(_e)
                >   enqueue_location(_i)
                >  -consumption(_g, _h)
                > -(parse::label(EffectsGroups_token)> parse::detail::effects_group_parser() [ _f = _1 ])
                ) [ _val = construct<CommonParams>(_a, _b, _c, _d, _e, _f, _g, _h, _i) ]
                ;

            consumption
                =   parse::label(Consumption_token)
                > (
                        consumable_meter(_r1, _r2)
                    |   consumable_special(_r1, _r2)
                    |(  '['
                        >> *(
                                consumable_meter(_r1, _r2)
                            |   consumable_special(_r1, _r2)
                            )
                        >   ']'
                     )
                  )
                ;

            typedef std::map<std::string, val_cond_pair>::value_type special_consumable_map_value_type;
            consumable_special
                =   tok.Special_
                > (
                    parse::label(Name_token)        > tok.string [ _b = _1 ]
                >   parse::label(Consumption_token) > double_value_ref [ _c = _1 ]
                > -(parse::label(Condition_token)   > parse::detail::condition_parser [ _d = _1 ])
                  )
                [ insert(_r2, construct<special_consumable_map_value_type>(_b, construct<val_cond_pair>(_c, _d))) ]
                ;

            typedef std::map<MeterType, val_cond_pair>::value_type meter_consumable_map_value_type;
            consumable_meter
                = (
                    parse::non_ship_part_meter_type_enum() [ _a = _1 ]
                >   parse::label(Consumption_token) > double_value_ref [ _c = _1 ]
                > -(parse::label(Condition_token)   > parse::detail::condition_parser [ _d = _1 ])
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

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<MeterType, val_cond_pair>&,
                  std::map<std::string, val_cond_pair>&),
            parse::skipper_type
        > consumption_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<MeterType, val_cond_pair>&, std::map<std::string, val_cond_pair>&),
            qi::locals<
                MeterType,
                std::string,
                ValueRef::ValueRefBase<double>*,
                Condition::ConditionBase*
            >,
            parse::skipper_type
        > consumable_rule;

        typedef qi::rule<
            token_iterator,
            void (std::set<std::string>&),
            parse::skipper_type
        > exclusions_rule;

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

    rules& GetRules() {
        static rules retval;
        return retval;
    }

    const producible_rule& producible_parser()
    { return GetRules().producible; }

    const location_rule& location_parser()
    { return GetRules().location; }

    const common_params_rule& common_params_parser()
    { return GetRules().common; }

    const more_common_params_rule& more_common_params_parser()
    { return GetRules().more_common; }
} }

