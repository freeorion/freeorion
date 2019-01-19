#ifndef _Fighter_h_
#define _Fighter_h_

#include "UniverseObject.h"
#include "../universe/Condition.h"
#include "../util/Export.h"

////////////////////////////////////////////////
// Fighter
///////////////////////////////////////////////
/** a class representing a Fighter in combat. Derived from UniverseObject so it
  * can be stored in the same container as other combat objects. */
class FO_COMMON_API Fighter : public UniverseObject {
public:
    Fighter(int empire_id, int launched_from_id, const std::string& species_name, float damage, const ::Condition::ConditionBase* combat_targets);
    Fighter();
    ~Fighter();

    bool HostileToEmpire(int empire_id) const override;
    UniverseObjectType ObjectType() const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    std::shared_ptr<UniverseObject> Accept(const UniverseObjectVisitor& visitor) const override;
    void Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id = ALL_EMPIRES) override;
    const ::Condition::ConditionBase* CombatTargets() const;
    float                       Damage() const;
    bool                        Destroyed() const;
    int                         LaunchedFrom() const;
    const std::string&          SpeciesName() const;
    void                        SetDestroyed(bool destroyed = true);

protected:
    Fighter* Clone(int empire_id = ALL_EMPIRES) const override;

private:
    float       m_damage = 0.0f;                        // strength of fighter's attack
    bool        m_destroyed = false;                    // was attacked by anything -> destroyed
    int         m_launched_from_id = INVALID_OBJECT_ID; // from what object (ship?) was this fighter launched
    std::string m_species_name;
    const ::Condition::ConditionBase* m_combat_targets;
};

#endif // _Fighter_h_
