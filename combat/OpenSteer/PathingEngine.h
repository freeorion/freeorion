// -*- C++ -*-
#ifndef PATHING_ENGINE_H
#define PATHING_ENGINE_H

#include "CombatFighter.h"
#include "CombatShip.h"
#include "PathingEngineFwd.h"
#include "../../universe/Ship.h"

#ifdef FREEORION_MACOSX
#  ifdef check
#    undef check
#  endif
#endif

#include <boost/ptr_container/ptr_vector.hpp>

#include <set>


class FighterStats;

class PathingEngine
{
public:
    typedef boost::ptr_vector<OpenSteer::AbstractObstacle> ObstacleVec;
    typedef std::multimap<CombatObjectPtr, CombatObjectWeakPtr> Attackees;
    typedef std::pair<Attackees::const_iterator, Attackees::const_iterator> ConstAttackerRange;
    typedef std::pair<Attackees::iterator, Attackees::iterator> AttackerRange;
    typedef std::set<CombatObjectPtr>::const_iterator const_iterator;

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

    const_iterator begin () const;
    const_iterator end () const;

    CombatShipPtr FindShip(int id) const;
    CombatFighterPtr FindLeader(int id) const;
    CombatFighterPtr FindFighter(int id) const;

    void Update(const float elapsed_time, bool force);
    void TurnStarted(unsigned int number);

    void AddObject(const CombatObjectPtr& obj);
    void RemoveObject(const CombatObjectPtr& obj);
    int NextFighterID();

    void BeginAttack(const CombatObjectPtr& attacker, const CombatObjectPtr& attackee);
    void EndAttack(const CombatObjectPtr& attacker, const CombatObjectPtr& attackee);

    // fighters
    template <class Iter>
    CombatFighterFormationPtr
    CreateFighterFormation(CombatShipPtr base, Iter first, Iter last);
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
    static const std::size_t SECONDS_PER_TURN;

    static const std::map<int, UniverseObject*>* s_combat_universe;

private:
    void RemoveFighter(const CombatFighterPtr& fighter,
                       std::set<CombatFighterFormationPtr>::iterator formation_it);

    int m_next_fighter_id;
    std::size_t m_update_number;
    std::set<CombatObjectPtr> m_objects;
    std::set<CombatFighterFormationPtr> m_fighter_formations;
    Attackees m_attackees;
    ProximityDB* m_proximity_database;
    ObstacleVec m_obstacles;

    // not serialized
    std::map<int, CombatShipPtr> m_ships_by_id;
    std::map<int, CombatFighterPtr> m_leaders_by_id;
    std::map<int, CombatFighterPtr> m_fighters_by_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

extern const unsigned int INTERCEPTOR_FLAG;
extern const unsigned int BOMBER_FLAG;
extern const unsigned int SHIP_FLAG;
extern const unsigned int MISSILE_FLAG;
extern const unsigned int FIGHTER_FLAGS;
extern const unsigned int NONFIGHTER_FLAGS;

inline unsigned int EmpireFlag(int empire_id)
{ return 1 << static_cast<unsigned int>(empire_id); }

unsigned int EnemyOfEmpireFlags(int empire_id);


// implementations

template <class Iter>
CombatFighterFormationPtr
PathingEngine::CreateFighterFormation(CombatShipPtr base, Iter first, Iter last)
{
    assert(first != last);
    assert(base->GetShip().Owners().size() == 1u);
    int empire_id = *base->GetShip().Owners().begin();

    CombatFighterFormationPtr formation(new CombatFighterFormation(*this));
    CombatFighterPtr fighter(new CombatFighter(CombatObjectPtr(), empire_id, *this));
    formation->SetLeader(fighter);
    fighter->SetWeakPtr(fighter);

    for (Iter it = first; it != last; ++it) {
        CombatFighterPtr fighter = *it;
        fighter->SetFormation(formation);
        formation->push_back(fighter);
        m_objects.insert(fighter);
    }

    m_fighter_formations.insert(formation);

    return formation;
}

#endif
