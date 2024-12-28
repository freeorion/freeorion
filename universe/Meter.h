#ifndef _Meter_h_
#define _Meter_h_

#if __has_include(<charconv>)
#include <charconv>
#endif

#include <array>
#include <compare>
#include <cstdint>
#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include "../util/Export.h"

/** A Meter is a value and associated initial value that is used to track information
  * about gamestate. A typical example is the population meter of a planet. */
class FO_COMMON_API Meter {
private:
    static constexpr float FLOAT_INT_SCALE = 1000.0f;
    static constexpr bool from_float_noexcept = noexcept(noexcept(
        static_cast<int32_t>(float{} * FLOAT_INT_SCALE + (float{} > 0 ? 0.5f : -0.5f))));

    static constexpr bool from_int_noexcept = noexcept(noexcept(int32_t{} / FLOAT_INT_SCALE));

public:
    constexpr Meter() = default;
    constexpr explicit Meter(float v) noexcept(from_float_noexcept) :
        cur(FromFloat(v)),
        init(FromFloat(v))
    {};
    constexpr Meter(float c, float i) noexcept(from_float_noexcept) :
        cur(FromFloat(c)),
        init(FromFloat(i))
    {}

    [[nodiscard]] constexpr float Current() const noexcept(from_int_noexcept) { return FromInt(cur); };

    [[nodiscard]] constexpr float Initial() const noexcept(from_int_noexcept) { return FromInt(init); };

    [[nodiscard]] constexpr auto operator<=>(const Meter&) const noexcept = default;

    constexpr void SetCurrent(float current_value) noexcept(from_float_noexcept)
    { cur = FromFloat(current_value); }

    constexpr void Set(float current_value, float initial_value) noexcept(from_float_noexcept) {
        cur = FromFloat(current_value);
        init = FromFloat(initial_value);
    }

    constexpr void ResetCurrent() noexcept
    { cur = DEFAULT_INT; } // initial unchanged

    constexpr void Reset() noexcept {
        cur = DEFAULT_INT;
        init = DEFAULT_INT;
    }

    constexpr void AddToCurrent(float adjustment) noexcept(from_float_noexcept)
    { cur += FromFloat(adjustment); }

    // no noexcept, even though could be in MSVC, due to issues with definitions
    // of std::max and std::max causing problems with referring to them in this
    // header, which means they probably can't be tested with noexcept(...) here
    void ClampCurrentToRange(float min = DEFAULT_VALUE, float max = LARGE_VALUE); ///< ensures the current value falls in the range [\a min, \a max]

    constexpr void BackPropagate() noexcept { init = cur; }

private:
    static constexpr bool have_noexcept_to_chars =
#if defined(__cpp_lib_to_chars)
        noexcept(std::from_chars(static_cast<const char*>(nullptr),
                                 static_cast<const char*>(nullptr),
                                 std::declval<int&>()));
#else
        false;
#endif

public:
    static constexpr bool dump_noexcept = have_noexcept_to_chars && from_int_noexcept;
    ///< returns text of meter values
    [[nodiscard]] std::array<std::string::value_type, 64> Dump(uint8_t ntabs = 0) const noexcept(dump_noexcept);

    int ToChars(char* buffer, char* const buffer_end) const noexcept(have_noexcept_to_chars);

    using ToCharsArrayT = std::array<std::string::value_type, 24>;
    [[nodiscard]] ToCharsArrayT ToChars() const noexcept(have_noexcept_to_chars);

    int SetFromChars(std::string_view chars) noexcept(have_noexcept_to_chars);

    static constexpr float DEFAULT_VALUE = 0.0f;        ///< value assigned to current or initial when resetting or when no value is specified in a constructor
    static constexpr float LARGE_VALUE = 99999.0f;      ///< a very large number, which is useful to set current to when it will be later clamped, to ensure that the result is the max value in the clamp range
    static constexpr float INVALID_VALUE = -LARGE_VALUE;///< sentinel value to indicate no valid value for this meter

private:
    // Value must be rounded, otherwise a calculated increase of 0.99999997 will be truncated to 0.999.
    // Negative values are increased by truncation, so the offset must be negative, too.
    static constexpr int32_t FromFloat(float f) noexcept(from_float_noexcept)
    { return static_cast<int32_t>(f * FLOAT_INT_SCALE + (f > 0 ? 0.5f : -0.5f)); }

    static constexpr float FromInt(int32_t i) noexcept(from_int_noexcept)
    { return i / FLOAT_INT_SCALE; }

    static constexpr int32_t DEFAULT_INT = 0; // should be equal to Meter::FromFloat(DEFAULT_VALUE);

    int32_t cur = DEFAULT_INT;
    int32_t init = DEFAULT_INT;

    static constexpr int32_t LARGE_INT = 99999000; // should be equal to Meter::FromFloat(LARGE_VALUE);

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Meter, 2)


template <typename Archive>
void Meter::serialize(Archive& ar, const unsigned int version)
{
    if constexpr (Archive::is_loading::value) {
        if (version < 2) {
            float c = 0.0f, i = 0.0f;
            ar  & boost::serialization::make_nvp("c", c)
                & boost::serialization::make_nvp("i", i);
            cur = FromFloat(c);
            init = FromFloat(i);
            return;
        }
    }
    ar  & boost::serialization::make_nvp("c", cur)
        & boost::serialization::make_nvp("i", init);
}

namespace boost::archive {
    class xml_iarchive;
    class xml_oarchive;
}

template<> void Meter::serialize(boost::archive::xml_iarchive&, const unsigned int version);
template<> void Meter::serialize(boost::archive::xml_oarchive&, const unsigned int version);

#endif
