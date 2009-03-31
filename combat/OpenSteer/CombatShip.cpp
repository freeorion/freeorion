#include "CombatShip.h"

#include "../../universe/Enums.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "CombatFighter.h"
#include "Missile.h"
#include "PathingEngine.h"

#include <boost/cast.hpp>
#include <boost/assign/list_of.hpp>

#include <map>


#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


namespace {
    const float NO_PD_FIGHTER_ATTACK_SCALE_FACTOR = 50.0;

    template <class Stats, bool IncludeDamage = false>
    struct CopyStatsPtr
    {
        const Stats* operator()(const std::pair<double, Stats>& elem)
            { return &elem.second; }
    };

    template <class Stats>
    struct CopyStatsPtr<Stats, true>
    {
        CopyStatsPtr(double health_factor) : m_health_factor(health_factor) {}
        std::pair<const Stats*, double> operator()(const std::pair<double, Stats>& elem)
            {
                return std::make_pair(
                    &elem.second,
                    elem.second.m_damage * elem.second.m_ROF * m_health_factor);
            }
        const double m_health_factor;
    };

    void FireAt(CombatObjectPtr target,
                double range_squared,
                double health_factor,
                CombatShip::SRVec& unfired_SR_weapons,
                CombatShip::LRVec& unfired_LR_weapons,
                CombatShip::PDList& unfired_PD_weapons)
    {
        for (CombatShip::SRVec::reverse_iterator it = unfired_SR_weapons.rbegin();
             it != unfired_SR_weapons.rend();
             ++it) {
            double weapon_range = (*it)->m_range;
            double weapon_range_squared = weapon_range * weapon_range;
            if (range_squared < weapon_range_squared) {
                double damage = (*it)->m_damage * (*it)->m_ROF * health_factor;
                target->Damage(damage, CombatObject::NON_PD_DAMAGE);
            } else {
                unfired_SR_weapons.resize(std::distance(it, unfired_SR_weapons.rend()));
                break;
            }
        }
        for (CombatShip::LRVec::reverse_iterator it = unfired_LR_weapons.rbegin();
             it != unfired_LR_weapons.rend();
             ++it) {
            double weapon_range = (*it)->m_range;
            double weapon_range_squared = weapon_range * weapon_range;
            if (range_squared < weapon_range_squared) {
                double damage = (*it)->m_damage * (*it)->m_ROF * health_factor;
                target->Damage(damage, CombatObject::NON_PD_DAMAGE);
            } else {
                unfired_LR_weapons.resize(std::distance(it, unfired_LR_weapons.rend()));
                break;
            }
        }
        for (CombatShip::PDList::reverse_iterator it = unfired_PD_weapons.rbegin();
             it != unfired_PD_weapons.rend();
             ++it) {
            double weapon_range = it->first->m_range;
            double weapon_range_squared = weapon_range * weapon_range;
            if (range_squared < weapon_range_squared) {
                target->Damage(it->second, CombatObject::PD_DAMAGE);
            } else {
                unfired_PD_weapons.resize(std::distance(it, unfired_PD_weapons.rend()));
                break;
            }
        }
    }

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
const double CombatShip::PD_VS_SHIP_FACTOR = 1.0 / 50.0;
const double CombatShip::NON_PD_VS_FIGHTER_FACTOR = 1.0 / 50.0;

CombatShip::CombatShip() :
    m_proximity_token(0),
    m_empire_id(-1),
    m_ship(),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_pathing_engine(0),
    m_raw_PD_strength(0.0),
    m_raw_SR_strength(0.0),
    m_raw_LR_strength(0.0),
    m_is_PD_ship(false)
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
    m_raw_PD_strength(0.0),
    m_raw_SR_strength(0.0),
    m_raw_LR_strength(0.0),
    m_is_PD_ship(false)
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{ Init(position, direction); }

CombatShip::~CombatShip()
{ delete m_proximity_token; }

Ship* CombatShip::GetShip() const
{ return m_ship; }

const ShipMission& CombatShip::CurrentMission() const
{ return m_mission_queue.back(); }

double CombatShip::HealthAndShield() const
{ return Health() + m_ship->GetMeter(METER_SHIELD)->Current(); }

double CombatShip::Health() const
{ return m_ship->GetMeter(METER_HEALTH)->Current(); }

double CombatShip::FractionalHealth() const
{ return Health() / m_ship->GetMeter(METER_HEALTH)->Max(); }

double CombatShip::AntiFighterStrength() const
{ return m_raw_PD_strength * FractionalHealth(); }

double CombatShip::AntiShipStrength(CombatShipPtr target/* = CombatShipPtr()*/) const
{
    double sr = m_raw_SR_strength * FractionalHealth();
    double lr = m_raw_LR_strength * FractionalHealth();
    if (target)
        lr /= 1.0 + target->m_raw_PD_strength * target->FractionalHealth();
    return sr + lr;
}

void CombatShip::LaunchFighters()
{
    // Note that this just launches the fighters that can be launched on this
    // turn.  There is currently no code that accounts for turns(!), so we're
    // only launching part of the fighters here and not providing for the
    // launches of the rest.

    for (FighterMap::iterator it = m_unlaunched_fighters.begin();
         it != m_unlaunched_fighters.end();
         ++it) {
        const PartType* part = GetPartType(it->first);
        assert(part && part->Class() == PC_FIGHTERS);
        const FighterStats& stats = boost::get<FighterStats>(part->Stats());

        std::vector<CombatFighterPtr>& fighters_vec = it->second.second;
        std::size_t num_fighters = fighters_vec.size();
        std::size_t launch_size =
            std::min<std::size_t>(num_fighters, stats.m_launch_rate * it->second.first);

        std::size_t formation_size =
            std::min(CombatFighter::FORMATION_SIZE, launch_size);
        std::size_t num_formations = launch_size / formation_size;
        std::size_t final_formation_size = launch_size % formation_size;
        if (final_formation_size)
            ++num_formations;
        else
            final_formation_size = formation_size;
        for (std::size_t j = 0; j < num_formations; ++j) {
            std::size_t size =
                j == num_formations - 1 ? final_formation_size : formation_size;
            std::set<CombatFighterFormationPtr>::iterator formation_it =
                m_launched_formations.insert(
                    m_pathing_engine->CreateFighterFormation(
                        shared_from_this(),
                        fighters_vec.end() - size,
                        fighters_vec.end())).first;
            fighters_vec.resize(fighters_vec.size() - size);
            m_pathing_engine->AddFighterFormation(*formation_it);
        }
    }
}

void CombatShip::RecoverFighters(const CombatFighterFormationPtr& formation)
{
    assert(!formation->empty());
    m_launched_formations.erase(formation);
    m_pathing_engine->RemoveFighterFormation(formation);
    std::vector<CombatFighterPtr>& fighter_vec =
        m_unlaunched_fighters[(*formation->begin())->PartName()].second;
    fighter_vec.insert(fighter_vec.end(), formation->begin(), formation->end());
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
        (*it)->Leader().AppendMission(mission);
    }
}

