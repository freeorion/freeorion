// -*- C++ -*-
#ifndef _Missile_h_
#define _Missile_h_

#include "PathingEngineFwd.h"

#include "CombatObject.h"
#include "../../universe/ShipDesign.h"

#include <boost/enable_shared_from_this.hpp>


class Missile :
    public CombatObject,
    public boost::enable_shared_from_this<Missile>
{
public:
    Missile(int empire_id, const PartType& part, CombatObjectPtr target,
            const OpenSteer::Vec3& position, const OpenSteer::Vec3& direction,
            PathingEngine& pathing_engine);
    ~Missile();

    virtual double HealthAndShield() const;
    virtual double Health() const;
    virtual double FractionalHealth() const;
    virtual double AntiFighterStrength() const;
    virtual double AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const;

    virtual void update(const float /*current_time*/, const float elapsed_time);
    virtual void regenerateLocalSpace(const OpenSteer::Vec3& newVelocity,
                                      const float elapsedTime);

    virtual void Damage(double d);

private:
    Missile();

    void Init(const OpenSteer::Vec3& position_, const OpenSteer::Vec3& direction);
    OpenSteer::Vec3 Steer();

    ProximityDBToken* m_proximity_token;
    int m_empire_id;
    OpenSteer::Vec3 m_last_steer;
    LRStats m_stats;
    OpenSteer::Vec3 m_destination; // Only the X and Y values should be nonzero.
    CombatObjectWeakPtr m_target;
    double m_health;
    PathingEngine* m_pathing_engine;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        {
            ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CombatObject)
                & BOOST_SERIALIZATION_NVP(m_proximity_token)
                & BOOST_SERIALIZATION_NVP(m_empire_id)
                & BOOST_SERIALIZATION_NVP(m_destination)
                & BOOST_SERIALIZATION_NVP(m_target)
                & BOOST_SERIALIZATION_NVP(m_pathing_engine);
        }
};

#endif
