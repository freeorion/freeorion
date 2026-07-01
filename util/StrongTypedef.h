#ifndef _StrongTypedef_h_
#define _StrongTypedef_h_

#include <concepts>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <type_traits>

#define FO_STRONG_ID_TYPEDEF(typeName, underlyingType)                                          \
static_assert(std::is_integral_v<underlyingType> && std::is_signed_v<underlyingType>);          \
struct typeName {                                                                               \
private:                                                                                        \
    underlyingType value{-1};                                                                   \
    static constexpr auto MAX = std::numeric_limits<underlyingType>::max();                     \
    static constexpr auto MIN = std::numeric_limits<underlyingType>::min();                     \
public:                                                                                         \
    constexpr typeName() noexcept = default;                                                    \
    constexpr explicit typeName(underlyingType utv) noexcept : value(utv) {}                    \
    template <std::integral T> constexpr explicit typeName(T v) noexcept :                      \
        value((std::cmp_greater_equal(v, MIN) && std::cmp_less_equal(v, MAX)) ?                 \
              static_cast<underlyingType>(v) : -1) {}                                           \
    constexpr auto Value() const noexcept { return value; }                                     \
    constexpr auto& UnderRef() noexcept { return value; }                                       \
    constexpr const auto& UnderRef() const noexcept { return value; }                           \
    constexpr auto& operator+=(typeName rhs) noexcept { value += rhs.Value(); return *this; }   \
    constexpr auto& operator-=(typeName rhs) noexcept { value -= rhs.Value(); return *this; }   \
    constexpr auto& operator+=(underlyingType rhs) noexcept { value += rhs; return *this; }     \
    constexpr auto& operator-=(underlyingType rhs) noexcept { value -= rhs; return *this; }     \
    constexpr auto& operator++() noexcept {                                                     \
        if (value < MAX) value += 1;                                                            \
        return *this;                                                                           \
    }                                                                                           \
    constexpr auto& operator--() noexcept {                                                     \
        if (value > MIN) value -= 1;                                                            \
        return *this;                                                                           \
    }                                                                                           \
    constexpr auto operator<=>(const typeName&) const noexcept = default;                       \
};                                                                                              \
constexpr auto Value(typeName t) noexcept { return t.Value(); }                                 \
constexpr auto& UnderRef(typeName& t) noexcept { return t.UnderRef(); }                         \
constexpr const auto& UnderRef(const typeName& t) noexcept { return t.UnderRef(); }             \
auto to_string(typeName t) { return std::to_string(t.Value()); }                                \
constexpr auto operator+(typeName x, underlyingType i) noexcept { return x += i; }              \
constexpr auto operator++(typeName& x, int) noexcept { auto rv = x; ++x; return rv; }           \
constexpr auto operator-(typeName x, underlyingType i) noexcept { return x -= i; }              \
constexpr auto operator--(typeName& x, int) noexcept { auto rv = x; --x; return rv; }           \
                                                                                                \
template<> struct std::hash<typeName> {                                                         \
    size_t operator()(typeName x) const noexcept                                                \
    { return std::hash<underlyingType>{}(x.Value()); }                                          \
};                                                                                              \
                                                                                                \
std::ostream& operator<<(std::ostream& os, typeName t) = delete; // TODO: implement
    


#endif