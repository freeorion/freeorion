// -*- C++ -*-
#ifndef _ResourceCenter_h_
#define _ResourceCenter_h_

#ifndef _Meter_h_
#include "Meter.h"
#endif

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _Ship_h_
#include "Ship.h"
#endif

#ifndef _GG_Enum_h_
#include <GG/Enum.h>
#endif

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
    ResourceCenter(const Meter& pop); ///< default ctor
    ResourceCenter(const XMLElement& elem, const Meter& pop); ///< ctor that constructs a ResourceCenter object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a ResourceCenter object
    virtual ~ResourceCenter(); ///< dtor
    //@}

    /** \name Accessors */ //@{
    FocusType      PrimaryFocus() const     {return m_primary;}
    FocusType      SecondaryFocus() const   {return m_secondary;}
    const Meter&   FarmingMeter() const     {return m_farming;}      ///< returns the farming Meter for this center
    const Meter&   IndustryMeter() const    {return m_industry;}     ///< returns the industry Meter for this center
    const Meter&   MiningMeter() const      {return m_mining;}       ///< returns the mining Meter for this center
    const Meter&   ResearchMeter() const    {return m_research;}     ///< returns the research Meter for this center
    const Meter&   TradeMeter() const       {return m_trade;}        ///< returns the trade Meter for this center
    const Meter&   ConstructionMeter() const{return m_construction;} ///< returns the construction Meter for this center
    const Meter*   GetMeter(MeterType type) const;
    double         FarmingPoints() const;
    double         IndustryPoints() const;
    double         MiningPoints() const;
    double         ResearchPoints() const;
    double         TradePoints() const;

    /** Returns the Current() value that the requested type of meter is projected to have next turn.  This projection is
        only a rough estimate, based on the meter's current and max from this turn. */
    double         ProjectedCurrent(MeterType type) const;

    virtual XMLElement XMLEncode(UniverseObject::Visibility vis) const; ///< constructs an XMLElement from a ResourceCenter object with the given visibility
     
    mutable ResourceCenterChangedSignalType ResourceCenterChangedSignal; ///< the state changed signal object for this ResourceCenter
    //@}

    /** \name Mutators */ //@{
    void     SetPrimaryFocus(FocusType focus);
    void     SetSecondaryFocus(FocusType focus);
    Meter&   FarmingMeter()                  {return m_farming;}      ///< returns the farming Meter for this center
    Meter&   IndustryMeter()                 {return m_industry;}     ///< returns the industry Meter for this center
    Meter&   MiningMeter()                   {return m_mining;}       ///< returns the mining Meter for this center
    Meter&   ResearchMeter()                 {return m_research;}     ///< returns the research Meter for this center
    Meter&   TradeMeter()                    {return m_trade;}        ///< returns the trade Meter for this center
    Meter&   ConstructionMeter()             {return m_construction;} ///< returns the construction Meter for this center
    Meter*   GetMeter(MeterType type);

    virtual void AdjustMaxMeters();
    virtual void PopGrowthProductionResearchPhase();

    /// Resets the meters, etc.  This should be called when a ResourceCenter is wiped out due to starvation, etc.
    void Reset();
    //@}

protected:
    mutable GetObjectSignalType GetObjectSignal; ///< the UniverseObject-retreiving signal object for this ResourceCenter

private:
    FocusType  m_primary;
    FocusType  m_secondary;

    Meter      m_farming;
    Meter      m_industry;
    Meter      m_mining;
    Meter      m_research;
    Meter      m_trade;
    Meter      m_construction;

    const Meter& m_pop;    ///< current / max pop present in this center (may be the one from m_object, e.g. if m_object is a Planet)

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
    if (UniverseObject::ALL_OBJECTS_VISIBLE ||
        vis == UniverseObject::FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_primary)
            & BOOST_SERIALIZATION_NVP(m_secondary)
            & BOOST_SERIALIZATION_NVP(m_farming)
            & BOOST_SERIALIZATION_NVP(m_industry)
            & BOOST_SERIALIZATION_NVP(m_mining)
            & BOOST_SERIALIZATION_NVP(m_research)
            & BOOST_SERIALIZATION_NVP(m_trade)
            & BOOST_SERIALIZATION_NVP(m_construction)
            & BOOST_SERIALIZATION_NVP(m_pop);
    }
}

inline std::string ResourceCenterRevision()
{return "$Id$";}

#endif // _ResourceCenter_h_
