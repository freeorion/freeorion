#ifndef _Special_h_
#define _Special_h_


#include "ValueRefFwd.h"

#include <boost/serialization/nvp.hpp>
#include <boost/optional/optional.hpp>

#include "../util/Export.h"
#include "../util/Pending.h"

#include <memory>
#include <string>
#include <vector>
#include <map>


namespace Effect {
    class EffectsGroup;
}
namespace Condition {
    struct ConditionBase;
}

/** A predefined set of EffectsGroups that can be attached to a UniverseObject
  * (often referred to as the "source" object).  The effects of a Special are
  * not limited to the object to which it is attached.  Each kind of Special
  * must have a \a unique name string, by which it can be looked up using
  * GetSpecial(). */
class FO_COMMON_API Special {
public:
    /** \name Structors */ //@{
    Special(const std::string& name, const std::string& description,
            std::unique_ptr<ValueRef::ValueRefBase<double>>&& stealth,
            std::vector<std::unique_ptr<Effect::EffectsGroup>>&& effects,
            double spawn_rate = 1.0, int spawn_limit = 99999,
            std::unique_ptr<ValueRef::ValueRefBase<double>>&& initial_capaicty = nullptr,
            std::unique_ptr<Condition::ConditionBase>&& location = nullptr,
            const std::string& graphic = "");

    ~Special();
    //@}

    /** \name Accessors */ //@{
    const std::string&                      Name() const        { return m_name; }          ///< returns the unique name for this type of special
    std::string                             Description() const;                            ///< returns a text description of this type of special
    std::string                             Dump(unsigned short ntabs = 0) const;           ///< returns a data file format representation of this object
    const ValueRef::ValueRefBase<double>*   Stealth() const     { return m_stealth.get(); } ///< returns the stealth of the special, which determines how easily it is seen by empires

    /** Returns the EffectsGroups that encapsulate the effects that specials of
        this type have. */
    const std::vector<std::shared_ptr<Effect::EffectsGroup>>& Effects() const
    { return m_effects; }

    float                                   SpawnRate() const       { return m_spawn_rate; }
    int                                     SpawnLimit() const      { return m_spawn_limit; }
    const ValueRef::ValueRefBase<double>*   InitialCapacity() const { return m_initial_capacity.get(); }///< returns the ValueRef to use to set the initial capacity of the special when placed
    float                                   InitialCapacity(int object_id) const;                       ///< evaluates initial apacity ValueRef using the object with specified \a object_id as the object on which the special will be placed
    const Condition::ConditionBase*         Location() const        { return m_location.get(); }        ///< returns the condition that determines whether an UniverseObject can have this special applied during universe creation
    const std::string&                      Graphic() const         { return m_graphic; };              ///< returns the name of the grapic file for this special

    /** Returns a number, calculated from the contained data, which should be
      * different for different contained data, and must be the same for
      * the same contained data, and must be the same on different platforms
      * and executions of the program and the function. Useful to verify that
      * the parsed content is consistent without sending it all between
      * clients and server. */
    unsigned int                            GetCheckSum() const;
    //@}

private:
    void    Init();

    std::string                                         m_name = "";
    std::string                                         m_description = "";
    std::unique_ptr<ValueRef::ValueRefBase<double>>     m_stealth;
    std::vector<std::shared_ptr<Effect::EffectsGroup>>  m_effects;
    float                                               m_spawn_rate = 0.0f;
    int                                                 m_spawn_limit = 99999;
    std::unique_ptr<ValueRef::ValueRefBase<double>>     m_initial_capacity;
    std::unique_ptr<Condition::ConditionBase>           m_location;
    std::string                                         m_graphic ="";

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns the Special object used to represent specials of type \a name.
  * If no such Special exists, 0 is returned instead. */
FO_COMMON_API const Special* GetSpecial(const std::string& name);

/** Returns names of all specials. */
FO_COMMON_API std::vector<std::string> SpecialNames();

// template implementations
template <class Archive>
void Special::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_stealth)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_spawn_rate)
        & BOOST_SERIALIZATION_NVP(m_spawn_limit)
        & BOOST_SERIALIZATION_NVP(m_initial_capacity)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_graphic);
}

/** Look up table for specials.*/
class FO_COMMON_API SpecialsManager {
public:
    using SpecialsTypeMap = std::map<std::string, std::unique_ptr<Special>>;

    /** \name Structors */ //@{
    SpecialsManager();
    ~SpecialsManager();
    //@}

    /** \name Accessors */ //@{
    std::vector<std::string> SpecialNames() const;
    const Special* GetSpecial(const std::string& name) const;
    unsigned int GetCheckSum() const;
    //@}

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

#endif // _Special_h_
