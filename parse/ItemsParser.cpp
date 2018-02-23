#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "../universe/Tech.h"

#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
}
#endif

namespace {
    using start_rule_payload = std::vector<ItemSpec>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            item_spec_parser(tok, label)
        {
            namespace phoenix = boost::phoenix;
            namespace qi = boost::spirit::qi;

            using phoenix::push_back;

            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;

            start
                =   +item_spec_parser [ push_back(_r1, _1) ]
                ;

            start.name("start");

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        parse::detail::item_spec_grammar item_spec_parser;
        start_rule start;
    };
}

namespace parse {
    start_rule_payload items(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload items_;
        /*auto success =*/ detail::parse_file<grammar, start_rule_payload>(lexer, path, items_);
        return items_;
    }

    start_rule_payload starting_buildings(const boost::filesystem::path& path) {
        const lexer lexer;
        start_rule_payload starting_buildings_;
        /*auto success =*/ detail::parse_file<grammar, start_rule_payload >(lexer, path, starting_buildings_);
        return starting_buildings_;
    }
}
