#include "UnlockableItem.h"
#include "Parse.h"

#include "ParseImpl.h"
#include "EnumParser.h"
#include "../universe/UnlockableItem.h"

#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<UnlockableItem>&) { return os; }
}
#endif

namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;

namespace {
    struct grammar : public parse::detail::grammar<void(std::vector<UnlockableItem>)> {
        grammar(const parse::lexer& tok,
                const std::string& filename,
                const parse::text_iterator& first, const parse::text_iterator& last) :
            grammar::base_type(start),
            unlockable_item_parser(tok, label)
        {
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

        parse::detail::Labeller label;
        parse::detail::unlockable_item_grammar unlockable_item_parser;
        parse::detail::rule<void(std::vector<UnlockableItem>)> start;
    };
}


namespace parse { namespace detail {
    unlockable_item_type_grammar::unlockable_item_type_grammar(const parse::lexer& tok) :
        unlockable_item_type_grammar::base_type(rule, "unlockable_item_type_grammar")
    {
        qi::_val_type _val;

        rule
            =   tok.Building_       [ _val = UIT_BUILDING ]
            |   tok.ShipPart_       [ _val = UIT_SHIP_PART ]
            |   tok.ShipHull_       [ _val = UIT_SHIP_HULL ]
            |   tok.ShipDesign_     [ _val = UIT_SHIP_DESIGN ]
            |   tok.Tech_           [ _val = UIT_TECH ]
            |   tok.Policy_         [ _val = UIT_POLICY ]
            ;
    }

    unlockable_item_grammar::unlockable_item_grammar(
        const parse::lexer& tok,
        Labeller& label
    ) :
        unlockable_item_grammar::base_type(start, "unlockable_item_grammar"),
        unlockable_item_type_enum(tok)
    {
        using phoenix::construct;

        qi::_1_type _1;
        qi::_2_type _2;
        qi::_val_type _val;
        qi::omit_type omit_;

        start
            =  ( omit_[tok.Item_]
            >    label(tok.Type_) > unlockable_item_type_enum
            >    label(tok.Name_) > tok.string
               ) [ _val = construct<UnlockableItem>(_1, _2) ]
            ;

        start.name("UnlockableItem");

#if DEBUG_PARSERS
        debug(start);
#endif
    }
} }


namespace parse {
    std::vector<UnlockableItem> items(const boost::filesystem::path& path) {
        const lexer lexer;
        std::vector<UnlockableItem> items_;
        items_.reserve(128);    // should be more than enough as of this writing
        detail::parse_file<grammar, std::vector<UnlockableItem>>(lexer, path, items_);
        return items_;
    }

    std::vector<UnlockableItem> starting_buildings(const boost::filesystem::path& path) {
        const lexer lexer;
        std::vector<UnlockableItem> starting_buildings_;
        starting_buildings_.reserve(32); // should be more than enough as of this writing...
        detail::parse_file<grammar, std::vector<UnlockableItem>>(lexer, path, starting_buildings_);
        return starting_buildings_;
    }
}
