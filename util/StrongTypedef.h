#ifndef _StrongTypedef_h_
#define _StrongTypedef_h_

#include <cstdint>
#include <limits>

#define FO_STRONG_ID_TYPEDEF(typeName, underlyingType)                                                  \
enum class typeName : underlyingType {};                                                                \
                                                                                                        \
constexpr underlyingType Value(typeName x) noexcept { return static_cast<underlyingType>(x); }          \
                                                                                                        \
constexpr typeName operator+(typeName x, underlyingType i) noexcept { return typeName{Value(x) + i}; }  \
constexpr typeName operator+(underlyingType i, typeName x) noexcept { return typeName{Value(x) + i}; }  \
                                                                                                        \
constexpr typeName& operator++(typeName& x) {                                                           \
    constexpr underlyingType MAX = std::numeric_limits<underlyingType>::max();                          \
    x = (Value(x) < MAX) ? typeName(Value(x) + 1) : typeName(-1);                                       \
    return x;                                                                                           \
}                                                                                                       \
constexpr typeName operator++(typeName& x, int) { typeName rv = x; ++x; return rv; }

#endif