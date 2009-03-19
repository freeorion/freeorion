#include "CombatShip.h"

#include "../universe/Ship.h"
#include "../universe/System.h"
#include "CombatFighter.h"
#include "PathingEngine.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

#include <map>


namespace {
#define ECHO_TOKEN(x) (x, #x)
    std::map<ShipMission::Type, std::string> SHIP_MISSION_STRINGS =
        boost::assign::map_list_of
        ECHO_TOKEN(ShipMission::NONE)
        ECHO_TOKEN(ShipMission::MOVE_TO)
        ECHO_TOKEN(ShipMission::ATTACK_THIS_STANDOFF)
        ECHO_TOKEN(ShipMission::ATTACK_THIS)
        ECHO_TOKEN(ShipMission::DEFEND_THIS)
        ECHO_TOKEN(ShipMission::PATROL_TO)
        ECHO_TOKEN(ShipMission::ATTACK_SHIPS_WEAKEST_FIRST_STANDOFF)
        ECHO_TOKEN(ShipMission::ATTACK_SHIPS_NEAREST_FIRST_STANDOFF)
        ECHO_TOKEN(ShipMission::ATTACK_SHIPS_WEAKEST_FIRST)
        ECHO_TOKEN(ShipMission::ATTACK_SHIPS_NEAREST_FIRST)
        ECHO_TOKEN(ShipMission::ENTER_STARLANE);
#undef ECHO_TOKEN
}


////////////////////////////////////////////////////////////////////////////////
// CombatShip
////////////////////////////////////////////////////////////////////////////////
CombatShip::CombatShip() :
    m_proximity_token(0),
    m_empire_id(-1),
    m_ship(),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_pathing_engine(0),
    m_anti_fighter_strength(0)
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{}

CombatShip::CombatShip(int empire_id, Ship* ship, const OpenSteer::Vec3& position,
                       const OpenSteer::Vec3& direction, PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_empire_id(empire_id),
    m_ship(ship),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_pathing_engine(&pathing_engine),
    m_anti_fighter_strength(/*TODO: derive from m_ship*/)
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{ Init(position, direction); }

CombatShip::~CombatShip()
{ delete m_proximity_token; }

float CombatShip::AntiFighterStrength() const
{ return m_anti_fighter_strength; }

Ship* CombatShip::GetShip() const
{ return m_ship; }

const ShipMission& CombatShip::CurrentMission() const
{ return m_mission_queue.back(); }

void CombatShip::LaunchFighters()
{
    assert(!m_unlaunched_formations.empty());
    // TODO: Launch only the maximum number of formations launchable in a
    // single combat turn.
    for (std::set<CombatFighterFormationPtr>::iterator it = m_unlaunched_formations.begin();
         it != m_unlaunched_formations.end();
         ++it) {
        m_pathing_engine->AddFighterFormation(*it);
        m_launched_formations.insert(*it);
    }
    m_unlaunched_formations.clear();
}

void CombatShip::RecoverFighters(const CombatFighterFormationPtr& formation)
{
    m_launched_formations.erase(formation);
    m_unlaunched_formations.insert(formation);
    m_pathing_engine->RemoveFighterFormation(formation);
}

void CombatShip::AppendMission(const ShipMission& mission)
{
    assert(!m_mission_queue.empty());
    if (m_mission_queue.back().m_type == ShipMission::NONE) {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
    }
    m_mission_queue.push_front(mission);
}

void CombatShip::ClearMissions()
{
    // We call this even though we're about to clear the queue, in order to
    // trigger removal of this ship from the attacker list, if applicable.
    RemoveMission();
    m_mission_queue.clear();
    m_mission_queue.push_front(ShipMission(ShipMission::NONE));
}

void CombatShip::AppendFighterMission(const FighterMission& mission)
{
    assert(!m_launched_formations.empty());
    for (std::set<CombatFighterFormationPtr>::iterator it = m_launched_formations.begin();
         it != m_launched_formations.end();
         ++it) {
        for (CombatFighterFormation::iterator formation_it = (*it)->begin();
             formation_it != (*it)->end();
             ++formation_it) {
            (*formation_it)->AppendMission(mission);
        }
    }
}

void CombatShip::ClearFighterMissions()
{
    assert(!m_launched_formations.empty());
    for (std::set<CombatFighterFormationPtr>::iterator it = m_launched_formations.begin();
         it != m_launched_formations.end();
         ++it) {
        for (CombatFighterFormation::iterator formation_it = (*it)->begin();
             formation_it != (*it)->end();
             ++formation_it) {
            (*formation_it)->ClearMissions();
        }
    }
}

void CombatShip::update(const float /*current_time*/, const float elapsed_time)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (m_pathing_engine->UpdateNumber() % PathingEngine::UPDATE_SETS ==
        serialNumber % PathingEngine::UPDATE_SETS) {
        UpdateMissionQueue();
        steer = Steer();
    }
    applySteeringForce(steer, elapsed_time);
    m_last_steer = steer;
    m_proximity_token->UpdatePosition(position());
}

