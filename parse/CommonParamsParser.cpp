#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParams.h"
#include "Label.h"
#include "ConditionParserImpl.h"
#include "../universe/Condition.h"

#include <boost/spirit/include/phoenix.hpp>

namespace phoenix = boost::phoenix;

namespace parse { namespace detail {
    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_1_type _1;
            qi::_r1_type _r1;
            qi::_val_type _val;
            qi::eps_type eps;
            using phoenix::new_;

            producible
                =   tok.Unproducible_ [ _val = false ]
                |   tok.Producible_ [ _val = true ]
                |   eps [ _val = true ]
                ;

            location
                =    parse::label(Location_token) > parse::detail::condition_parser [ _r1 = _1 ]
                |    eps [ _r1 = new_<Condition::All>() ]
                ;

            producible.name("Producible or Unproducible");
            location.name("Location");

#if DEBUG_PARSERS
            debug(producible);
            debug(location);
            debug(common);
#endif
        }

        producible_rule                 producible;
        location_rule                   location;
        part_hull_common_params_rule    common;
    };

    rules& GetRules() {
        static rules retval;
        return retval;
    }

    const producible_rule& producible_parser()
    { return GetRules().producible; }

    const location_rule& location_parser()
    { return GetRules().location; }

    const part_hull_common_params_rule& common_params_parser()
    { return GetRules().common; }

} }

