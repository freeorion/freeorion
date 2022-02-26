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
    [[nodiscard]] constexpr float Current() const { return m_current_value; };
    [[nodiscard]] constexpr float Initial() const { return m_initial_value; };

    [[nodiscard]] std::string Dump(unsigned short ntabs = 0) const;   ///< returns text of meter values

    [[nodiscard]] constexpr bool operator==(const Meter& rhs) const
    { return m_current_value == rhs.m_current_value && m_initial_value == rhs.m_initial_value; }

    [[nodiscard]] constexpr bool operator<(const Meter& rhs) const
    { return m_current_value < rhs.m_current_value || (m_current_value == rhs.m_current_value && m_initial_value < rhs.m_initial_value); }

    constexpr void SetCurrent(float current_value) { m_current_value = current_value; }

    constexpr void Set(float current_value, float initial_value) {
        m_current_value = current_value;
        m_initial_value = initial_value;
    }

    constexpr void ResetCurrent() { m_current_value = DEFAULT_VALUE; } // initial unchanged

    constexpr void Reset() {
        m_current_value = DEFAULT_VALUE;
        m_initial_value = DEFAULT_VALUE;
    }

    constexpr void AddToCurrent(float adjustment) { m_current_value += adjustment; }

    void ClampCurrentToRange(float min = DEFAULT_VALUE, float max = LARGE_VALUE);   ///< ensures the current value falls in the range [\a min, \a max]

    constexpr void BackPropagate() { m_initial_value = m_current_value; }


    static constexpr float DEFAULT_VALUE = 0.0f;                        ///< value assigned to current or initial when resetting or when no value is specified in a constructor
    static constexpr float LARGE_VALUE = static_cast<float>(2 << 15);   ///< a very large number, which is useful to set current to when it will be later clamped, to ensure that the result is the max value in the clamp range
    static constexpr float INVALID_VALUE = -LARGE_VALUE;                ///< sentinel value to indicate no valid value for this meter

    float m_current_value = DEFAULT_VALUE;
    float m_initial_value = DEFAULT_VALUE;

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION(Meter, 1)


template <typename Archive>
void Meter::serialize(Archive& ar, const unsigned int version)
{
    if constexpr (Archive::is_loading::value) {
        if (version < 1) {
            ar  & BOOST_SERIALIZATION_NVP(m_current_value)
                & BOOST_SERIALIZATION_NVP(m_initial_value);
            return;
        }
    }
    // use minimum size NVP label to reduce archive size bloat for very-often serialized meter values...
    ar  & boost::serialization::make_nvp("c", m_current_value)
        & boost::serialization::make_nvp("i", m_initial_value);
}


#endif
