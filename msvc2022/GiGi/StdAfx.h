
#pragma once

// We include all external headers used in any of the header files,
// plus external headers used in at least five .cpp files.

// https://hownot2code.wordpress.com/2016/08/16/stdafx-h/ 

// ----------------
// includes from .h

// GiGi
#include <cassert>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iosfwd>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

#include <GL/glew.h>

#include <boost/algorithm/string/trim.hpp>
#include <filesystem>
#include <boost/functional/hash.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/trackable.hpp>

#ifdef _MSC_VER
// Note: This is a workaround for Visual C++ non-conformant pre-processor
// handling of empty macro arguments.
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/is_empty_variadic.hpp>
#endif
