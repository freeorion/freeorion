#include "Parse.h"

#include "ParseImpl.h"

#include "../util/Directories.h"

#include <boost/spirit/include/phoenix.hpp>


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
}
#endif

namespace {
    struct rules {
        rules(const std::string& filename,
              const parse::text_iterator& first, const parse::text_iterator& last)
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
                =   +parse::detail::item_spec_parser() [ push_back(_r1, _1) ]
                ;

            start.name("start");

            qi::on_error<qi::fail>(start, parse::report_error(filename, first, last, _1, _2, _3, _4));
        }

        typedef parse::detail::rule<
            void (std::vector<ItemSpec>&)
        > start_rule;

        start_rule start;
    };
}

namespace parse {
    std::vector<ItemSpec> items() {
        std::vector<ItemSpec> items_;
        const boost::filesystem::path& path = GetResourceDir() / "scripting/starting_unlocks/items.inf";
        /*auto success =*/ detail::parse_file<rules, std::vector<ItemSpec>>(path, items_);
        return items_;
    }

    std::vector<ItemSpec> starting_buildings() {
        std::vector<ItemSpec> starting_buildings_;
        const boost::filesystem::path& path = GetResourceDir() / "scripting/starting_unlocks/buildings.inf";
        /*auto success =*/ detail::parse_file<rules, std::vector<ItemSpec> >(path, starting_buildings_);
        return starting_buildings_;
    }
}
