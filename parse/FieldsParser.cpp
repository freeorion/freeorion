#include "Parse.h"

#include "ParseImpl.h"
#include "EffectParser.h"

#include "../universe/Effect.h"
#include "../universe/FieldType.h"
#include "../util/Directories.h"

#include <boost/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const parse::effects_group_payload&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<FieldType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<FieldType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_fieldtype(std::map<std::string, std::unique_ptr<FieldType>, std::less<>>& fieldtypes,
                          std::string& name, std::string& description,
                          float stealth, std::set<std::string>& tags,
                          boost::optional<parse::effects_group_payload>& effects,
                          std::string& graphic,
                          bool& pass)
    {
        auto fieldtype_ptr = std::make_unique<FieldType>(
            std::string(name), std::move(description), stealth, std::move(tags),
            (effects ? OpenEnvelopes(*effects, pass) : std::vector<std::unique_ptr<Effect::EffectsGroup>>{}),
            std::move(graphic));

        fieldtypes.emplace(std::move(name), std::move(fieldtype_ptr));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_fieldtype_, insert_fieldtype, 8)

    using start_rule_payload = std::map<std::string, std::unique_ptr<FieldType>, std::less<>>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start),
            condition_parser(tok, label),
            string_grammar(tok, label, condition_parser),
            effects_group_grammar(tok, label, condition_parser, string_grammar),
            tags_parser(tok, label),
            double_rule(tok)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_5_type _5;
            qi::_6_type _6;
            qi::_7_type _7;
            qi::_pass_type _pass;
            qi::_r1_type _r1;

            field
                = ( tok.FieldType_
                >   label(tok.name_)
                >   tok.string
                >   label(tok.description_)         > tok.string
                >   label(tok.stealth_)             > double_rule
                >   tags_parser
                > -(label(tok.effectsgroups_)       > effects_group_grammar )
                >   label(tok.graphic_)             > tok.string )
                [ _pass = is_unique_(_r1, _1, _2),
                  insert_fieldtype_(_r1, _2, _3, _4, _5, _6, _7, _pass) ]
                ;

            start
                =   +field(_r1)
                ;

            field.name("FieldType");

#if DEBUG_PARSERS
            debug(field);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using field_rule = parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<FieldType>, std::less<>>&)
            >;

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::effects_group_grammar effects_group_grammar;
        parse::detail::tags_grammar tags_parser;
        parse::detail::double_grammar double_rule;
        field_rule          field;
        start_rule          start;
    };
}

namespace parse {
    start_rule_payload fields(const boost::filesystem::path& path) {
        start_rule_payload field_types;

        ScopedTimer timer("Fields Parsing");

        for (const auto& file : ListDir(path, IsFOCScript))
            detail::parse_file<grammar, start_rule_payload>(GetLexer(), file, field_types);

        return field_types;
    }
}
