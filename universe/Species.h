// -*- C++ -*-
#ifndef _Species_h_
#define _Species_h_

#include "Enums.h"
#include "Universe.h"
#include "Condition.h"

#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <string>
#include <vector>
#include <map>
#include <set>

namespace Effect {
    class EffectsGroup;
}

/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FocusType {
public:
    /** \name Structors */ //@{
    /** default ctor */
    FocusType() :
        m_name(),
        m_description(),
        m_location(),
        m_graphic()
    {}
    /** basic ctor */
    FocusType(const std::string& name, const std::string& description,
              const Condition::ConditionBase* location, const std::string& graphic) :
        m_name(name),
        m_description(description),
        m_location(location),
        m_graphic(graphic)
    {}
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const        { return m_name; }          ///< returns the name for this focus type
    const std::string&              Description() const { return m_description; }   ///< returns a text description of this focus type
    const Condition::ConditionBase* Location() const    { return m_location.get(); }///< returns the condition that determines whether an UniverseObject can use this FocusType
    const std::string&              Graphic() const     { return m_graphic; }       ///< returns the name of the grapic file for this focus type
    std::string                     Dump() const;       ///< returns a data file format representation of this object
    //@}

private:
    std::string                                         m_name;
    std::string                                         m_description;
    boost::shared_ptr<const Condition::ConditionBase>   m_location;
    std::string                                         m_graphic;
};

/** Used by parser due to limits on number of sub-items per parsed main item. */
struct SpeciesParams {
    SpeciesParams() :
        playable(false),
        native(false),
        can_colonize(false),
        can_produce_ships(false)
    {}
    SpeciesParams(bool playable_, bool native_, bool can_colonize_, bool can_produce_ships_) :
        playable(playable_),
        native(native_),
        can_colonize(can_colonize_),
        can_produce_ships(can_produce_ships_)
    {}
    bool    playable;
    bool    native;
    bool    can_colonize;
    bool    can_produce_ships;
};

/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other 
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class Species {
public:
    /** \name Structors */ //@{
    /** basic ctor */
    Species(const std::string& name, const std::string& description,
            const std::vector<FocusType>& foci,
            const std::map<PlanetType, PlanetEnvironment>& planet_environments,
            const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects,
            const SpeciesParams& params,
            const std::vector<std::string>& tags,
            const std::string& graphic) :
        m_name(name),
        m_description(description),
        m_foci(foci),
        m_planet_environments(planet_environments),
        m_effects(effects),
        m_playable(params.playable),
        m_native(params.native),
        m_can_colonize(params.can_colonize),
        m_can_produce_ships(params.can_produce_ships),
        m_tags(tags),
        m_graphic(graphic)
    {}
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const        { return m_name; }          ///< returns the unique name for this type of species
    const std::string&              Description() const { return m_description; }   ///< returns a text description of this type of species
    const std::set<int>&            Homeworlds() const  { return m_homeworlds; }    ///< returns the ids of objects that are homeworlds for this species
    std::string                     Dump() const;                                   ///< returns a data file format representation of this object
    const std::vector<FocusType>&   Foci() const        { return m_foci; }          ///< returns the focus types this species can use, indexed by name
    const std::map<PlanetType, PlanetEnvironment>& PlanetEnvironments() const { return m_planet_environments; } ///< returns a map from PlanetType to the PlanetEnvironment this Species has on that PlanetType
    PlanetEnvironment               GetPlanetEnvironment(PlanetType planet_type) const;         ///< returns the PlanetEnvironment this species has on PlanetType \a planet_type
    PlanetType                      NextBetterPlanetType(PlanetType initial_planet_type) const; ///< returns the next better PlanetType for this species from the \a initial_planet_type specified
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const { return m_effects; }///< returns the EffectsGroups that encapsulate the effects that species of this type have
    bool                            Playable() const        { return m_playable; }          ///< returns whether this species is a suitable starting species for players
    bool                            Native() const          { return m_native; }            ///< returns whether this species is a suitable native species (for non player-controlled planets)
    bool                            CanColonize() const     { return m_can_colonize; }      ///< returns whether this species can colonize planets
    bool                            CanProduceShips() const { return m_can_produce_ships; } ///< returns whether this species can produce ships
    const std::vector<std::string>& Tags() const            { return m_tags; }
    const std::string&              Graphic() const         { return m_graphic; }           ///< returns the name of the grapic file for this species
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
    bool                                    m_playable;
    bool                                    m_native;
    bool                                    m_can_colonize;
    bool                                    m_can_produce_ships;
    std::vector<std::string>                m_tags;
    std::string                             m_graphic;
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class SpeciesManager {
private:
    struct PlayableSpecies
    { bool operator()(const std::map<std::string, Species*>::value_type& species_map_iterator) const; };
    struct NativeSpecies
    { bool operator()(const std::map<std::string, Species*>::value_type& species_map_iterator) const; };
public:
    typedef std::map<std::string, Species*>::const_iterator     iterator;
    typedef boost::filter_iterator<PlayableSpecies, iterator>   playable_iterator;
    typedef boost::filter_iterator<NativeSpecies, iterator>     native_iterator;

    /** \name Accessors */ //@{
    /** returns the building type with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    const Species*          GetSpecies(const std::string& name) const;
    Species*                GetSpecies(const std::string& name);

    /** iterators for all species */
    iterator                begin() const;
    iterator                end() const;

    /** iterators for playble species. */
    playable_iterator       playable_begin() const;
    playable_iterator       playable_end() const;

    /** iterators for native species. */
    native_iterator         native_begin() const;
    native_iterator         native_end() const;

    /** returns true iff this SpeciesManager is empty. */
    bool                    empty() const;

    /** returns the number of species stored in this manager. */
    int                     NumSpecies() const;
    int                     NumPlayableSpecies() const;
    int                     NumNativeSpecies() const;

    /** returns the name of a species in this manager, or an empty string if
      * this manager is empty. */
    const std::string&      RandomSpeciesName() const;

    /** returns the name of a playable species in this manager, or an empty
      * string if there are no playable species. */
    const std::string&      RandomPlayableSpeciesName() const;

    /** returns the instance of this singleton class; you should use the free
      * function GetSpeciesManager() instead */
    static SpeciesManager&  GetSpeciesManager();
    //@}

    /** \name Mutators */ //@{
    /** sets all species to have no homeworlds.  this is useful when generating
      * a new game, when any homeworlds species had in the previous game should
      * be removed before the new game's homeworlds are added. */
    void                    ClearSpeciesHomeworlds();
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
