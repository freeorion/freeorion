// -*- C++ -*-
#ifndef _Building_h_
#define _Building_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

class BuildingType;
namespace Effect {
    class EffectsGroup;
}
class Planet;

/** A Building UniverseObject type. */
class Building : public UniverseObject
{
public:
    /** \name Structors */ //@{
    Building(); ///< default ctor
    Building(int empire_id, const std::string& building_type, int planet_id); ///< basic ctor
    Building(const XMLElement& elem); ///< ctor that constructs a Building object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Building object
    //@}

    /** \name Accessors */ //@{
    /** returns the BuildingType object for this building, specific to the owning empire (or the default version if there
        is other than exactly one owner) */
    const BuildingType* GetBuildingType() const;

    const std::string&  BuildingTypeName() const; ///< returns the name of the BuildingType object for this building
    bool                Operating() const;        ///< returns true iff this building is currently operating
    int                 PlanetID() const;         ///< returns the ID number of the planet this building is on
    Planet*             GetPlanet() const;        ///< returns a pointer to the planet this building is on

    virtual XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a Building object with visibility limited relative to the input empire

    virtual UniverseObject* Accept(const UniverseObjectVisitor& visitor) const;
    //@}

    /** \name Mutators */ //@{
    void Activate(bool activate);    ///< activates or deactivates the building
    void SetPlanetID(int planet_id); ///< sets the planet on which the building is located

    virtual void MovementPhase();
    virtual void PopGrowthProductionResearchPhase();
    //@}

private:
    std::string m_building_type;
    bool        m_operating;
    int         m_planet_id;
};

/** A specification for a building of a certain type.  Each building type must have a \a unique name string, 
    by which it can be looked up using GetBuildingType(). */
class BuildingType
{
public:
    /** \name Structors */ //@{
    BuildingType(); ///< default ctor
    BuildingType(const std::string& name, const std::string& description); ///< basic ctor
    BuildingType(const XMLElement& elem); ///< ctor that constructs a BuildingType object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a BuildingType object
    //@}

    /** \name Accessors */ //@{
    const std::string&          Name() const;             ///< returns the unique name for this type of building
    const std::string&          Description() const;      ///< returns a text description of this type of building
    double                      BuildCost() const;        ///< returns the number of production points required to build this building
    int                         BuildTime() const;        ///< returns the number of turns required to build this building
    double                      MaintenanceCost() const;  ///< returns the number of monetary points required per turn to operate this building
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                                Effects() const;          ///< returns the EffectsGroups that encapsulate the effects that buildings of this type have when operational
    const std::string&          Graphic() const;          ///< returns the name of the grapic file for this building type
    //@}

    /** \name Mutators */ //@{
    void AddEffects(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects);
    //@}

private:
    std::string                                                 m_name;
    std::string                                                 m_description;
    double                                                      m_build_cost;
    int                                                         m_build_time;
    double                                                      m_maintenance_cost;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> > m_effects;
    std::string                                                 m_graphic;
};

/** Returns the BuildingType specification object for a building of type \a name.  If no such BuildingType
    exists, 0 is returned instead. */
const BuildingType* GetBuildingType(const std::string& name);


inline std::string BuildingRevision()
{return "$Id$";}

#endif // _Building_h_
