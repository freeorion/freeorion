// -*- C++ -*-
#ifndef COMBAT_OBJECT_H
#define COMBAT_OBJECT_H

#include "PathingEngineFwd.h"
#include "SimpleVehicle.h"

#include <boost/serialization/nvp.hpp>


class CombatObject :
    public OpenSteer::SimpleVehicle
{
public:
    enum DamageSource {
        PD_DAMAGE,
        NON_PD_DAMAGE
    };

    virtual double HealthAndShield() const = 0;
    virtual double Health() const = 0;
    virtual double FractionalHealth() const = 0;
    virtual double AntiFighterStrength() const = 0;
    virtual double AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const = 0;
    virtual void Damage(double d, DamageSource source) = 0;
    virtual void Damage(const CombatFighterPtr& source) = 0;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OpenSteer::SimpleVehicle); }
};

#endif
