#include "CombatShip.h"

#include "../../universe/Enums.h"
#include "../../universe/ShipDesign.h"
#include "../../universe/System.h"
#include "../CombatEventListener.h"
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
    const double INVALID_TURN = -1.0;

    template <class Stats>
    struct CopyStats
    {
        CopyStats(double health_factor) : m_health_factor(health_factor) {}
        CombatShip::DirectWeapon operator()(const std::pair<double, const PartType*>& elem)
            {
                const Stats& stats = boost::get<Stats>(elem.second->Stats());
                return CombatShip::DirectWeapon(
                    elem.second->Name(),
                    stats.m_range,
                    stats.m_damage * stats.m_ROF * m_health_factor);
            }
        const double m_health_factor;
    };

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
// CombatShip::DirectWeapon
////////////////////////////////////////////////////////////////////////////////
CombatShip::DirectWeapon::DirectWeapon() :
    m_name(),
    m_range(),
    m_damage()
{}

CombatShip::DirectWeapon::DirectWeapon(const std::string& name, double range, double damage) :
    m_name(name),
    m_range(range),
    m_damage(damage)
{}


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
    m_last_queue_update_turn(std::numeric_limits<unsigned int>::max()),
    m_next_LR_fire_turns(),
    m_turn_start_health(),
    m_turn(std::numeric_limits<unsigned int>::max()),
    m_pathing_engine(0),
    m_raw_PD_strength(0.0),
    m_raw_SR_strength(0.0),
    m_raw_LR_strength(0.0),
    m_is_PD_ship(false),
    m_unfired_SR_weapons(),
    m_unfired_PD_weapons()
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{}

CombatShip::CombatShip(Ship* ship, const OpenSteer::Vec3& position,
                       const OpenSteer::Vec3& direction, PathingEngine& pathing_engine) :
    m_proximity_token(0),
    m_empire_id(*ship->Owners().begin()),
    m_ship(ship),
    m_mission_queue(),
    m_mission_weight(0.0),
    m_last_queue_update_turn(std::numeric_limits<unsigned int>::max()),
    m_next_LR_fire_turns(m_ship->Design()->LRWeapons().size(), INVALID_TURN),
    m_turn_start_health(m_ship->GetMeter(METER_HEALTH)->Current()),
    m_turn(std::numeric_limits<unsigned int>::max()),
    m_pathing_engine(&pathing_engine),
    m_raw_PD_strength(0.0),
    m_raw_SR_strength(0.0),
    m_raw_LR_strength(0.0),
    m_is_PD_ship(false),
    m_unfired_SR_weapons(),
    m_unfired_PD_weapons()
    ,m_instrument(false)
    ,m_last_mission(ShipMission::NONE)
{ Init(position, direction); }

CombatShip::~CombatShip()
{ delete m_proximity_token; }

Ship& CombatShip::GetShip() const
{ return *m_ship; }

const ShipMission& CombatShip::CurrentMission() const
{ return m_mission_queue.back(); }

double CombatShip::HealthAndShield() const
{ return Health() + m_ship->GetMeter(METER_SHIELD)->Current(); }

double CombatShip::Health() const
{ return m_ship->GetMeter(METER_HEALTH)->Current(); }

double CombatShip::FractionalHealth() const
{ return m_turn_start_health / m_ship->GetMeter(METER_HEALTH)->Max(); }

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

bool CombatShip::IsFighter() const
{ return false; }

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
        m_ship->RemoveFighters(it->first, launch_size);
    }
}

