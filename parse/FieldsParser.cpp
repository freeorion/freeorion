#include "Parse.h"

#include "ParseImpl.h"
#include "EffectParser.h"

#include "../universe/Condition.h"
#include "../universe/Field.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, std::unique_ptr<FieldType>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, std::unique_ptr<FieldType>>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    void insert_fieldtype(std::map<std::string, std::unique_ptr<FieldType>>& fieldtypes,
                          const std::string& name, const std::string& description,
                          float stealth, const std::set<std::string>& tags,
                          const std::vector<std::shared_ptr<Effect::EffectsGroup>>& effects,
                          const std::string& graphic)
    {
        // TODO use make_unique when converting to C++14
        auto fieldtype_ptr = std::unique_ptr<FieldType>(
            new FieldType(name, description, stealth, tags, effects, graphic));

        fieldtypes.insert(std::make_pair(fieldtype_ptr->Name(), std::move(fieldtype_ptr)));
    }

    BOOST_PHOENIX_ADAPT_FUNCTION(void, insert_fieldtype_, insert_fieldtype, 7)

    struct rules {
        rules(const parse::lexer& tok,
              parse::detail::Labeller& labeller,
              const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last) :
            condition_parser(tok),
            string_grammar(tok, condition_parser),
            effects_group_grammar(tok, labeller, condition_parser, string_grammar)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_pass_type _pass;
            qi::_r1_type _r1;

            field
                =   tok.FieldType_
                >   labeller.rule(Name_token)
                >   tok.string        [ _pass = is_unique_(_r1, FieldType_token, _1), _a = _1 ]
                >   labeller.rule(Description_token)         > tok.string [ _b = _1 ]
                >   labeller.rule(Stealth_token)             > parse::detail::double_ [ _c = _1]
                >   parse::detail::tags_parser()(_d)
                > -(labeller.rule(EffectsGroups_token)       > effects_group_grammar [ _e = _1 ])
                >   labeller.rule(Graphic_token)             > tok.string
                [ insert_fieldtype_(_r1, _a, _b, _c, _d, _e, _1) ]
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

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<FieldType>>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                float,
                std::set<std::string>,
                std::vector<std::shared_ptr<Effect::EffectsGroup>>
            >
        > field_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, std::unique_ptr<FieldType>>&)
        > start_rule;

        const parse::conditions_parser_grammar condition_parser;
        const parse::string_parser_grammar string_grammar;
        parse::effects_group_grammar effects_group_grammar;
        field_rule          field;
        start_rule          start;
    };
}

namespace parse {
    std::map<std::string, std::unique_ptr<FieldType>> fields() {
        std::map<std::string, std::unique_ptr<FieldType>> field_types;

        for (const boost::filesystem::path& file : ListScripts("scripting/fields")) {
            /*auto success =*/ detail::parse_file<rules, std::map<std::string, std::unique_ptr<FieldType>>>(file, field_types);
        }

        return field_types;
    }
}
