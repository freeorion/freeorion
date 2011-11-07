#define PHOENIX_LIMIT 11

#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"
#include "../universe/Condition.h"


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

    struct insert_
    {
        template <typename Arg1, typename Arg2>
        struct result
        { typedef void type; };

        void operator()(std::map<std::string, HullType*>& hulls, HullType* hull) const
            {
                if (!hulls.insert(std::make_pair(hull->Name(), hull)).second) {
                    std::string error_str = "ERROR: More than one ship hull in ship_hulls.txt has the name " + hull->Name();
                    throw std::runtime_error(error_str.c_str());
                }
            }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules
    {
        rules()
            {
                const parse::lexer& tok = parse::lexer::instance();

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
                qi::_g_type _g;
                qi::_h_type _h;
                qi::_i_type _i;
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
                    >    parse::label(Name_name)        > tok.string [ _r1 = _1 ]
                    >    parse::label(Description_name) > tok.string [ _r2 = _1 ]
                    ;

                cost
                    =    parse::label(BuildCost_name) > parse::double_ [ _r1 = _1 ]
                    >    parse::label(BuildTime_name) > parse::int_ [ _r2 = _1 ]
                    ;

                hull_stats
                    =    parse::label(Speed_name)         > parse::double_ [ _a = _1 ]
                    >    parse::label(StarlaneSpeed_name) > parse::double_ [ _b = _1 ]
                    >    parse::label(Fuel_name)          > parse::double_ [ _c = _1 ]
                    >    parse::label(Stealth_name)       > parse::double_ [ _d = _1 ]
                    >    parse::label(Structure_name)     > parse::double_ [ _val = construct<HullTypeStats>(_a, _b, _c, _d, _1) ]
                    ;

                producible
                    =    tok.Unproducible_ [ _val = false ]
                    |    tok.Producible_ [ _val = true ]
                    |    eps [ _val = true ]
                    ;

                slot
                    =    tok.Slot_
                    >    parse::label(Type_name) > parse::enum_parser<ShipSlotType>() [ _a = _1 ]
                    >    parse::label(Position_name)
                    >    '(' > parse::double_ [ _b = _1 ] > ',' > parse::double_ [ _c = _1 ] > lit(')')
                         [ _val = construct<HullType::Slot>(_a, _b, _c) ]
                    ;

                slots
                    =    parse::label(Slots_name)
                    >>   (
                              '[' > +slot [ push_back(_r1, _1) ] > ']'
                          |   slot [ push_back(_r1, _1) ]
                         )
                    ;

                location
                    =    parse::label(Location_name) >> parse::detail::condition_parser [ _r1 = _1 ]
                    |    eps [ _r1 = new_<Condition::All>() ]
                    ;

                hull
                    =    hull_prefix(_a, _b)
                    >    hull_stats [ _c = _1 ]
                    >    cost(_d, _e)
                    >    producible [ _f = _1 ]
                    >   -slots(_g)
                    >    location(_h)
                    >   -(
                              parse::label(EffectsGroups_name) >> parse::detail::effects_group_parser() [ _i = _1 ]
                         )
                    >    parse::label(Graphic_name) > tok.string [ insert(_r1, new_<HullType>(_a, _b, _c, _d, _e, _f, _g, _h, _i, _1)) ]
                    ;

                start
                    =   +hull(_r1)
                    ;

                hull_prefix.name("Hull");
                cost.name("build cost");
                hull_stats.name("Hull stats");
                producible.name("Producible or Unproducible");
                slot.name("Slot");
                slots.name("Slots");
                location.name("Location");
                hull.name("Hull");

#if DEBUG_PARSERS
                debug(hull_prefix);
                debug(cost);
                debug(hull_stats);
                debug(producible);
                debug(slot);
                debug(slots);
                debug(location);
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
            void (double&, int&),
            parse::skipper_type
        > cost_rule;

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
            void (std::map<std::string, HullType*>&),
            qi::locals<
                std::string,
                std::string,
                HullTypeStats,
                double,
                int,
                bool,
                std::vector<HullType::Slot>,
                Condition::ConditionBase*,
                std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > hull_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, HullType*>&),
            parse::skipper_type
        > start_rule;

        hull_prefix_rule hull_prefix;
        cost_rule cost;
        hull_stats_rule hull_stats;
        producible_rule producible;
        slot_rule slot;
        slots_rule slots;
        location_rule location;
        hull_rule hull;
        start_rule start;
    };

}

namespace parse {

    bool ship_hulls(const boost::filesystem::path& path, std::map<std::string, HullType*>& hulls)
    { return detail::parse_file<rules, std::map<std::string, HullType*> >(path, hulls); }

}
