#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

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

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_parttype(std::map<std::string, std::unique_ptr<PartType>>& part_types,
                         ShipPartClass part_class,
                         std::pair<boost::optional<double>, boost::optional<double>> capacity_and_stat2,
                         const parse::detail::MovableEnvelope<CommonParams>& common_params,
                         const MoreCommonParams& more_common_params,
                         boost::optional<std::vector<ShipSlotType>> mountable_slot_types,
                         const std::string& icon,
                         boost::optional<const char*> add_standard_capacity_effect,
                         bool& pass)
    {
        auto part_type = boost::make_unique<PartType>(
            part_class,
            (capacity_and_stat2.first ? *capacity_and_stat2.first : 0.0),
            (capacity_and_stat2.second ? *capacity_and_stat2.second : 1.0),
            *common_params.OpenEnvelope(pass), more_common_params,
            (mountable_slot_types ? *mountable_slot_types : std::vector<ShipSlotType>()),
            icon,
            (add_standard_capacity_effect ? false : true));

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
            labeller(tok),
            condition_parser(tok, labeller),
            string_grammar(tok, labeller, condition_parser),
            tags_parser(tok, labeller),
            common_rules(tok, labeller, condition_parser, string_grammar, tags_parser),
            ship_slot_type_enum(tok),
            ship_part_class_enum(tok),
            double_rule(tok),
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
            qi::_pass_type _pass;
            qi::_r1_type _r1;
            qi::omit_type omit_;

            part_type
                = ( omit_[tok.Part_]
                >   common_rules.more_common
                    [_pass = is_unique_(_r1, PartType_token, phoenix::bind(&MoreCommonParams::name, _1)) ]
                >   labeller.rule(Class_token)       > ship_part_class_enum
                > -(  (labeller.rule(Capacity_token)  > double_rule)
                   | (labeller.rule(Damage_token)    > double_rule)
                   )
                > -(  (labeller.rule(Damage_token)    > double_rule )   // damage is secondary for fighters
                   | (labeller.rule(Shots_token)     > double_rule )   // shots is secondary for direct fire weapons
                   )
                > -tok.NoDefaultCapacityEffect_
                > -(labeller.rule(MountableSlotTypes_token) > one_or_more_slots)
                >   common_rules.common
                >   labeller.rule(Icon_token)        > tok.string
                  ) [ insert_parttype_(_r1, _2,
                                       construct<std::pair<boost::optional<double>, boost::optional<double>>>(_3, _4)
                                       , _7, _1, _6, _8, _5, _pass) ]
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

        parse::detail::Labeller labeller;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::common_params_rules common_rules;
        parse::ship_slot_enum_grammar  ship_slot_type_enum;
        parse::ship_part_class_enum_grammar ship_part_class_enum;
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
