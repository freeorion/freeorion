#ifndef _StrongTypedef_h_
#define _StrongTypedef_h_

#include <cstdint>
#include <limits>
#include <ostream>
#include <string>
#include <type_traits>

template <typename Tag,
          typename UnderlyingType_t = int32_t,
          UnderlyingType_t invalid_value = -1,
          UnderlyingType_t default_value = invalid_value>
    requires (std::is_integral_v<UnderlyingType_t> && std::is_signed_v<UnderlyingType_t>)
struct StrongIDTypedef
{
    using UnderlyingType = UnderlyingType_t;

private:
    static constexpr UnderlyingType DEF{default_value};
    static constexpr UnderlyingType INV{invalid_value};
    static constexpr UnderlyingType MAX = std::numeric_limits<UnderlyingType>::max();
    static constexpr UnderlyingType MIN = std::numeric_limits<UnderlyingType>::min();

    UnderlyingType value = default_value;

public:
    [[nodiscard]] constexpr StrongIDTypedef() noexcept = default;
    [[nodiscard]] constexpr explicit StrongIDTypedef(UnderlyingType utv) noexcept : value(utv) {}

    template <typename T> requires std::is_integral_v<T> && std::is_signed_v<T>
    [[nodiscard]] constexpr explicit StrongIDTypedef(T v) noexcept :
        value(((v >= MIN) && (v <= MAX)) ? static_cast<UnderlyingType>(v) : invalid_value)
    {}

    [[nodiscard]] static consteval StrongIDTypedef Default() noexcept { return StrongIDTypedef{DEF}; }
    [[nodiscard]] static consteval StrongIDTypedef Invalid() noexcept { return StrongIDTypedef{INV}; }

    [[nodiscard]] constexpr UnderlyingType Value() const noexcept { return value; }
    [[nodiscard]] constexpr UnderlyingType& UnderRef() noexcept { return value; }
    [[nodiscard]] constexpr const UnderlyingType& UnderRef() const noexcept { return value; }

    constexpr StrongIDTypedef& operator+=(StrongIDTypedef rhs) noexcept { value += rhs.Value(); return *this; }
    constexpr StrongIDTypedef& operator-=(StrongIDTypedef rhs) noexcept { value -= rhs.Value(); return *this; }
    constexpr StrongIDTypedef& operator+=(UnderlyingType rhs) noexcept { value += rhs; return *this; }
    constexpr StrongIDTypedef& operator-=(UnderlyingType rhs) noexcept { value -= rhs; return *this; }

    constexpr auto& operator++() noexcept {
        if (value < MAX) ++value;
        return *this;
    }
    constexpr auto& operator--() noexcept {
        if (value > MIN) --value;
        return *this;
    }

    constexpr auto operator<=>(const StrongIDTypedef&) const noexcept = default;
};

template <typename> struct is_strong_id_typedef : std::false_type {};
template <typename TT, typename UVT, UVT IV, UVT DV>
struct is_strong_id_typedef<StrongIDTypedef<TT, UVT, IV, DV>> : std::true_type {};
template <typename T> concept strong_id_typedef = is_strong_id_typedef<T>::value;

constexpr auto Value(strong_id_typedef auto t) noexcept { return t.Value(); }
constexpr auto& UnderRef(strong_id_typedef auto& t) noexcept { return t.UnderRef(); }
constexpr const auto& UnderRef(const strong_id_typedef auto& t) noexcept { return t.UnderRef(); }
inline auto to_string(strong_id_typedef auto t) { return std::to_string(t.Value()); }

template <strong_id_typedef ST>
constexpr auto operator+(ST x, typename ST::UnderlyingType i) noexcept { return x += i; }
constexpr auto operator++(strong_id_typedef auto& x, int) noexcept { auto rv = x; ++x; return rv; }
template <strong_id_typedef ST>
constexpr auto operator-(ST x, typename ST::UnderlyingType i) noexcept { return x -= i; }
constexpr auto operator--(strong_id_typedef auto& x, int) noexcept { auto rv = x; --x; return rv; }

template<strong_id_typedef ST>
struct std::hash<ST>
{
    constexpr size_t operator()(ST x) const noexcept
    { return std::hash<typename ST::UnderlyingType>::operator()(x.Value()); }
};

std::ostream& operator<<(std::ostream& os, strong_id_typedef auto t) { return os << static_cast<int>(Value(t)); }

#endif