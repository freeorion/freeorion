// -*- C++ -*-
#ifndef _Special_h_
#define _Special_h_

#include "Effect.h"

/** A predefined set of EffectsGroups that can be attached to a UniverseObject (often referred to as the "source" object).
    The effects of a Special are not limited to the object to which it is attached.  Each kind of Special must have a
    \a unique name string, by which it can be looked up using GetSpecial(). */
class Special
{
public:
    /** \name Structors */ //@{
    Special(const std::string& name, const std::string& description); ///< basic ctor
    Special(const GG::XMLElement& elem); ///< XML ctor
    //@}

    /** \name Accessors */ //@{
    const std::string&          Name() const;               ///< returns the unique name for this type of special
    const std::string&          Description() const;        ///< returns a text description of this type of special
    const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >&
                                Effects() const;            ///< returns the EffectsGroups that encapsulate the effects that specials of this type have
    //@}

private:
    std::string          m_name;
    std::string          m_description;
    std::vector<boost::shared_ptr<const Effect::EffectsGroup> >
                         m_effects;
};

/** Returns the Special object used to represent specials of type \a name.  If no such Special exists, 0 is returned instead. */
Special* GetSpecial(const std::string& name);

/** Returns the names of all "planet specials"; though there are other specials that may be applied to planets, these
    are ok to use when generating random specials during universe creation.  Note that "planet specials" can be attached
    to non-planet UniverseObjects, just like all Specials, and are not restricted to use during universe creation. */
const std::set<std::string>& PlanetSpecialNames();

inline std::pair<std::string, std::string> SpecialRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Special_h_
