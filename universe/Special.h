// -*- C++ -*-
#ifndef _Special_h_
#define _Special_h_

#include "Effect.h"

/** A predefined group of effects that can be attached to a UniverseObject (often referred to as the "source" object).
    The effects of a Special are not limited to the object to which it is attached.  Each building type must have a
    \a unique name string, by which it can be looked up using GetBuildingType(). */
class Special
{
public:
    Special(const std::string& name, const std::string& description, Effect::EffectsGroup* effects); ///< basic ctor
    Special(const GG::XMLElement& elem); ///< XML ctor
    ~Special(); /// dtor

    const std::string&          Name() const;               ///< returns the unique name for this type of special
    const std::string&          Description() const;        ///< returns a text description of this type of special
    const Effect::EffectsGroup* Effects() const;            ///< returns the EffectsGroup that encapsulates the effects that specials of this type have
    void                        Execute(int host_id) const; ///< executes the effects of the special

private:
    std::string           m_name;
    std::string           m_description;
    Effect::EffectsGroup* m_effects;
};

/** Returns the Special object used to represent specials of type \a name.  If no such Special exists, 0 is returned instead. */
Special* GetSpecial(const std::string& name);

inline std::pair<std::string, std::string> SpecialRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Special_h_
