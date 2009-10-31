#include "CombatFighter.h"

#include "../CombatEventListener.h"
#include "CombatShip.h"
#include "PathingEngine.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

#include <map>
#include <iostream>


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


namespace {

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: BEGIN section for testing only.
    ////////////////////////////////////////////////////////////////////////////////

    const float MAX_POINT_DEFENSE_RANGE = 10.0;

    ////////////////////////////////////////////////////////////////////////////////
    // TODO: END section for testing only.
    ////////////////////////////////////////////////////////////////////////////////

#define ECHO_TOKEN(x) (x, #x)
    std::map<FighterMission::Type, std::string> FIGHTER_MISSION_STRINGS =
        boost::assign::map_list_of
        ECHO_TOKEN(FighterMission::NONE)
        ECHO_TOKEN(FighterMission::MOVE_TO)
        ECHO_TOKEN(FighterMission::ATTACK_THIS)
        ECHO_TOKEN(FighterMission::DEFEND_THIS)
        ECHO_TOKEN(FighterMission::PATROL_TO)
        ECHO_TOKEN(FighterMission::ATTACK_FIGHTERS_BOMBERS_FIRST)
        ECHO_TOKEN(FighterMission::ATTACK_FIGHTERS_INTERCEPTORS_FIRST)
        ECHO_TOKEN(FighterMission::ATTACK_SHIPS_WEAKEST_FIRST)
        ECHO_TOKEN(FighterMission::ATTACK_SHIPS_NEAREST_FIRST)
        ECHO_TOKEN(FighterMission::RETURN_TO_BASE);
#undef ECHO_TOKEN
}


////////////////////////////////////////////////////////////////////////////////
// CombatFighterFormation
////////////////////////////////////////////////////////////////////////////////
CombatFighterFormation::CombatFighterFormation() :
    m_pathing_engine(0)
{}

CombatFighterFormation::CombatFighterFormation(PathingEngine& pathing_engine) :
    m_pathing_engine(&pathing_engine)
{}

CombatFighterFormation::~CombatFighterFormation()
{ m_pathing_engine->RemoveObject(m_leader); }

const CombatFighter& CombatFighterFormation::Leader() const
{ return *m_leader; }

double CombatFighterFormation::Damage(double d)
{
    for (iterator it = begin(); it != end(); ) {
        CombatFighterPtr f = *it++;
        double damage_to_this_fighter = std::min(d, f->HealthAndShield());
        f->DamageImpl(damage_to_this_fighter);
        d -= damage_to_this_fighter;
    }
    return d;
}

OpenSteer::Vec3 CombatFighterFormation::Centroid() const
{
    OpenSteer::Vec3 retval;
    std::size_t count = 0;
    for (const_iterator it = begin(); it != end(); ++it) {
        retval += (*it)->position();
        ++count;
    }
    retval /= count;
    return retval;
}

bool CombatFighterFormation::empty() const
{ return m_members.empty(); }

std::size_t CombatFighterFormation::size() const
{ return m_members.size(); }

CombatFighterFormation::const_iterator CombatFighterFormation::begin() const
{ return m_members.begin(); }

CombatFighterFormation::const_iterator CombatFighterFormation::end() const
{ return m_members.end(); }

CombatFighter& CombatFighterFormation::Leader()
{ return *m_leader; }

void CombatFighterFormation::push_back(const CombatFighterPtr& fighter)
{
    assert(fighter);
    m_members.push_back(fighter);
}

void CombatFighterFormation::erase(const CombatFighterPtr& fighter)
{
    std::list<CombatFighterPtr>::iterator it =
        std::find(m_members.begin(), m_members.end(), fighter);
    if (it != m_members.end())
        m_members.erase(it);
}

void CombatFighterFormation::erase(CombatFighter* fighter)
{
    std::list<CombatFighterPtr>::iterator it = m_members.begin();
    for (; it != m_members.end(); ++it) {
        if (it->get() == fighter)
            break;
    }
    if (it != m_members.end())
        m_members.erase(it);
}