void CombatShip::RecoverFighters(const CombatFighterFormationPtr& formation)
{
    assert(!formation->empty());
    m_launched_formations.erase(formation);
    m_pathing_engine->RemoveFighterFormation(formation);
    FighterMap::value_type& map_entry =
        *m_unlaunched_fighters.find((*formation->begin())->PartName());
    std::vector<CombatFighterPtr>& fighter_vec = map_entry.second.second;
    fighter_vec.insert(fighter_vec.end(), formation->begin(), formation->end());
    for (CombatFighterFormation::const_iterator it = formation->begin();
         it != formation->end();
         ++it) {
        (*it)->ExitSpace();
    }
    m_ship->AddFighters(map_entry.first, formation->size());
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
        if (m_last_queue_update_turn != m_turn)
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

void CombatShip::TurnStarted(unsigned int number)
{
    m_turn = number;
    m_turn_start_health = Health();
    if (m_turn - m_enter_starlane_start_turn == ENTER_STARLANE_DELAY_TURNS) {
        Listener().ShipEnteredStarlane(shared_from_this());
        delete m_proximity_token;
        m_proximity_token = 0;
        m_pathing_engine->RemoveObject(shared_from_this());
    } else {
        const ShipDesign& design = *m_ship->Design();
        m_unfired_SR_weapons.resize(design.SRWeapons().size());
        m_unfired_PD_weapons.clear();
        std::transform(design.SRWeapons().begin(), design.SRWeapons().end(),
                       m_unfired_SR_weapons.begin(),
                       CopyStats<DirectFireStats>(FractionalHealth()));
        std::transform(design.PDWeapons().begin(), design.PDWeapons().end(),
                       std::back_inserter(m_unfired_PD_weapons),
                       CopyStats<DirectFireStats>(FractionalHealth()));
    }
}

void CombatShip::SignalDestroyed()
{ Listener().ShipDestroyed(shared_from_this()); }

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
    SimpleVehicle::setMaxSpeed(m_ship->Design()->BattleSpeed());

    // TODO: setMass()

    SimpleVehicle::regenerateOrthonormalBasis(direction, OpenSteer::Vec3(0, 0, 1));

    SimpleVehicle::setPosition(position_);
    SimpleVehicle::setSpeed(0);

    m_proximity_token->UpdatePosition(position());

    m_mission_queue.push_front(ShipMission(ShipMission::NONE));

    const Ship::ConsumablesMap& fighters = m_ship->Fighters();
    for (Ship::ConsumablesMap::const_iterator it = fighters.begin(); it != fighters.end(); ++it) {
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

    m_missiles = m_ship->Missiles();

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

    m_last_queue_update_turn = m_turn;

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
            if (target->IsFighter()) {
                assert(boost::dynamic_pointer_cast<CombatFighter>(target));
                CombatFighterPtr f = boost::static_pointer_cast<CombatFighter>(target);
                target_position = f->Formation()->Centroid();
            }
            OpenSteer::Vec3 from_target_vec = position() - target_position;
            float from_target_length = from_target_vec.length();
            from_target_vec /= from_target_length;
            const float WEAPON_RANGE_FACTOR = 0.9f;
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
        System* system = GetShip().GetSystem();
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
                m_enter_starlane_start_turn = m_turn;
                break;
            }
            assert(!"Illegal ENTER_STARLANE ShipMission was issued -- ship is not within "
                   "minimum distance of any starlane entrance.");
        }
    }
    }

    if (print_needed)
        std::cout << "    position   =" << position() << "\n"
                  << "    destination=" << m_mission_destination << "\n"
                  << "    mission_weight=" << m_mission_weight << "\n"
                  << std::endl;
}

