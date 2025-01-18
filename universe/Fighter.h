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
class FO_COMMON_API Fighter final : public UniverseObject {
public:
    Fighter(int empire_id, int launched_from_id, const std::string& species_name,
            float damage, const ::Condition::Condition* combat_targets);
    Fighter() : UniverseObject(UniverseObjectType::OBJ_FIGHTER) {}

    [[nodiscard]] bool               HostileToEmpire(int empire_id, const EmpireManager& empires) const override;
    [[nodiscard]] std::string        Dump(uint8_t ntabs = 0) const override;

    void Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id = ALL_EMPIRES) override;
    void Copy(const Fighter& copied_fighter, const Universe& universe, int empire_id = ALL_EMPIRES);

    [[nodiscard]] auto* CombatTargets() const noexcept { return m_combat_targets; }
    [[nodiscard]] float Damage() const noexcept { return m_damage; }
    [[nodiscard]] bool  Destroyed() const noexcept { return m_destroyed; }
    [[nodiscard]] int   LaunchedFrom() const noexcept { return m_launched_from_id; }
    [[nodiscard]] auto& SpeciesName() const noexcept { return m_species_name; }

    void SetDestroyed(bool destroyed = true) noexcept { m_destroyed = destroyed; }

private:
    [[nodiscard]] std::shared_ptr<UniverseObject> Clone(const Universe& universe, int empire_id = ALL_EMPIRES) const override;

    float                         m_damage = 0.0f;                        // strength of fighter's attack
    bool                          m_destroyed = false;                    // was attacked by anything -> destroyed
    int                           m_launched_from_id = INVALID_OBJECT_ID; // from what object (ship?) was this fighter launched
    std::string                   m_species_name;
    const ::Condition::Condition* m_combat_targets = nullptr;
};


#endif
