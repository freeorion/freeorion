// -*- C++ -*-
#ifndef _Meter_h_
#define _Meter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

/** A Meter is a value with an associated maximum value.  A typical example is the population meter.  The max represents the max 
    pop for a planet, and the current represents the current pop there.  The max may be adjusted upwards or downwards, and the 
    current may be as well. */
class Meter
{
public:
    Meter(); ///< default ctor.
    Meter(double current, double max); ///< basic ctor
    Meter(const GG::XMLElement& elem); ///< ctor that constructs a Meter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Meter object

    double    Current() const; ///< returns the current value of the meter, which will be in [METER_MIN, Max()]
    double    Max() const;     ///< returns the maximum value of the meter, which will be in [METER_MIN, METER_MAX]

    GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a System object with visibility limited relative to the input empire

    void ResetMax();                    ///< resets the max value of the Meter to METER_MIN, during max value recalculation
    void SetCurrent(double current);    ///< sets the current value of the Meter, clamping it to the range [METER_MIN, Max()]
    void SetMax(double max);            ///< sets the maximum value of the Meter, clamping it to the range [METER_MIN, METER_MAX]
    void AdjustCurrent(double current); ///< adds \a current to the current value of the Meter, clamping it to the range [METER_MIN, Max()]
    void AdjustMax(double max);         ///< adds \a max to the maximum value of the Meter, clamping it to the range [METER_MIN, METER_MAX]

    static const double METER_MIN;
    static const double METER_MAX;

private:
    double    m_current;
    double    m_max;
};

#endif // _Meter_h_
