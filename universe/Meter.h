// -*- C++ -*-
#ifndef _Meter_h_
#define _Meter_h_

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <string>

/** A Meter is a value with an associated maximum value.  A typical example is the population meter.  The max represents the max 
    pop for a planet, and the current represents the current pop there.  The max may be adjusted upwards or downwards, and the 
    current may be as well. */
class Meter
{
public:
    /** \name Structors */ //@{
    Meter();                                ///< default ctor.  values all set to DEFAULT_VALUE
    explicit Meter(double current_value);   ///< basic ctor.  current value set to \a cuurent_value and initial and current values set to DEFAULT_VALUE
    Meter(double current_value, double initial_value, double previous_value);   ///< full ctor
    //@}

    /** \name Accessors */ //@{
    double      Current() const;                    ///< returns the current value of the meter
    double      Initial() const;                    ///< returns the value of the meter as it was at the beginning of the turn
    double      Previous() const;                   ///< returns the value of the meter as it was at the beginning of the previous turn

    std::string Dump() const;                       ///< returns text of meter values
    //@}

    /** \name Mutators */ //@{
    void        SetCurrent(double current_value);   ///< sets current value, leaving initial and previous values unchanged
    void        Set(double current_value, double initial_value, double previous_value); ///< sets current, initial and previous values
    void        ResetCurrent();                     ///< sets current value equal to DEFAULT_VALUE
    void        Reset();                            ///< sets current, initial and previous values to DEFAULT_VALUE

    void        AddToCurrent(double adjustment);    ///< adds \a current to the current value of the Meter
    void        ClampCurrentToRange(double min = DEFAULT_VALUE, double max = LARGE_VALUE);  ///< ensures the current value falls in the range [\a min, \a max]

    void        BackPropegate();                    ///< sets previous equal to initial, then sets initial equal to current
    //@}

    static const double DEFAULT_VALUE;              ///< value assigned to current, initial, or previous when resetting or when no value is specified in a constructor
    static const double LARGE_VALUE;                ///< a very large number, which is useful to set current to when it will be later clamped, to ensure that the result is the max value in the clamp range
    static const double INVALID_VALUE;              ///< sentinel value to indicate no valid value for this meter

private:
    double  m_current_value;
    double  m_initial_value;
    double  m_previous_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Meter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_current_value)
        & BOOST_SERIALIZATION_NVP(m_initial_value)
        & BOOST_SERIALIZATION_NVP(m_previous_value);
}

#endif // _Meter_h_
