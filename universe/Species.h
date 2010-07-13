// -*- C++ -*-
#ifndef _Species_h_
#define _Species_h_

#include "Enums.h"
#include "Universe.h"

#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>
#include <map>
#include <set>

namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
}

/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FocusType
{
public:
    /** \name Structors */ //@{
    /** default ctor */
    FocusType();
    /** basic ctor */
    FocusType(const std::string& name, const std::string& description, const Condition::ConditionBase* location, const std::string& graphic);
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const;       ///< returns the name for this focus type
    const std::string&              Description() const;///< returns a text description of this focus type
    const Condition::ConditionBase* Location() const;   ///< returns the condition that determines whether an UniverseObject can use this FocusType
    const std::string&              Graphic() const;    ///< returns the name of the grapic file for this focus type
    std::string                     Dump() const;       ///< returns a data file format representation of this object
    //@}

private:
    std::string                                         m_name;
    std::string                                         m_description;
    boost::shared_ptr<const Condition::ConditionBase>   m_location;
    std::string                                         m_graphic;
};

/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other 
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class Species
{
public:
    /** \name Structors */ //@{
    /** basic ctor */
    Species(const std::string& name, const std::string& description,
            const std::vector<FocusType>& foci,
            const std::map<PlanetType, PlanetEnvironment>& planet_environments,
            const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
            const std::string& graphic);
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const;       ///< returns the unique name for this type of species
    const std::string&              Description() const;///< returns a text description of this type of species
    const std::set<int>&            Homeworlds() const; ///< returns the ids of objects that are homeworlds for this species
    std::string                     Dump() const;       ///< returns a data file format representation of this object
    const std::vector<FocusType>&   Foci() const;       ///< returns the focus types this species can use, indexed by name
    const std::map<PlanetType, PlanetEnvironment>&
                                    PlanetEnvironments() const;                         ///< returns a map from PlanetType to the PlanetEnvironment this Species has on that PlanetType
    PlanetEnvironment               GetPlanetEnvironment(PlanetType planet_type) const; ///< returns the PlanetEnvironment this species has on PlanetType \a planet_type
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                                    Effects() const;    ///< returns the EffectsGroups that encapsulate the effects that species of this type have
    const std::string&              Graphic() const;    ///< returns the name of the grapic file for this species
    //@}

    /** \name Mutators */ //@{
    void                            AddHomeworld(int homeworld_id);
    void                            RemoveHomeworld(int homeworld_id);
    void                            SetHomeworlds(const std::set<int>& homeworld_ids);
    //@}

private:
    std::string                             m_name;
    std::string                             m_description;
    std::set<int>                           m_homeworlds;
    std::vector<FocusType>                  m_foci;
    std::map<PlanetType, PlanetEnvironment> m_planet_environments;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                            m_effects;
    std::string                             m_graphic;
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class SpeciesManager
{
public:
    typedef std::map<std::string, Species*>::const_iterator iterator;

    /** \name Accessors */ //@{
    /** returns the building type with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    const Species*          GetSpecies(const std::string& name) const;
    Species*                GetSpecies(const std::string& name);

    /** iterator to the first building type */
    iterator                begin() const;

    /** iterator to the last + 1th building type */
    iterator                end() const;

    /** returns true iff this SpeciesManager is empty. */
    bool                    empty() const;

    /** returns the number of species stored in this manager. */
    int                     NumSpecies() const;

    /** returns the instance of this singleton class; you should use the free
      * function GetSpeciesManager() instead */
    static SpeciesManager&  GetSpeciesManager();
    //@}

private:
    SpeciesManager();
    ~SpeciesManager();

    /** sets the homeworld ids of species in this SpeciesManager to those
      * specified in \a species_homeworld_ids */
    void                    SetSpeciesHomeworlds(const std::map<std::string, std::set<int> >& species_homeworld_ids);

    /** returns a map from species name to a set of object IDs that are the
      * homeworld(s) of that species in the current game. */
    std::map<std::string, std::set<int> >   GetSpeciesHomeworldsMap(int encoding_empire = ALL_EMPIRES) const;

    std::map<std::string, Species*> m_species;

    static SpeciesManager* s_instance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** returns the singleton species manager */
SpeciesManager& GetSpeciesManager();

/** Returns the Species object used to represent species of type \a name.
  * If no such Species exists, 0 is returned instead. */
const Species* GetSpecies(const std::string& name);

// template implementations
template <class Archive>
void SpeciesManager::serialize(Archive& ar, const unsigned int version)
{
    // Don't need to send all the data about species, as this is derived from
    // content data files in species.txt that should be available to any
    // client or server.  Instead, just need to send the gamestate portion of
    // species: their homeworlds in the current game

    std::map<std::string, std::set<int> > species_homeworlds_map;

    if (Archive::is_saving::value) {
        species_homeworlds_map = GetSpeciesHomeworldsMap(Universe::s_encoding_empire);
    }

    ar  & BOOST_SERIALIZATION_NVP(species_homeworlds_map);

    if (Archive::is_loading::value) {
        SetSpeciesHomeworlds(species_homeworlds_map);
    }
}

#endif // _Species_h_
