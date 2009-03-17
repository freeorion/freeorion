// -*- C++ -*-
#ifndef PATHING_ENGINE_H
#define PATHING_ENGINE_H

#include "PathingEngineFwd.h"

#include <boost/ptr_container/ptr_vector.hpp>

#include <set>


class PathingEngine
{
public:
    typedef boost::ptr_vector<OpenSteer::AbstractObstacle> ObstacleVec;
    typedef std::multimap<CombatObjectPtr, CombatObjectWeakPtr> Attackees;
    typedef std::pair<Attackees::const_iterator, Attackees::const_iterator> ConstAttackerRange;
    typedef std::pair<Attackees::iterator, Attackees::iterator> AttackerRange;

    PathingEngine();
    ~PathingEngine();

    const ProximityDB& GetProximityDB() const;
    const ObstacleVec& Obstacles() const;

    CombatFighterPtr NearestHostileFighterInRange(const OpenSteer::Vec3& position,
                                                  int empire_id, float range) const;
    CombatObjectPtr NearestHostileNonFighterInRange(const OpenSteer::Vec3& position,
                                                    int empire_id, float range) const;
    CombatFighterPtr NearestHostileInterceptor(const OpenSteer::Vec3& position,
                                               int empire_id) const;
    CombatFighterPtr NearestHostileBomber(const OpenSteer::Vec3& position,
                                          int empire_id) const;
    CombatShipPtr NearestHostileShip(const OpenSteer::Vec3& position,
                                     int empire_id) const;
    std::size_t UpdateNumber() const;
    ConstAttackerRange Attackers (const CombatObjectPtr& attackee) const;

    void Update(const float current_time, const float elapsed_time);

    void AddObject(const CombatObjectPtr& obj);
    void RemoveObject(const CombatObjectPtr& obj);

    void BeginAttack(const CombatObjectPtr& attacker, const CombatObjectPtr& attackee);
    void EndAttack(const CombatObjectPtr& attacker, const CombatObjectPtr& attackee);

    // fighters
    CombatFighterFormationPtr
    CreateFighterFormation(CombatShipPtr base, CombatFighterType type, std::size_t size);
    void AddFighterFormation(const CombatFighterFormationPtr& formation);
    void RemoveFighter(const CombatObjectPtr& fighter);
    void RemoveFighterFormation(const CombatFighterFormationPtr& formation);
    ProximityDB& GetProximityDB();

    // obstacles
    void ClearObstacles();
    void AddObstacle(OpenSteer::AbstractObstacle* obstacle);

    static const std::size_t TARGET_FPS;
    static const std::size_t TARGET_OBJECT_UPDATES_PER_SEC;
    static const std::size_t UPDATE_SETS;

private:
    int m_next_fighter_id;
    std::size_t m_update_number;
    std::set<CombatObjectPtr> m_objects;
    std::set<CombatFighterFormationPtr> m_fighter_formations;
    Attackees m_attackees;
    ProximityDB* m_proximity_database;
    ObstacleVec m_obstacles;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_NVP(m_update_number)
                & BOOST_SERIALIZATION_NVP(m_objects)
                & BOOST_SERIALIZATION_NVP(m_fighter_formations)
                & BOOST_SERIALIZATION_NVP(m_proximity_database)
                & BOOST_SERIALIZATION_NVP(m_obstacles);
        }
};

extern const unsigned int INTERCEPTOR_FLAG;
extern const unsigned int BOMBER_FLAG;
extern const unsigned int SHIP_FLAG;
extern const unsigned int FIGHTER_FLAGS;
extern const unsigned int NONFIGHTER_FLAGS;

inline unsigned int EmpireFlag(int empire_id)
{ return 1 << static_cast<unsigned int>(empire_id); }

inline unsigned int NotEmpireFlag(int empire_id)
{ return ~(1 << static_cast<unsigned int>(empire_id)); }

#endif
