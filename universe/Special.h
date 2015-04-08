// -*- C++ -*-
#ifndef _Special_h_
#define _Special_h_

#include "ValueRefFwd.h"

#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>

#include "../util/Export.h"

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
    /** basic ctor */
    Special(const std::string& name, const std::string& description,
            ValueRef::ValueRefBase<double>* stealth,
            const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects,
            double spawn_rate = 1.0, int spawn_limit = 99999,
            ValueRef::ValueRefBase<double>* initial_capaicty = 0,
            Condition::ConditionBase* location = 0,
            const std::string& graphic = "") :
        m_name(name),
        m_description(description),
        m_stealth(stealth),
        m_effects(effects),
        m_spawn_rate(spawn_rate),
        m_spawn_limit(spawn_limit),
        m_initial_capacity(initial_capaicty),
        m_location(location),
        m_graphic(graphic)
    { Init(); }

    ~Special();
    //@}

    /** \name Accessors */ //@{
    const std::string&                      Name() const        { return m_name; }          ///< returns the unique name for this type of special
    const std::string&                      Description() const { return m_description; }   ///< returns a text description of this type of special
    std::string                             Dump() const;                                   ///< returns a data file format representation of this object
    const ValueRef::ValueRefBase<double>*   Stealth() const     { return m_stealth; }       ///< returns the stealth of the special, which determines how easily it is seen by empires
    const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& Effects() const    { return m_effects; }           ///< returns the EffectsGroups that encapsulate the effects that specials of this type have
    float                                   SpawnRate() const   { return m_spawn_rate; }
    int                                     SpawnLimit() const  { return m_spawn_limit; }
    const ValueRef::ValueRefBase<double>*   InitialCapacity() const                 { return m_initial_capacity; }  ///< returns the ValueRef to use to set the initial capacity of the special when placed
    float                                   InitialCapacity(int object_id) const;           ///< evaluates initial apacity ValueRef using the object with specified \a object_id as the object on which the special will be placed
    const Condition::ConditionBase*         Location() const    { return m_location; }      ///< returns the condition that determines whether an UniverseObject can have this special applied during universe creation
    const std::string&                      Graphic() const     { return m_graphic; };      ///< returns the name of the grapic file for this special
    //@}

private:
    void    Init();

    std::string                     m_name;
    std::string                     m_description;
    ValueRef::ValueRefBase<double>* m_stealth;
    std::vector<boost::shared_ptr<Effect::EffectsGroup> >
                                    m_effects;
    float                           m_spawn_rate;
    int                             m_spawn_limit;
    ValueRef::ValueRefBase<double>* m_initial_capacity;
    Condition::ConditionBase*       m_location;
    std::string                     m_graphic;

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

#endif // _Special_h_