void CombatShip::regenerateLocalSpace(const OpenSteer::Vec3& new_velocity, const float elapsed_time)
{ regenerateLocalSpaceForBanking(new_velocity, elapsed_time); }

float CombatShip::MaxWeaponRange() const
{
    // TODO: Use ship design to determine this.
    return 20.0;
}

float CombatShip::MinNonPDWeaponRange() const
{
    // TODO: Use ship design to determine this.
    return 5.0;
}

void CombatShip::Init(const OpenSteer::Vec3& position_, const OpenSteer::Vec3& direction)
{
    m_proximity_token =
        m_pathing_engine->GetProximityDB().Insert(this, SHIP_FLAG, EmpireFlag(m_empire_id));

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(3.0);
    SimpleVehicle::setMaxSpeed(8.0);

    // TODO: setMass()

    SimpleVehicle::regenerateOrthonormalBasis(direction, OpenSteer::Vec3(0, 0, 1));

    SimpleVehicle::setPosition(position_);
    SimpleVehicle::setSpeed(0);

    m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(ShipMission(ShipMission::NONE));

    // TODO: Based on number of fighter bays and their rates of launch, create
    // fighters and group them into small formations.
    // for (...) {
    //     m_formations.insert(m_pathing_engine->CreateFighterFormation(...));
    // }
    m_unlaunched_formations = m_formations;
}

void CombatShip::PushMission(const ShipMission& mission)
{
    m_mission_queue.push_back(mission);
    if (mission.m_type == ShipMission::ATTACK_THIS ||
        mission.m_type == ShipMission::ATTACK_THIS_STANDOFF) {
        assert(mission.m_target.lock());
        m_pathing_engine->BeginAttack(mission.m_target.lock(), shared_from_this());
    }
}

void CombatShip::RemoveMission()
{
    if (m_mission_queue.back().m_type == ShipMission::ATTACK_THIS ||
        m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF) {
        assert(m_mission_queue.back().m_target.lock());
        m_pathing_engine->EndAttack(m_mission_queue.back().m_target.lock(),
                                    shared_from_this());
    }
    m_mission_queue.pop_back();
    if (m_mission_queue.empty())
        m_mission_queue.push_front(ShipMission(ShipMission::NONE));
}

