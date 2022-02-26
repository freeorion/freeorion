#ifndef _Meter_h_
#define _Meter_h_


#include <string>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/version.hpp>
#include "../util/Export.h"


/** A Meter is a value and associated initial value that is used to track information
  * about gamestate. A typical example is the population meter of a planet. */
class FO_COMMON_API Meter {
public:
    [[nodiscard]] constexpr float Current() const { return cur; };
    [[nodiscard]] constexpr float Initial() const { return init; };

    [[nodiscard]] std::string Dump(unsigned short ntabs = 0) const;   ///< returns text of meter values

    [[nodiscard]] constexpr bool operator==(const Meter& rhs) const
    { return cur == rhs.cur && init == rhs.init; }

    [[nodiscard]] constexpr bool operator<(const Meter& rhs) const
    { return cur < rhs.cur || (cur == rhs.cur && init < rhs.init); }

    constexpr void SetCurrent(float current_value) { cur = current_value; }

    constexpr void Set(float current_value, float initial_value) {
        cur = current_value;
        init = initial_value;
    }

    constexpr void ResetCurrent() { cur = DEFAULT_VALUE; } // initial unchanged

    constexpr void Reset() {
        cur = DEFAULT_VALUE;
        init = DEFAULT_VALUE;
    }

    constexpr void AddToCurrent(float adjustment) { cur += adjustment; }

    void ClampCurrentToRange(float min = DEFAULT_VALUE, float max = LARGE_VALUE);   ///< ensures the current value falls in the range [\a min, \a max]

    constexpr void BackPropagate() { init = cur; }


    static constexpr float DEFAULT_VALUE = 0.0f;                        ///< value assigned to current or initial when resetting or when no value is specified in a constructor
    static constexpr float LARGE_VALUE = static_cast<float>(2 << 15);   ///< a very large number, which is useful to set current to when it will be later clamped, to ensure that the result is the max value in the clamp range
    static constexpr float INVALID_VALUE = -LARGE_VALUE;                ///< sentinel value to indicate no valid value for this meter

    float cur = DEFAULT_VALUE;
    float init = DEFAULT_VALUE;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Meter, 1)


template <typename Archive>
void Meter::serialize(Archive& ar, const unsigned int version)
{
    // use minimum size NVP label to reduce archive size bloat for very-often serialized meter values...
    ar  & boost::serialization::make_nvp("c", cur)
        & boost::serialization::make_nvp("i", init);
}


#endif
