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

/** */
class Building : public UniverseObject
{
public:
    /** \name Structors */ //@{
    Building();                                                ///< default ctor
    Building(int empire_id, const std::string& building_type, int planet_id); ///< basic ctor
    Building(const GG::XMLElement& elem); ///< ctor that constructs a Building object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a Building object
    //@}

    /** \name Accessors */ //@{
    const std::string& BuildingTypeName() const; ///< returns the name of the type of building this is
    int                PlanetID() const;         ///< returns the ID number of the planet this building is on
    Planet*            GetPlanet() const;        ///< returns a pointer to the planet this building is on

    virtual GG::XMLElement XMLEncode(int empire_id = Universe::ALL_EMPIRES) const; ///< constructs an XMLElement from a Building object with visibility limited relative to the input empire
    //@}

    /** \name Mutators */ //@{
    void SetPlanetID(int planet_id);

    virtual void MovementPhase();
    virtual void PopGrowthProductionResearchPhase();
    //@}

private:
    std::string m_building_type;
    int         m_planet_id;
};

/** */
class BuildingType
{
public:
    /** \name Structors */ //@{
    BuildingType();
    BuildingType(const std::string& name, const std::string& description, const Effect::EffectsGroup* effects);
    BuildingType(const GG::XMLElement& elem); ///< ctor that constructs a BuildingType object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a BuildingType object
    //@}

    /** \name Accessors */ //@{
    const std::string&          Name() const;
    const std::string&          Description() const;
    const Effect::EffectsGroup* Effects() const;
    //@}

private:
    std::string                 m_name;
    std::string                 m_description;
    const Effect::EffectsGroup* m_effects;
};

class BuildingTypeManager
{
public:
    BuildingTypeManager();

    BuildingType* GetBuildingType(const std::string& name) const;

private:
    std::map<std::string, BuildingType*> m_building_types;
};

BuildingType* GetBuildingType(const std::string& name);

#endif // _Building_h_
