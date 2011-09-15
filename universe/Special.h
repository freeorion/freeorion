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
class Special
{
public:
    /** \name Structors */ //@{
    /** basic ctor */
    Special(const std::string& name, const std::string& description,
            const std::vector<boost::shared_ptr<const Effect::EffectsGroup> > effects,
            const Condition::ConditionBase* location,
            const std::string& graphic);
    //@}

    /** \name Accessors */ //@{
    const std::string&              Name() const;       ///< returns the unique name for this type of special
    const std::string&              Description() const;///< returns a text description of this type of special
    std::string                     Dump() const;       ///< returns a data file format representation of this object
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                                    Effects() const;    ///< returns the EffectsGroups that encapsulate the effects that specials of this type have
    const Condition::ConditionBase* Location() const;   ///< returns the condition that determines whether an UniverseObject can have this special applied during universe creation
    const std::string&              Graphic() const;    ///< returns the name of the grapic file for this special
    //@}

private:
    std::string                     m_name;
    std::string                     m_description;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                                    m_effects;
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

/** Returns the names of all "planet specials"; though there are other specials
  * that may be applied to planets, these are ok to use when generating random
  * specials during universe creation.  Note that "planet specials" can also be
  * attached to non-planet UniverseObjects - like all Specials - and are not
  * restricted to use only during universe creation. */
const std::set<std::string>& PlanetSpecialNames();

// template implementations
template <class Archive>
void Special::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_location)
        & BOOST_SERIALIZATION_NVP(m_graphic);
}

#endif // _Special_h_
