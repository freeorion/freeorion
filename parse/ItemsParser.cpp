#include "ParseImpl.h"

#include "Label.h"


#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const std::vector<ItemSpec>&) { return os; }
}
#endif

namespace {
    struct rules {
        rules() {
            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_r1_type _r1;
            using phoenix::push_back;

            start
                =   +parse::detail::item_spec_parser() [ push_back(_r1, _1) ]
                ;

            start.name("start");

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        typedef boost::spirit::qi::rule<
            parse::token_iterator,
            void (std::vector<ItemSpec>&),
            parse::skipper_type
        > start_rule;

        start_rule start;
    };
}

namespace parse {
    bool items(const boost::filesystem::path& path, std::vector<ItemSpec>& items_)
    { return detail::parse_file<rules, std::vector<ItemSpec> >(path, items_); }
}
