// -*- C++ -*-
#ifndef _CombatShip_h_
#define _CombatShip_h_

#include "PathingEngineFwd.h"

#include "CombatObject.h"
#include "../CombatOrder.h"
#include "../../universe/Ship.h"

#include <boost/enable_shared_from_this.hpp>

#include <list>
#include <set>


struct DirectFireStats;
struct LRStats;

class CombatShip :
    public CombatObject,
    public boost::enable_shared_from_this<CombatShip>
{
public:
    struct DirectWeapon
    {
        DirectWeapon();
        DirectWeapon(const std::string& name, double range, double damage);

        std::string m_name;
        double m_range;
        double m_damage;

        template <class Archive>
        void serialize(Archive& ar, const unsigned int version)
            {
                ar  & BOOST_SERIALIZATION_NVP(m_name)
                    & BOOST_SERIALIZATION_NVP(m_range)
                    & BOOST_SERIALIZATION_NVP(m_damage);
            }
    };
    typedef std::vector<DirectWeapon> SRVec;
    typedef std::list<DirectWeapon> PDList;

    CombatShip(Ship* ship, const OpenSteer::Vec3& position,
               const OpenSteer::Vec3& direction, PathingEngine& pathing_engine);
    ~CombatShip();

    Ship& GetShip() const;
    const ShipMission& CurrentMission() const;
    virtual double HealthAndShield() const;
    virtual double Health() const;
    virtual double FractionalHealth() const;
    virtual double AntiFighterStrength() const;
    virtual double AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const;
    virtual bool IsFighter() const;

    void LaunchFighters();
    void RecoverFighters(const CombatFighterFormationPtr& formation);
    void AppendMission(const ShipMission& mission);
    void ClearMissions();
    void AppendFighterMission(const FighterMission& mission);
    void ClearFighterMissions();

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity,
                                      const float elapsedTime);

    virtual void Damage(double d, DamageSource source);
    virtual void Damage(const CombatFighterPtr& source);
    virtual void TurnStarted(unsigned int number);
    virtual void SignalDestroyed();

    static const double PD_VS_SHIP_FACTOR;
    static const double NON_PD_VS_FIGHTER_FACTOR;

private:
    CombatShip();

    double MaxWeaponRange() const;
    double MinNonPDWeaponRange() const;
    double MaxPDRange() const;

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

    ProximityDBToken* m_proximity_token;
    int m_empire_id;
    Ship* m_ship;
    OpenSteer::Vec3 m_last_steer;

    std::list<ShipMission> m_mission_queue;
    float m_mission_weight;
    OpenSteer::Vec3 m_mission_destination;
    CombatObjectWeakPtr m_mission_subtarget;

    unsigned int m_last_queue_update_turn;
    unsigned int m_enter_starlane_start_turn;
    std::vector<double> m_next_LR_fire_turns;
    double m_turn_start_health;
    unsigned int m_turn;

    PathingEngine* m_pathing_engine;

    double m_raw_PD_strength;
    double m_raw_SR_strength;
    double m_raw_LR_strength;
    bool m_is_PD_ship;

    SRVec m_unfired_SR_weapons;
    PDList m_unfired_PD_weapons;

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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
                & BOOST_SERIALIZATION_NVP(m_proximity_token)
                & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_ship)
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
        }
};

#endif
