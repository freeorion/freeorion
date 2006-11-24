// -*- C++ -*-
#ifndef _Meter_h_
#define _Meter_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

class ServerApp;

/** A Meter is a value with an associated maximum value.  A typical example is the population meter.  The max represents the max 
    pop for a planet, and the current represents the current pop there.  The max may be adjusted upwards or downwards, and the 
    current may be as well. */
class Meter
{
public:
    Meter(); ///< default ctor.
    Meter(double current, double max); ///< basic ctor

    double    Current() const; ///< returns the current value of the meter, which will be in [METER_MIN, Max()]
    double    Max() const;     ///< returns the maximum value of the meter, which will be in [METER_MIN, METER_MAX]

    double    InitialCurrent() const;  ///< returns the current value of the meter, as it was at the beginning of the turn
    double    InitialMax() const;      ///< returns the maximum value of the meter, as it was at the beginning of the turn
    double    PreviousCurrent() const; ///< returns the current value of the meter, as it was at the beginning of last turn
    double    PreviousMax() const;     ///< returns the maximum value of the meter, as it was at the beginning of last turn
    double    DeltaCurrent() const;    ///< returns InitialCurrent() - PreviousCurrent()
    double    DeltaMax() const;        ///< returns InitialMax() - PreviousMax()

    void ResetMax();                       ///< resets the max value of the Meter to METER_MIN, during max value recalculation
    void SetCurrent(double current);       ///< sets the current value of the Meter, clamping it to the range [METER_MIN, METER_MAX]
    void SetMax(double max);               ///< sets the maximum value of the Meter, clamping it to the range [METER_MIN, METER_MAX]
    void AdjustCurrent(double adjustment); ///< adds \a current to the current value of the Meter, clamping it to the range [METER_MIN, METER_MAX]
    void AdjustMax(double max);            ///< adds \a max to the maximum value of the Meter, clamping it to the range [METER_MIN, METER_MAX]
    void Clamp();                          ///< clamps Current() to the range [METER_MIN, Max()]

    static const double METER_MIN;
    static const double METER_MAX;

private:
    double    m_current;
    double    m_max;
    double    m_initial_current;
    double    m_initial_max;
    double    m_previous_current;
    double    m_previous_max;

    friend class ServerApp;
    friend class Universe;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Meter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_current)
        & BOOST_SERIALIZATION_NVP(m_max)
        & BOOST_SERIALIZATION_NVP(m_initial_current)
        & BOOST_SERIALIZATION_NVP(m_initial_max)
        & BOOST_SERIALIZATION_NVP(m_previous_current)
        & BOOST_SERIALIZATION_NVP(m_previous_max);
}

#endif // _Meter_h_
