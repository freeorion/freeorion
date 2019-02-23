#include "Fighter.h"

#include "Predicates.h"
#include "Enums.h"
#include "../Empire/EmpireManager.h"
#include "../util/AppInterface.h"
#include "../util/Logger.h"


Fighter::Fighter()
{}

Fighter::Fighter(int empire_id, int launched_from_id, const std::string& species_name, float damage, const ::Condition::ConditionBase* combat_targets) :
    UniverseObject(),
    m_damage(damage),
    m_launched_from_id(launched_from_id),
    m_species_name(species_name),
    m_combat_targets(combat_targets)
{
    this->SetOwner(empire_id);
    UniverseObject::Init();
}

Fighter::~Fighter()
{}

bool Fighter::HostileToEmpire(int empire_id) const
{
    if (OwnedBy(empire_id))
        return false;
    return empire_id == ALL_EMPIRES || Unowned() ||
           Empires().GetDiplomaticStatus(Owner(), empire_id) == DIPLO_WAR;
}

UniverseObjectType Fighter::ObjectType() const
{ return OBJ_FIGHTER; }

const ::Condition::ConditionBase* Fighter::CombatTargets() const
{ return m_combat_targets; }

float Fighter::Damage() const
{ return m_damage; }

bool Fighter::Destroyed() const
{ return m_destroyed; }

int Fighter::LaunchedFrom() const
{ return m_launched_from_id; }

const std::string& Fighter::SpeciesName() const
{ return m_species_name; }

void Fighter::SetDestroyed(bool destroyed)
{ m_destroyed = destroyed; }

std::string Fighter::Dump(unsigned short ntabs) const {
    std::stringstream os;
    os << UniverseObject::Dump(ntabs);
    os << " (Combat Fighter) damage: " << m_damage;
    if (m_destroyed)
        os << "  (DESTROYED)";
    return os.str();
}

std::shared_ptr<UniverseObject> Fighter::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(std::const_pointer_cast<Fighter>(std::static_pointer_cast<const Fighter>(shared_from_this()))); }

Fighter* Fighter::Clone(int empire_id) const {
    Fighter* retval = new Fighter();
    retval->Copy(shared_from_this(), empire_id);
    return retval;
}

void Fighter::Copy(std::shared_ptr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object.get() == this)
        return;
    std::shared_ptr<const Fighter> copied_fighter = std::dynamic_pointer_cast<const Fighter>(copied_object);
    if (!copied_fighter) {
        ErrorLogger() << "Fighter::Copy passed an object that wasn't a Fighter";
        return;
    }

    UniverseObject::Copy(copied_object, VIS_FULL_VISIBILITY, std::set<std::string>());

    this->m_damage = copied_fighter->m_damage;
    this->m_destroyed = copied_fighter->m_destroyed;
    this->m_combat_targets = copied_fighter->m_combat_targets;
}
