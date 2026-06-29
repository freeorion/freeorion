#ifndef _StrongTypedef_h_
#define _StrongTypedef_h_

#include <cstdint>
#include <functional>
#include <limits>
#include <string>

#define FO_STRONG_ID_TYPEDEF(typeName, underlyingType)                                          \
struct typeName {                                                                               \
private:                                                                                        \
    underlyingType value{-1};                                                                   \
public:                                                                                         \
    constexpr typeName() noexcept = default;                                                    \
    constexpr explicit typeName(underlyingType utv) noexcept : value(utv) {}                    \
    constexpr auto Value() const noexcept { return value; }                                     \
    constexpr auto& UnderRef() noexcept { return value; }                                       \
    constexpr const auto& UnderRef() const noexcept { return value; }                           \
    constexpr auto& operator+=(typeName rhs) noexcept { value += rhs.Value(); return *this; }   \
    constexpr auto& operator-=(typeName rhs) noexcept { value -= rhs.Value(); return *this; }   \
    constexpr auto& operator+=(underlyingType rhs) noexcept { value += rhs; return *this; }     \
    constexpr auto& operator-=(underlyingType rhs) noexcept { value -= rhs; return *this; }     \
    constexpr auto operator<=>(const typeName&) const noexcept = default;                       \
};                                                                                              \
constexpr auto Value(typeName t) noexcept { return t.Value(); }                                 \
constexpr auto& UnderRef(typeName& t) noexcept { return t.UnderRef(); }                         \
constexpr const auto& UnderRef(const typeName& t) noexcept { return t.UnderRef(); }             \
auto to_string(typeName t) { return std::to_string(t.Value()); }                                \
constexpr auto operator+(typeName x, underlyingType i) noexcept { return x += i; }              \
constexpr auto operator+(underlyingType i, typeName x) noexcept { return x += i; }              \
constexpr auto& operator++(typeName& x) noexcept {                                              \
    constexpr auto MAX = std::numeric_limits<underlyingType>::max();                            \
    if (Value(x) < MAX) x += 1;                                                                 \
    return x;                                                                                   \
}                                                                                               \
constexpr auto operator++(typeName& x, int) noexcept { auto rv = x; ++x; return rv; }           \
constexpr auto operator-(typeName x, underlyingType i) noexcept { return x -= i; }              \
constexpr auto operator-(underlyingType i, typeName x) noexcept { return x -= i; }              \
constexpr auto& operator--(typeName& x) noexcept {                                              \
    constexpr auto MIN = std::numeric_limits<underlyingType>::min();                            \
    if (Value(x) > MIN) x -= 1;                                                                 \
    return x;                                                                                   \
}                                                                                               \
constexpr auto operator--(typeName& x, int) noexcept { auto rv = x; --x; return rv; }           \
                                                                                                \
template<> struct std::hash<typeName> {                                                         \
    size_t operator()(typeName x) const noexcept                                                \
    { return std::hash<underlyingType>{}(x.Value()); }                                          \
};                                                                                              \
                                                                                                \
std::ostream& operator<<(std::ostream& os, typeName t) = delete; // TODO: implement
    


#endif