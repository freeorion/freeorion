
#pragma once

// We include all external headers used in any of the header files,
// plus external headers used in at least five .cpp files.

// https://hownot2code.com/2016/08/16/stdafx-h/ 

// ----------------
// includes from .h

// Parsers
#include <map>
#include <memory>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/spirit/include/lex_lexertl.hpp>
#include <boost/spirit/include/lex_lexertl_position_token.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_argument.hpp>
#include <boost/timer.hpp>
#include <boost/uuid/uuid.hpp>

// ------------------
// includes from .cpp
#include <boost/smart_ptr/make_unique.hpp>

#ifdef _MSC_VER
// Note: This is a workaround for Visual C++ non-conformant pre-processor
// handling of empty macro arguments.
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#endif