CombatFighterFormation::iterator CombatFighterFormation::begin()
{ return m_members.begin(); }

CombatFighterFormation::iterator CombatFighterFormation::end()
{ return m_members.end(); }

void CombatFighterFormation::SetLeader(const CombatFighterPtr& fighter)
{
    assert(!m_leader);
    m_leader = fighter;
    m_pathing_engine->AddObject(m_leader);
}


////////////////////////////////////////////////////////////////////////////////
// CombatFighter
////////////////////////////////////////////////////////////////////////////////
const std::size_t CombatFighter::FORMATION_SIZE = 5;

CombatFighter::CombatFighter() :
    m_proximity_token(0),
    m_leader(false),
    m_empire_id(ALL_EMPIRES),
    m_id(-1),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_base(),
    m_formation_position(-1),
    m_formation(),
    m_health(0),
    m_last_queue_update_turn(std::numeric_limits<unsigned int>::max()),
    m_last_fired_turn(std::numeric_limits<unsigned int>::max()),
    m_turn(std::numeric_limits<unsigned int>::max()),
    m_pathing_engine(0),
    m_stats(0)
    ,m_instrument(false)
    ,m_last_mission(FighterMission::NONE)
{}

CombatFighter::CombatFighter(CombatObjectPtr base, int empire_id,
                             PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_leader(true),
    m_part_name(),
    m_empire_id(empire_id),
    m_id(pathing_engine.NextFighterID()),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_base(base),
    m_formation_position(-1),
    m_formation(),
    m_health(0),
    m_last_queue_update_turn(std::numeric_limits<unsigned int>::max()),
    m_last_fired_turn(std::numeric_limits<unsigned int>::max()),
    m_turn(std::numeric_limits<unsigned int>::max()),
    m_pathing_engine(&pathing_engine),
    m_stats(0)
    ,m_instrument(false)
    ,m_last_mission(FighterMission::NONE)
{}

CombatFighter::CombatFighter(CombatObjectPtr base, const PartType& part, int empire_id,
                             PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_leader(false),
    m_part_name(part.Name()),
    m_empire_id(empire_id),
    m_id(pathing_engine.NextFighterID()),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_base(base),
    m_formation_position(-1),
    m_formation(),
    m_health(0),
    m_last_queue_update_turn(std::numeric_limits<unsigned int>::max()),
    m_last_fired_turn(std::numeric_limits<unsigned int>::max()),
    m_turn(std::numeric_limits<unsigned int>::max()),
    m_pathing_engine(&pathing_engine),
    m_stats(0)
    ,m_instrument(false)
    ,m_last_mission(FighterMission::NONE)
{}

CombatFighter::~CombatFighter()
{
    delete m_proximity_token;
    if (m_formation)
        m_formation->erase(this);
}

float CombatFighter::maxForce() const
{
    float retval = SimpleVehicle::maxForce();
    if (!m_leader) {
        // Use sigmoid function to smooth force diminution when moving into
        // position in formation.  Without cutting the max force, the fighters
        // wiggle wildly side-to-side when they are very close to their
        // formation positions.
        const float MAX_FORCE_DISTANCE = 12.0;
        const float MIN_FORCE = retval / 5.0;
        const float MAX_FORCE = retval * 1.1;
        float sig =
            1.0 / (1.0 + std::exp(-(m_out_of_formation.length() - MAX_FORCE_DISTANCE / 2.0f)));
        retval = MIN_FORCE + (MAX_FORCE - MIN_FORCE) * sig;
    }
    return retval;
}

float CombatFighter::maxSpeed() const
{
    float retval = SimpleVehicle::maxSpeed();
    if (!m_leader) {
        // Use sigmoid function to smooth speed scaling when moving into
        // position in formation.
        const float MAX_SPEED_DISTANCE = 12.0;
        const float MIN_SPEED = m_formation->Leader().speed();
        const float MAX_SPEED = retval * 1.1;
        float sig =
            1.0 / (1.0 + std::exp(-(m_out_of_formation.length() - MAX_SPEED_DISTANCE / 2.0f)));
        retval = MIN_SPEED + (MAX_SPEED - MIN_SPEED) * sig;
    }
    return retval;
}

int CombatFighter::ID() const
{ return m_id; }

const FighterStats& CombatFighter::Stats() const
{
    if (!m_stats)
        m_stats = &boost::get<FighterStats>(GetPartType(m_part_name)->Stats());
    return *m_stats;
}

const std::string& CombatFighter::PartName() const
{ return m_part_name; }

const FighterMission& CombatFighter::CurrentMission() const
{ return m_mission_queue.back(); }

double CombatFighter::HealthAndShield() const
{ return m_health; }

double CombatFighter::Health() const
{ return m_health; }

double CombatFighter::FractionalHealth() const
{ return 1.0; }

double CombatFighter::AntiFighterStrength() const
{ return Stats().m_anti_fighter_damage * Stats().m_fighter_weapon_range; }

double CombatFighter::AntiShipStrength(CombatShipPtr/* target = CombatShipPtr()*/) const
{ return Stats().m_anti_ship_damage * Stats().m_fighter_weapon_range; }

bool CombatFighter::IsFighter() const
{ return true; }

void CombatFighter::update(const float /*current_time*/, const float elapsed_time)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (m_pathing_engine->UpdateNumber() % PathingEngine::UPDATE_SETS ==
        serialNumber % PathingEngine::UPDATE_SETS) {
        if (m_leader) {
            if (m_last_queue_update_turn != m_turn)
                UpdateMissionQueue();
            if (m_last_fired_turn != m_turn)
                FireAtHostiles();
        }
        steer = Steer();
    }
    applySteeringForce(steer, elapsed_time);
    m_last_steer = steer;
    if (m_leader)
        m_proximity_token->UpdatePosition(position());
}

void CombatFighter::regenerateLocalSpace(const OpenSteer::Vec3& new_velocity,
                                         const float elapsed_time)
{ regenerateLocalSpaceForBanking(new_velocity, elapsed_time); }

CombatFighterFormationPtr CombatFighter::Formation()
{ return m_formation; }

void CombatFighter::EnterSpace()
{
    if (m_leader) {
        m_proximity_token =
            m_pathing_engine->GetProximityDB().Insert(
                this,
                Stats().m_type == INTERCEPTOR ? INTERCEPTOR_FLAG : BOMBER_FLAG,
                EmpireFlag(m_empire_id));
    }

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(3.0 * 9.0);
    SimpleVehicle::setMaxSpeed(Stats().m_speed);

    // TODO: setMass()

    if (m_leader) {
        CombatObjectPtr base = m_base.lock();
        assert(base);
        SimpleVehicle::setPosition(base->position());
        SimpleVehicle::regenerateOrthonormalBasis(base->forward(), base->up());
        SimpleVehicle::setSpeed(CombatFighter::maxSpeed());
    } else {
        SimpleVehicle::regenerateOrthonormalBasis(m_formation->Leader().forward(),
                                                  m_formation->Leader().up());
        SimpleVehicle::setPosition(GlobalFormationPosition());
        SimpleVehicle::setSpeed(0.0);
    }

    if (m_leader)
        m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(FighterMission(FighterMission::NONE));

    Listener().FighterLaunched(shared_from_this());
}

void CombatFighter::AppendMission(const FighterMission& mission)
{
    assert(m_leader);
    assert(!m_mission_queue.empty());
    if (m_mission_queue.back().m_type == FighterMission::NONE) {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
    }
    m_mission_queue.push_front(mission);
}

void CombatFighter::ClearMissions()
{
    assert(m_leader);
    m_mission_queue.clear();
    m_mission_queue.push_front(FighterMission(FighterMission::NONE));
}

void CombatFighter::ExitSpace()
{
    delete m_proximity_token;
    m_proximity_token = 0;
    Listener().FighterDocked(shared_from_this());
}

void CombatFighter::Damage(double d, DamageSource source)
{
    if (source == PD_DAMAGE)
        m_formation->Damage(d);
    else
        DamageImpl(d * CombatShip::NON_PD_VS_FIGHTER_FACTOR);
}

void CombatFighter::Damage(const CombatFighterPtr& source)
{
    double damage = source->Stats().m_anti_fighter_damage * source->Formation()->size();
    m_formation->Damage(damage);
}

void CombatFighter::TurnStarted(unsigned int number)
{ m_turn = number; }

void CombatFighter::SignalDestroyed()
{ Listener().FighterDestroyed(shared_from_this()); }

void CombatFighter::DamageImpl(double d)
{ m_health = std::max(0.0, m_health - d); }

void CombatFighter::SetFormation(const CombatFighterFormationPtr& formation)
{ m_formation = formation; }

void CombatFighter::PushMission(const FighterMission& mission)
{
    m_mission_queue.push_back(mission);
    if (mission.m_type == FighterMission::ATTACK_THIS) {
        assert(mission.m_target.lock());
        m_pathing_engine->BeginAttack(mission.m_target.lock(), shared_from_this());
    }
}

OpenSteer::Vec3 CombatFighter::GlobalFormationPosition()
{
    OpenSteer::Vec3 retval;
    const OpenSteer::Vec3 FORMATION_POSITIONS[FORMATION_SIZE] = {
        OpenSteer::Vec3( 0.0,  0.0, -0.5),
        OpenSteer::Vec3( 3.0,  0.0, -1.5),
        OpenSteer::Vec3( 0.0,  3.0, -1.5),
        OpenSteer::Vec3(-3.0,  0.0, -1.5),
        OpenSteer::Vec3( 0.0, -3.0, -1.5)
    };
    if (!m_leader) {
        OpenSteer::Vec3 position = FORMATION_POSITIONS[m_formation_position];
        retval = m_formation->Leader().globalizePosition(position);
    }
    return retval;
}

void CombatFighter::RemoveMission()
{
    assert(!m_mission_queue.empty());
    m_mission_queue.pop_back();
    m_mission_subtarget.reset();
    if (m_mission_queue.empty())
        m_mission_queue.push_front(FighterMission(FighterMission::NONE));
}

void CombatFighter::UpdateMissionQueue()
{
    assert(m_leader);
    assert(!m_mission_queue.empty());

    const float DEFAULT_MISSION_WEIGHT = 12.0;
    const float MAX_MISSION_WEIGHT = 48.0;

    const float AT_DESTINATION = std::max(3.0f, speed());
    const float AT_DEST_SQUARED = AT_DESTINATION * AT_DESTINATION;

    bool print_needed = false;
    if (m_instrument && m_last_mission != m_mission_queue.back().m_type) {
        std::string prev_mission = FIGHTER_MISSION_STRINGS[m_last_mission];
        std::string new_mission = FIGHTER_MISSION_STRINGS[m_mission_queue.back().m_type];
        std::cout << "empire=" << m_empire_id
                  << " type=" << (Stats().m_type ? "BOMBER" : "INTERCEPTOR") << "\n"
                  << "    prev mission=" << prev_mission.c_str() << "\n"
                  << "    new mission =" << new_mission.c_str() << "\n";
        print_needed = true;
        m_last_mission = m_mission_queue.back().m_type;
    }

    m_last_queue_update_turn = m_turn;

    m_mission_weight = 0.0;
    m_mission_destination = OpenSteer::Vec3();

    switch (m_mission_queue.back().m_type) {
    case FighterMission::NONE: {
        assert(m_mission_queue.size() == 1u);
        m_mission_queue.clear();
        if (Stats().m_type == INTERCEPTOR)
            m_mission_queue.push_front(FighterMission(FighterMission::ATTACK_FIGHTERS_BOMBERS_FIRST));
        else
            m_mission_queue.push_front(FighterMission(FighterMission::ATTACK_SHIPS_WEAKEST_FIRST));
        if (print_needed) std::cout << "    [STARTING DEFAULT MISSION]\n";
        break;
    }
    case FighterMission::MOVE_TO: {
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = MAX_MISSION_WEIGHT;
            m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::ATTACK_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = target->position();
            if (target->IsFighter()) {
                assert(boost::dynamic_pointer_cast<CombatFighter>(target));
                CombatFighterPtr f = boost::static_pointer_cast<CombatFighter>(target);
                m_mission_destination = f->Formation()->Centroid();
            }
        } else {
            if (print_needed) std::cout << "    [ATTACK TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::DEFEND_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            if (m_mission_subtarget.expired()) {
                m_mission_subtarget = WeakestAttacker(target);
                if (CombatObjectPtr subtarget = m_mission_subtarget.lock())
                    m_mission_destination = subtarget->position();
                else
                    m_mission_destination = target->position();
            }
        } else {
            if (print_needed) std::cout << "    [DEFEND TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::PATROL_TO: {
        // TODO: Consider making the engagement range dynamically adjustable by the user.
        const float PATROL_ENGAGEMENT_RANGE = 50.0;
        if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            bool found_target = false;
            if (Stats().m_type == INTERCEPTOR) {
                if (CombatFighterPtr fighter =
                    m_pathing_engine->NearestHostileFighterInRange(position(), m_empire_id,
                                                                   PATROL_ENGAGEMENT_RANGE)) {
                    m_mission_destination = fighter->position();
                    PushMission(FighterMission(FighterMission::ATTACK_THIS, fighter));
                    found_target = true;
                    if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
                }
            } else {
                if (CombatObjectPtr object =
                    m_pathing_engine->NearestHostileNonFighterInRange(position(), m_empire_id,
                                                                      PATROL_ENGAGEMENT_RANGE)) {
                    m_mission_destination = object->position();
                    PushMission(FighterMission(FighterMission::ATTACK_THIS, object));
                    found_target = true;
                    if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
                }
            }
            if (!found_target)
                m_mission_destination = m_mission_queue.back().m_destination;
        } else {
            if (print_needed) std::cout << "    [ARRIVED]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::ATTACK_FIGHTERS_BOMBERS_FIRST: {
        CombatFighterPtr fighter = m_pathing_engine->NearestHostileBomber(position(), m_empire_id);
        if (!fighter)
            fighter = m_pathing_engine->NearestHostileInterceptor(position(), m_empire_id);
        if (fighter) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = fighter->position();
            PushMission(FighterMission(FighterMission::ATTACK_THIS, fighter));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::ATTACK_FIGHTERS_INTERCEPTORS_FIRST: {
        CombatFighterPtr fighter = m_pathing_engine->NearestHostileInterceptor(position(), m_empire_id);
        if (!fighter)
            fighter = m_pathing_engine->NearestHostileBomber(position(), m_empire_id);
        if (fighter) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = fighter->position();
            PushMission(FighterMission(FighterMission::ATTACK_THIS, fighter));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE FIGHTER]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::ATTACK_SHIPS_WEAKEST_FIRST: {
        if (CombatObjectPtr object = WeakestHostileShip()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            PushMission(FighterMission(FighterMission::ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        m_mission_weight = DEFAULT_MISSION_WEIGHT;
        break;
    }
    case FighterMission::ATTACK_SHIPS_NEAREST_FIRST: {
        if (CombatObjectPtr object = m_pathing_engine->NearestHostileShip(position(), m_empire_id)) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            m_mission_destination = object->position();
            PushMission(FighterMission(FighterMission::ATTACK_THIS, object));
            if (print_needed) std::cout << "    [ENGAGING HOSTILE SHIP]\n";
        } else {
            if (print_needed) std::cout << "    [NO TARGETS]\n";
            RemoveMission();
        }
        break;
    }
    case FighterMission::RETURN_TO_BASE: {
        if (!m_base.expired()) {
            if (AT_DEST_SQUARED < (position() - m_mission_queue.back().m_destination).lengthSquared()) {
                m_mission_weight = MAX_MISSION_WEIGHT;
                m_mission_destination = m_mission_queue.back().m_destination;
            } else {
                if (CombatObjectPtr b = m_base.lock()) {
                    assert(boost::dynamic_pointer_cast<CombatShip>(b));
                    CombatShipPtr base = boost::static_pointer_cast<CombatShip>(b);
                    base->RecoverFighters(m_formation);
                    if (print_needed) std::cout << "    [ARRIVED AT BASE]\n";
                    RemoveMission();
                } else {
                    // TODO: Rebase!
                }
            }
        } else {
            if (print_needed) std::cout << "    [BASE GONE]\n";
            RemoveMission();
        }
        break;
    }
    }

    if (print_needed)
        std::cout << "    position   =" << position() << "\n"
                  << "    destination=" << m_mission_destination << "\n"
                  << "    mission_weight=" << m_mission_weight << "\n"
                  << std::endl;
}

void CombatFighter::FireAtHostiles()
{
    assert(m_leader);
    assert(!m_formation->empty());
    assert(!m_mission_queue.empty());

    OpenSteer::Vec3 position_to_use = m_formation->Centroid();
    const double WEAPON_RANGE = Stats().m_fighter_weapon_range;
    const double WEAPON_RANGE_SQUARED = WEAPON_RANGE * WEAPON_RANGE;

    CombatObjectPtr target = m_mission_subtarget.lock();
    if (!target && m_mission_queue.back().m_type == FighterMission::ATTACK_THIS) {
        assert(m_mission_queue.back().m_target.lock());
        target = m_mission_queue.back().m_target.lock();
        if (WEAPON_RANGE_SQUARED < (target->position() - position_to_use).lengthSquared())
            target.reset();
    }

    // find a target of opportunity
    if (!target) {
        target = m_pathing_engine->NearestHostileFighterInRange(
            position_to_use, m_empire_id, WEAPON_RANGE);
    }
    if (!target) {
        target = m_pathing_engine->NearestHostileNonFighterInRange(
            position_to_use, m_empire_id, WEAPON_RANGE);
    }

    if (target) {
        target->Damage(shared_from_this());
        m_last_fired_turn = m_turn;
        Listener().FighterFired(shared_from_this(), target);
    }
}

OpenSteer::Vec3 CombatFighter::Steer()
{
    static bool once = true;
    if (once && Stats().m_type == BOMBER && m_leader) {
        //m_instrument = true;
        once = false;
    }

    // A note about obstacle avoidance.  Avoidance of static obstacles (planets,
    // asteroid fields, etc.) are represented in static_obstacle_avoidance.  The
    // analogous avoidance of dynamic obstacles (ships, etc.) -- that is, an
    // overriding move away from an object that the fighter is about to collide
    // with -- is represented in dynamic_obstacle_avoidance.  Another object
    // avoidance factor exists, nonfighter_obstacle_evasion_vec, but it works a
    // bit differently from the other two.  It provides a distance-variable
    // force moving the fighter away, but does not get treated as the only
    // movement force if it exists (as static_obstacle_avoidance and
    // dynamic_obstacle_avoidance do).  Instead, it is just one of the many
    // forces moving the fighter, along with steering towards mission objectives
    // and away from PD, etc.

    const float OBSTACLE_AVOIDANCE_TIME = 1.0;

    const OpenSteer::Vec3 static_obstacle_avoidance =
        steerToAvoidObstacles(OBSTACLE_AVOIDANCE_TIME,
                              m_pathing_engine->Obstacles().begin(),
                              m_pathing_engine->Obstacles().end());

    if (static_obstacle_avoidance != OpenSteer::Vec3::zero)
        return static_obstacle_avoidance;

    const float SEPARATION_RADIUS =  5.0f;
    const float SEPARATION_ANGLE  = -0.707f;
    const float SEPARATION_WEIGHT =  6.0f;

    const float ALIGNMENT_RADIUS = 7.5f;
    const float ALIGNMENT_ANGLE  = 0.7f;
    const float ALIGNMENT_WEIGHT = 6.0f;

    const float COHESION_RADIUS =  9.0f;
    const float COHESION_ANGLE  = -0.15f;
    const float COHESION_WEIGHT =  4.0f;

    const float FORMATION_WEIGHT = 8.0f;

    const float BOMBER_INTERCEPTOR_EVASION_RADIUS =
        Stats().m_type == BOMBER ? 25.0f : SEPARATION_RADIUS;
    const float BOMBER_INTERCEPTOR_EVASION_WEIGHT = 8.0f;

    const float NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS = 10.0;
    const float NONFIGHTER_OBSTACLE_AVOIDANCE_WEIGHT = 10.0;

    const float POINT_DEFENSE_AVOIDANCE_RADIUS = MAX_POINT_DEFENSE_RANGE * 1.5;
    const float POINT_DEFENSE_AVOIDANCE_WEIGHT = 8.0;

    // The leader (a "fake" fighter that the real fighters in a formation
    // follow) takes into account all other ("fake") fighters in proximity.
    // Followers ("real", or "normal" fighters) only consider its fellow
    // formation-mates its neighbors.
    OpenSteer::AVGroup neighbors;
    OpenSteer::AVGroup nonfighters;
    if (m_leader) {
        const float FIGHTER_RADIUS = std::max(SEPARATION_RADIUS,
                                              std::max(ALIGNMENT_RADIUS,
                                                       COHESION_RADIUS));
        const float NONFIGHTER_RADIUS = std::max(NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS,
                                                 POINT_DEFENSE_AVOIDANCE_RADIUS);
        m_pathing_engine->GetProximityDB().FindInRadius(
            position(), FIGHTER_RADIUS, neighbors, FIGHTER_FLAGS, EmpireFlag(m_empire_id));
        m_pathing_engine->GetProximityDB().FindInRadius(
            position(), NONFIGHTER_RADIUS, nonfighters, NONFIGHTER_FLAGS);
    } else {
        for (CombatFighterFormation::const_iterator it = m_formation->begin();
             it != m_formation->end();
             ++it) {
            CombatFighterPtr fighter = *it;
            if (fighter->m_formation_position != m_formation_position)
                neighbors.push_back(fighter.get());
        }
    }

    // steer towards mission objectives
    OpenSteer::Vec3 mission_vec;
    if (m_leader && m_mission_weight)
        mission_vec = (m_mission_destination - position()).normalize();

    // steer to maintain formation
    OpenSteer::Vec3 formation_vec;
    m_out_of_formation = OpenSteer::Vec3();
    if (!m_leader) {
        m_out_of_formation = GlobalFormationPosition() - position();
        formation_vec = m_out_of_formation.normalize();
    }

    // steer to avoid interceptors (bombers only)
    OpenSteer::Vec3 bomber_interceptor_evasion_vec;
    OpenSteer::AVGroup interceptor_neighbors;
    if (Stats().m_type == BOMBER) {
        m_pathing_engine->GetProximityDB().FindInRadius(
            position(), BOMBER_INTERCEPTOR_EVASION_RADIUS, interceptor_neighbors,
            INTERCEPTOR_FLAG, EnemyOfEmpireFlags(m_empire_id));
        OpenSteer::Vec3 direction;
        for (std::size_t i = 0; i < interceptor_neighbors.size(); ++i) {
            direction += steerForFlee(interceptor_neighbors[i]->position());
        }
        bomber_interceptor_evasion_vec = direction.normalize();
    }

    // See note at top of function for partial explanation of these variables'
    // meanings.
    OpenSteer::Vec3 nonfighter_obstacle_evasion_vec;
    OpenSteer::Vec3 point_defense_evasion_vec;
    OpenSteer::Vec3 dynamic_obstacle_avoidance;
    if (m_leader) {
        for (std::size_t i = 0; i < nonfighters.size(); ++i) {
            // TODO: Add code to handle non-ships as necessary.
            CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(nonfighters[i]);
            // handle PD avoidance
            OpenSteer::Vec3 away_vec = position() - ship->position();
            float away_vec_length = away_vec.length();
            away_vec /= away_vec_length;
            point_defense_evasion_vec += away_vec * ship->AntiFighterStrength();
            float collision_avoidance_scale_factor =
                std::max(0.0f, NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS - away_vec_length) /
                NONFIGHTER_OBSTACLE_AVOIDANCE_RADIUS;
            nonfighter_obstacle_evasion_vec += away_vec * collision_avoidance_scale_factor;
            if (OBSTACLE_AVOIDANCE_TIME * speed() < away_vec_length)
                dynamic_obstacle_avoidance += away_vec;
        }
        nonfighter_obstacle_evasion_vec = nonfighter_obstacle_evasion_vec.normalize();
        point_defense_evasion_vec = point_defense_evasion_vec.normalize();
        dynamic_obstacle_avoidance = dynamic_obstacle_avoidance.normalize();
    }
    if (0.0 < dynamic_obstacle_avoidance.lengthSquared())
        return dynamic_obstacle_avoidance;

    // flocking behaviors
    OpenSteer::AVGroup neighbors_to_use;
    if (m_leader) {
        neighbors_to_use.reserve(neighbors.size());
        for (std::size_t i = 0; i < neighbors.size(); ++i) {
            CombatFighter* fighter = boost::polymorphic_downcast<CombatFighter*>(neighbors[i]);
            // exclude self and formation-mates from list
            if (fighter == this || &fighter->m_formation->Leader() == this)
                continue;
            neighbors_to_use.push_back(neighbors[i]);
        }
    } else {
        std::swap(neighbors, neighbors_to_use);
    }

    const OpenSteer::Vec3 separation_vec = steerForSeparation(SEPARATION_RADIUS,
                                                              SEPARATION_ANGLE,
                                                              neighbors_to_use);
    const OpenSteer::Vec3 alignment_vec  = steerForAlignment(ALIGNMENT_RADIUS,
                                                             ALIGNMENT_ANGLE,
                                                             neighbors_to_use);
    const OpenSteer::Vec3 cohesion_vec  = steerForCohesion(COHESION_RADIUS,
                                                           COHESION_ANGLE,
                                                           neighbors_to_use);

    return
        mission_vec * m_mission_weight +
        formation_vec * FORMATION_WEIGHT +
        bomber_interceptor_evasion_vec * BOMBER_INTERCEPTOR_EVASION_WEIGHT +
        nonfighter_obstacle_evasion_vec * NONFIGHTER_OBSTACLE_AVOIDANCE_WEIGHT +
        point_defense_evasion_vec * POINT_DEFENSE_AVOIDANCE_WEIGHT +
        separation_vec * SEPARATION_WEIGHT +
        alignment_vec * ALIGNMENT_WEIGHT +
        cohesion_vec * COHESION_WEIGHT;
}

CombatObjectPtr CombatFighter::WeakestAttacker(const CombatObjectPtr& attackee)
{
    CombatObjectPtr retval;

    float weakest = FLT_MAX;

    PathingEngine::ConstAttackerRange attackers = m_pathing_engine->Attackers(attackee);
    for (PathingEngine::Attackees::const_iterator it = attackers.first;
         it != attackers.second;
         ++it) {
        CombatFighterPtr fighter;
        float strength = FLT_MAX;
        if (Stats().m_anti_fighter_damage &&
            (fighter = boost::dynamic_pointer_cast<CombatFighter>(it->second.lock()))) {
            strength = fighter->HealthAndShield() * (1.0 + fighter->AntiFighterStrength());
        } else if (CombatObjectPtr ptr = it->second.lock()) {
            strength = ptr->HealthAndShield() * (1.0 + ptr->AntiFighterStrength());
        }
        if (strength < weakest) {
            retval = it->second.lock();
            weakest = strength;
        }
    }

    return retval;
}

CombatShipPtr CombatFighter::WeakestHostileShip()
{
    CombatShipPtr retval;
    OpenSteer::AVGroup all;
    m_pathing_engine->GetProximityDB().FindAll(
        all, SHIP_FLAG, EnemyOfEmpireFlags(m_empire_id));
    float weakest = FLT_MAX;
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(all[i]);
        if (ship->HealthAndShield() * (1.0 + ship->AntiFighterStrength()) < weakest) {
            retval = ship->shared_from_this();
            weakest = ship->HealthAndShield() * (1.0 + ship->AntiFighterStrength());
        }
    }
    return retval;
}
