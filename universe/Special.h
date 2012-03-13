// -*- C++ -*-
#ifndef _Special_h_
#define _Special_h_

#include <boost/shared_ptr.hpp>
#include <boost/serialization/nvp.hpp>

#include <string>
#include <vector>
#include <set>

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
class Special {
public:
    /** \name Structors */ //@{
    /** basic ctor */
    Special(const std::string& name, const std::string& description,
            const std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects,
            double spawn_rate = 1.0, int spawn_limit = 99999,
            const Condition::ConditionBase* location = 0,
            const std::string& graphic = "") :
        m_name(name),
        m_description(description),
        m_effects(effects),
        m_spawn_rate(spawn_rate),
        m_spawn_limit(spawn_limit),
        m_location(location),
        m_graphic(graphic)
    {}

    ~Special();
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const        { return m_name; }          ///< returns the unique name for this type of special
    const std::string&              Description() const { return m_description; }   ///< returns a text description of this type of special
    std::string                     Dump() const;       ///< returns a data file format representation of this object
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& Effects() const
    { return m_effects; }                               ///< returns the EffectsGroups that encapsulate the effects that specials of this type have
    double                          SpawnRate() const   { return m_spawn_rate; }
    int                             SpawnLimit() const  { return m_spawn_limit; }
    const Condition::ConditionBase* Location() const    { return m_location; }      ///< returns the condition that determines whether an UniverseObject can have this special applied during universe creation
    const std::string&              Graphic() const     { return m_graphic; };      ///< returns the name of the grapic file for this special
    //@}

private:
    std::string                     m_name;
    std::string                     m_description;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                    m_effects;
    double                          m_spawn_rate;
    int                             m_spawn_limit;
    const Condition::ConditionBase* m_location;
    std::string                     m_graphic;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns the Special object used to represent specials of type \a name.
  * If no such Special exists, 0 is returned instead. */
const Special* GetSpecial(const std::string& name);

/** Returns names of all specials. */
std::vector<std::string> SpecialNames();

// template implementations
template <class Archive>
void Special::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_spawn_rate)
        & BOOST_SERIALIZATION_NVP(m_spawn_limit)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_graphic);
}

#endif // _Special_h_
