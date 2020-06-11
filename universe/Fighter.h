#ifndef _Fighter_h_
#define _Fighter_h_


#include "UniverseObject.h"


namespace Condition {
    struct Condition;
}


//! A class representing a Fighter in combat.
//!
//! Derived from UniverseObject so it can be stored in the same container as
//! other combat objects.
class FO_COMMON_API Fighter : public UniverseObject {
public:
    Fighter(int empire_id, int launched_from_id, std::string const& species_name, float damage, Condition::Condition const* combat_targets);

    Fighter();

    ~Fighter();

    auto HostileToEmpire(int empire_id) const -> bool override;

    auto ObjectType() const -> UniverseObjectType override;

    auto Dump(unsigned short ntabs = 0) const -> std::string override;

    auto Accept(UniverseObjectVisitor const& visitor) const -> std::shared_ptr<UniverseObject> override;

    void Copy(std::shared_ptr<UniverseObject const> copied_object, int empire_id = ALL_EMPIRES) override;

    auto CombatTargets() const -> Condition::Condition const*
    { return m_combat_targets; }

    auto Damage() const -> float
    { return m_damage; }

    auto Destroyed() const -> bool
    { return m_destroyed; }

    void SetDestroyed(bool destroyed = true);

    auto LaunchedFrom() const -> int
    { return m_launched_from_id; }

    auto SpeciesName() const -> std::string const&
    { return m_species_name; }

protected:
    auto Clone(int empire_id = ALL_EMPIRES) const -> Fighter* override;

private:
    float       m_damage = 0.0f;
    //! Was attacked by anything -> destroyed
    bool        m_destroyed = false;
    //! From what object (ship?) was this fighter launched
    int         m_launched_from_id = INVALID_OBJECT_ID;
    std::string m_species_name;
    Condition::Condition const* m_combat_targets = nullptr;
};


#endif // _Fighter_h_