void CombatShip::ClearFighterMissions()
{
    assert(!m_launched_formations.empty());
    for (std::set<CombatFighterFormationPtr>::iterator it = m_launched_formations.begin();
         it != m_launched_formations.end();
         ++it) {
        (*it)->Leader().ClearMissions();
    }
}

void CombatShip::update(const float /*current_time*/, const float elapsed_time)
{
    OpenSteer::Vec3 steer = m_last_steer;
    if (m_pathing_engine->UpdateNumber() % PathingEngine::UPDATE_SETS ==
        serialNumber % PathingEngine::UPDATE_SETS) {
        UpdateMissionQueue();
        if (m_ship->IsArmed())
            FireAtHostiles();
        steer = Steer();
    }
    applySteeringForce(steer, elapsed_time);
    m_last_steer = steer;
    m_proximity_token->UpdatePosition(position());
}

void CombatShip::regenerateLocalSpace(const OpenSteer::Vec3& new_velocity,
                                      const float elapsed_time)
{ regenerateLocalSpaceForBanking(new_velocity, elapsed_time); }

void CombatShip::Damage(double d, DamageSource source)
{
    assert(0.0 < d);
    if (source == PD_DAMAGE)
        d *= PD_VS_SHIP_FACTOR;
    double shield_damage = std::min(d, m_ship->GetMeter(METER_SHIELD)->Current());
    m_ship->GetMeter(METER_SHIELD)->AdjustCurrent(-shield_damage);
    d -= shield_damage;
    m_ship->GetMeter(METER_HEALTH)->AdjustCurrent(-d);
}

