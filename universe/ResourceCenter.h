// -*- C++ -*-
#ifndef _ResourceCenter_h_
#define _ResourceCenter_h_

#include "Enums.h"

#include <boost/signal.hpp>
#include <boost/serialization/nvp.hpp>

class Empire;
class Meter;
class UniverseObject;

/** The ResourceCenter class is an abstract base class for anything in the FreeOrion gamestate that generates
 *  resources (food, minerals, etc.).  Most likely, such an object will also be a subclass of UniverseObject.
 *  
 *  Planet is the most obvious class to inherit ResourceCenter, but other classes could be made from it as well
 *  (e.g., a trade-ship or mining vessel, or a non-Planet UniverseObject- and PopCenter- derived object of some 
 *  sort. */
class ResourceCenter
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()>  ResourceCenterChangedSignalType;    ///< emitted when the ResourceCenter is altered in any way
    //@}

    /** \name Structors */ //@{
    ResourceCenter();           ///< default ctor
    virtual ~ResourceCenter();  ///< dtor
    //@}

    /** \name Accessors */ //@{
    FocusType               PrimaryFocus() const     {return m_primary;}
    FocusType               SecondaryFocus() const   {return m_secondary;}

    virtual double          ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn
    virtual double          MeterPoints(MeterType type) const;              ///< returns "true amount" associated with a meter.  In some cases (METER_POPULATION) this is just the meter value.  In other cases (METER_FARMING) this is some other value (a function of population and meter value)
    virtual double          ProjectedMeterPoints(MeterType type) const;     ///< returns expected "true amount" associated with a meter on the next turn

    mutable ResourceCenterChangedSignalType ResourceCenterChangedSignal;    ///< the state changed signal object for this ResourceCenter
    //@}

    /** \name Mutators */ //@{
    void                    SetPrimaryFocus(FocusType focus);
    void                    SetSecondaryFocus(FocusType focus);

    virtual void            ApplyUniverseTableMaxMeterAdjustments(MeterType meter_type = INVALID_METER_TYPE);
    virtual void            PopGrowthProductionResearchPhase();

    void                    Reset();                                        /// Resets the meters, etc.  This should be called when a ResourceCenter is wiped out due to starvation, etc.
    //@}

protected:
    void                    Init();                                         ///< initialization that needs to be called by derived class after derived class is constructed

private:
    FocusType  m_primary;
    FocusType  m_secondary;

    virtual Visibility      GetVisibility(int empire_id) const = 0;         ///< implementation should return the visbility of this ResourceCenter for the empire with the specified \a empire_id
    virtual const Meter*    GetPopMeter() const = 0;                        ///< implimentation should return the population meter to use when calculating meter points for this resource center
    virtual const Meter*    GetMeter(MeterType type) const = 0;             ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual Meter*          GetMeter(MeterType type) = 0;                   ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual const
        UniverseObject*     GetObject() const = 0;                          ///< implementation should return the UniverseObject associated with this ResourceCenter

    virtual void            InsertMeter(MeterType meter_type, Meter meter) = 0; ///< implimentation should add \a meter to the object so that it can be accessed with the GetMeter() functions

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void ResourceCenter::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    if (Archive::is_saving::value) {
        vis = GetVisibility(Universe::s_encoding_empire);
    }

    ar  & BOOST_SERIALIZATION_NVP(vis);

    if (vis == VIS_FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_primary)
            & BOOST_SERIALIZATION_NVP(m_secondary);
    }
}

#endif // _ResourceCenter_h_
