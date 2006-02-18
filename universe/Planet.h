// -*- C++ -*-
#ifndef _Planet_h_
#define _Planet_h_

#ifndef _Universe_h_
#include "Universe.h"
#endif

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

#ifndef _PopCenter_h_
#include "PopCenter.h"
#endif

#ifndef _ResourceCenter_h_
#include "ResourceCenter.h"
#endif

#ifndef _Ship_h_
#include "Ship.h"
#endif


/** a class representing a FreeOrion planet.*/
class Planet :
    public UniverseObject,
    public PopCenter,
    public ResourceCenter
{
public:
    /** \name Structors */ //@{
    Planet(); ///< default ctor
    Planet(PlanetType type, PlanetSize size); ///< general ctor taking just the planet's type and size
    Planet(const XMLElement& elem); ///< ctor that constructs a Planet object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Planet object
    //@}

    /** \name Accessors */ //@{
    PlanetType           Type() const {return m_type;}
    PlanetSize           Size() const {return m_size;}
    PlanetEnvironment    Environment() const;
    const std::set<int>& Buildings() const {return m_buildings;}

    double AvailableTrade() const; ///< returns the trade available at this planet for use in building maintenance
    double BuildingCosts() const;  ///< returns the cost in trade for the upkeep of all currently-enabled buildings

    /** Returns true iff this Planet contains a building with ID \a id. */
    bool ContainsBuilding(int id) const {return m_buildings.find(id) != m_buildings.end();}

    bool IsAboutToBeColonized() const {return m_is_about_to_be_colonized;}

    virtual const Meter* GetMeter(MeterType type) const;

    /////////////////////////////////////////////////////////////////////////////
    // V0.2 ONLY!!!!
    int DefBases() const {return m_def_bases;}
    // V0.2 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////
   
    virtual UniverseObject::Visibility GetVisibility(int empire_id) const; ///< returns the visibility status of this universe object relative to the input empire.
    virtual XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a planet object with visibility limited relative to the input empire

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;
    //@}
  	
    /** \name Mutators */ //@{
    virtual Meter* GetMeter(MeterType type);

    virtual void MovementPhase( );
    virtual void AdjustMaxMeters( );
    virtual void PopGrowthProductionResearchPhase( );

    /////////////////////////////////////////////////////////////////////////////
    // V0.2 ONLY!!!!
    void AdjustDefBases(int bases) {m_def_bases += bases; if (m_def_bases < 0) m_def_bases = 0; StateChangedSignal();}
    // V0.2 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////

    void SetType(PlanetType type);        ///< sets the type of this Planet to \a type
    void SetSize(PlanetSize size);        ///< sets the size of this Planet to \a size
    void AddBuilding(int building_id);    ///< adds the building to the planet
    bool RemoveBuilding(int building_id); ///< removes the building from the planet; returns false if no such building was found
    bool DeleteBuilding(int building_id); ///< removes the building from the planet and deletes it; returns false if no such building was found

    void SetAvailableTrade(double trade); ///< sets the trade available at this planet for use in building maintenance

    virtual void AddOwner   (int id); ///< adds the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets
    virtual void RemoveOwner(int id); ///< removes the Empire with ID \a id to the list of owners of this planet, update system owners and empire planets

    /// Resets the meters, specials, etc., of a planet to an unowned state.  This should be called when a planet is wiped out due to starvation, etc.
    void Reset();

    /// Called during combat when a planet changes hands
    void Conquer( int conquerer ) ;
    
    /// Called during colonization when a planet is about to be colonized
    void SetIsAboutToBeColonized(bool b);

    /// Called after colonization, to reset the number of prospective colonizers to 0
    void ResetIsAboutToBeColonized();
    //@}

    static PlanetEnvironment Environment(PlanetType type); ///< returns the environment that corresponds to each planet type

private:
    UniverseObject* This();

    PlanetType    m_type;
    PlanetSize    m_size;
    std::set<int> m_buildings;
    double        m_available_trade;
   
    bool m_just_conquered;

    int m_is_about_to_be_colonized;

    /////////////////////////////////////////////////////////////////////////////
    // V0.3 ONLY!!!!
    int            m_def_bases;
    // V0.3 ONLY!!!!
    /////////////////////////////////////////////////////////////////////////////

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Planet::serialize(Archive& ar, const unsigned int version)
{
    Visibility vis;
    if (Archive::is_saving::value)
        vis = GetVisibility(Universe::s_encoding_empire);
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(UniverseObject)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(PopCenter)
        & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ResourceCenter)
        & BOOST_SERIALIZATION_NVP(vis)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_just_conquered);
    if (ALL_OBJECTS_VISIBLE ||
        vis == FULL_VISIBILITY) {
        ar  & BOOST_SERIALIZATION_NVP(m_buildings)
            & BOOST_SERIALIZATION_NVP(m_available_trade)
            & BOOST_SERIALIZATION_NVP(m_is_about_to_be_colonized)
            & BOOST_SERIALIZATION_NVP(m_def_bases);
    }
}

inline std::string PlanetRevision()
{return "$Id$";}

#endif // _Planet_h_