void CombatShip::Damage(const CombatFighterPtr& source)
{
    double damage = source->Stats().m_anti_ship_damage * source->Formation()->size();
    double shield_damage = std::min(damage, m_ship->GetMeter(METER_SHIELD)->Current());
    m_ship->GetMeter(METER_SHIELD)->AdjustCurrent(-shield_damage);
    damage -= shield_damage;
    m_ship->GetMeter(METER_HEALTH)->AdjustCurrent(-damage);
}

double CombatShip::MaxWeaponRange() const
{ return m_ship->Design()->MaxWeaponRange(); }

double CombatShip::MinNonPDWeaponRange() const
{ return m_ship->Design()->MinNonPDWeaponRange(); }

double CombatShip::MaxPDRange() const
{ return m_ship->Design()->MaxPDRange(); }

void CombatShip::Init(const OpenSteer::Vec3& position_, const OpenSteer::Vec3& direction)
{
    m_proximity_token =
        m_pathing_engine->GetProximityDB().Insert(this, SHIP_FLAG, EmpireFlag(m_empire_id));

    SimpleVehicle::reset();
    SimpleVehicle::setMaxForce(3.0);
    SimpleVehicle::setMaxSpeed(m_ship->Design()->Speed());

    // TODO: setMass()

    SimpleVehicle::regenerateOrthonormalBasis(direction, OpenSteer::Vec3(0, 0, 1));

    SimpleVehicle::setPosition(position_);
    SimpleVehicle::setSpeed(0);

    m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(ShipMission(ShipMission::NONE));

    const Ship::FighterMap& fighters = m_ship->Fighters();
    for (Ship::FighterMap::const_iterator it = fighters.begin(); it != fighters.end(); ++it) {
        const PartType* part = GetPartType(it->first);
        assert(part && part->Class() == PC_FIGHTERS);
        std::size_t num_fighters = it->second.second;

        m_unlaunched_fighters[it->first].first = it->second.first;
        std::vector<CombatFighterPtr>& fighter_vec =
            m_unlaunched_fighters[it->first].second;
        fighter_vec.resize(num_fighters);
        for (std::size_t i = 0; i < num_fighters; ++i) {
            fighter_vec[i].reset(
                new CombatFighter(shared_from_this(), *part, m_empire_id, *m_pathing_engine));
        }
    }

    double PD_minus_non_PD = 0.0;
    const std::vector<std::string>& part_names = m_ship->Design()->Parts();
    for (std::size_t i = 0; i < part_names.size(); ++i) {
        if (part_names[i].empty())
            continue;

        const PartType* part = GetPartType(part_names[i]);
        assert(part);
        if (part->Class() == PC_POINT_DEFENSE) {
            const DirectFireStats& stats = boost::get<DirectFireStats>(part->Stats());
            m_raw_PD_strength += stats.m_damage * stats.m_ROF * stats.m_range;
            PD_minus_non_PD += stats.m_damage;
        } else if (part->Class() == PC_SHORT_RANGE) {
            const DirectFireStats& stats = boost::get<DirectFireStats>(part->Stats());
            m_raw_SR_strength += stats.m_damage * stats.m_ROF * stats.m_range;
            PD_minus_non_PD -= stats.m_damage;
        } else if (part->Class() == PC_MISSILES) {
            const LRStats& stats = boost::get<LRStats>(part->Stats());
            m_raw_LR_strength += stats.m_damage * stats.m_ROF * stats.m_range;
            PD_minus_non_PD -= stats.m_damage;
        }
    }
    m_is_PD_ship = 0.0 < PD_minus_non_PD;
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
    m_mission_subtarget.reset();
    m_mission_queue.pop_back();
    if (m_mission_queue.empty())
        m_mission_queue.push_front(ShipMission(ShipMission::NONE));
}

