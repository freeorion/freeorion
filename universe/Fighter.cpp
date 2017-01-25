#include "Fighter.h"

#include "Predicates.h"
#include "Enums.h"
#include "../util/Logger.h"


Fighter::Fighter() :
    UniverseObject(),
    m_damage(0.0f),
    m_destroyed(false),
    m_launched_from_id(INVALID_OBJECT_ID),
    m_species_name()
{}

Fighter::Fighter(int empire_id, int launched_from_id, const std::string& species_name, float damage) :
    UniverseObject(),
    m_damage(damage),
    m_destroyed(false),
    m_launched_from_id(launched_from_id),
    m_species_name(species_name)
{
    this->SetOwner(empire_id);
    UniverseObject::Init();
}

Fighter::~Fighter()
{}

UniverseObjectType Fighter::ObjectType() const
{ return OBJ_FIGHTER; }

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

std::string Fighter::Dump() const {
    std::stringstream os;
    os << UniverseObject::Dump();
    os << " (Combat Fighter) damage: " << m_damage;
    if (m_destroyed)
        os << "  (DESTROYED)";
    return os.str();
}

TemporaryPtr<UniverseObject> Fighter::Accept(const UniverseObjectVisitor& visitor) const
{ return visitor.Visit(boost::const_pointer_cast<Fighter>(boost::static_pointer_cast<const Fighter>(TemporaryFromThis()))); }

Fighter* Fighter::Clone(int empire_id) const {
    Fighter* retval = new Fighter();
    retval->Copy(TemporaryFromThis(), empire_id);
    return retval;
}

void Fighter::Copy(TemporaryPtr<const UniverseObject> copied_object, int empire_id) {
    if (copied_object == this)
        return;
    TemporaryPtr<const Fighter> copied_fighter = boost::dynamic_pointer_cast<const Fighter>(copied_object);
    if (!copied_fighter) {
        ErrorLogger() << "Fighter::Copy passed an object that wasn't a Fighter";
        return;
    }

    UniverseObject::Copy(copied_object, VIS_FULL_VISIBILITY, std::set<std::string>());

    this->m_damage = copied_fighter->m_damage;
    this->m_destroyed= copied_fighter->m_destroyed;
}
