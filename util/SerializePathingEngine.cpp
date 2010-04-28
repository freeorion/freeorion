#include "Serialize.h"

#include "../combat/OpenSteer/AsteroidBeltObstacle.h"
#include "../combat/OpenSteer/CombatShip.h"
#include "../combat/OpenSteer/CombatFighter.h"
#include "../combat/OpenSteer/Missile.h"
#include "../combat/OpenSteer/Obstacle.h"
#include "../combat/OpenSteer/PathingEngine.h"

#include "Serialize.ipp"


// exports for boost serialization of PathingEngine-related classes
BOOST_CLASS_EXPORT(CombatShip)
BOOST_CLASS_EXPORT(CombatFighter)
BOOST_CLASS_EXPORT(Missile)
BOOST_CLASS_EXPORT(OpenSteer::SphereObstacle)
BOOST_CLASS_EXPORT(OpenSteer::BoxObstacle)
BOOST_CLASS_EXPORT(OpenSteer::PlaneObstacle)
BOOST_CLASS_EXPORT(OpenSteer::RectangleObstacle)
BOOST_CLASS_EXPORT(AsteroidBeltObstacle)

template <class Archive>
void CombatShip::DirectWeapon::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_range)
        & BOOST_SERIALIZATION_NVP(m_damage);
}

template <class Archive>
void CombatShip::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_ship_id)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_mission_queue)
        & BOOST_SERIALIZATION_NVP(m_mission_weight)
        & BOOST_SERIALIZATION_NVP(m_mission_destination)
        & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
        & BOOST_SERIALIZATION_NVP(m_last_queue_update_turn)
        & BOOST_SERIALIZATION_NVP(m_next_LR_fire_turns)
        & BOOST_SERIALIZATION_NVP(m_turn_start_health)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_enter_starlane_start_turn)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine)
        & BOOST_SERIALIZATION_NVP(m_raw_PD_strength)
        & BOOST_SERIALIZATION_NVP(m_raw_SR_strength)
        & BOOST_SERIALIZATION_NVP(m_raw_LR_strength)
        & BOOST_SERIALIZATION_NVP(m_is_PD_ship)
        & BOOST_SERIALIZATION_NVP(m_unfired_SR_weapons)
        & BOOST_SERIALIZATION_NVP(m_unfired_PD_weapons)
        & BOOST_SERIALIZATION_NVP(m_unlaunched_fighters)
        & BOOST_SERIALIZATION_NVP(m_launched_formations)
        & BOOST_SERIALIZATION_NVP(m_instrument)
        & BOOST_SERIALIZATION_NVP(m_last_mission);

    if (Archive::is_loading::value)
        m_combat_universe = PathingEngine::s_combat_universe;
}

template <class Archive>
void CombatFighterFormation::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_leader)
        & BOOST_SERIALIZATION_NVP(m_members)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void CombatFighter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_leader)
        & BOOST_SERIALIZATION_NVP(m_part_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_id)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_mission_queue)
        & BOOST_SERIALIZATION_NVP(m_mission_weight)
        & BOOST_SERIALIZATION_NVP(m_mission_destination)
        & BOOST_SERIALIZATION_NVP(m_mission_subtarget)
        & BOOST_SERIALIZATION_NVP(m_base)
        & BOOST_SERIALIZATION_NVP(m_formation_position)
        & BOOST_SERIALIZATION_NVP(m_formation)
        & BOOST_SERIALIZATION_NVP(m_out_of_formation)
        & BOOST_SERIALIZATION_NVP(m_health)
        & BOOST_SERIALIZATION_NVP(m_last_queue_update_turn)
        & BOOST_SERIALIZATION_NVP(m_last_fired_turn)
        & BOOST_SERIALIZATION_NVP(m_turn)
        & BOOST_SERIALIZATION_NVP(m_stats)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void Missile::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
        & BOOST_SERIALIZATION_NVP(m_proximity_token)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_last_steer)
        & BOOST_SERIALIZATION_NVP(m_destination)
        & BOOST_SERIALIZATION_NVP(m_target)
        & BOOST_SERIALIZATION_NVP(m_health)
        & BOOST_SERIALIZATION_NVP(m_stats)
        & BOOST_SERIALIZATION_NVP(m_pathing_engine);
}

template <class Archive>
void PathingEngine::serialize(Archive& ar, const unsigned int version)
{
    std::set<CombatObjectPtr> objects;
    if (Archive::is_saving::value) {
        for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
             it != m_objects.end();
             ++it) {
            // TODO: only copy the objects that are visible from m_objects into objects
            objects.insert(*it);
        }
    }

    ar  & BOOST_SERIALIZATION_NVP(m_next_fighter_id)
        & BOOST_SERIALIZATION_NVP(m_update_number)
        & BOOST_SERIALIZATION_NVP(objects)
        & BOOST_SERIALIZATION_NVP(m_fighter_formations)
        & BOOST_SERIALIZATION_NVP(m_attackees)
        & BOOST_SERIALIZATION_NVP(m_proximity_database)
        & BOOST_SERIALIZATION_NVP(m_obstacles);

    if (Archive::is_loading::value) {
        m_objects.swap(objects);
        for (std::set<CombatObjectPtr>::iterator it = m_objects.begin();
             it != m_objects.end();
             ++it) {
            if ((*it)->IsShip()) {
                assert(boost::dynamic_pointer_cast<CombatShip>(*it));
                CombatShipPtr ship = boost::static_pointer_cast<CombatShip>(*it);
                ship->SetWeakPtr(ship);
            } else if ((*it)->IsFighter()) {
                assert(boost::dynamic_pointer_cast<CombatFighter>(*it));
                CombatFighterPtr fighter = boost::static_pointer_cast<CombatFighter>(*it);
                fighter->SetWeakPtr(fighter);
            } else if (MissilePtr missile = boost::dynamic_pointer_cast<Missile>(*it)) {
                missile->SetWeakPtr(missile);
            }
        }
    }
}

void Serialize(FREEORION_OARCHIVE_TYPE& oa, const PathingEngine& pathing_engine)
{ oa << BOOST_SERIALIZATION_NVP(pathing_engine); }

void Deserialize(FREEORION_IARCHIVE_TYPE& ia, PathingEngine& pathing_engine)
{ ia >> BOOST_SERIALIZATION_NVP(pathing_engine); }
