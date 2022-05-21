#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

#include "../universe/BuildingType.h"
#include "../universe/Condition.h"
#include "../universe/ValueRef.h"
#include "../util/Directories.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<BuildingType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<BuildingType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_building(std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>& building_types,
                         std::string& name, std::string& description,
                         parse::detail::MovableEnvelope<CommonParams>& common_params,
                         CaptureResult& capture_result, std::string& icon,
                         bool& pass)
    {
        auto building_type = std::make_unique<BuildingType>(
            std::string(name), std::move(description),
            std::move(*common_params.OpenEnvelope(pass)),
            capture_result, std::move(icon));

        building_types.emplace(std::move(name), std::move(building_type));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_building_, insert_building, 7)

    using start_rule_payload = std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first,
                const parse::text_iterator& last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            tags_parser(tok, label),
            common_rules(tok, label, condition_parser, string_grammar, tags_parser),
            capture_result_enum(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_pass_type _pass;
            qi::_val_type _val;
            qi::_r1_type _r1;
            qi::eps_type eps;

            capture %=
                (label(tok.captureresult_) >> capture_result_enum)
                | eps [ _val = CaptureResult::CR_CAPTURE ]
                ;

            building_type
                = ( tok.BuildingType_                       // _1
                >   label(tok.name_)        > tok.string    // _2
                >   label(tok.description_) > tok.string    // _3
                >   capture                                 // _4
                >   common_rules.common                     // _5
                >   label(tok.icon_)        > tok.string)   // _6
                [ _pass = is_unique_(_r1, _1, _2),
                  insert_building_(_r1, _2, _3, _5, _4, _6, _pass) ]
                ;

            start
                =   +building_type(_r1)
                ;

            building_type.name("BuildingType");

#if DEBUG_PARSERS
            debug(building_type);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using building_type_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<BuildingType>, std::less<>>&)>;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller                 label;
        const parse::conditions_parser_grammar  condition_parser;
        const parse::string_parser_grammar      string_grammar;
        parse::detail::tags_grammar             tags_parser;
        parse::detail::common_params_rules      common_rules;
        parse::capture_result_enum_grammar      capture_result_enum;
        parse::detail::rule<CaptureResult ()>   capture;
        building_type_rule                      building_type;
        start_rule                              start;
    };
}

namespace parse {
    start_rule_payload buildings(const boost::filesystem::path& path) {
        start_rule_payload building_types;

        ScopedTimer timer("Buildings Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(lexer::tok, file, building_types);

        return building_types;
    }
}
