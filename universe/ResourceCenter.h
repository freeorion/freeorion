// -*- C++ -*-
#ifndef _ResourceCenter_h_
#define _ResourceCenter_h_

#include "Enums.h"
#include <boost/signal.hpp>
#include <boost/serialization/nvp.hpp>

#include "../util/Export.h"

class Empire;
class Meter;
class UniverseObject;

/** The ResourceCenter class is an abstract base class for anything in the
  * FreeOrion gamestate that generates resources (minerals, etc.).  Most
  * likely, such an object will also be a subclass of UniverseObject.
  *  
  * Planet is the most obvious class to inherit ResourceCenter, but other
  * classes could be made from it as well (e.g., a trade-ship or mining vessel,
  * or a non-Planet UniverseObject- and PopCenter- derived object of some
  * sort. */
class FO_COMMON_API ResourceCenter {
public:
    /** \name Structors */ //@{
    ResourceCenter();                               ///< default ctor
    virtual ~ResourceCenter();                      ///< dtor
    ResourceCenter(const ResourceCenter& rhs);      ///< copy ctor
    //@}

    /** \name Accessors */ //@{
    const std::string&              Focus() const;                                  ///< current focus to which this ResourceCenter is set
    virtual std::vector<std::string>AvailableFoci() const;                          ///< focus settings available to this ResourceCenter
    virtual const std::string&      FocusIcon(const std::string& focus_name) const; ///< icon representing focus with name \a focus_name for this ResourceCenter

    std::string     Dump() const;

    virtual float   InitialMeterValue(MeterType type) const = 0;            ///< implementation should return the initial value of the specified meter \a type
    virtual float   CurrentMeterValue(MeterType type) const = 0;            ///< implementation should return the current value of the specified meter \a type
    virtual float   NextTurnCurrentMeterValue(MeterType type) const = 0;    ///< implementation should return an estimate of the next turn's current value of the specified meter \a type

    mutable boost::signal<void ()> ResourceCenterChangedSignal;             ///< the state changed signal object for this ResourceCenter
    //@}

    /** \name Mutators */ //@{
    void            Copy(const ResourceCenter* copied_object, Visibility vis = VIS_FULL_VISIBILITY);

    void            SetFocus(const std::string& focus);

    virtual void    Reset();                                                        ///< Resets the meters, etc.  This should be called when a ResourceCenter is wiped out due to starvation, etc.
    //@}

protected:
    void            Init();                                                         ///< initialization that needs to be called by derived class after derived class is constructed

    double          ResourceCenterNextTurnMeterValue(MeterType meter_type) const;   ///< returns estimate of the next turn's current values of meters relevant to this ResourceCenter
    void            ResourceCenterResetTargetMaxUnpairedMeters();
    void            ResourceCenterClampMeters();

    void            ResourceCenterPopGrowthProductionResearchPhase();


private:
    std::string m_focus;

    virtual Visibility      GetVisibility(int empire_id) const = 0;         ///< implementation should return the visibility of this ResourceCenter for the empire with the specified \a empire_id
    virtual const Meter*    GetMeter(MeterType type) const = 0;             ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object
    virtual Meter*          GetMeter(MeterType type) = 0;                   ///< implementation should return the requested Meter, or 0 if no such Meter of that type is found in this object

    virtual void            AddMeter(MeterType meter_type) = 0;             ///< implementation should add a meter to the object so that it can be accessed with the GetMeter() functions

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void ResourceCenter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_focus);
}

#endif // _ResourceCenter_h_
