#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "../universe/UnlockableItem.h"

#include <boost/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<UnlockableItem>&) { return os; }
}
#endif

namespace {
    using start_rule_payload = std::vector<UnlockableItem>;
    using start_rule_signature = void(start_rule_payload&);

    struct grammar : public parse::detail::grammar<start_rule_signature> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator first, const parse::text_iterator last) :
            grammar::base_type(start),
            unlockable_item_parser(tok, label)
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
                =   +unlockable_item_parser [ push_back(_r1, _1) ]
                ;

            start.name("start");

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        using start_rule = parse::detail::rule<start_rule_signature>;

        parse::detail::Labeller label;
        parse::detail::unlockable_item_grammar unlockable_item_parser;
        start_rule start;
    };
}

namespace parse {
    start_rule_payload items(const boost::filesystem::path& path) {
        start_rule_payload items_;
        items_.reserve(128);    // should be more than enough as of this writing
        detail::parse_file<grammar, start_rule_payload>(GetLexer(), path, items_);
        return items_;
    }

    start_rule_payload starting_buildings(const boost::filesystem::path& path) {
        start_rule_payload starting_buildings_;
        starting_buildings_.reserve(32); // should be more than enough as of this writing...
        detail::parse_file<grammar, start_rule_payload>(GetLexer(), path, starting_buildings_);
        return starting_buildings_;
    }
}
