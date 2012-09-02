// -*- C++ -*-
#ifndef _ShipPartStatsParser_h_
#define _ShipPartStatsParser_h_

#include "../universe/ShipDesign.h"
#include "Lexer.h"

#include <boost/spirit/include/qi.hpp>

namespace parse { namespace detail {
    typedef boost::spirit::qi::rule<
        parse::token_iterator,
        PartTypeStats (),
        parse::skipper_type
    > part_stats_parser_rule;

    part_stats_parser_rule& part_stats_parser();
} }


#endif