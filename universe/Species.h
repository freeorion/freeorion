#ifndef _Species_h_
#define _Species_h_


#include "EnumsFwd.h"
#include "../util/Export.h"
#include "../util/Serialize.h"
#include "../util/Pending.h"

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/optional/optional.hpp>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>


namespace Condition {
    struct ConditionBase;
}
namespace Effect {
    class EffectsGroup;
}

FO_COMMON_API extern const int ALL_EMPIRES;

/** A setting that a ResourceCenter can be assigned to influence what it
  * produces.  Doesn't directly affect the ResourceCenter, but effectsgroups
  * can use activation or scope conditions that check whether a potential
  * target has a particular focus.  By this method, techs or buildings or
  * species can act on planets or other ResourceCenters depending what their
  * focus setting is. */
class FO_COMMON_API FocusType {
public:
    /** \name Structors */ //@{
    FocusType() :
        m_name(),
        m_description(),
        m_location(),
        m_graphic()
    {}

    FocusType(const std::string& name, const std::string& description,
              std::unique_ptr<Condition::ConditionBase>&& location,
              const std::string& graphic);

    ~FocusType();
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const        { return m_name; }          ///< returns the name for this focus type
    const std::string&              Description() const { return m_description; }   ///< returns a text description of this focus type
    const Condition::ConditionBase* Location() const    { return m_location.get(); }///< returns the condition that determines whether an UniverseObject can use this FocusType
    const std::string&              Graphic() const     { return m_graphic; }       ///< returns the name of the grapic file for this focus type
    std::string                     Dump(unsigned short ntabs = 0) const;       ///< returns a data file format representation of this object

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;
    //@}

private:
    std::string                                     m_name;
    std::string                                     m_description;
    std::shared_ptr<const Condition::ConditionBase> m_location;
    std::string                                     m_graphic;
};

/** Used by parser due to limits on number of sub-items per parsed main item. */
struct SpeciesParams {
    SpeciesParams()
    {}
    SpeciesParams(bool playable_, bool native_, bool can_colonize_, bool can_produce_ships_) :
        playable(playable_),
        native(native_),
        can_colonize(can_colonize_),
        can_produce_ships(can_produce_ships_)
    {}
    bool playable = false;
    bool native = false;
    bool can_colonize = false;
    bool can_produce_ships = false;
};

/** Used by parser due to limits on number of sub-items per parsed main item. */
struct SpeciesStrings {
    SpeciesStrings()
    {}
    SpeciesStrings(const std::string& name_, const std::string& desc_,
                   const std::string& gameplay_desc_) :
        name(name_),
        desc(desc_),
        gameplay_desc(gameplay_desc_)
    {}
    std::string name;
    std::string desc;
    std::string gameplay_desc;
};

/** A predefined type of population that can exist on a PopulationCenter.
  * Species have associated sets of EffectsGroups, and various other 
  * properties that affect how the object on which they reside functions.
  * Each kind of Species must have a \a unique name string, by which it can be
  * looked up using GetSpecies(). */
class FO_COMMON_API Species {
public:
    /** \name Structors */ //@{
    Species(const SpeciesStrings& strings,
            const std::vector<FocusType>& foci,
            const std::string& preferred_focus,
            const std::map<PlanetType, PlanetEnvironment>& planet_environments,
            std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
            std::unique_ptr<Condition::ConditionBase>&& combat_targets,
            const SpeciesParams& params,
            const std::set<std::string>& tags,
            const std::string& graphic);

    ~Species();
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const                        { return m_name; }                  ///< returns the unique name for this type of species
    const std::string&              Description() const                 { return m_description; }           ///< returns a text description of this type of species
    /** returns a text description of this type of species */
    std::string                     GameplayDescription() const;

    const std::set<int>&                    Homeworlds() const          { return m_homeworlds; }            ///< returns the ids of objects that are homeworlds for this species
    const std::map<int, double>&            EmpireOpinions() const      { return m_empire_opinions; }       ///< returns the positive/negative opinions of this species about empires
    const std::map<std::string, double>&    OtherSpeciesOpinions() const{ return m_other_species_opinions; }///< returns the positive/negative opinions of this species about other species

    const Condition::ConditionBase*         Location() const            { return m_location.get(); }        ///< returns the condition determining what planets on which this species may spawn
    const Condition::ConditionBase*         CombatTargets() const       { return m_combat_targets.get(); }  ///< returns the condition for possible targets. may be nullptr if no condition was specified.

