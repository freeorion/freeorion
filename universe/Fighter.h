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
    Fighter(int empire_id, int launched_from_id, const std::string& species_name, float damage);
    Fighter();
    ~Fighter() {}

    virtual UniverseObjectType  ObjectType() const;
    virtual std::string         Dump() const;
    float                       Damage() const;
    bool                        Destroyed() const;
    int                         LaunchedFrom() const;
    const std::string&          SpeciesName() const;
    void                        SetDestroyed(bool destroyed = true);

    virtual TemporaryPtr<UniverseObject>    Accept(const UniverseObjectVisitor& visitor) const;
    virtual void                            Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES);

protected:
    virtual Fighter*    Clone(int empire_id = ALL_EMPIRES) const;

private:
    float       m_damage;           // strength of fighter's attack
    bool        m_destroyed;        // was attacked by anything -> destroyed
    int         m_launched_from_id; // from what object (ship?) was this fighter launched
    std::string m_species_name;
};

#endif // _Ship_h_
