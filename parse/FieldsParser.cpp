#include "Double.h"
#include "Label.h"
#include "Parse.h"
#include "ParseImpl.h"

#include "../universe/Condition.h"
#include "../universe/Field.h"

#include <boost/spirit/include/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<boost::shared_ptr<Effect::EffectsGroup> >&) { return os; }
    inline ostream& operator<<(ostream& os, const std::map<std::string, FieldType*>&) { return os; }
    inline ostream& operator<<(ostream& os, const std::pair<const std::string, FieldType*>&) { return os; }
}
#endif

namespace {
    struct insert_ {
#if BOOST_VERSION < 105600
        template <typename Arg1, typename Arg2> // Phoenix v2
        struct result
        { typedef void type; };
#else
        typedef void result_type;
#endif

        void operator()(std::map<std::string, FieldType*>& fields, FieldType* field_type) const {
            if (!fields.insert(std::make_pair(field_type->Name(), field_type)).second) {
                std::string error_str = "ERROR: More than one field type in fields.txt has the name " + field_type->Name();
                throw std::runtime_error(error_str.c_str());
            }
        }
    };
    const boost::phoenix::function<insert_> insert;

    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_a_type _a;
            qi::_b_type _b;
            qi::_c_type _c;
            qi::_d_type _d;
            qi::_e_type _e;
            qi::_r1_type _r1;
            using phoenix::new_;

            field
                =   tok.FieldType_
                >   parse::label(Name_token)                > tok.string [ _a = _1 ]
                >   parse::label(Description_token)         > tok.string [ _b = _1 ]
                >   parse::label(Stealth_token)             > parse::double_ [ _c = _1]
                >   parse::detail::tags_parser()(_d)
                > -(parse::label(EffectsGroups_token)       > parse::detail::effects_group_parser() [ _e = _1 ])
                >   parse::label(Graphic_token)             > tok.string
                [ insert(_r1, new_<FieldType>(_a, _b, _c, _d, _e, _1)) ]
                ;

            start
                =   +field(_r1)
                ;

            field.name("FieldType");

#if DEBUG_PARSERS
            debug(field);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, FieldType*>&),
            qi::locals<
                std::string,
                std::string,
                float,
                std::set<std::string>,
                std::vector<boost::shared_ptr<Effect::EffectsGroup> >
            >,
            parse::skipper_type
        > field_rule;

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::map<std::string, FieldType*>&),
            parse::skipper_type
        > start_rule;

        field_rule          field;
        start_rule          start;
    };
}

namespace parse {
    bool fields(const boost::filesystem::path& path, std::map<std::string, FieldType*>& field_types)
    { return detail::parse_file<rules, std::map<std::string, FieldType*> >(path, field_types); }
}
