
#pragma once

// We include all external headers used in any of the header files,
// plus boost headers used in any .cpp files.

// https://hownot2code.com/2016/08/16/stdafx-h/

// ----------------
// includes from .h or .cpp

// Parsers
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../../util/boost_fix.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/find_iterator.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/optional/optional.hpp>
#include <boost/phoenix/function/adapt_function.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_position_token.hpp>
#include <boost/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/spirit/include/support_argument.hpp>
#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <boost/python.hpp>

#ifdef _MSC_VER
// Note: This is a workaround for Visual C++ non-conformant pre-processor
// handling of empty macro arguments.
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#endif

