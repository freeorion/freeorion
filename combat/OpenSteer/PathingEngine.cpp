#include "PathingEngine.h"

#include "CombatFighter.h"
#include "CombatShip.h"
#include "../../universe/Ship.h"
#include "../../universe/System.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>


const unsigned int INTERCEPTOR_FLAG = 1 << 0;
const unsigned int BOMBER_FLAG = 1 << 1;
const unsigned int SHIP_FLAG = 1 << 2;
const unsigned int FIGHTER_FLAGS = INTERCEPTOR_FLAG | BOMBER_FLAG;
const unsigned int NONFIGHTER_FLAGS = ~(INTERCEPTOR_FLAG | BOMBER_FLAG);

////////////////////////////////////////////////////////////////////////////////
// PathingEngine
////////////////////////////////////////////////////////////////////////////////
const std::size_t PathingEngine::TARGET_FPS = 60;
const std::size_t PathingEngine::TARGET_OBJECT_UPDATES_PER_SEC = 2;
const std::size_t PathingEngine::UPDATE_SETS = TARGET_FPS / TARGET_OBJECT_UPDATES_PER_SEC;

PathingEngine::PathingEngine() :
    m_next_fighter_id(0),
    m_update_number(0)
{
    m_proximity_database = new ProximityDB(OpenSteer::Vec3(), 2.0 * SystemRadius(), 100);
}

PathingEngine::~PathingEngine()
{
    m_objects.clear();
    m_fighter_formations.clear();
    m_obstacles.clear();
    delete m_proximity_database;
}

const ProximityDB& PathingEngine::GetProximityDB() const
{ return *m_proximity_database; }

const PathingEngine::ObstacleVec& PathingEngine::Obstacles() const
{ return m_obstacles; }

CombatFighterPtr PathingEngine::NearestHostileFighterInRange(const OpenSteer::Vec3& position,
                                                             int empire_id, float range) const
{
    CombatFighterPtr retval;
    if (OpenSteer::AbstractVehicle* v =
        m_proximity_database->FindNearestInRadius(position, range,
                                                  BOMBER_FLAG | INTERCEPTOR_FLAG,
                                                  NotEmpireFlag(empire_id))) {
        retval = boost::polymorphic_downcast<CombatFighter*>(v)->shared_from_this();
    }
    return retval;
}

CombatObjectPtr
PathingEngine::NearestHostileNonFighterInRange(const OpenSteer::Vec3& position,
                                               int empire_id, float range) const
{
    CombatObjectPtr retval;
    if (OpenSteer::AbstractVehicle* v =
        m_proximity_database->FindNearestInRadius(position, range,
                                                  NONFIGHTER_FLAGS,
                                                  NotEmpireFlag(empire_id))) {
        // TODO: Handle non-ship objects (e.g. stations) as well.
        retval = boost::polymorphic_downcast<CombatShip*>(v)->shared_from_this();
    }
    return retval;
}

CombatFighterPtr PathingEngine::NearestHostileInterceptor(const OpenSteer::Vec3& position,
                                                          int empire_id) const
{
    CombatFighterPtr retval;
    if (OpenSteer::AbstractVehicle* v =
        m_proximity_database->FindNearest(position,
                                          INTERCEPTOR_FLAG,
                                          NotEmpireFlag(empire_id))) {
        retval = boost::polymorphic_downcast<CombatFighter*>(v)->shared_from_this();
    }
    return retval;
}

CombatFighterPtr PathingEngine::NearestHostileBomber(const OpenSteer::Vec3& position,
                                                     int empire_id) const
{
    CombatFighterPtr retval;
    if (OpenSteer::AbstractVehicle* v =
        m_proximity_database->FindNearest(position,
                                          BOMBER_FLAG,
                                          NotEmpireFlag(empire_id))) {
        retval = boost::polymorphic_downcast<CombatFighter*>(v)->shared_from_this();
    }
    return retval;
}

CombatShipPtr PathingEngine::NearestHostileShip(const OpenSteer::Vec3& position,
                                                int empire_id) const
{
    CombatShipPtr retval;
    if (OpenSteer::AbstractVehicle* v =
        m_proximity_database->FindNearest(position,
                                          SHIP_FLAG,
                                          NotEmpireFlag(empire_id))) {
        retval = boost::polymorphic_downcast<CombatShip*>(v)->shared_from_this();
    }
    return retval;
}

