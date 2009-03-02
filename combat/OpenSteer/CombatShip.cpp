#include "CombatShip.h"

#include "../universe/Ship.h"
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

    const std::size_t TARGET_FPS = 60;
    const std::size_t TARGET_SHIP_UPDATES_PER_SEC = 2;
    const std::size_t UPDATE_SETS = TARGET_FPS / TARGET_SHIP_UPDATES_PER_SEC;
}


////////////////////////////////////////////////////////////////////////////////
// CombatShip
////////////////////////////////////////////////////////////////////////////////
CombatShip::CombatShip(int empire_id, Ship* ship, const OpenSteer::Vec3& position,
                       PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_empire_id(empire_id),
    m_ship(ship),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_pathing_engine(pathing_engine),
    m_anti_fighter_strength(/*TODO: derive from m_ship*/)
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{ Init(position); }

CombatShip::~CombatShip()
{ delete m_proximity_token; }

float CombatShip::AntiFighterStrength() const
{ return m_anti_fighter_strength; }

Ship* CombatShip::GetShip() const
{ return m_ship; }

const ShipMission& CombatShip::CurrentMission() const
{ return m_mission_queue.back(); }

void CombatShip::update(const float /*current_time*/, const float elapsed_time)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (m_pathing_engine.UpdateNumber() % UPDATE_SETS == serialNumber % UPDATE_SETS) {
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

void CombatShip::Init(const OpenSteer::Vec3& position_)
{
    m_proximity_token =
        m_pathing_engine.GetProximityDB().Insert(this, SHIP_FLAG, EmpireFlag(m_empire_id));

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(3.0);
    SimpleVehicle::setMaxSpeed(8.0);

    // TODO: setMass()

    // TODO: For testing only!
    regenerateOrthonormalBasisUF(OpenSteer::Vec3(-1, 0, 0));

    SimpleVehicle::setPosition(position_);
    SimpleVehicle::setSpeed(0);//SimpleVehicle::maxSpeed());

    m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(ShipMission(ShipMission::NONE));
}

void CombatShip::RemoveMission()
{
    m_mission_queue.pop_back();
    if (m_mission_queue.empty())
        m_mission_queue.push_front(ShipMission(ShipMission::NONE));
}

void CombatShip::UpdateMissionQueue()
{
    //assert(m_leader);
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
            float weapon_range = m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF ?
                MaxWeaponRange() : MinNonPDWeaponRange();
            float standoff_distance = std::min(from_target_length, weapon_range);
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
                m_pathing_engine.NearestHostileNonFighterInRange(position(), m_empire_id,
                                                                 PATROL_ENGAGEMENT_RANGE)) {
                m_mission_destination = object->position();
                m_mission_queue.push_back(ShipMission(ShipMission::ATTACK_THIS, object));
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
    case ShipMission::ATTACK_SHIPS_WEAKEST_FIRST: {
        if (CombatObjectPtr object = WeakestHostileShip()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            m_mission_queue.push_back(ShipMission(ShipMission::ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        m_mission_weight = DEFAULT_MISSION_WEIGHT;
        break;
    }
    case ShipMission::ATTACK_SHIPS_NEAREST_FIRST: {
        if (CombatObjectPtr object = m_pathing_engine.NearestHostileShip(position(), m_empire_id)) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            m_mission_queue.push_back(ShipMission(ShipMission::ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::ATTACK_SHIPS_WEAKEST_FIRST_STANDOFF: {
        // TODO
    }
    case ShipMission::ATTACK_SHIPS_NEAREST_FIRST_STANDOFF: {
        // TODO
    }
    case ShipMission::ENTER_STARLANE: {
        // TODO
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
    const OpenSteer::Vec3 avoidance =
        steerToAvoidObstacles(6.0f,
                              m_pathing_engine.Obstacles().begin(),
                              m_pathing_engine.Obstacles().end());

    if (avoidance != OpenSteer::Vec3::zero)
        return avoidance;

    // TODO
    return OpenSteer::Vec3();
}

CombatObjectPtr CombatShip::WeakestAttacker(const CombatObjectPtr& /*attackee*/)
{
    // TODO: This should act as WeakestShip(), but should include fighters.
    return CombatObjectPtr();
}

CombatShipPtr CombatShip::WeakestHostileShip()
{
    // TODO: Note that the efficient evaluation of this mission requires a
    // single fighter-vulerability number and a single fighter-attack number to
    // be calculated per design.

    CombatShipPtr retval;
    OpenSteer::AVGroup all;
    m_pathing_engine.GetProximityDB().FindAll(all, SHIP_FLAG, NotEmpireFlag(m_empire_id));
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