void CombatShip::FirePDDefensively()
{
    if (m_unfired_PD_weapons.empty())
        return;

    OpenSteer::AVGroup all;
    m_pathing_engine->GetProximityDB().FindInRadius(
        position(), MaxPDRange(), all,
        FIGHTER_FLAGS | MISSILE_FLAG, EnemyOfEmpireFlags(m_empire_id));
    for (std::size_t i = 0; i < all.size(); ++i) {
        CombatObject* obj = boost::polymorphic_downcast<CombatObject*>(all[i]);
        double distance_squared = (obj->position() - position()).lengthSquared();
        for (PDList::reverse_iterator it = m_unfired_PD_weapons.rbegin();
             it != m_unfired_PD_weapons.rend(); ) {
            if (distance_squared < it->m_range * it->m_range) {
                CombatObjectPtr shared_obj;
                if (obj->IsFighter()) {
                    shared_obj =
                        boost::polymorphic_downcast<CombatFighter*>(obj)->shared_from_this();
                } else {
                    shared_obj =
                        boost::polymorphic_downcast<Missile*>(obj)->shared_from_this();
                }
                Listener().ShipFired(shared_from_this(), shared_obj, it->m_name);
                double damage = std::min(obj->HealthAndShield(), it->m_damage);
                // HACK! This looks weird, but does what we want.  Non-PD
                // damage on a fighter only affects the fighter itself -- it
                // is not spread out over its formation mates.  This is
                // appropriate here, since we already have our in-range
                // targets.  Finally, since non-PD damage on fighters and
                // missiles gets multiplied by CombatShip::
                // NON_PD_VS_FIGHTER_FACTOR, we divide by it here.
                obj->Damage(damage / CombatShip::NON_PD_VS_FIGHTER_FACTOR, NON_PD_DAMAGE);
                it->m_damage -= damage;
                if (!it->m_damage) {
                    PDList::reverse_iterator temp = boost::next(it);
                    m_unfired_PD_weapons.erase((--it).base());
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

    const double MAX_WEAPON_RANGE_SQUARED = MaxWeaponRange() * MaxWeaponRange();

    FirePDDefensively();

    CombatObjectPtr target = m_mission_subtarget.lock();
    if (!target &&
        (m_mission_queue.back().m_type == ShipMission::ATTACK_THIS ||
         m_mission_queue.back().m_type == ShipMission::ATTACK_THIS_STANDOFF)) {
        assert(m_mission_queue.back().m_target.lock());
        target = m_mission_queue.back().m_target.lock();
        if (MAX_WEAPON_RANGE_SQUARED < (target->position() - position()).lengthSquared())
            target.reset();
    }

    if (target)
        FireAt(target);

    // now find a target of opportunity, in case we didn't fire all of our
    // weapons already
    if (target = m_pathing_engine->NearestHostileShip(position(), m_empire_id))
        FireAt(target);
}

void CombatShip::FireAt(CombatObjectPtr target)
{
    double range_squared = (target->position() - position()).lengthSquared();
    double health_factor = FractionalHealth();

    for (CombatShip::SRVec::reverse_iterator it = m_unfired_SR_weapons.rbegin();
         it != m_unfired_SR_weapons.rend();
         ++it) {
        if (range_squared < it->m_range * it->m_range) {
            Listener().ShipFired(shared_from_this(), target, it->m_name);
            target->Damage(it->m_damage, CombatObject::NON_PD_DAMAGE);
        } else {
            m_unfired_SR_weapons.resize(std::distance(it, m_unfired_SR_weapons.rend()));
            break;
        }
    }
    std::size_t i = 0;
    for (std::multimap<double, const PartType*>::const_iterator it =
             m_ship->Design()->LRWeapons().begin();
         it != m_ship->Design()->LRWeapons().end();
         ++it, ++i) {
        if (m_next_LR_fire_turns[i] < m_turn) {
            double weapon_range_squared = it->first * it->first;
            if (range_squared < weapon_range_squared) {
                OpenSteer::Vec3 direction = (target->position() - position()).normalize();
                MissilePtr missile(
                    new Missile(m_empire_id, *it->second, target,
                                position(), direction, *m_pathing_engine));
                m_pathing_engine->AddObject(missile);
                m_ship->RemoveMissiles(it->second->Name(), 1);
                if (m_next_LR_fire_turns[i] == INVALID_TURN)
                    m_next_LR_fire_turns[i] = m_turn;
                m_next_LR_fire_turns[i] +=
                    boost::get<LRStats>(it->second->Stats()).m_ROF * health_factor;
                Listener().MissileLaunched(missile);
            }
        }
    }
    for (CombatShip::PDList::iterator it = m_unfired_PD_weapons.begin();
         it != m_unfired_PD_weapons.end();
         ++it) {
        if (range_squared < it->m_range * it->m_range) {
            Listener().ShipFired(shared_from_this(), target, it->m_name);
            target->Damage(it->m_damage, CombatObject::PD_DAMAGE);
        } else {
            m_unfired_PD_weapons.erase(it, m_unfired_PD_weapons.end());
            break;
        }
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
    m_pathing_engine->GetProximityDB().FindAll(
        all, SHIP_FLAG, EnemyOfEmpireFlags(m_empire_id));
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
