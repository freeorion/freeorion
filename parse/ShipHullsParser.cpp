#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

#include "../universe/Condition.h"
#include "../universe/ShipHull.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipHull::Slot>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<ShipHull>>&) { return os; }
    inline ostream& operator<<(ostream& os, const ShipHull::Slot&) { return os; }
}
#endif

namespace {
struct ShipHullStats {
    ShipHullStats() = default;

    ShipHullStats(float fuel_,
                  float speed_,
                  float stealth_,
                  float structure_,
                  bool default_fuel_effects_,
                  bool default_speed_effects_,
                  bool default_stealth_effects_,
                  bool default_structure_effects_) :
        fuel(fuel_),
        speed(speed_),
        stealth(stealth_),
        structure(structure_),
        default_fuel_effects(default_fuel_effects_),
        default_speed_effects(default_speed_effects_),
        default_stealth_effects(default_stealth_effects_),
        default_structure_effects(default_structure_effects_)
    {}

    float   fuel = 0.0f;
    float   speed = 0.0f;
    float   stealth = 0.0f;
    float   structure = 0.0f;
    bool    default_fuel_effects = true;
    bool    default_speed_effects = true;
    bool    default_stealth_effects = true;
    bool    default_structure_effects = true;
};

    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_shiphull(std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>& shiphulls,
                         ShipHullStats& stats,
                         parse::detail::MovableEnvelope<CommonParams>& common_params,
                         parse::detail::MoreCommonParams& more_common_params,
                         boost::optional<std::vector<ShipHull::Slot>>& slots,
                         std::string& icon, std::string& graphic,
                         bool& pass)
    {
        auto shiphull = std::make_unique<ShipHull>(
            stats.fuel,
            stats.speed,
            stats.stealth,
            stats.structure,
            stats.default_fuel_effects,
            stats.default_speed_effects,
            stats.default_stealth_effects,
            stats.default_structure_effects,
            std::move(*common_params.OpenEnvelope(pass)),
            std::move(more_common_params.name),
            std::move(more_common_params.description),
            std::move(more_common_params.exclusions),
            (slots ? std::move(*slots) : std::vector<ShipHull::Slot>{}),
            std::move(icon),
            std::move(graphic));

        auto& hull_name{shiphull->Name()};
        shiphulls.emplace(hull_name, std::move(shiphull));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_shiphull_, insert_shiphull, 8)

    using start_rule_payload = std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
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
            qi::_8_type _8;
            qi::_r1_type _r1;
            qi::matches_type matches_;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::eps_type eps;
            qi::lit_type lit;
            qi::omit_type omit_;

            hull_stats
                =  (label(tok.speed_)       >   double_rule // _1
                >   matches_[tok.NoDefaultSpeedEffect_]     // _2
                >   label(tok.fuel_)        >   double_rule // _3
                >   matches_[tok.NoDefaultFuelEffect_]      // _4
                >   label(tok.stealth_)     >   double_rule // _5
                >   matches_[tok.NoDefaultStealthEffect_]   // _6
                >   label(tok.structure_)   >   double_rule // _7
                >   matches_[tok.NoDefaultStructureEffect_])// _8
                    [ _val = construct<ShipHullStats>(_3, _1, _5, _7, !_4, !_2, !_6, !_8) ]
                ;

            slot
                =  (omit_[tok.Slot_]
                >   label(tok.type_) > ship_slot_type_enum
                >   label(tok.position_)
                >   '(' > double_rule > ',' > double_rule > lit(')'))
                    [ _val = construct<ShipHull::Slot>(_1, _2, _3) ]
                ;

            hull
                =   (tok.Hull_                              // _1
                >   common_rules.more_common                // _2
                >   hull_stats                              // _3
                >  -(label(tok.slots_) > one_or_more_slots) // _4
                >   common_rules.common                     // _5
                >   label(tok.icon_)    > tok.string        // _6
                >   label(tok.graphic_) > tok.string)       // _7
                [ _pass = is_unique_(_r1, _1, phoenix::bind(&parse::detail::MoreCommonParams::name, _2)),
                  insert_shiphull_(_r1, _3, _5, _2, _4, _6, _7, _pass) ]
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

        using hull_stats_rule = parse::detail::rule<ShipHullStats ()>;

        using slot_rule =  parse::detail::rule<ShipHull::Slot ()>;

        using hull_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<ShipHull>, std::less<>>&)
        >;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller             label;
        parse::conditions_parser_grammar    condition_parser;
        const parse::string_parser_grammar  string_grammar;
        parse::detail::tags_grammar         tags_parser;
        parse::detail::common_params_rules  common_rules;
        parse::ship_slot_enum_grammar       ship_slot_type_enum;
        parse::detail::double_grammar       double_rule;
        hull_stats_rule                     hull_stats;
        slot_rule                           slot;
        parse::detail::single_or_bracketed_repeat<slot_rule> one_or_more_slots;
        hull_rule                           hull;
        start_rule                          start;
    };
}

namespace parse {
    start_rule_payload ship_hulls(const boost::filesystem::path& path) {
        start_rule_payload hulls;

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, hulls);

        return hulls;
    }
}
