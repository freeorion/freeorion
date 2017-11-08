#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParams.h"

#include "../universe/ValueRef.h"
#include "../universe/Condition.h"
#include "../universe/ShipDesign.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<HullType::Slot>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<HullType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<HullType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const HullType::Slot&) { return os; }
    inline ostream& operator<<(ostream& os, const HullTypeStats&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_hulltype(std::map<std::string, std::unique_ptr<HullType>>& hulltypes,
                         const HullTypeStats& stats,
                         const std::unique_ptr<CommonParams>& common_params,
                         const MoreCommonParams& more_common_params,
                         const std::vector<HullType::Slot>& slots,
                         const std::string& icon, const std::string& graphic)
    {
        auto hulltype = boost::make_unique<HullType>(
            stats, std::move(*common_params), more_common_params, slots, icon, graphic);
        hulltypes.emplace(hulltype->Name(), std::move(hulltype));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_hulltype_, insert_hulltype, 7)

    using start_rule_payload = std::map<std::string, std::unique_ptr<HullType>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            tags_parser(tok, labeller),
            common_rules(tok, labeller, condition_parser, string_grammar, tags_parser),
            ship_slot_type_enum(tok),
            double_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;
            using phoenix::push_back;

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
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            hull_stats
                =   labeller.rule(Speed_token)       >   double_rule [ _a = _1 ]
                >   labeller.rule(Fuel_token)        >   double_rule [ _c = _1 ]
                >   labeller.rule(Stealth_token)     >   double_rule [ _d = _1 ]
                >   labeller.rule(Structure_token)   >   double_rule
                    [ _val = construct<HullTypeStats>(_c, _a, _d, _1) ]
                ;

            slot
                =   tok.Slot_
                >   labeller.rule(Type_token) > ship_slot_type_enum [ _a = _1 ]
                >   labeller.rule(Position_token)
                >   '(' > double_rule [ _b = _1 ] > ',' > double_rule [ _c = _1 ] > lit(')')
                    [ _val = construct<HullType::Slot>(_a, _b, _c) ]
                ;

            slots
                =  -(
                        labeller.rule(Slots_token)
                    >   (
                                ('[' > +slot [ push_back(_r1, _1) ] > ']')
                            |    slot [ push_back(_r1, _1) ]
                        )
                     )
                ;

            hull
                =   tok.Hull_
                >   common_rules.more_common
                    [_pass = is_unique_(_r1, HullType_token, phoenix::bind(&MoreCommonParams::name, _1)), _a = _1 ]
                >   hull_stats                                  [ _c = _1 ]
                >  -slots(_e)
                >   common_rules.common       [ _d = _1 ]
                >   labeller.rule(Icon_token)    > tok.string    [ _f = _1 ]
                >   labeller.rule(Graphic_token) > tok.string
                [ insert_hulltype_(_r1, _c,
                                   deconstruct_movable_(_d, _pass),
                                   _a, _e, _f, _1) ]
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
            void (std::map<std::string, std::unique_ptr<HullType>>&),
            boost::spirit::qi::locals<
                MoreCommonParams,
                std::string,    // dummy
                HullTypeStats,
                parse::detail::MovableEnvelope<CommonParams>,
                std::vector<HullType::Slot>,
                std::string
            >
        > hull_rule;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller labeller;
        parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::common_params_rules common_rules;
        parse::ship_slot_enum_grammar ship_slot_type_enum;
        parse::detail::double_grammar double_rule;
        hull_stats_rule                             hull_stats;
        slot_rule                                   slot;
        slots_rule                                  slots;
        hull_rule                                   hull;
        start_rule                                  start;
    };
}

namespace parse {
    start_rule_payload ship_hulls(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload hulls;

        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, hulls);
        }

        return hulls;
    }
}
