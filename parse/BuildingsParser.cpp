#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "ConditionParserImpl.h"
#include "ValueRefParser.h"
#include "CommonParamsParser.h"

#include "../universe/Building.h"
#include "../universe/Enums.h"
#include "../universe/ValueRef.h"

//TODO: replace with std::make_unique when transitioning to C++14
#include <boost/smart_ptr/make_unique.hpp>

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

    void insert_building(std::map<std::string, std::unique_ptr<BuildingType>>& building_types,
                         const std::string& name,
                         const std::string& description,
                         const parse::detail::MovableEnvelope<CommonParams>& common_params,
                         CaptureResult& capture_result,
                         const std::string& icon,
                         bool& pass)
    {
        auto building_type = boost::make_unique<BuildingType>(
            name, description, *common_params.OpenEnvelope(pass), capture_result, icon);

        building_types.insert(std::make_pair(building_type->Name(), std::move(building_type)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_building_, insert_building, 7)

    using start_rule_payload = std::map<std::string, std::unique_ptr<BuildingType>>;
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
                (label(tok.CaptureResult_) >> capture_result_enum)
                | eps [ _val = CR_CAPTURE ]
                ;

            building_type
                = ( tok.BuildingType_
                >   label(tok.Name_)        > tok.string
                >   label(tok.Description_) > tok.string
                >   capture
                >   common_rules.common
                >   label(tok.Icon_)        > tok.string)
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
            void (std::map<std::string, std::unique_ptr<BuildingType>>&)>;

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
        const lexer lexer;
        start_rule_payload building_types;
        for (const boost::filesystem::path& file : ListScripts(path)) {
            /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, file, building_types);
        }

        return building_types;
    }
}
