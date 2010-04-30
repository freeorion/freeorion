#include "PathingEngine.h"

#include "../../universe/ShipDesign.h"
#include "../../universe/System.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>


const unsigned int ENTER_STARLANE_DELAY_TURNS = 5;

const unsigned int INTERCEPTOR_FLAG = 1 << 0;
const unsigned int BOMBER_FLAG = 1 << 1;
const unsigned int SHIP_FLAG = 1 << 2;
const unsigned int MISSILE_FLAG = 1 << 3;
const unsigned int FIGHTER_FLAGS = INTERCEPTOR_FLAG | BOMBER_FLAG;
const unsigned int NONFIGHTER_FLAGS = ~(INTERCEPTOR_FLAG | BOMBER_FLAG);

unsigned int EnemyOfEmpireFlags(int empire_id)
{
    // TODO: Use diplomatic status here, instead of just returning all empires
    // that are not us.
    return ~(1 << static_cast<unsigned int>(empire_id));
}

////////////////////////////////////////////////////////////////////////////////
// PathingEngine
////////////////////////////////////////////////////////////////////////////////
const std::size_t PathingEngine::TARGET_FPS = 60;
const std::size_t PathingEngine::TARGET_OBJECT_UPDATES_PER_SEC = 2;
const std::size_t PathingEngine::UPDATE_SETS = TARGET_FPS / TARGET_OBJECT_UPDATES_PER_SEC;
const std::size_t PathingEngine::SECONDS_PER_TURN = 5;
const std::map<int, UniverseObject*>* PathingEngine::s_combat_universe = 0;

PathingEngine::PathingEngine() :
    m_next_fighter_id(0),
    m_update_number(0),
    m_proximity_database(new ProximityDB(OpenSteer::Vec3(), 2.0 * SystemRadius(), 100))
{}

PathingEngine::~PathingEngine()
{
    m_objects.clear();
    m_fighter_formations.clear();
    m_attackees.clear();
    m_obstacles.clear();
    m_ships_by_id.clear();
    m_leaders_by_id.clear();
    m_fighters_by_id.clear();
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
                                                  EnemyOfEmpireFlags(empire_id))) {
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
                                                  EnemyOfEmpireFlags(empire_id))) {
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
                                          EnemyOfEmpireFlags(empire_id))) {
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
                                          EnemyOfEmpireFlags(empire_id))) {
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
                                          EnemyOfEmpireFlags(empire_id))) {
        retval = boost::polymorphic_downcast<CombatShip*>(v)->shared_from_this();
    }
    return retval;
}

std::size_t PathingEngine::UpdateNumber() const
{ return m_update_number; }

PathingEngine::ConstAttackerRange
PathingEngine::Attackers (const CombatObjectPtr& attackee) const
{ return m_attackees.equal_range(attackee); }

PathingEngine::const_iterator PathingEngine::begin () const
{ return m_objects.begin(); }

PathingEngine::const_iterator PathingEngine::end () const
{ return m_objects.end(); }

CombatShipPtr PathingEngine::FindShip(int id) const
{
    CombatShipPtr retval;
    std::map<int, CombatShipPtr>::const_iterator it = m_ships_by_id.find(id);
    if (it != m_ships_by_id.end())
        retval = it->second;
    return retval;
}

CombatFighterPtr PathingEngine::FindLeader(int id) const
{
    CombatFighterPtr retval;
    std::map<int, CombatFighterPtr>::const_iterator it = m_leaders_by_id.find(id);
    if (it != m_leaders_by_id.end())
        retval = it->second;
    return retval;
}

CombatFighterPtr PathingEngine::FindFighter(int id) const
{
    CombatFighterPtr retval;
    std::map<int, CombatFighterPtr>::const_iterator it = m_fighters_by_id.find(id);
    if (it != m_fighters_by_id.end())
        retval = it->second;
    return retval;
}

void PathingEngine::TurnStarted(unsigned int number)
{
    for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
         it != m_objects.end(); ) {
        if (!(*it)->HealthAndShield()) {
            if ((*it)->IsFighter()) {
                assert(boost::dynamic_pointer_cast<CombatFighter>(*it));
                CombatFighterPtr fighter = boost::static_pointer_cast<CombatFighter>(*it);
                std::set<CombatFighterFormationPtr>::iterator formation_it =
                    m_fighter_formations.find(fighter->Formation());
                assert(formation_it != m_fighter_formations.end());
                fighter->SignalDestroyed();
                RemoveFighter(fighter, formation_it);
            } else {
                (*it)->SignalDestroyed();
                RemoveObject(*it);
            }
            ++it;
        } else {
            (*it++)->TurnStarted(number);
        }
    }
}

