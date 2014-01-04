// -*- C++ -*-
#ifndef _CombatObject_h_
#define _CombatObject_h_

#include "PathingEngineFwd.h"
#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <OpenSteer/SimpleVehicle.h>


class CombatEventListener;

class FO_COMMON_API CombatObject :
    public OpenSteer::SimpleVehicle
{
public:
    enum DamageSource {
        PD_DAMAGE,
        NON_PD_DAMAGE
    };

    CombatObject();

    void SetListener(CombatEventListener& listener);

    virtual float   StructureAndShield() const = 0;
    virtual float   Structure() const = 0;
    virtual float   FractionalStructure() const = 0;
    virtual float   AntiFighterStrength() const = 0;
    virtual float   AntiShipStrength(CombatShipPtr target = CombatShipPtr()) const = 0;
    virtual bool    IsFighter() const = 0;
    virtual bool    IsShip() const = 0;
    virtual int     Owner() const = 0;

    virtual void    Damage(float d, DamageSource source) = 0;
    virtual void    Damage(const CombatFighterPtr& source) = 0;
    virtual void    TurnStarted(unsigned int number) = 0;
    virtual void    SignalDestroyed() = 0;

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
