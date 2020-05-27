#ifndef _Meter_h_
#define _Meter_h_


#include <string>
#include "../util/Export.h"


/** A Meter is a value with an associated maximum value.  A typical example is
  * the population meter.  The max represents the max pop for a planet, and the
  * current represents the current pop there.  The max may be adjusted upwards
  * or downwards, and the current may be as well. */
class FO_COMMON_API Meter {
public:
    /** Creates a new meter with both initial and current value set to
        DEFAULT_VALUE. */
    Meter() = default;

    /** Creates a new meter with the current value set to @p current_value and
        the initial value set to @p initial_value. */
    Meter(float current_value, float initial_value);

    float Current() const;                  ///< returns the current value of the meter
    float Initial() const;                  ///< returns the value of the meter as it was at the beginning of the turn

    std::string Dump(unsigned short ntabs = 0) const;   ///< returns text of meter values

    bool operator==(const Meter& rhs) const
    { return m_current_value == rhs.m_current_value && m_initial_value == rhs.m_initial_value; }

    bool operator<(const Meter& rhs) const
    { return m_current_value < rhs.m_current_value || (m_current_value == rhs.m_current_value && m_initial_value < rhs.m_initial_value); }

    void SetCurrent(float current_value);   ///< sets current value, leaving initial value unchanged
    void Set(float current_value, float initial_value); ///< sets current and initial values
    void ResetCurrent();                    ///< sets current value to DEFAULT_VALUE
    void Reset();                           ///< sets current and initial values to DEFAULT_VALUE

    void AddToCurrent(float adjustment);    ///< adds \a current to the current value of the Meter
    void ClampCurrentToRange(float min = DEFAULT_VALUE, float max = LARGE_VALUE);   ///< ensures the current value falls in the range [\a min, \a max]

    void BackPropagate();                   ///< sets previous equal to initial, then sets initial equal to current

    static constexpr float DEFAULT_VALUE = 0.0f;///< value assigned to current or initial when resetting or when no value is specified in a constructor
    static const float LARGE_VALUE;             ///< a very large number, which is useful to set current to when it will be later clamped, to ensure that the result is the max value in the clamp range
    static const float INVALID_VALUE;           ///< sentinel value to indicate no valid value for this meter

private:
    float m_current_value = DEFAULT_VALUE;
    float m_initial_value = DEFAULT_VALUE;

    template <typename Archive>
    friend void serialize(Archive&, Meter&, unsigned int const);
};


#endif
