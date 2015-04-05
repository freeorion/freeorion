#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "ConditionParserImpl.h"
#include "Double.h"
#include "EnumParser.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"
#include "ValueRefParser.h"

#include "../universe/Condition.h"
#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<HullType::Slot>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, HullType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, HullType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const HullType::Slot&) { return os; }
    inline ostream& operator<<(ostream& os, const HullTypeStats&) { return os; }
}
#endif

namespace {
    struct insert_ {
#if BOOST_VERSION < 105600
        template <typename Arg1, typename Arg2> // Phoenix v2
        struct result
        { typedef void type; };
#else
        typedef void result_type;
#endif

        void operator()(std::map<std::string, HullType*>& hulls, HullType* hull) const {
            if (!hulls.insert(std::make_pair(hull->Name(), hull)).second) {
                std::string error_str = "ERROR: More than one ship hull in ship_hulls.txt has the name " + hull->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            const parse::value_ref_parser_rule<double>::type& double_value_ref =    parse::value_ref_parser<double>();
            const parse::value_ref_parser_rule< int >::type& flexible_int_ref =     parse::value_ref_parser_flexible_int();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_f_type _f;
            qi::_r1_type _r1;
            qi::_r2_type _r2;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;

            hull_prefix
                =    tok.Hull_
                >    parse::label(Name_token)        > tok.string [ _r1 = _1 ]
                >    parse::label(Description_token) > tok.string [ _r2 = _1 ]
                ;

            hull_stats
                =   parse::label(Speed_token)       >   parse::double_ [ _a = _1 ]
                >   parse::label(Fuel_token)        >   parse::double_ [ _c = _1 ]
                >   parse::label(Stealth_token)     >   parse::double_ [ _d = _1 ]
                >   parse::label(Structure_token)   >   parse::double_
                    [ _val = construct<HullTypeStats>(_c, _a, _d, _1) ]
                ;

            producible
                =   tok.Unproducible_ [ _val = false ]
                |   tok.Producible_ [ _val = true ]
                |   eps [ _val = true ]
                ;

            slot
                =   tok.Slot_
                >   parse::label(Type_token) > parse::enum_parser<ShipSlotType>() [ _a = _1 ]
                >   parse::label(Position_token)
                >   '(' > parse::double_ [ _b = _1 ] > ',' > parse::double_ [ _c = _1 ] > lit(')')
                    [ _val = construct<HullType::Slot>(_a, _b, _c) ]
                ;

            slots
                =  -(
                        parse::label(Slots_token)
                    >   (
                            '[' > +slot [ push_back(_r1, _1) ] > ']'
                            |   slot [ push_back(_r1, _1) ]
                        )
                     )
                ;

            location
                =    parse::label(Location_token) > parse::detail::condition_parser [ _r1 = _1 ]
                |    eps [ _r1 = new_<Condition::All>() ]
                ;

            common_params
                =   parse::label(BuildCost_token)    > double_value_ref  [ _a = _1 ]
                >   parse::label(BuildTime_token)    > flexible_int_ref  [ _b = _1 ]
                >   producible                                           [ _c = _1 ]
                >   parse::detail::tags_parser()(_d)
                >   location(_e)
                > -(parse::label(EffectsGroups_token) > parse::detail::effects_group_parser() [ _f = _1 ])
                >   parse::label(Icon_token)        > tok.string
                    [ _val = construct<PartHullCommonParams>(_a, _b, _c, _d, _e, _f, _1) ]
            ;

            hull
                =   hull_prefix(_a, _b)
                >   hull_stats [ _c = _1 ]
                >  -slots(_e)
                >   common_params [ _d = _1 ]
                >   parse::label(Graphic_token) > tok.string
                    [ insert(_r1, new_<HullType>(_a, _b, _c, _d, _e, _1)) ]
                ;

            start
                =   +hull(_r1)
                ;

            hull_prefix.name("Hull");
            hull_stats.name("Hull stats");
            producible.name("Producible or Unproducible");
            slot.name("Slot");
            slots.name("Slots");
            location.name("Location");
            common_params.name("Part Hull Common Params");
            hull.name("Hull");

#if DEBUG_PARSERS
            debug(hull_prefix);
            debug(cost);
            debug(hull_stats);
            debug(producible);
            debug(slot);
            debug(slots);
            debug(location);
            debug(common_params);
            debug(hull);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::string&, std::string&),
            parse::skipper_type
        > hull_prefix_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (),
            qi::locals<
                std::string&,
                std::string&,
                std::string&
            >,
            parse::skipper_type
        > art_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            HullTypeStats (),
            qi::locals<
                double,
                double,
                double,
                double
            >,
            parse::skipper_type
        > hull_stats_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            bool (),
            parse::skipper_type
        > producible_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            HullType::Slot (),
            qi::locals<
                ShipSlotType,
                double,
                double
            >,
            parse::skipper_type
        > slot_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<HullType::Slot>&),
            parse::skipper_type
        > slots_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (Condition::ConditionBase*&),
            parse::skipper_type
        > location_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            PartHullCommonParams (),
            qi::locals<
                ValueRef::ValueRefBase<double>*,
                ValueRef::ValueRefBase<int>*,
                bool,
                std::set<std::string>,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > part_hull_common_params_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, HullType*>&),
            qi::locals<
                std::string,
                std::string,
                HullTypeStats,
                PartHullCommonParams,
                std::vector<HullType::Slot>
            >,
            parse::skipper_type
        > hull_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, HullType*>&),
            parse::skipper_type
        > start_rule;

        hull_prefix_rule                hull_prefix;
        hull_stats_rule                 hull_stats;
        producible_rule                 producible;
        slot_rule                       slot;
        slots_rule                      slots;
        location_rule                   location;
        part_hull_common_params_rule    common_params;
        hull_rule                       hull;
        art_rule                        art;
        start_rule                      start;
    };
}

namespace parse {
    bool ship_hulls(const boost::filesystem::path& path, std::map<std::string, HullType*>& hulls)
    { return detail::parse_file<rules, std::map<std::string, HullType*> >(path, hulls); }
}