void CombatShip::UpdateMissionQueue()
{
    assert(!m_mission_queue.empty());

    const float DEFAULT_MISSION_WEIGHT = 12.0;
    const float MAX_MISSION_WEIGHT = 48.0;

    const float AT_DESTINATION = std::max(3.0f, speed());
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
            const float WEAPON_RANGE_FACTOR = 0.9;
            float distance =
                std::min<float>(MaxWeaponRange() * WEAPON_RANGE_FACTOR, from_target_length);
            if (m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF)
                distance = MinNonPDWeaponRange() * WEAPON_RANGE_FACTOR;
            m_mission_destination = target_position + distance * from_target_vec;
        } else {
            if (print_needed) std::cout << "    [ATTACK TARGET GONE]\n";
            RemoveMission();
        }
        break;
    }
    case ShipMission::DEFEND_THIS: {
        if (CombatObjectPtr target = m_mission_queue.back().m_target.lock()) {
            m_mission_weight = DEFAULT_MISSION_WEIGHT;
            if (m_is_PD_ship) {
                PathingEngine::ConstAttackerRange attackers =
                    m_pathing_engine->Attackers(target);
                CombatShipPtr ship;
                for (PathingEngine::Attackees::const_iterator it = attackers.first;
                     it != attackers.second;
                     ++it) {
                    if (CombatShipPtr temp =
                        boost::dynamic_pointer_cast<CombatShip>(it->second.lock())) {
                        if (!ship || ship->m_raw_LR_strength < temp->m_raw_LR_strength)
                            ship = temp;
                    }
                }
                if (ship) {
                    m_mission_weight = MAX_MISSION_WEIGHT;
                    OpenSteer::Vec3 target_position = target->position();
                    OpenSteer::Vec3 attacker_position = ship->position();
                    OpenSteer::Vec3 target_to_attacker =
                        (attacker_position - target_position).normalize();
                    double min_PD_range = m_ship->Design()->PDWeapons().begin()->first;
                    m_mission_destination =
                        target_position + target_to_attacker * min_PD_range / 2.0;
                } else {
                    // No attacker found; just get close to the target to keep
                    // it blanketted with PD.
                    OpenSteer::Vec3 target_position = target->position();
                    OpenSteer::Vec3 target_to_here =
                        (position() - target->position()).normalize();
                    double min_PD_range = m_ship->Design()->PDWeapons().begin()->first;
                    m_mission_destination =
                        target_position + target_to_here * min_PD_range / 3.0;
                }
            } else {
                if (m_mission_subtarget.expired()) {
                    m_mission_subtarget = WeakestAttacker(target);
                    if (CombatObjectPtr subtarget = m_mission_subtarget.lock())
                        m_mission_destination = subtarget->position();
                    else
                        m_mission_destination = target->position();
                }
            }
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

void CombatShip::FirePDDefensively(PDList& unfired_PD_weapons)
{
    if (unfired_PD_weapons.empty())
        return;

    OpenSteer::AVGroup all;
    // TODO: NotEmpireFlag() should become EnemyOfEmpireFlag()
    m_pathing_engine->GetProximityDB().FindInRadius(
        position(), MaxPDRange(), all,
        FIGHTER_FLAGS | MISSILE_FLAG, NotEmpireFlag(m_empire_id));
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatObject* obj = boost::polymorphic_downcast<CombatObject*>(all[i]);
        double distance_squared = (obj->position() - position()).lengthSquared();
        for (PDList::reverse_iterator it = unfired_PD_weapons.rbegin();
             it != unfired_PD_weapons.rend(); ) {
            double weapon_range = it->first->m_range;
            if (distance_squared < weapon_range * weapon_range) {
                double damage = std::min(obj->HealthAndShield(), it->second);
                // HACK! This looks weird, but does what we want.  Non-PD
                // damage on a fighter only affects the fighter itself -- it
                // is not spread out over its formation mates.  This is
                // appropriate here, since we already have our in-range
                // targets.  Finally, since non-PD damage on fighters and
                // missiles gets multiplied by CombatShip::
                // NON_PD_VS_FIGHTER_FACTOR, we divide by it here.
                obj->Damage(damage / CombatShip::NON_PD_VS_FIGHTER_FACTOR, NON_PD_DAMAGE);
                it->second -= damage;
                if (!it->second) {
                    PDList::reverse_iterator temp = boost::next(it);
                    unfired_PD_weapons.erase((--it).base());
                    it = temp;
                }
                if (!obj->HealthAndShield())
                    break;
            }
        }
    }
}

void CombatShip::FireAtHostiles()
{
    assert(!m_mission_queue.empty());

    const ShipDesign& design = *m_ship->Design();

    SRVec unfired_SR_weapons(design.SRWeapons().size());
    LRVec unfired_LR_weapons(design.LRWeapons().size());
    PDList unfired_PD_weapons;
    std::transform(design.SRWeapons().begin(), design.SRWeapons().end(),
                   unfired_SR_weapons.begin(), CopyStatsPtr<DirectFireStats>());
    std::transform(design.LRWeapons().begin(), design.LRWeapons().end(),
                   unfired_LR_weapons.begin(), CopyStatsPtr<LRStats>());
    std::transform(design.PDWeapons().begin(), design.PDWeapons().end(),
                   std::back_inserter(unfired_PD_weapons),
                   CopyStatsPtr<DirectFireStats, true>(FractionalHealth()));

    const double MAX_WEAPON_RANGE_SQUARED = MaxWeaponRange() * MaxWeaponRange();

    FirePDDefensively(unfired_PD_weapons);

    CombatFighterPtr fighter;
    CombatObjectPtr target = m_mission_subtarget.lock();
    if (!target &&
        (m_mission_queue.back().m_type == ShipMission::ATTACK_THIS ||
         m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF)) {
        assert(m_mission_queue.back().m_target.lock());
        target = m_mission_queue.back().m_target.lock();
        if (MAX_WEAPON_RANGE_SQUARED < (target->position() - position()).lengthSquared())
            target.reset();
    }

    if (target) {
        double range_squared = (target->position() - position()).lengthSquared();
        FireAt(target, range_squared, FractionalHealth(),
               unfired_SR_weapons, unfired_LR_weapons, unfired_PD_weapons);
    }

    // now find a target of opportunity, if we didn't fire all our weapons already
    if (!unfired_SR_weapons.empty() &&
        !unfired_LR_weapons.empty() &&
        !unfired_PD_weapons.empty() &&
        (target = m_pathing_engine->NearestHostileShip(position(), m_empire_id))) {
        double range_squared = (target->position() - position()).lengthSquared();
        FireAt(target, range_squared, FractionalHealth(),
               unfired_SR_weapons, unfired_LR_weapons, unfired_PD_weapons);
    }
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

    // This enforces the preference for attacking bombers over interceptors.
    const float BOMBER_SCALE_FACTOR = 0.95f;
    const float INTERCEPTOR_SCALE_FACTOR = 1.0f;

    PathingEngine::ConstAttackerRange attackers = m_pathing_engine->Attackers(attackee);
    for (PathingEngine::Attackees::const_iterator it = attackers.first;
         it != attackers.second;
         ++it) {
        CombatFighterPtr fighter;
        CombatShipPtr ship;
        float strength = FLT_MAX;
        if (fighter = boost::dynamic_pointer_cast<CombatFighter>(it->second.lock())) {
            strength =
                fighter->HealthAndShield() * (fighter->Stats().m_type == INTERCEPTOR ?
                                              INTERCEPTOR_SCALE_FACTOR : BOMBER_SCALE_FACTOR);
            strength /= (1.0 + AntiFighterStrength());
            if (AntiFighterStrength())
                strength *= NO_PD_FIGHTER_ATTACK_SCALE_FACTOR;
        } else if (CombatObjectPtr ptr = it->second.lock()) {
            strength =
                ptr->HealthAndShield() * (1.0 + ptr->AntiShipStrength(shared_from_this()));
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
    CombatShipPtr retval;
    OpenSteer::AVGroup all;
    // TODO: Replace NotEmpireFlag with EnemyOfEmpireFlag later.
    m_pathing_engine->GetProximityDB().FindAll(all, SHIP_FLAG, NotEmpireFlag(m_empire_id));
    float weakest = FLT_MAX;
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatShip* ship = boost::polymorphic_downcast<CombatShip*>(all[i]);
        double strength =
            ship->HealthAndShield() * (1.0 + ship->AntiShipStrength(shared_from_this()));
        if (strength < weakest) {
            retval = ship->shared_from_this();
            weakest = strength;
        }
    }
    return retval;
}
