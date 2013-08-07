#include "Names.h"

#include <boost/preprocessor/stringize.hpp>


#define DEFINE_TOKEN(r, _, name) adobe::aggregate_name_t BOOST_PP_CAT(name, _token) = { BOOST_PP_STRINGIZE(name) };
BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_1)
BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_2)
BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_3)
BOOST_PP_SEQ_FOR_EACH(DEFINE_TOKEN, _, TOKEN_SEQ_4)
#undef DEFINE_TOKEN
