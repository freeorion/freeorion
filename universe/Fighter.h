// -*- C++ -*-
#ifndef _Fighter_h_
#define _Fighter_h_

#include "UniverseObject.h"
#include "../util/Export.h"

////////////////////////////////////////////////
// Fighter
///////////////////////////////////////////////
/** a class representing a Fighter in combat. Derived from UniverseObject so it
  * can be stored in the same container as other combat objects. */
class FO_COMMON_API Fighter : public UniverseObject {
public:
    virtual UniverseObjectType  ObjectType() const;
    virtual std::string         Dump() const;
    float                       Damage() const;
    bool                        Destroyed() const;

    virtual TemporaryPtr<UniverseObject>    Accept(const UniverseObjectVisitor& visitor) const;
    virtual void                            Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

protected:
    Fighter();
    Fighter(int empire_id, int launched_from_id, const std::string& species_name, float damage);

    ~Fighter() {}
    virtual Fighter*    Clone(int empire_id = ALL_EMPIRES) const;

private:
    float   m_damage;
    bool    m_destroyed;    // was attacked by anything...
};

#endif // _Ship_h_
