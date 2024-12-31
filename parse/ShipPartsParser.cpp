#define FUSION_MAX_VECTOR_SIZE 15
#define PHOENIX_LIMIT 12
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ConditionParserImpl.h"
#include "CommonParamsParser.h"

#include "../universe/Condition.h"
#include "../universe/ShipPart.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<ShipPart>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<ShipPart>>&) { return os; }
}
#endif

namespace parse {
    namespace detail {
        typedef std::tuple<
            boost::optional<double>,
            boost::optional<double>,
            boost::optional<parse::detail::MovableEnvelope<Condition::Condition>>,
            boost::optional<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>>,
            boost::optional<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>>
        > OptCap_OptStat2_OptMovTargets_OptMovFighterDam_OptMovShipDam;
    }
}

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_shippart(std::map<std::string, std::unique_ptr<ShipPart>, std::less<>>& ship_parts,
                         ShipPartClass part_class,
                         const parse::detail::OptCap_OptStat2_OptMovTargets_OptMovFighterDam_OptMovShipDam& capacity__stat2__targets__fighterdam__shipdam,
                         parse::detail::MovableEnvelope<CommonParams>& common_params,
                         parse::detail::MoreCommonParams& more_common_params,
                         boost::optional<std::vector<ShipSlotType>> mountable_slot_types,
                         std::string& icon,
                         bool no_default_capacity_effect,
                         bool& pass)
    {
        boost::optional<double> capacity, stat2;
        boost::optional<parse::detail::MovableEnvelope<Condition::Condition>> combat_targets;
        boost::optional<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>> total_fighter_damage;
        boost::optional<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>> total_ship_damage;
        std::tie(capacity, stat2, combat_targets, total_fighter_damage, total_ship_damage) = capacity__stat2__targets__fighterdam__shipdam;


        auto ship_part = std::make_unique<ShipPart>(
            part_class,
            (capacity ? *capacity : 0.0),
            (stat2 ? *stat2 : 1.0),
            std::move(*common_params.OpenEnvelope(pass)),
            std::move(more_common_params.name),
            std::move(more_common_params.description),
            std::move(more_common_params.exclusions),
            (mountable_slot_types ? std::move(*mountable_slot_types) : std::vector<ShipSlotType>{}),
            std::move(icon),
            !no_default_capacity_effect,
            (combat_targets ? (*combat_targets).OpenEnvelope(pass) : nullptr),
            (total_fighter_damage ? (*total_fighter_damage).OpenEnvelope(pass) : nullptr),
            (total_ship_damage ? (*total_ship_damage).OpenEnvelope(pass) : nullptr));

        auto& part_name{ship_part->Name()};
        ship_parts.emplace(part_name, std::move(ship_part));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_shippart_, insert_shippart, 9)

    using start_rule_payload = std::map<std::string, std::unique_ptr<ShipPart>, std::less<>>;
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
            ship_part_class_enum(tok),
            double_rule(tok),
            one_or_more_slots(ship_slot_type_enum),
            double_rules(tok, label, condition_parser, string_grammar)
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
            qi::_9_type _9;
            phoenix::actor<boost::spirit::argument<9>>  _10; // qi::_10_type is not predefined
            phoenix::actor<boost::spirit::argument<10>> _11; // qi::_11_type is not predefined
            phoenix::actor<boost::spirit::argument<11>> _12; // qi::_12_type is not predefined
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::matches_type matches_;

            ship_part
                = ( tok.Part_                                       // _1
                >   common_rules.more_common                        // _2
                >   label(tok.class_)       > ship_part_class_enum  // _3
                > -( (label(tok.capacity_)  > double_rule)          // _4
                   | (label(tok.damage_)    > double_rule)          // _4
                   )
                > -( (label(tok.damage_)    > double_rule )         // _5 : damage is secondary stat for fighters
                   | (label(tok.shots_)     > double_rule )         // _5 : shots is secondary stat for direct fire weapons
                   )
                > matches_[tok.NoDefaultCapacityEffect_]                // _6
//                > -(label(tok.value_)       > qi::as<parse::detail::MovableEnvelope<ValueRef::ValueRef<double>>>()[double_rules.expr]) // _7
                > -(label(tok.destroyFightersPerBattleMax_) > double_rules.expr) // _7
                > -(label(tok.damageStructurePerBattleMax_) > double_rules.expr) // _8
                > -(label(tok.combatTargets_)       > condition_parser) // _9
                > -(label(tok.mountableSlotTypes_)  > one_or_more_slots)// _10
                >   common_rules.common                                 // _11
                >   label(tok.icon_)        > tok.string                // _12
                  ) [ _pass = is_unique_(_r1, _1, phoenix::bind(&parse::detail::MoreCommonParams::name, _2)),
                      insert_shippart_(
                          _r1, _3,
                          construct<parse::detail::OptCap_OptStat2_OptMovTargets_OptMovFighterDam_OptMovShipDam>(_4, _5, _9, _7, _8),
                          _11, _2, _10, _12, _6, _pass) ]
                ;

            start
                =   +ship_part(_r1)
                ;

            ship_part.name("Part");

#if DEBUG_PARSERS
            debug(ship_part);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using ship_part_rule = parse::detail::rule<void (start_rule_payload&)>;
        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                 label;
        const parse::conditions_parser_grammar  condition_parser;
        const parse::string_parser_grammar      string_grammar;
        parse::detail::tags_grammar             tags_parser;
        parse::detail::common_params_rules      common_rules;
        parse::ship_slot_enum_grammar           ship_slot_type_enum;
        parse::ship_part_class_enum_grammar     ship_part_class_enum;
        parse::detail::double_grammar           double_rule;
        parse::detail::single_or_bracketed_repeat<parse::ship_slot_enum_grammar>
                                                one_or_more_slots;
        parse::double_parser_rules              double_rules;
        ship_part_rule                          ship_part;
        start_rule                              start;
    };
}

namespace parse {
    start_rule_payload ship_parts(const boost::filesystem::path& path) {
        start_rule_payload parts;

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, parts);

        return parts;
    }
}
