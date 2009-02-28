#include "PathingEngine.h"

#include "CombatFighter.h"
#include "CombatShip.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>


namespace {
    int g_population = 0; // TOOD: Temporary only.
    const float WORLD_RADIUS = 50.0f;
}

const unsigned int INTERCEPTOR_FLAG = 1 << 0;
const unsigned int BOMBER_FLAG = 1 << 1;
const unsigned int SHIP_FLAG = 1 << 2;
const unsigned int FIGHTER_FLAGS = INTERCEPTOR_FLAG | BOMBER_FLAG;
const unsigned int NONFIGHTER_FLAGS = ~(INTERCEPTOR_FLAG | BOMBER_FLAG);

////////////////////////////////////////////////////////////////////////////////
// PathingEngine
////////////////////////////////////////////////////////////////////////////////
PathingEngine::PathingEngine() :
    m_update_number(0)
{
    // TODO: Change to more appropriate world geometry.
    const float DIVISION = 10.0f;
    const float DIAMETER = WORLD_RADIUS * 1.1f * 2;

    m_proximity_database =
        new ProximityDB(OpenSteer::Vec3(),
                        OpenSteer::Vec3(DIAMETER, DIAMETER, DIAMETER),
                        OpenSteer::Vec3(DIVISION, DIVISION, DIVISION));
}

PathingEngine::~PathingEngine()
{ delete m_proximity_database; }

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
{ m_objects.erase(obj); }

CombatFighterFormationPtr PathingEngine::AddFighterFormation(std::size_t size)
{
    int empire_id = g_population / size % 2 ? 0 : 1;
    CombatFighterType type = g_population / (size * 2) % 2 ? INTERCEPTOR : BOMBER;

    CombatFighterFormationPtr formation(new CombatFighterFormation(*this));
    formation->SetLeader(
        CombatFighterPtr(new CombatFighter(CombatObjectPtr(), type, empire_id, 0/*TODO: Generate a fighter id*/, *this, formation)));
    ++g_population;

    for (std::size_t i = 0; i < size; ++i) {
        CombatFighterPtr fighter(
            new CombatFighter(CombatObjectPtr(), type, empire_id, 0/*TODO: Generate a fighter id*/, *this, formation, g_population % size));
        formation->push_back(fighter);
        ++g_population;
        m_objects.insert(fighter);
    }

    m_fighter_formations.insert(formation);

    return formation;
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

    std::set<CombatObjectPtr>::iterator object_it = m_objects.find(f);
    assert(object_it != m_objects.end());
    m_objects.erase(object_it);

    --g_population; // TODO: temporary
}

void PathingEngine::RemoveFighterFormation(const CombatFighterFormationPtr& formation)
{
    while (!formation->empty()) {
        RemoveFighter(*formation->begin());
    }
}

ProximityDB& PathingEngine::GetProximityDB()
{ return *m_proximity_database; }

void PathingEngine::ClearObstacles()
{ m_obstacles.clear(); }

void PathingEngine::AddObstacle(OpenSteer::AbstractObstacle* obstacle)
{ m_obstacles.push_back(obstacle); }
