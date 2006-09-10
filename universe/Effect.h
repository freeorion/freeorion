// -*- C++ -*-
#ifndef _Effect_h_
#define _Effect_h_

#include <set>
#include <vector>

#include "Enums.h"

#include <boost/shared_ptr.hpp>

class UniverseObject;

namespace Condition {
    class ConditionBase;
    typedef std::set<UniverseObject*> ObjectSet;
}

namespace Effect {
    class EffectsGroup;
    class EffectBase;
    class SetMeter;
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
    class SetTechAvailability;
    class SetEffectTarget;
}

namespace ValueRef {
    template <class T>
    struct ValueRefBase;
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
    typedef Condition::ObjectSet TargetSet;

    struct Description
    {
        std::string scope_description;
        std::string activation_description;
        std::vector<std::string> effect_descriptions;
    };

    EffectsGroup(const Condition::ConditionBase* scope, const Condition::ConditionBase* activation,
                 const std::vector<EffectBase*>& effects, const std::string& stacking_group = "");
    virtual ~EffectsGroup();

    void GetTargetSet(int source_id, TargetSet& targets) const;
    void Execute(int source_id, const TargetSet& targets) const;
    const std::string& StackingGroup() const;
    const std::vector<EffectBase*>& EffectsList() const;
    Description GetDescription() const;
    std::string DescriptionString() const;
    std::string Dump() const;

protected:
    const Condition::ConditionBase* m_scope;
    const Condition::ConditionBase* m_activation;
    std::string                     m_stacking_group;
    std::string                     m_explicit_description;
    std::vector<EffectBase*>        m_effects;
};

/** Returns a single string which describes a vector of EffectsGroups. */
std::string EffectsDescription(const std::vector<boost::shared_ptr<const Effect::EffectsGroup> >& effects_groups);

/** The base class for all Effects.  When an Effect is executed, the source object (the object to which the Effect or its containing
    EffectGroup is attached) and the target object are both required.  Note that this means that ValueRefs contained within Effects
    can refer to values in either the source or target objects. */
class Effect::EffectBase
{
public:
    virtual ~EffectBase();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const = 0;
    virtual std::string Description() const = 0;
    virtual std::string Dump() const = 0;
};

/** Sets the meter of the given kind to \a value.  The max value of the meter is set if \a max == true; otherwise the
    current value of the meter is set.  If the target of the Effect does not have the requested meter, nothing is
    done. */
class Effect::SetMeter : public Effect::EffectBase
{
public:
    SetMeter(MeterType meter, const ValueRef::ValueRefBase<double>* value, bool max);
    virtual ~SetMeter();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    MeterType                             m_meter;
    const ValueRef::ValueRefBase<double>* m_value;
    bool                                  m_max;
};

/** Sets the empire stockpile of the target's owning empire to \a value.  If the target does not have exactly one owner,
    nothing is done. */
class Effect::SetEmpireStockpile : public Effect::EffectBase
{
public:
    SetEmpireStockpile(ResourceType stockpile, const ValueRef::ValueRefBase<double>* value);
    virtual ~SetEmpireStockpile();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    ResourceType                          m_stockpile;
    const ValueRef::ValueRefBase<double>* m_value;
};

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets.  Note that changing the
    type of a PT_ASTEROID or PT_GASGIANT planet will also change its size to SZ_TINY or SZ_HUGE, respectively.
    Similarly, changing type to PT_ASTEROID or PT_GASGIANT will also cause the size to change to SZ_ASTEROID or
    SZ_GASGIANT, respectively. */
class Effect::SetPlanetType : public Effect::EffectBase
{
public:
    SetPlanetType(const ValueRef::ValueRefBase<PlanetType>* type);
    virtual ~SetPlanetType();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<PlanetType>* m_type;
};

/** Sets the planet size of the target to \a size.  This has no effect on non-Planet targets.  Note that changing the
    size of a PT_ASTEROID or PT_GASGIANT planet will also change its type to PT_BARREN.  Similarly, changing size to
    SZ_ASTEROID or SZ_GASGIANT will also cause the type to change to PT_ASTEROID or PT_GASGIANT, respectively. */
class Effect::SetPlanetSize : public Effect::EffectBase
{
public:
    SetPlanetSize(const ValueRef::ValueRefBase<PlanetSize>* size);
    virtual ~SetPlanetSize();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<PlanetSize>* m_size;
};

/** Adds empire \a empire_id as an owner of the target.  This has no effect if \a empire_id was already an owner of the target object. */
class Effect::AddOwner : public Effect::EffectBase
{
public:
    AddOwner(const ValueRef::ValueRefBase<int>* empire_id);
    virtual ~AddOwner();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_empire_id;
};

/** Removes empire \a empire_id as an owner of the target.  This has no effect if \a empire_id was not already an owner of the target object. */
class Effect::RemoveOwner : public Effect::EffectBase
{
public:
    RemoveOwner(const ValueRef::ValueRefBase<int>* empire_id);
    virtual ~RemoveOwner();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_empire_id;
};

// TODO: create multiple Create*s for different kinds of objects
/*class Effect::Create : public Effect::EffectBase
{
public:
    Create();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
};*/

/** Destroys the target object.  When executed on objects that contain other objects (such as Fleets and Planets), all
    contained objects are destroyed as well.  Has no effect on System objects.  Destroy effects are executed after all
    other effects. */
class Effect::Destroy : public Effect::EffectBase
{
public:
    Destroy();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
};

/** Adds the Special with the name \a name to the target object. */
class Effect::AddSpecial : public Effect::EffectBase
{
public:
    AddSpecial(const std::string& name);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    std::string m_name;
};

/** Removes the Special with the name \a name to the target object.  This has no effect if no such Special was already attached to the target object. */
class Effect::RemoveSpecial : public Effect::EffectBase
{
public:
    RemoveSpecial(const std::string& name);

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    std::string m_name;
};

/** Sets the star type of the target to \a type.  This has no effect on non-System targets. */
class Effect::SetStarType : public Effect::EffectBase
{
public:
    SetStarType(const ValueRef::ValueRefBase<StarType>* type);
    virtual ~SetStarType();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<StarType>* m_type;
};

/** Sets the availability of tech \a tech_name to empire \a empire_id.  If \a include_tech is true, the tech is fully available, just as if it were
    researched normally; otherwise, only the items that the tech includes are made available.  Note that this means this Effect is intended also to
    be used to unlock buildings, ships, etc.  The tech and/or its items are made available if \a available is true, or unavailable otherwise. */
class Effect::SetTechAvailability : public Effect::EffectBase
{
public:
    SetTechAvailability(const std::string& tech_name, const ValueRef::ValueRefBase<int>* empire_id, bool available, bool include_tech);
    virtual ~SetTechAvailability();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    std::string                        m_tech_name;
    const ValueRef::ValueRefBase<int>* m_empire_id;
    bool                               m_available;
    bool                               m_include_tech;
};

class Effect::SetEffectTarget : public Effect::EffectBase
{
public:
    SetEffectTarget(const ValueRef::ValueRefBase<int>* effect_target_id);
    virtual ~SetEffectTarget();

    virtual void Execute(const UniverseObject* source, UniverseObject* target) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

private:
    const ValueRef::ValueRefBase<int>* m_effect_target_id;
};

#endif // _Effect_h_
