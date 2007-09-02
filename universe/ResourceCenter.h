// -*- C++ -*-
#ifndef _ResourceCenter_h_
#define _ResourceCenter_h_

#include "Enums.h"
#include "Meter.h"
#include "UniverseObject.h"

#include <boost/signal.hpp>

class Empire;


/** a production center decoration for a UniverseObject. */
class ResourceCenter
{
public:
    /** \name Signal Types */ //@{
    typedef boost::signal<void ()> ResourceCenterChangedSignalType; ///< emitted when the ResourceCenter is altered in any way
    typedef boost::signal<UniverseObject* (), Default0Combiner> GetObjectSignalType; ///< emitted as a request for the UniverseObject to which this ResourceCenter is attached
    //@}

    /** \name Structors */ //@{
    ResourceCenter();           ///< default ctor
    virtual ~ResourceCenter();  ///< dtor
    //@}

    /** \name Accessors */ //@{
    FocusType       PrimaryFocus() const     {return m_primary;}
    FocusType       SecondaryFocus() const   {return m_secondary;}

    virtual double  ProjectedCurrentMeter(MeterType type) const;    ///< returns expected value of  specified meter current value on the next turn
    virtual double  MeterPoints(MeterType type) const;              ///< returns "true amount" associated with a meter.  In some cases (METER_POPULATION) this is just the meter value.  In other cases (METER_FARMING) this is some other value (a function of population and meter value)
    virtual double  ProjectedMeterPoints(MeterType type) const;     ///< returns expected "true amount" associated with a meter on the next turn

    mutable ResourceCenterChangedSignalType ResourceCenterChangedSignal; ///< the state changed signal object for this ResourceCenter
    //@}

    /** \name Mutators */ //@{
    void            SetPrimaryFocus(FocusType focus);
    void            SetSecondaryFocus(FocusType focus);

    virtual void    ApplyUniverseTableMaxMeterAdjustments();
    virtual void    PopGrowthProductionResearchPhase();

    /// Resets the meters, etc.  This should be called when a ResourceCenter is wiped out due to starvation, etc.
    void            Reset();
    //@}

protected:
    mutable GetObjectSignalType GetObjectSignal;    ///< the UniverseObject-retreiving signal object for this ResourceCenter

    void Init();                                    ///< initialization that needs to be called by derived class after derived class is constructed

private:
    FocusType  m_primary;
    FocusType  m_secondary;

    virtual const Meter*    GetPopMeter() const = 0;            ///< implimentation should return the population meter to use when calculating meter points for this resource center

    virtual const Meter*    GetMeter(MeterType type) const = 0; ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual Meter*          GetMeter(MeterType type) = 0;       ///< implimentation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void InsertMeter(MeterType meter_type, Meter meter) = 0; ///< implimentation should add \a meter to the object so that it can be accessed with the GetMeter() functions

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void ResourceCenter::serialize(Archive& ar, const unsigned int version)
{
    UniverseObject::Visibility vis;
    if (Archive::is_saving::value) {
        UniverseObject* object = GetObjectSignal();
        assert(object);
        vis = object->GetVisibility(Universe::s_encoding_empire);
    }
    ar  & BOOST_SERIALIZATION_NVP(vis);
    if (Universe::ALL_OBJECTS_VISIBLE ||
        vis == UniverseObject::FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_primary)
            & BOOST_SERIALIZATION_NVP(m_secondary);
    }
}

#endif // _ResourceCenter_h_
