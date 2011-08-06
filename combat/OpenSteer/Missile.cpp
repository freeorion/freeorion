#include "Missile.h"

#include "../CombatEventListener.h"
#include "PathingEngine.h"


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


Missile::Missile() :
    m_proximity_token(0),
    m_empire_id(ALL_EMPIRES),
    m_part_name(),
    m_last_steer(),
    m_destination(),
    m_target(),
    m_structure(0.0),
    m_pathing_engine()
{}

Missile::Missile(const Ship& launcher, const PartType& part, CombatObjectPtr target,
                 const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
                 PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_empire_id(),
    m_part_name(part.Name()),
    m_last_steer(),
    m_destination(target->position()),
    m_target(target),
    m_structure(),
    m_pathing_engine(&pathing_engine)
{ Init(launcher, position, direction); }

Missile::~Missile()
{ delete m_proximity_token; }

const LRStats& Missile::Stats() const
{ return m_stats; }

const std::string& Missile::PartName() const
{ return m_part_name; }

double Missile::StructureAndShield() const
{ return m_structure; }

double Missile::Structure() const
{ return m_structure; }

double Missile::FractionalStructure() const
{ return 1.0; }

double Missile::AntiFighterStrength() const
{ return 0.0; }

double Missile::AntiShipStrength(CombatShipPtr target/* = CombatShipPtr()*/) const
{ return 0.0; }

bool Missile::IsFighter() const
{ return false; }

bool Missile::IsShip() const
{ return false; }

int Missile::Owner() const
{ return m_empire_id; }

void Missile::update(const float elapsed_time, bool force)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (force ||
        m_pathing_engine->UpdateNumber() % PathingEngine::UPDATE_SETS ==
        serialNumber % PathingEngine::UPDATE_SETS) {
        const float AT_DESTINATION = speed();
        const float AT_DEST_SQUARED = AT_DESTINATION * AT_DESTINATION;
        float distance_squared = (m_destination - position()).lengthSquared();
        CombatObjectPtr target = m_target.lock();
        if (distance_squared < AT_DEST_SQUARED) {
            if (target) {
                Listener().MissileExploded(shared_from_this());
                target->Damage(Stats().m_damage, NON_PD_DAMAGE);
            } else {
                Listener().MissileRemoved(shared_from_this());
            }
            delete m_proximity_token;
            m_proximity_token = 0;
            m_pathing_engine->RemoveObject(shared_from_this());
            return;
        } else {
            if (target)
                m_destination = target->position();
        }
        steer = Steer();
    }
    applySteeringForce(steer, elapsed_time);
    m_last_steer = steer;
    m_proximity_token->UpdatePosition(position());
}

void Missile::regenerateLocalSpace(const OpenSteer::Vec3& newVelocity,
                                   const float elapsedTime)
{}

void Missile::Damage(double d, DamageSource source)
{
    if (source == NON_PD_DAMAGE)
        d *= CombatShip::NON_PD_VS_FIGHTER_FACTOR;
    m_structure = std::max(0.0, m_structure - d);
}

void Missile::Damage(const CombatFighterPtr& source)
{ assert(!"Fighters can't attack missiles."); }

void Missile::TurnStarted(unsigned int number)
{}

void Missile::SignalDestroyed()
{ Listener().MissileRemoved(shared_from_this()); }

void Missile::SetWeakPtr(const MissilePtr& ptr)
{ m_weak_ptr = ptr; }

MissilePtr Missile::shared_from_this()
{
    MissilePtr ptr(m_weak_ptr);
    return ptr;
}

void Missile::Init(const Ship& launcher,
                   const OpenSteer::Vec3& position_,
                   const OpenSteer::Vec3& direction)
{
    assert(!launcher.Unowned());
    m_empire_id = launcher.Owner();

    m_stats.m_damage =      launcher.GetMeter(METER_DAMAGE,     m_part_name)->Current();
    m_stats.m_ROF =         launcher.GetMeter(METER_ROF,        m_part_name)->Current();
    m_stats.m_range =       launcher.GetMeter(METER_RANGE,      m_part_name)->Current();
    m_stats.m_speed =       launcher.GetMeter(METER_SPEED,      m_part_name)->Current();
    m_stats.m_stealth =     launcher.GetMeter(METER_STEALTH,    m_part_name)->Current();
    m_stats.m_structure =   launcher.GetMeter(METER_STRUCTURE,  m_part_name)->Current();
    m_stats.m_capacity=     launcher.GetMeter(METER_CAPACITY,   m_part_name)->Current();

    m_structure = m_stats.m_structure;

    m_proximity_token =
        m_pathing_engine->GetProximityDB().Insert(
            this, MISSILE_FLAG, EmpireFlag(m_empire_id));

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(9.0 * 18.0);
    SimpleVehicle::setMaxSpeed(Stats().m_speed);

    // TODO: setMass()

    SimpleVehicle::regenerateOrthonormalBasis(direction, OpenSteer::Vec3(0, 0, 1));

    SimpleVehicle::setPosition(position_);
    SimpleVehicle::setSpeed(0);

    m_proximity_token->UpdatePosition(position());
}

OpenSteer::Vec3 Missile::Steer()
{
    const float OBSTACLE_AVOIDANCE_TIME = 2.0;

    const OpenSteer::Vec3 avoidance =
        steerToAvoidObstacles(OBSTACLE_AVOIDANCE_TIME,
                              m_pathing_engine->Obstacles().begin(),
                              m_pathing_engine->Obstacles().end());

    if (avoidance != OpenSteer::Vec3::zero)
        return avoidance;

    return (m_destination - position()).normalize();
}