void PathingEngine::Update(const float elapsed_time, bool force)
{
    // We use a temporary pointer, because an object may remove itself from the
    // engine during its update.
    for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
         it != m_objects.end(); ) {
        CombatObjectPtr ptr = *it++;
        ptr->update(elapsed_time, force);
    }
    ++m_update_number;
}

void PathingEngine::AddObject(const CombatObjectPtr& obj)
{
    m_objects.insert(obj);
    if (obj->IsFighter()) {
        CombatFighterPtr combat_fighter = boost::static_pointer_cast<CombatFighter>(obj);
        if (combat_fighter->IsLeader())
            m_leaders_by_id[combat_fighter->ID()] = combat_fighter;
        else
            m_fighters_by_id[combat_fighter->ID()] = combat_fighter;
    } else if (obj->IsShip()) {
        assert(boost::dynamic_pointer_cast<CombatShip>(obj));
        CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(obj);
        m_ships_by_id[combat_ship->GetShip().ID()] = combat_ship;
        combat_ship->SetWeakPtr(combat_ship);
    }
}

void PathingEngine::RemoveObject(const CombatObjectPtr& obj)
{
    m_attackees.erase(obj);
    m_objects.erase(obj);
    if (obj->IsFighter()) {
        CombatFighterPtr combat_fighter = boost::static_pointer_cast<CombatFighter>(obj);
        if (combat_fighter->IsLeader())
            m_leaders_by_id.erase(combat_fighter->ID());
        else
            m_fighters_by_id.erase(combat_fighter->ID());
    } else if (obj->IsShip()) {
        assert(boost::dynamic_pointer_cast<CombatShip>(obj));
        CombatShipPtr combat_ship = boost::static_pointer_cast<CombatShip>(obj);
        m_ships_by_id.erase(combat_ship->GetShip().ID());
    }
}

int PathingEngine::NextFighterID()
{ return m_next_fighter_id++; }

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

void PathingEngine::AddFighterFormation(const CombatFighterFormationPtr& formation)
{
    formation->Leader().EnterSpace();
    m_leaders_by_id[formation->Leader().ID()] = formation->Leader().shared_from_this();
    for (CombatFighterFormation::iterator it = formation->begin(); it != formation->end(); ++it) {
        (*it)->EnterSpace();
        m_objects.insert(*it);
        m_fighters_by_id[(*it)->ID()] = *it;
    }
    m_fighter_formations.insert(formation);
}

void PathingEngine::RemoveFighter(const CombatObjectPtr& f)
{
    assert(boost::dynamic_pointer_cast<CombatFighter>(f));
    CombatFighterPtr fighter = boost::static_pointer_cast<CombatFighter>(f);
    CombatFighterFormationPtr formation = fighter->Formation();
    std::set<CombatFighterFormationPtr>::iterator formation_it =
        m_fighter_formations.find(formation);
    assert(formation_it != m_fighter_formations.end());
    RemoveFighter(fighter, formation_it);
}

void PathingEngine::RemoveFighterFormation(const CombatFighterFormationPtr& formation)
{
    std::set<CombatFighterFormationPtr>::iterator formation_it =
        m_fighter_formations.find(formation);
    while (!formation->empty()) {
        RemoveFighter(*formation->begin(), formation_it);
    }
}

ProximityDB& PathingEngine::GetProximityDB()
{ return *m_proximity_database; }

void PathingEngine::ClearObstacles()
{ m_obstacles.clear(); }

void PathingEngine::AddObstacle(OpenSteer::AbstractObstacle* obstacle)
{ m_obstacles.push_back(obstacle); }

void PathingEngine::RemoveFighter(const CombatFighterPtr& fighter,
                                  std::set<CombatFighterFormationPtr>::iterator formation_it)
{
    assert(formation_it != m_fighter_formations.end());
    (*formation_it)->erase(fighter);
    if ((*formation_it)->empty())
        m_fighter_formations.erase(formation_it);
    RemoveObject(fighter);
}
