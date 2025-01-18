#include "Fighter.h"

#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"


Fighter::Fighter(int empire_id, int launched_from_id, const std::string& species_name,
                 float damage, const ::Condition::Condition* combat_targets) :
    UniverseObject(UniverseObjectType::OBJ_FIGHTER),
    m_damage(damage),
    m_launched_from_id(launched_from_id),
    m_species_name(species_name),
    m_combat_targets(combat_targets)
{
    this->SetOwner(empire_id);
}

bool Fighter::HostileToEmpire(int empire_id, const EmpireManager& empires) const {
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
        empires.GetDiplomaticStatus(Owner(), empire_id) == DiplomaticStatus::DIPLO_WAR;
}

std::string Fighter::Dump(uint8_t ntabs) const {
    auto retval = UniverseObject::Dump(ntabs) + " (Combat Fighter) damage: " + std::to_string(m_damage);
    if (m_destroyed)
        retval.append("  (DESTROYED)");
    return retval;
}

std::shared_ptr<UniverseObject> Fighter::Clone(const Universe& universe, int empire_id) const {
    auto retval = std::make_shared<Fighter>();
    retval->Copy(*this, universe, empire_id);
    return retval;
}

void Fighter::Copy(const UniverseObject& copied_object, const Universe& universe, int empire_id) {
    if (&copied_object == this)
        return;
    if (copied_object.ObjectType() != UniverseObjectType::OBJ_FIGHTER) {
        ErrorLogger() << "Fighter::Copy passed an object that wasn't a Fighter";
        return;
    }

    Copy(static_cast<const Fighter&>(copied_object), universe, empire_id);
}

void Fighter::Copy(const Fighter& copied_fighter, const Universe& universe, int empire_id) {
    if (&copied_fighter == this)
        return;
    UniverseObject::Copy(copied_fighter, Visibility::VIS_FULL_VISIBILITY, {}, universe);

    this->m_damage = copied_fighter.m_damage;
    this->m_destroyed = copied_fighter.m_destroyed;
    this->m_combat_targets = copied_fighter.m_combat_targets;
}
