#ifndef _boost_fix_h_
#define _boost_fix_h_

#include <boost/version.hpp>

#if BOOST_VERSION >= 107900 && defined(_MSC_VER)
#include <cmath>
namespace boost::core {
    inline float _copysign(float x, float y) { return std::copysignf(x, y); }
}
namespace std {
    inline float _copysign(float x, float y) { return std::copysignf(x, y); }
}
#endif

#endif