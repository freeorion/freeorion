// -*- C++ -*-
#ifndef _Building_h_
#define _Building_h_

#ifndef _UniverseObject_h_
#include "UniverseObject.h"
#endif

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
    Building(const GG::XMLElement& elem); ///< ctor that constructs a Building object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Building object
    //@}

    /** \name Accessors */ //@{
    const std::string& BuildingTypeName() const; ///< returns the name of the type of building this is
    bool               Operating() const;        ///< true iff this building is currently operating
    int                PlanetID() const;         ///< returns the ID number of the planet this building is on
    Planet*            GetPlanet() const;        ///< returns a pointer to the planet this building is on

    virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a Building object with visibility limited relative to the input empire
    //@}

    /** \name Mutators */ //@{
    void Activate(bool activate);    ///< activates or deactivates the building
    void SetPlanetID(int planet_id); ///< sets the planet on which the building is located

    void ExecuteEffects();           ///< executes the effects of the building

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
    BuildingType(const std::string& name, const std::string& description, const Effect::EffectsGroup* effects); ///< basic ctor
    BuildingType(const GG::XMLElement& elem); ///< ctor that constructs a BuildingType object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a BuildingType object
    //@}

    /** \name Accessors */ //@{
    const std::string&          Name() const;             ///< returns the unique name for this type of building
    const std::string&          Description() const;      ///< returns a text description of this type of building
    double                      BuildCost() const;        ///< returns the number of production points required to build this building
    double                      MaintenanceCost() const;  ///< returns the number of monetary points required per turn to operate this building
    const Effect::EffectsGroup* Effects() const;          ///< returns the EffectsGroup that encapsulates the effects that buildings of this type have when operational
    //@}

private:
    std::string                 m_name;
    std::string                 m_description;
    double                      m_build_cost;
    double                      m_maintenance_cost;
    const Effect::EffectsGroup* m_effects;
};

/** Returns the BuildingType specification object for a building of type \a name.  If no such BuildingType
    exists, 0 is returned instead. */
BuildingType* GetBuildingType(const std::string& name);


inline std::pair<std::string, std::string> BuildingRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Building_h_