    std::string                     Dump(unsigned short ntabs = 0) const;                                           ///< returns a data file format representation of this object
    const std::vector<FocusType>&   Foci() const            { return m_foci; }              ///< returns the focus types this species can use
    const std::string&              PreferredFocus() const  { return m_preferred_focus; }   ///< returns the name of the planetary focus this species prefers. Default for new colonies and may affect happiness if on a different focus?
    const std::map<PlanetType, PlanetEnvironment>& PlanetEnvironments() const { return m_planet_environments; } ///< returns a map from PlanetType to the PlanetEnvironment this Species has on that PlanetType
    PlanetEnvironment               GetPlanetEnvironment(PlanetType planet_type) const;     ///< returns the PlanetEnvironment this species has on PlanetType \a planet_type
    PlanetType                      NextBetterPlanetType(PlanetType initial_planet_type) const; ///< returns the next better PlanetType for this species from the \a initial_planet_type specified

    /** Returns the EffectsGroups that encapsulate the effects that species of
        this type have. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    bool                            Playable() const        { return m_playable; }          ///< returns whether this species is a suitable starting species for players
    bool                            Native() const          { return m_native; }            ///< returns whether this species is a suitable native species (for non player-controlled planets)
    bool                            CanColonize() const     { return m_can_colonize; }      ///< returns whether this species can colonize planets
    bool                            CanProduceShips() const { return m_can_produce_ships; } ///< returns whether this species can produce ships
    const std::set<std::string>&    Tags() const            { return m_tags; }
    const std::string&              Graphic() const         { return m_graphic; }           ///< returns the name of the grapic file for this species

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                    GetCheckSum() const;
    //@}

    /** \name Mutators */ //@{
    void AddHomeworld(int homeworld_id);
    void RemoveHomeworld(int homeworld_id);
    void SetHomeworlds(const std::set<int>& homeworld_ids);
    void SetEmpireOpinions(const std::map<int, double>& opinions);
    void SetEmpireOpinion(int empire_id, double opinion);
    void SetOtherSpeciesOpinions(const std::map<std::string, double>& opinions);
    void SetOtherSpeciesOpinion(const std::string& species_name, double opinion);
    //@}

private:
    void    Init();

    std::string                             m_name;
    std::string                             m_description;
    std::string                             m_gameplay_description;

    std::set<int>                           m_homeworlds;
    std::map<int, double>                   m_empire_opinions;          // positive/negative rating of how this species views empires in the game
    std::map<std::string, double>           m_other_species_opinions;   // positive/negative rating of how this species views other species in the game

    std::vector<FocusType>                  m_foci;
    std::string                             m_preferred_focus;
    std::map<PlanetType, PlanetEnvironment> m_planet_environments;

    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    std::unique_ptr<Condition::ConditionBase>           m_location;
    std::unique_ptr<Condition::ConditionBase>           m_combat_targets;

    bool                                    m_playable;
    bool                                    m_native;
    bool                                    m_can_colonize;
    bool                                    m_can_produce_ships;
    std::set<std::string>                   m_tags;
    std::string                             m_graphic;
};


/** Holds all FreeOrion species.  Types may be looked up by name. */
class FO_COMMON_API SpeciesManager {
private:
    struct FO_COMMON_API PlayableSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };
    struct FO_COMMON_API NativeSpecies
    { bool operator()(const std::map<std::string, std::unique_ptr<Species>>::value_type& species_entry) const; };