std::size_t PathingEngine::UpdateNumber() const
{ return m_update_number; }

PathingEngine::ConstAttackerRange
PathingEngine::Attackers (const CombatObjectPtr& attackee) const
{ return m_attackees.equal_range(attackee); }

void PathingEngine::Update(const float current_time, const float elapsed_time)
{
    // We use a temporary iterator, because an object may remove itself from the
    // engine during its update.
    for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
         it != m_objects.end(); ) {
        std::set<CombatObjectPtr>::iterator temp_it = it++;
        (*temp_it)->update(current_time, elapsed_time);
    }
    ++m_update_number;
}

void PathingEngine::AddObject(const CombatObjectPtr& obj)
{ m_objects.insert(obj); }

void PathingEngine::RemoveObject(const CombatObjectPtr& obj)
{
    m_attackees.erase(obj);
    m_objects.erase(obj);
}

void PathingEngine::BeginAttack(const CombatObjectPtr& attacker,
                                const CombatObjectPtr& attackee)
{ m_attackees.insert(Attackees::value_type(attackee, attacker)); }

void PathingEngine::EndAttack(const CombatObjectPtr& attacker,
                              const CombatObjectPtr& attackee)
{
    AttackerRange range = m_attackees.equal_range(attackee);
    if (range.first != range.second) {
        Attackees::iterator it = range.first;
        while (it != range.second && it->second.lock() != attacker)
            ++it;
        if (it != range.second)
            m_attackees.erase(it);
    }
}

CombatFighterFormationPtr
PathingEngine::CreateFighterFormation(CombatShipPtr base, CombatFighterType type, std::size_t size)
{
    assert(base->GetShip()->Owners().size() == 1u);
    int empire_id = *base->GetShip()->Owners().begin();

    CombatFighterFormationPtr formation(new CombatFighterFormation(*this));
    formation->SetLeader(
        CombatFighterPtr(
            new CombatFighter(CombatObjectPtr(), type, empire_id, m_next_fighter_id++,
                              *this, formation)));

    for (std::size_t i = 0; i < size; ++i) {
        CombatFighterPtr fighter(
            new CombatFighter(CombatObjectPtr(), type, empire_id, m_next_fighter_id++,
                              *this, formation, i % size));
        formation->push_back(fighter);
        m_objects.insert(fighter);
    }

    m_fighter_formations.insert(formation);

    return formation;
}

void PathingEngine::AddFighterFormation(const CombatFighterFormationPtr& formation)
{
    formation->Leader().EnterSpace();
    for (CombatFighterFormation::iterator it = formation->begin(); it != formation->end(); ++it) {
        (*it)->EnterSpace();
        m_objects.insert(*it);
    }
    m_fighter_formations.insert(formation);
}

void PathingEngine::RemoveFighter(const CombatObjectPtr& f)
{
    assert(boost::dynamic_pointer_cast<CombatFighter>(f));
    CombatFighterPtr fighter = boost::static_pointer_cast<CombatFighter>(f);
    CombatFighterFormationPtr formation = fighter->Formation();
    std::set<CombatFighterFormationPtr>::iterator formation_it = m_fighter_formations.find(formation);
    assert(formation_it != m_fighter_formations.end());
    (*formation_it)->erase(fighter);
    if ((*formation_it)->empty())
        m_fighter_formations.erase(formation_it);
    RemoveObject(f);
}

void PathingEngine::RemoveFighterFormation(const CombatFighterFormationPtr& formation)
{
    while (!formation->empty()) {
        RemoveFighter(*formation->begin());
    }
    m_fighter_formations.erase(formation);
}

ProximityDB& PathingEngine::GetProximityDB()
{ return *m_proximity_database; }

void PathingEngine::ClearObstacles()
{ m_obstacles.clear(); }

void PathingEngine::AddObstacle(OpenSteer::AbstractObstacle* obstacle)
{ m_obstacles.push_back(obstacle); }
