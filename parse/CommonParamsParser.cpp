#define PHOENIX_LIMIT 11
#define BOOST_RESULT_OF_NUM_ARGS PHOENIX_LIMIT

#include "CommonParams.h"

namespace parse { namespace detail {
    struct rules {
        rules() {
            const parse::lexer& tok = parse::lexer::instance();

            qi::_val_type _val;
            qi::eps_type eps;

            producible
                =   tok.Unproducible_ [ _val = false ]
                |   tok.Producible_ [ _val = true ]
                |   eps [ _val = true ]
                ;

            producible.name("Producible or Unproducible");

#if DEBUG_PARSERS
            debug(producible);
#endif
        }

        producible_rule                 producible;
    };

    rules& GetRules() {
        static rules retval;
        return retval;
    }

    const producible_rule& producible_parser()
    { return GetRules().producible; }
} }