public:
    using SpeciesTypeMap = std::map<std::string, std::unique_ptr<Species>>;
    using CensusOrder = std::vector<std::string>;
    using iterator = SpeciesTypeMap::const_iterator;
    typedef boost::filter_iterator<PlayableSpecies, iterator>   playable_iterator;
    typedef boost::filter_iterator<NativeSpecies, iterator>     native_iterator;

    /** \name Accessors */ //@{
    /** returns the building type with the name \a name; you should use the
      * free function GetSpecies() instead, mainly to save some typing. */
    const Species*      GetSpecies(const std::string& name) const;
    Species*            GetSpecies(const std::string& name);

    /** returns a unique numeric id for reach species, or -1 for an invalid species name. */
    int                 GetSpeciesID(const std::string& name) const;

    /** iterators for all species */
    iterator            begin() const;
    iterator            end() const;

    /** iterators for playble species. */
    playable_iterator   playable_begin() const;
    playable_iterator   playable_end() const;

    /** iterators for native species. */
    native_iterator     native_begin() const;
    native_iterator     native_end() const;

    /** returns an ordered list of tags that should be considered for census listings. */
    const CensusOrder&  census_order() const;

    /** returns true iff this SpeciesManager is empty. */
    bool                empty() const;

    /** returns the number of species stored in this manager. */
    int                 NumSpecies() const;
    int                 NumPlayableSpecies() const;
    int                 NumNativeSpecies() const;

    /** returns the name of a species in this manager, or an empty string if
      * this manager is empty. */
    const std::string&  RandomSpeciesName() const;

    /** returns the name of a playable species in this manager, or an empty
      * string if there are no playable species. */
    const std::string&  RandomPlayableSpeciesName() const;
    const std::string&  SequentialPlayableSpeciesName(int id) const;

    /** returns a map from species name to a set of object IDs that are the
      * homeworld(s) of that species in the current game. */
    std::map<std::string, std::set<int>>
        GetSpeciesHomeworldsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns a map from species name to a map from empire id to each the
      * species' opinion of the empire */
    const std::map<std::string, std::map<int, float>>&
        GetSpeciesEmpireOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a species_name about empire with
      * id \a empire_id or 0.0 if there is no such opinion yet recorded. */
    float SpeciesEmpireOpinion(const std::string& species_name, int empire_id) const;

    /** returns a map from species name to a map from other species names to the
      * opinion of the first species about the other species. */
    const std::map<std::string, std::map<std::string, float>>&
        GetSpeciesSpeciesOpinionsMap(int encoding_empire = ALL_EMPIRES) const;

    /** returns opinion of species with name \a opinionated_species_name about
      * other species with name \a rated_species_name or 0.0 if there is no
      * such opinion yet recorded. */
    float SpeciesSpeciesOpinion(const std::string& opinionated_species_name,
                                const std::string& rated_species_name) const;

    /** returns the instance of this singleton class; you should use the free
      * function GetSpeciesManager() instead */
    static SpeciesManager&  GetSpeciesManager();

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;
    //@}

    /** \name Mutators */ //@{
    /** sets all species to have no homeworlds.  this is useful when generating
      * a new game, when any homeworlds species had in the previous game should
      * be removed before the new game's homeworlds are added. */
    void ClearSpeciesHomeworlds();

    /** sets the opinions of species (indexed by name string) of empires (indexed
      * by id) as a double-valued number. */
    void SetSpeciesEmpireOpinions(const std::map<std::string, std::map<int, float>>& species_empire_opinions);
    void SetSpeciesEmpireOpinion(const std::string& species_name, int empire_id, float opinion);

    /** sets the opinions of species (indexed by name string) of other species
      * (indexed by name string) as a double-valued number. */
    void SetSpeciesSpeciesOpinions(const std::map<std::string, std::map<std::string, float>>& species_species_opinions);
    void SetSpeciesSpeciesOpinion(const std::string& opinionated_species, const std::string& rated_species, float opinion);

    /** clears all species opinion data */
    void ClearSpeciesOpinions();

    void UpdatePopulationCounter();

    std::map<std::string, std::map<int, float>>&        SpeciesObjectPopulations(int encoding_empire = ALL_EMPIRES);
    std::map<std::string, std::map<std::string, int>>&  SpeciesShipsDestroyed(int encoding_empire = ALL_EMPIRES);

    /** Sets species types to the value of \p future. */
    void SetSpeciesTypes(Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>&& future);
    //@}

private:
    SpeciesManager();

    /** sets the homeworld ids of species in this SpeciesManager to those
      * specified in \a species_homeworld_ids */
    void SetSpeciesHomeworlds(const std::map<std::string, std::set<int>>& species_homeworld_ids);

    /** Assigns any m_pending_types to m_species. */
    void CheckPendingSpeciesTypes() const;

    /** Future types being parsed by parser.  mutable so that it can
        be assigned to m_species_types when completed.*/
    mutable boost::optional<Pending::Pending<std::pair<SpeciesTypeMap, CensusOrder>>> m_pending_types = boost::none;

    mutable SpeciesTypeMap                              m_species;
    mutable CensusOrder                                 m_census_order;
    std::map<std::string, std::map<int, float>>         m_species_empire_opinions;
    std::map<std::string, std::map<std::string, float>> m_species_species_opinions;

    std::map<std::string, std::map<int, float>>         m_species_object_populations;
    std::map<std::string, std::map<std::string, int>>   m_species_species_ships_destroyed;

    static SpeciesManager* s_instance;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

extern template
FO_COMMON_API void SpeciesManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void SpeciesManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void SpeciesManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void SpeciesManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);

/** returns the singleton species manager */
FO_COMMON_API SpeciesManager& GetSpeciesManager();

/** Returns the Species object used to represent species of type \a name.
  * If no such Species exists, 0 is returned instead. */
FO_COMMON_API const Species* GetSpecies(const std::string& name);

#endif // _Species_h_