void CombatShip::UpdateMissionQueue()
{
    assert(!m_mission_queue.empty());

    const float DEFAULT_MISSION_WEIGHT = 12.0;
    const float MAX_MISSION_WEIGHT = 48.0;

    const float AT_DESTINATION = 3.0;
    const float AT_DEST_SQUARED = AT_DESTINATION * AT_DESTINATION;

    bool print_needed = false;
    if (m_instrument && m_last_mission != m_mission_queue.back().m_type) {
        std::cout << "empire=" << m_empire_id << "\n"
                  << "    prev mission=" << SHIP_MISSION_STRINGS[m_last_mission] << "\n"
                  << "    new mission =" << SHIP_MISSION_STRINGS[m_mission_queue.back().m_type] << "\n";
        print_needed = true;
        m_last_mission = m_mission_queue.back().m_type;
    }

    m_mission_weight = 0.0;
    m_mission_destination = OpenSteer::Vec3();

    switch (m_mission_queue.back().m_type) {
    case ShipMission::NONE: {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
        m_mission_queue.push_front(ShipMission(ShipMission::ATTACK_SHIPS_NEAREST_FIRST));
        if (print_needed) std::cout << "    [STARTING DEFAULT MISSION]\n";
        break;
    }
    case ShipMission::MOVE_TO: {
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = MAX_MISSION_WEIGHT;
            m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::ATTACK_THIS_STANDOFF:
    case ShipMission::ATTACK_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            OpenSteer::Vec3 target_position = target->position();
            if (CombatFighterPtr f = boost::dynamic_pointer_cast<CombatFighter>(target))
                target_position = f->Formation()->Centroid();
            OpenSteer::Vec3 from_target_vec = position() - target_position;
            float from_target_length = from_target_vec.length();
            from_target_vec /= from_target_length;
            float weapon_range =
                m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF ?
                MaxWeaponRange() : MinNonPDWeaponRange();
            float standoff_distance = std::min<float>(from_target_length, weapon_range);
            m_mission_destination = target_position + standoff_distance * from_target_vec;
        } else {
            if (print_needed) std::cout << "    [ATTACK TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::DEFEND_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            if (m_mission_subtarget = WeakestAttacker(target))
                m_mission_destination = m_mission_subtarget->position();
            else
                m_mission_destination = target->position();
        } else {
            if (print_needed) std::cout << "    [DEFEND TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::PATROL_TO: {
        // TODO: Consider making the engagement range dynamically adjustable by the user.
        const float PATROL_ENGAGEMENT_RANGE = 50.0;
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            bool found_target = false;
            if (CombatObjectPtr object =
                m_pathing_engine->NearestHostileNonFighterInRange(position(), m_empire_id,
                                                                  PATROL_ENGAGEMENT_RANGE)) {
                m_mission_destination = object->position();
                PushMission(ShipMission(ShipMission::ATTACK_THIS, object));
                found_target = true;
                if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
            }
            if (!found_target)
                m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::ATTACK_SHIPS_WEAKEST_FIRST_STANDOFF:
    case ShipMission::ATTACK_SHIPS_WEAKEST_FIRST: {
        if (CombatObjectPtr object = WeakestHostileShip()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            PushMission(
                ShipMission(
                    m_mission_queue.back().m_type == ShipMission::ATTACK_SHIPS_WEAKEST_FIRST ?
                    ShipMission::ATTACK_THIS :
                    ShipMission::ATTACK_THIS_STANDOFF,
                    object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        m_mission_weight = DEFAULT_MISSION_WEIGHT;
        break;
    }
    case ShipMission::ATTACK_SHIPS_NEAREST_FIRST_STANDOFF:
    case ShipMission::ATTACK_SHIPS_NEAREST_FIRST: {
        if (CombatObjectPtr object = m_pathing_engine->NearestHostileShip(position(), m_empire_id)) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            PushMission(
                ShipMission(
                    m_mission_queue.back().m_type == ShipMission::ATTACK_SHIPS_NEAREST_FIRST ?
                    ShipMission::ATTACK_THIS :
                    ShipMission::ATTACK_THIS_STANDOFF,
                    object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::ENTER_STARLANE: {
        System* system = GetShip()->GetSystem();
        assert(system);
        for (System::const_lane_iterator it = system->begin_lanes();
             it != system->end_lanes();
             ++it) {
            double rads = StarlaneEntranceOrbitalPosition(system->ID(), it->first);
            double radius = StarlaneEntranceOrbitalRadius();
            OpenSteer::Vec3 starlane_position(radius * std::cos(rads),
                                              radius * std::sin(rads),
                                              0.0);
            double entrance_radius_squared =
                StarlaneEntranceRadius() * StarlaneEntranceRadius();
            if ((starlane_position - position()).lengthSquared() < entrance_radius_squared) {
                delete m_proximity_token;
                m_proximity_token = 0;
                m_pathing_engine->RemoveObject(shared_from_this());
                break;
            }
        }
    }
    }

    if (print_needed)
        std::cout << "    position   =" << position() << "\n"
                  << "    destination=" << m_mission_destination << "\n"
                  << "    mission_weight=" << m_mission_weight << "\n"
                  << std::endl;
}

OpenSteer::Vec3 CombatShip::Steer()
{
    const float OBSTACLE_AVOIDANCE_TIME = 6.0;

    const OpenSteer::Vec3 avoidance =
        steerToAvoidObstacles(OBSTACLE_AVOIDANCE_TIME,
                              m_pathing_engine->Obstacles().begin(),
                              m_pathing_engine->Obstacles().end());

    if (avoidance != OpenSteer::Vec3::zero)
        return avoidance;

    // steer towards mission objectives
    OpenSteer::Vec3 mission_vec;
    if (m_mission_weight)
        mission_vec = (m_mission_destination - position()).normalize();

    return mission_vec * m_mission_weight;
}

CombatObjectPtr CombatShip::WeakestAttacker(const CombatObjectPtr& attackee)
{
    CombatObjectPtr retval;

    float weakest = FLT_MAX;

    const float NO_PD_FIGHTER_STRENGTH_SCALE_FACTOR = 25.0;
    const float BOMBER_SCALE_FACTOR = 1.0;
    const float INTERCEPTOR_SCALE_FACTOR = 2.0;

    PathingEngine::ConstAttackerRange attackers = m_pathing_engine->Attackers(attackee);
    for (PathingEngine::Attackees::const_iterator it = attackers.first;
         it != attackers.second;
         ++it) {
        CombatFighterPtr fighter;
        CombatShipPtr ship;
        // TODO: Some kind of "weakness to fighter attacks" should be taken
        // into account when calculating strength.
        float strength = FLT_MAX;
        if (fighter = boost::dynamic_pointer_cast<CombatFighter>(it->second.lock())) {
            // TODO: Use fighter's hit points, and prefer to attack bombers --
            // for now, just use 1.0
            strength =
                1.0 * (fighter->Type() == INTERCEPTOR ?
                       INTERCEPTOR_SCALE_FACTOR : BOMBER_SCALE_FACTOR);
            if (!AntiFighterStrength())
                strength *= NO_PD_FIGHTER_STRENGTH_SCALE_FACTOR;
        } else if (ship = boost::dynamic_pointer_cast<CombatShip>(it->second.lock())) {
            // TODO: Use ship's hit points and our firepower -- for now, just
            // use 5.0
            strength = 5.0;
        }
        if (strength < weakest) {
            retval = it->second.lock();
            weakest = strength;
        }
    }

    return retval;
}

CombatShipPtr CombatShip::WeakestHostileShip()
{
    // TODO: Note that the efficient evaluation of this mission requires a
    // single fighter-vulerability number and a single fighter-attack number to
    // be calculated per design.

    CombatShipPtr retval;
    OpenSteer::AVGroup all;
    m_pathing_engine->GetProximityDB().FindAll(all, SHIP_FLAG, NotEmpireFlag(m_empire_id));
    float weakest = FLT_MAX;
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(all[i]);
        // TODO: Some kind of "weakness to fighter attacks" should be taken into
        // account here later.
        if (ship->AntiFighterStrength() < weakest) {
            retval = ship->shared_from_this();
            weakest = ship->AntiFighterStrength();
        }
    }
    return retval;
}
