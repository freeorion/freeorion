#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParams.h"

#include "../universe/Condition.h"
#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<HullType::Slot>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, HullType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, HullType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const HullType::Slot&) { return os; }
    inline ostream& operator<<(ostream& os, const HullTypeStats&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    const boost::phoenix::function<parse::detail::insert> insert_;

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
            using phoenix::new_;
            using phoenix::push_back;
            using phoenix::insert;

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
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;

            const parse::lexer& tok = parse::lexer::instance();

            hull_stats
                =   parse::detail::label(Speed_token)       >   parse::detail::double_ [ _a = _1 ]
                >   parse::detail::label(Fuel_token)        >   parse::detail::double_ [ _c = _1 ]
                >   parse::detail::label(Stealth_token)     >   parse::detail::double_ [ _d = _1 ]
                >   parse::detail::label(Structure_token)   >   parse::detail::double_
                    [ _val = construct<HullTypeStats>(_c, _a, _d, _1) ]
                ;

            slot
                =   tok.Slot_
                >   parse::detail::label(Type_token) > parse::ship_slot_type_enum() [ _a = _1 ]
                >   parse::detail::label(Position_token)
                >   '(' > parse::detail::double_ [ _b = _1 ] > ',' > parse::detail::double_ [ _c = _1 ] > lit(')')
                    [ _val = construct<HullType::Slot>(_a, _b, _c) ]
                ;

            slots
                =  -(
                        parse::detail::label(Slots_token)
                    >   (
                                ('[' > +slot [ push_back(_r1, _1) ] > ']')
                            |    slot [ push_back(_r1, _1) ]
                        )
                     )
                ;

            hull
                =   tok.Hull_
                >   parse::detail::more_common_params_parser()
                    [_pass = is_unique_(_r1, HullType_token, phoenix::bind(&MoreCommonParams::name, _1)), _a = _1 ]
                >   hull_stats                                  [ _c = _1 ]
                >  -slots(_e)
                >   parse::detail::common_params_parser()       [ _d = _1 ]
                >   parse::detail::label(Icon_token)    > tok.string    [ _f = _1 ]
                >   parse::detail::label(Graphic_token) > tok.string
                [ insert_(_r1, phoenix::bind(&MoreCommonParams::name, _a),
                          new_<HullType>(_c, _d, _a, _e, _f, _1)) ]
                ;

            start
                =   +hull(_r1)
                ;

            hull_stats.name("Hull stats");
            slot.name("Slot");
            slots.name("Slots");
            hull.name("Hull");

#if DEBUG_PARSERS
            debug(cost);
            debug(hull_stats);
            debug(slot);
            debug(slots);
            debug(hull);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            HullTypeStats (),
            boost::spirit::qi::locals<
                double,
                double,
                double,
                double
            >
        > hull_stats_rule;

        typedef parse::detail::rule<
            HullType::Slot (),
            boost::spirit::qi::locals<
                ShipSlotType,
                double,
                double
            >
        > slot_rule;

        typedef parse::detail::rule<
            void (std::vector<HullType::Slot>&)
        > slots_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, HullType*>&),
            boost::spirit::qi::locals<
                MoreCommonParams,
                std::string,    // dummy
                HullTypeStats,
                CommonParams,
                std::vector<HullType::Slot>,
                std::string
            >
        > hull_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, HullType*>&)
        > start_rule;

        hull_stats_rule                             hull_stats;
        slot_rule                                   slot;
        slots_rule                                  slots;
        hull_rule                                   hull;
        start_rule                                  start;
    };
}

namespace parse {
    bool ship_hulls(std::map<std::string, HullType*>& hulls) {
        bool result = true;

        for (const boost::filesystem::path& file : ListScripts("scripting/ship_hulls")) {
            result &= detail::parse_file<rules, std::map<std::string, HullType*>>(file, hulls);
        }

        return result;
    }
}
