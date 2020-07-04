#ifndef _Special_h_
#define _Special_h_


#include <map>
#include <memory>
#include <string>
#include <vector>
#include <boost/optional/optional.hpp>
#include "../focs/focs.hpp"
#include "../util/Export.h"
#include "../util/Pending.h"


/** A predefined set of EffectsGroups that can be attached to a UniverseObject
  * (often referred to as the "source" object).  The effects of a Special are
  * not limited to the object to which it is attached.  Each kind of Special
  * must have a \a unique name string, by which it can be looked up using
  * GetSpecial(). */
class FO_COMMON_API Special {
public:
    Special(std::string&& name, std::string&& description,
            std::unique_ptr<focs::ValueRef<double>>&& stealth,
            std::vector<std::unique_ptr<focs::EffectsGroup>>&& effects,
            double spawn_rate = 1.0, int spawn_limit = 99999,
            std::unique_ptr<focs::ValueRef<double>>&& initial_capaicty = nullptr,
            std::unique_ptr<focs::Condition>&& location = nullptr,
            const std::string& graphic = "");

    ~Special();

    const std::string&                  Name() const            { return m_name; }          ///< returns the unique name for this type of special
    std::string                         Description() const;                                ///< returns a text description of this type of special
    std::string                         Dump(unsigned short ntabs = 0) const;               ///< returns a data file format representation of this object
    const focs::ValueRef<double>*   Stealth() const         { return m_stealth.get(); } ///< returns the stealth of the special, which determines how easily it is seen by empires

    /** Returns the EffectsGroups that encapsulate the effects that specials of
        this type have. */
    const std::vector<std::shared_ptr<focs::EffectsGroup>>& Effects() const
    { return m_effects; }

    float                               SpawnRate() const       { return m_spawn_rate; }
    int                                 SpawnLimit() const      { return m_spawn_limit; }
    const focs::ValueRef<double>*   InitialCapacity() const { return m_initial_capacity.get(); }///< returns the ValueRef to use to set the initial capacity of the special when placed
    float                               InitialCapacity(int object_id) const;                       ///< evaluates initial apacity ValueRef using the object with specified \a object_id as the object on which the special will be placed
    const focs::Condition*         Location() const        { return m_location.get(); }        ///< returns the condition that determines whether an UniverseObject can have this special applied during universe creation
    const std::string&                  Graphic() const         { return m_graphic; };              ///< returns the name of the grapic file for this special

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int GetCheckSum() const;

private:
    void Init();

    std::string                                         m_name;
    std::string                                         m_description;
    std::unique_ptr<focs::ValueRef<double>>         m_stealth;
    std::vector<std::shared_ptr<focs::EffectsGroup>>  m_effects;
    float                                               m_spawn_rate = 0.0f;
    int                                                 m_spawn_limit = 99999;
    std::unique_ptr<focs::ValueRef<double>>         m_initial_capacity;
    std::unique_ptr<focs::Condition>               m_location;
    std::string                                         m_graphic;
};

/** Returns the Special object used to represent specials of type \a name.
  * If no such Special exists, 0 is returned instead. */
FO_COMMON_API const Special* GetSpecial(const std::string& name);

/** Returns names of all specials. */
FO_COMMON_API std::vector<std::string> SpecialNames();


/** Look up table for specials.*/
class FO_COMMON_API SpecialsManager {
public:
    using SpecialsTypeMap = std::map<std::string, std::unique_ptr<Special>>;

    SpecialsManager();
    ~SpecialsManager();

    int                         NumSpecials() const { return m_specials.size(); }
    std::vector<std::string>    SpecialNames() const;
    const Special*              GetSpecial(const std::string& name) const;
    unsigned int                GetCheckSum() const;

    /** Sets types to the value of \p future. */
    void SetSpecialsTypes(Pending::Pending<SpecialsTypeMap>&& future);

private:
    /** Assigns any m_pending_types to m_specials. */
    void CheckPendingSpecialsTypes() const;

    /** Future types being parsed by parser.  mutable so that it can
        be assigned to m_species_types when completed.*/
    mutable boost::optional<Pending::Pending<SpecialsTypeMap>> m_pending_types = boost::none;

    mutable SpecialsTypeMap m_specials;
};

FO_COMMON_API SpecialsManager& GetSpecialsManager();


#endif
