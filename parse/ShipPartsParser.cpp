#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ValueRefParser.h"
#include "ConditionParserImpl.h"
#include "CommonParamsParser.h"

#include "../universe/ShipDesign.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"

#include <boost/spirit/include/phoenix.hpp>
//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ShipSlotType>&) { return os; }
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<PartType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<PartType>>&) { return os; }
}
#endif

namespace parse {
    namespace detail {
        typedef std::tuple<boost::optional<double>, boost::optional<double>, boost::optional<int>, boost::optional<parse::detail::MovableEnvelope<Condition::ConditionBase>>> OptCap_OptStat2_OptPrecision_OptMoveablePreferred;
    }
}

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;


    void insert_parttype(std::map<std::string, std::unique_ptr<PartType>>& part_types,
                         ShipPartClass part_class,
                         const parse::detail::OptCap_OptStat2_OptPrecision_OptMoveablePreferred& capacity_and_stat2_and,
                         const parse::detail::MovableEnvelope<CommonParams>& common_params,
                         const MoreCommonParams& more_common_params,
                         boost::optional<std::vector<ShipSlotType>> mountable_slot_types,
                         const std::string& icon,
                         bool no_default_capacity_effect,
                         bool& pass)
    {
        boost::optional<double> capacity, stat2;
        boost::optional<int> precision;
        boost::optional<parse::detail::MovableEnvelope<Condition::ConditionBase>> preferredPrey;
        std::tie(capacity, stat2, precision, preferredPrey) = capacity_and_stat2_and;

        auto part_type = boost::make_unique<PartType>(
            part_class,
            (capacity  ? *capacity  : 0.0),
            (stat2 ? *stat2 : 1.0),
            *common_params.OpenEnvelope(pass), more_common_params,
            (mountable_slot_types ? *mountable_slot_types : std::vector<ShipSlotType>()),
            icon,
            !no_default_capacity_effect,
            (precision ? *precision : 1),
            (preferredPrey ? (*preferredPrey).OpenEnvelope(pass) : nullptr)
        );

        part_types.insert(std::make_pair(part_type->Name(), std::move(part_type)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_parttype_, insert_parttype, 9)

    using start_rule_payload = std::map<std::string, std::unique_ptr<PartType>>;
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
            ship_part_class_enum(tok),
            double_rule(tok),
            int_rule(tok),
            one_or_more_slots(ship_slot_type_enum)
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
            phoenix::actor<boost::spirit::argument<9>> _10; // qi::_10_type is not predefined
            phoenix::actor<boost::spirit::argument<10>> _11; // qi::_11_type is not predefined

            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::matches_type matches_;

            part_type
                = ( tok.Part_
                >   common_rules.more_common
                >   label(tok.Class_)       > ship_part_class_enum
                > -(  (label(tok.Capacity_)  > double_rule)
                   | (label(tok.Damage_)    > double_rule)
                   )
                > -(  (label(tok.Damage_)    > double_rule )   // damage is secondary for fighters
                   | (label(tok.Shots_)     > double_rule )   // shots is secondary for direct fire weapons
                   )
                > matches_[tok.NoDefaultCapacityEffect_]
                > -(label(tok.MountableSlotTypes_) > one_or_more_slots)
                >   common_rules.common
                >   label(tok.Icon_)        > tok.string
                > -(label(tok.Precision_) > int_rule)
                > -(label(tok.PreferredPrey_) > condition_parser)
                  ) [ _pass = is_unique_(_r1, _1, phoenix::bind(&MoreCommonParams::name, _2)),
                      insert_parttype_(_r1, _3,
                                       construct<parse::detail::OptCap_OptStat2_OptPrecision_OptMoveablePreferred>(_4, _5, _10, _11)
                                       , _8, _2, _7, _9, _6, _pass) ]
                ;

            start
                =   +part_type(_r1)
                ;

            part_type.name("Part");

#if DEBUG_PARSERS
            debug(part_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using  part_type_rule = parse::detail::rule<void (start_rule_payload&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::common_params_rules common_rules;
        parse::ship_slot_enum_grammar  ship_slot_type_enum;
        parse::ship_part_class_enum_grammar ship_part_class_enum;
        parse::detail::int_grammar int_rule;
        parse::detail::double_grammar double_rule;
        parse::detail::single_or_bracketed_repeat<parse::ship_slot_enum_grammar> one_or_more_slots;
        part_type_rule                     part_type;
        start_rule                         start;
    };

}

namespace parse {
    start_rule_payload ship_parts(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload parts;

        for (const auto& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, parts);
        }

        return parts;
    }
}
