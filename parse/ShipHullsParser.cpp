#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

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
                         const boost::optional<std::vector<HullType::Slot>>& slots,
                         const std::string& icon, const std::string& graphic)
    {
        auto hulltype = boost::make_unique<HullType>(
            stats, std::move(*common_params), more_common_params,
            (slots ? *slots : std::vector<HullType::Slot>()),
            icon, graphic);
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
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            common_rules(tok, label, condition_parser, string_grammar, tags_parser),
            ship_slot_type_enum(tok),
            double_rule(tok),
            one_or_more_slots(slot)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::construct;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_r1_type _r1;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            qi::omit_type omit_;
            const boost::phoenix::function<parse::detail::deconstruct_movable> deconstruct_movable_;

            hull_stats
                =  (label(tok.Speed_)       >   double_rule
                >   label(tok.Fuel_)        >   double_rule
                >   label(tok.Stealth_)     >   double_rule
                >   label(tok.Structure_)   >   double_rule)
                    [ _val = construct<HullTypeStats>(_2, _1, _3, _4) ]
                ;

            slot
                =  (omit_[tok.Slot_]
                >   label(tok.Type_) > ship_slot_type_enum
                >   label(tok.Position_)
                >   '(' > double_rule > ',' > double_rule > lit(')'))
                    [ _val = construct<HullType::Slot>(_1, _2, _3) ]
                ;

            hull
                =   (tok.Hull_
                >   common_rules.more_common
                >   hull_stats
                >  -(label(tok.Slots_) > one_or_more_slots)
                >   common_rules.common
                >   label(tok.Icon_)    > tok.string
                >   label(tok.Graphic_) > tok.string)
                [ _pass = is_unique_(_r1, _1, phoenix::bind(&MoreCommonParams::name, _2)),
                  insert_hulltype_(_r1, _3,
                                   deconstruct_movable_(_5, _pass),
                                   _2, _4, _6, _7) ]
                ;

            start
                =   +hull(_r1)
                ;

            hull_stats.name("Hull stats");
            slot.name("Slot");
            hull.name("Hull");

#if DEBUG_PARSERS
            debug(cost);
            debug(hull_stats);
            debug(slot);
            debug(hull);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using hull_stats_rule = parse::detail::rule<HullTypeStats ()>;

        using slot_rule =  parse::detail::rule<HullType::Slot ()>;

        using hull_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<HullType>>&)
        >;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::common_params_rules common_rules;
        parse::ship_slot_enum_grammar ship_slot_type_enum;
        parse::detail::double_grammar double_rule;
        hull_stats_rule                             hull_stats;
        slot_rule                                   slot;
        parse::detail::single_or_bracketed_repeat<slot_rule> one_or_more_slots;
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
