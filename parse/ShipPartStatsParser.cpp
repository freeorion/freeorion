#include "ShipPartStatsParser.h"

#include "Double.h"
#include "EnumParser.h"
#include "Int.h"
#include "Label.h"
#include "ParseImpl.h"

#include <boost/phoenix/phoenix.hpp>

#define DEBUG_PARSERS 0

#if DEBUG_PARSERS
namespace std {
    inline ostream& operator<<(ostream& os, const PartTypeStats&) { return os; }
}
#endif

namespace {
    struct part_stats_parser_rules {
        part_stats_parser_rules() {
            qi::_1_type _1;
            qi::_2_type _2;
            qi::_3_type _3;
            qi::_4_type _4;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::construct;

            start
                =   (parse::label(Capacity_token) >> parse::double_ [ _val = _1 ])
                |    eps [ _val = 0.0 ]
                ;

            start.name("Part Stats");

#if DEBUG_PARSERS
            debug(start);
#endif

            qi::on_error<qi::fail>(start, parse::report_error(_1, _2, _3, _4));
        }

        parse::detail::part_stats_parser_rule   start;
    };
}

namespace parse { namespace detail {
    part_stats_parser_rule& part_stats_parser() {
        static part_stats_parser_rules rules;
        return rules.start;
    }
} }
