// -*- C++ -*-
#ifndef _CombatObject_h_
#define _CombatObject_h_

#include "PathingEngineFwd.h"
#include "SimpleVehicle.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>


class CombatEventListener;

class CombatObject :
    public OpenSteer::SimpleVehicle
{
public:
    enum DamageSource {
        PD_DAMAGE,
        NON_PD_DAMAGE
    };

    CombatObject();

    void SetListener(CombatEventListener& listener);

    virtual double HealthAndShield() const = 0;
    virtual double Health() const = 0;
    virtual double FractionalHealth() const = 0;
    virtual double AntiFighterStrength() const = 0;
    virtual double AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const = 0;
    virtual bool IsFighter() const = 0;

    virtual void Damage(double d, DamageSource source) = 0;
    virtual void Damage(const CombatFighterPtr& source) = 0;
    virtual void TurnStarted(unsigned int number) = 0;
    virtual void SignalDestroyed() = 0;

protected:
    CombatEventListener& Listener();

private:
    CombatEventListener* m_listener; // not serialized

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
        { ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(SimpleVehicle); }
};

#endif
