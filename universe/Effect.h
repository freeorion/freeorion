// -*- C++ -*-
#ifndef _Effect_h_
#define _Effect_h_

#include "Condition.h"

namespace Effect {
    class EffectsGroup;
    class EffectBase;
    class SetMeter;
    class SetEmpireStockpile;
    class SetEmpireStockpile;
    class SetPlanetType;
    class SetPlanetSize;
    class AddOwner;
    class RemoveOwner;
    //class Create;
    class Destroy;
    class AddSpecial;
    class RemoveSpecial;
    class SetStarType;
    class SetAvailability;
    class SetEffectTarget;
    GG::XMLObjectFactory<EffectBase> EffectFactory(); ///< an XML factory that creates the right subclass of EffectBase from a given XML element
}

/** Contains one or more Effects, a Condition which indicates the objects in the scope of the Effect(s), and a Condition which
    indicates whether or not the Effect(s) will be executed on the objects in scope during the current turn.  Since Conditions
    operate on sets of objects (usually all objects in the universe), the activation condition bears some explanation.  It
    exists to allow an EffectsGroup to be activated or suppressed based on the source object only (the object to which the
    EffectsGroup is attached).  It does this by considering the "universe" containing only the source object. If the source
    object meets the activation condition, the EffectsGroup will be active in the current turn. */
class Effect::EffectsGroup
{
public:
    EffectsGroup(const Condition::ConditionBase* scope, const Condition::ConditionBase* activation, const std::vector<EffectBase*>& effects);
    EffectsGroup(const GG::XMLElement& elem);
    virtual ~EffectsGroup();

    virtual void Execute(int source_id) const;

protected:
    const Condition::ConditionBase* m_scope;
    const Condition::ConditionBase* m_activation;
    std::vector<EffectBase*>        m_effects;
};

/** The base class for all Effects.  When an Effect is executed, the source object (the object to which the Effect or its containing
    EffectGroup is attached) and the target object are both required.  Note that this means that ValueRefs contained within Effects
    can refer to values in either the source or target objects. */
class Effect::EffectBase
{
public:
    virtual ~EffectBase();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const = 0;
};

/** Sets the meter of the given kind to \a value.  The max value of the meter is set if \a max == true; otherwise the current value of the
    meter is set.  If the target of the Effect has no meters, nothing is done. */
class Effect::SetMeter : public Effect::EffectBase
{
public:
    SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value, bool max);
    SetMeter(const GG::XMLElement& elem);
    virtual ~SetMeter();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_value;
    bool                                  m_max;
};

/*class Effect::SetEmpireStockpile : public Effect::EffectBase
{
public:
    SetEmpireStockpile(StockpileType stockpile, const ValueRef::ValueRefBase<int>* empire_id, const ValueRef::ValueRefBase<double>* amount);
    SetEmpireStockpile(const GG::XMLElement& elem);
    virtual ~SetEmpireStockpile();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
StockpileType                         m_stockpile;
    const ValueRef::ValueRefBase<int>*    m_empire_id;
    const ValueRef::ValueRefBase<double>* m_amount;
};*/

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets. */
class Effect::SetPlanetType : public Effect::EffectBase
{
public:
    SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type);
    SetPlanetType(const GG::XMLElement& elem);
    virtual ~SetPlanetType();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<PlanetType>* m_type;
};

/** Sets the planet size of the target to \a size.  This has no effect on non-Planet targets. */
class Effect::SetPlanetSize : public Effect::EffectBase
{
public:
    SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size);
    SetPlanetSize(const GG::XMLElement& elem);
    virtual ~SetPlanetSize();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<PlanetSize>* m_size;
};

/** Adds empire \a empire_id as an owner of the target.  This has no effect if \a empire_id was already an owner of the target object. */
class Effect::AddOwner : public Effect::EffectBase
{
public:
    AddOwner(const ValueRef::ValueRefBase<int>* empire_id);
    AddOwner(const GG::XMLElement& elem);
    virtual ~AddOwner();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<int>* m_empire_id;
};

/** Removes empire \a empire_id as an owner of the target.  This has no effect if \a empire_id was not already an owner of the target object. */
class Effect::RemoveOwner : public Effect::EffectBase
{
public:
    RemoveOwner(const ValueRef::ValueRefBase<int>* empire_id);
    RemoveOwner(const GG::XMLElement& elem);
    virtual ~RemoveOwner();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<int>* m_empire_id;
};

// TODO: create multiple Create*s for different kinds of objects
/*class Effect::Create : public Effect::EffectBase
{
public:
    Create();
    Create(const GG::XMLElement& elem);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
};*/

/** Destroys the target object. */
class Effect::Destroy : public Effect::EffectBase
{
public:
    Destroy();
    Destroy(const GG::XMLElement& elem);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
};

/** Adds the Special with the name \a name to the target object. */
class Effect::AddSpecial : public Effect::EffectBase
{
public:
    AddSpecial(const std::string& name);
    AddSpecial(const GG::XMLElement& elem);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    std::string m_name;
};

/** Removes the Special with the name \a name to the target object.  This has no effect if no such Special was already attached to the target object. */
class Effect::RemoveSpecial : public Effect::EffectBase
{
public:
    RemoveSpecial(const std::string& name);
    RemoveSpecial(const GG::XMLElement& elem);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    std::string m_name;
};

/** Sets the star type of the target to \a type.  This has no effect on non-System targets. */
class Effect::SetStarType : public Effect::EffectBase
{
public:
    SetStarType(const ValueRef::ValueRefBase<StarType>* type);
    SetStarType(const GG::XMLElement& elem);
    virtual ~SetStarType();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<StarType>* m_type;
};

/** Sets the availability of tech \a tech_name to empire \a empire_id.  If \a grant_tech is true, the tech is fully available, just as if it were
    researched normally; otherwise, only the items that the tech grants are made available.  Note that this means this Effect is intended also to
    be used to unlock buildings, ships, etc.  The tech and/or its items are made available if \a available is true, or unavailable otherwise. */
class Effect::SetAvailability : public Effect::EffectBase
{
public:
    SetAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool grant_tech);
    SetAvailability(const GG::XMLElement& elem);
    virtual ~SetAvailability();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    std::string                        m_tech_name;
    const ValueRef::ValueRefBase<int>* m_empire_id;
    bool                               m_available;
    bool                               m_grant_tech;
};

class Effect::SetEffectTarget : public Effect::EffectBase
{
public:
    SetEffectTarget(const ValueRef::ValueRefBase<int>* effect_target_id);
    SetEffectTarget(const GG::XMLElement& elem);
    virtual ~SetEffectTarget();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;

private:
    const ValueRef::ValueRefBase<int>* m_effect_target_id;
};

#endif // _Effect_h_
