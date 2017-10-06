#include "Parse.h"

#include "ParseImpl.h"

#include "../universe/Condition.h"
#include "../universe/Field.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<std::shared_ptr<Effect::EffectsGroup>>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, FieldType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, FieldType*>&) { return os; }
}
#endif

namespace {
    const boost::phoenix::function<parse::detail::is_unique> is_unique_;

    const boost::phoenix::function<parse::detail::insert> insert_;

    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::new_;

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

            const parse::lexer& tok = parse::lexer::instance();

            field
                =   tok.FieldType_
                >   parse::detail::label(Name_token)
                >   tok.string        [ _pass = is_unique_(_r1, FieldType_token, _1), _a = _1 ]
                >   parse::detail::label(Description_token)         > tok.string [ _b = _1 ]
                >   parse::detail::label(Stealth_token)             > parse::detail::double_ [ _c = _1]
                >   parse::detail::tags_parser()(_d)
                > -(parse::detail::label(EffectsGroups_token)       > parse::detail::effects_group_parser() [ _e = _1 ])
                >   parse::detail::label(Graphic_token)             > tok.string
                [ insert_(_r1, _a, new_<FieldType>(_a, _b, _c, _d, _e, _1)) ]
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
            void (std::map<std::string, FieldType*>&),
            boost::spirit::qi::locals<
                std::string,
                std::string,
                float,
                std::set<std::string>,
                std::vector<std::shared_ptr<Effect::EffectsGroup>>
            >
        > field_rule;

        typedef parse::detail::rule<
            void (std::map<std::string, FieldType*>&)
        > start_rule;

        field_rule          field;
        start_rule          start;
    };
}

namespace parse {
    bool fields(std::map<std::string, FieldType*>& field_types) {
        bool result = true;

        for (const boost::filesystem::path& file : ListScripts("scripting/fields")) {
            result &= detail::parse_file<rules, std::map<std::string, FieldType*>>(file, field_types);
        }

        return result;
    }
}
