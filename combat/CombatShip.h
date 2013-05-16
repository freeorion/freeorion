// -*- C++ -*-
#ifndef _CombatShip_h_
#define _CombatShip_h_

#include "CombatObject.h"
#include "CombatOrder.h"
#include "../universe/Ship.h"

#include <list>
#include <set>


struct DirectFireStats;
struct LRStats;

class CombatShip :
    public CombatObject
{
public:
    struct DirectWeapon
    {
        DirectWeapon();
        DirectWeapon(const std::string& name, float range, float damage);

        std::string m_name;
        float       m_range;
        float       m_damage;

        template <class Archive>
        void serialize(Archive& ar, const unsigned int version);
    };
    typedef std::vector<DirectWeapon> SRVec;
    typedef std::list<DirectWeapon> PDList;

    CombatShip(Ship* ship,
               const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
               const std::map<int, UniverseObject*>& combat_universe,
               PathingEngine& pathing_engine);
    ~CombatShip();

    Ship& GetShip() const;
    const ShipMission& CurrentMission() const;
    virtual float StructureAndShield() const;
    virtual float Structure() const;
    virtual float FractionalStructure() const;
    virtual float AntiFighterStrength() const;
    virtual float AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const;
    virtual bool IsFighter() const;
    virtual bool IsShip() const;
    virtual int Owner() const;

    void LaunchFighters();
    void RecoverFighters(const CombatFighterFormationPtr& formation);
    void AppendMission(const ShipMission& mission);
    void ClearMissions();
    void AppendFighterMission(const FighterMission& mission);
    void ClearFighterMissions();

    virtual void update(const float elapsed_time, bool force);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity,
                                      const float elapsedTime);

    virtual void Damage(float d, DamageSource source);
    virtual void Damage(const CombatFighterPtr& source);
    virtual void TurnStarted(unsigned int number);
    virtual void SignalDestroyed();

    void SetWeakPtr(const CombatShipPtr& ptr);
    CombatShipPtr shared_from_this();

    static const float PD_VS_SHIP_FACTOR;
    static const float NON_PD_VS_FIGHTER_FACTOR;

private:
    CombatShip();

    float MaxWeaponRange() const;
    float MinNonPDWeaponRange() const;
    float MaxPDRange() const;

    void Init(const OpenSteer::Vec3& position_, const OpenSteer::Vec3& direction);
    void PushMission(const ShipMission& mission);
    void RemoveMission();
    void UpdateMissionQueue();
    void FirePDDefensively();
    void FireAtHostiles();
    void FireAt(CombatObjectPtr target);
    OpenSteer::Vec3 Steer();
    CombatObjectPtr WeakestAttacker(const CombatObjectPtr& attackee);
    CombatShipPtr WeakestHostileShip();

    ProximityDBToken*       m_proximity_token;
    int                     m_empire_id;
    int                     m_ship_id;
    const std::map<int, UniverseObject*>* m_combat_universe;

    OpenSteer::Vec3         m_last_steer;

    std::list<ShipMission>  m_mission_queue;
    float                   m_mission_weight;
    OpenSteer::Vec3         m_mission_destination;
    CombatObjectWeakPtr     m_mission_subtarget;

    unsigned int            m_last_queue_update_turn;
    unsigned int            m_enter_starlane_start_turn;
    std::vector<float>      m_next_LR_fire_turns;
    float                   m_turn_start_structure;
    unsigned int            m_turn;

    PathingEngine*          m_pathing_engine;

    float                   m_raw_PD_strength;
    float                   m_raw_SR_strength;
    float                   m_raw_LR_strength;
    bool                    m_is_PD_ship;

    SRVec                   m_unfired_SR_weapons;
    PDList                  m_unfired_PD_weapons;

    // map from part type name to (number of parts in the design of that type,
    // the unlaunched fighters of that part type) pairs
    typedef std::map<
        std::string,
        std::pair<std::size_t, std::vector<CombatFighterPtr> >
    > FighterMap;

    FighterMap m_unlaunched_fighters;
    std::set<CombatFighterFormationPtr> m_launched_formations;

    Ship::ConsumablesMap m_missiles;

    // TODO: Temporary only!
    bool m_instrument;
    ShipMission::Type m_last_mission;

    boost::weak_ptr<CombatShip> m_weak_ptr;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

#endif
