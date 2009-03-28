// -*- C++ -*-
#ifndef COMBAT_OBJECT_H
#define COMBAT_OBJECT_H

#include "SimpleVehicle.h"

#include <boost/serialization/nvp.hpp>


class CombatObject :
    public OpenSteer::SimpleVehicle
{
public:
    virtual double HealthAndShield() const = 0;
    virtual double Health() const = 0;
    virtual double FractionalHealth() const = 0;
    virtual double AntiFighterStrength() const = 0;
    virtual double AntiShipStrength() const = 0;
    virtual void Damage(double d) = 0;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OpenSteer::SimpleVehicle); }
};

#endif
