// -*- C++ -*-
#ifndef _Effect_h_
#define _Effect_h_

#include "EffectAccounting.h"

#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include <vector>

class UniverseObject;
struct ScriptingContext;

namespace Condition {
    struct ConditionBase;
    typedef std::vector<TemporaryPtr<const UniverseObject> > ObjectSet;
}
namespace Effect {
    class EffectsGroup;
    class EffectBase;
    class SetMeter;
    class SetShipPartMeter;
    class SetEmpireMeter;
    class SetEmpireStockpile;
    class SetEmpireCapital;
    class SetPlanetType;
    class SetPlanetSize;
    class SetSpecies;
    class SetOwner;
    class SetSpeciesEmpireOpinion;
    class SetSpeciesSpeciesOpinion;
    class CreatePlanet;
    class CreateBuilding;
    class CreateShip;
    class CreateField;
    class CreateSystem;
    class Destroy;
    class AddSpecial;
    class RemoveSpecial;
    class AddStarlanes;
    class RemoveStarlanes;
    class SetStarType;
    class SetEmpireTechProgress;
    class GiveEmpireTech;
    class MoveTo;
    class MoveInOrbit;
    class MoveTowards;
    class Victory;
    class GenerateSitRepMessage;
    class SetDestination;
    class SetAggression;
    class SetOverlayTexture;
    class SetTexture;
}
namespace ValueRef {
    template <class T>
    struct ValueRefBase;
}


/** Contains one or more Effects, a Condition which indicates the objects in
  * the scope of the Effect(s), and a Condition which indicates whether or not
  * the Effect(s) will be executed on the objects in scope during the current
  * turn.  Since Conditions operate on sets of objects (usually all objects in
  * the universe), the activation condition bears some explanation.  It exists
  * to allow an EffectsGroup to be activated or suppressed based on the source
  * object only (the object to which the EffectsGroup is attached).  It does
  * this by considering the "universe" containing only the source object. If
  * the source object meets the activation condition, the EffectsGroup will be
  * active in the current turn. */
class FO_COMMON_API Effect::EffectsGroup {
public:
    EffectsGroup(Condition::ConditionBase* scope, Condition::ConditionBase* activation,
                 const std::vector<EffectBase*>& effects, const std::string& accounting_label = "",
                 const std::string& stacking_group = "") :
        m_scope(scope),
        m_activation(activation),
        m_stacking_group(stacking_group),
        m_effects(effects),
        m_accounting_label(accounting_label)
    {}
    virtual ~EffectsGroup();

    void    GetTargetSet(int source_id, TargetSet& targets) const;
    void    GetTargetSet(int source_id, TargetSet& targets, const TargetSet& potential_targets) const;
    /** WARNING: this GetTargetSet version will modify potential_targets.
      * in particular, it will move detected targets from potential_targets
      * to targets. Cast the second parameter to \c const \c TargetSet& in
      * order to leave potential_targets unchanged. */
    void    GetTargetSet(int source_id, TargetSet& targets, TargetSet& potential_targets) const;

    /** execute all effects in group */
    void    Execute(const Effect::TargetsCauses& targets_causes,
                    AccountingMap* accounting_map = 0,
                    bool only_meter_effects = false,
                    bool only_appearance_effects = false,
                    bool include_empire_meter_effects = false) const;

    const std::string&              StackingGroup() const       { return m_stacking_group; }
    Condition::ConditionBase* Scope() const               { return m_scope; }
    Condition::ConditionBase* Activation() const          { return m_activation; }
    const std::vector<EffectBase*>& EffectsList() const         { return m_effects; }
    std::string                     DescriptionString() const;
    const std::string&              AccountingLabel() const     { return m_accounting_label; }
    std::string                     Dump() const;

    void                            SetTopLevelContent(const std::string& content_name);

protected:
    Condition::ConditionBase*   m_scope;
    Condition::ConditionBase*   m_activation;
    std::string                 m_stacking_group;
    std::vector<EffectBase*>    m_effects;
    std::string                 m_accounting_label;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns a single string which describes a vector of EffectsGroups. */
FO_COMMON_API std::string EffectsDescription(const std::vector<boost::shared_ptr<Effect::EffectsGroup> >& effects_groups);

/** The base class for all Effects.  When an Effect is executed, the source
  * object (the object to which the Effect or its containing EffectGroup is
  * attached) and the target object are both required.  Note that this means
  * that ValueRefs contained within Effects can refer to values in either the
  * source or target objects. */
class Effect::EffectBase {
public:
    virtual ~EffectBase();

    virtual void        Execute(const ScriptingContext& context) const = 0;
    virtual void        Execute(const ScriptingContext& context, const TargetSet& targets) const;
    virtual void        Execute(const Effect::TargetsCauses& targets_causes,
                                bool stacking,
                                AccountingMap* accounting_map = 0,
                                bool only_meter_effects = false,
                                bool only_appearance_effects = false,
                                bool include_empire_meter_effects = false) const;
    virtual std::string Description() const = 0;
    virtual std::string Dump() const = 0;

    virtual void        SetTopLevelContent(const std::string& content_name) = 0;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the meter of the given kind to \a value.  The max value of the meter
  * is set if \a max == true; otherwise the current value of the meter is set.
  * If the target of the Effect does not have the requested meter, nothing is
  * done. */
class FO_COMMON_API Effect::SetMeter : public Effect::EffectBase {
public:
    SetMeter(MeterType meter, ValueRef::ValueRefBase<double>* value);
    virtual ~SetMeter();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual void        Execute(const ScriptingContext& context, const TargetSet& targets) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;
    MeterType GetMeterType() const {return m_meter;};

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    MeterType                       m_meter;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the indicated meter on all ship parts in the indicated subset.  This
  * has no effect on non-Ship targets.  If slot_type is specified, only parts
  * that can mount in the indicated slot type (internal or external) are
  * affected (this is not the same at the slot type in which the part is
  * actually located, as a part might be mountable in both types, and
  * located in a different type than specified, and would be matched). */
class FO_COMMON_API Effect::SetShipPartMeter : public Effect::EffectBase {
public:
    /** Affects the \a meter_type meter that belongs to part(s) named \a
        part_name. */
    SetShipPartMeter(MeterType meter_type,
                     ValueRef::ValueRefBase<std::string>* part_name,
                     ValueRef::ValueRefBase<double>* value);

    virtual ~SetShipPartMeter();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    const ValueRef::ValueRefBase<std::string>*  GetPartName() const { return m_part_name; }
    MeterType                                   GetMeterType() const { return m_meter; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_part_name;
    MeterType                               m_meter;
    ValueRef::ValueRefBase<double>*         m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the indicated meter on the empire with the indicated id to the
  * indicated value.  If \a meter is not a valid meter for empires,
  * does nothing. */
class FO_COMMON_API Effect::SetEmpireMeter : public Effect::EffectBase {
public:
    SetEmpireMeter(const std::string& meter, ValueRef::ValueRefBase<double>* value);
    SetEmpireMeter(ValueRef::ValueRefBase<int>* empire_id, const std::string& meter,
                   ValueRef::ValueRefBase<double>* value);
    virtual ~SetEmpireMeter();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<int>*    m_empire_id;
    std::string                     m_meter;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the empire stockpile of the target's owning empire to \a value.  If
  * the target does not have exactly one owner, nothing is done. */
class FO_COMMON_API Effect::SetEmpireStockpile : public Effect::EffectBase {
public:
    SetEmpireStockpile(ResourceType stockpile,
                       ValueRef::ValueRefBase<double>* value);
    SetEmpireStockpile(ValueRef::ValueRefBase<int>* empire_id,
                       ResourceType stockpile,
                       ValueRef::ValueRefBase<double>* value);
    virtual ~SetEmpireStockpile();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<int>*    m_empire_id;
    ResourceType                    m_stockpile;
    ValueRef::ValueRefBase<double>* m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Makes the target planet the capital of its owner's empire.  If the target
  * object is not a planet, does not have an owner, or has more than one owner
  * the effect does nothing. */
class FO_COMMON_API Effect::SetEmpireCapital : public Effect::EffectBase {
public:
    explicit SetEmpireCapital();
    explicit SetEmpireCapital(ValueRef::ValueRefBase<int>* empire_id);
    virtual ~SetEmpireCapital();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<int>*    m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets.  Note that changing the
    type of a PT_ASTEROID or PT_GASGIANT planet will also change its size to SZ_TINY or SZ_HUGE, respectively.
    Similarly, changing type to PT_ASTEROID or PT_GASGIANT will also cause the size to change to SZ_ASTEROID or
    SZ_GASGIANT, respectively. */
class FO_COMMON_API Effect::SetPlanetType : public Effect::EffectBase {
public:
    explicit SetPlanetType(ValueRef::ValueRefBase<PlanetType>* type);
    virtual ~SetPlanetType();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<PlanetType>* m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet size of the target to \a size.  This has no effect on non-
  * Planet targets.  Note that changing the size of a PT_ASTEROID or PT_GASGIANT
  * planet will also change its type to PT_BARREN.  Similarly, changing size to
  * SZ_ASTEROID or SZ_GASGIANT will also cause the type to change to PT_ASTEROID
  * or PT_GASGIANT, respectively. */
class FO_COMMON_API Effect::SetPlanetSize : public Effect::EffectBase {
public:
    explicit SetPlanetSize(ValueRef::ValueRefBase<PlanetSize>* size);
    virtual ~SetPlanetSize();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<PlanetSize>* m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the species on the target to \a species_name.  This works on planets
  * and ships, but has no effect on other objects. */
class FO_COMMON_API Effect::SetSpecies : public Effect::EffectBase {
public:
    explicit SetSpecies(ValueRef::ValueRefBase<std::string>* species);
    virtual ~SetSpecies();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_species_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets empire \a empire_id as the owner of the target.  This has no effect if
  * \a empire_id was already the owner of the target object. */
class FO_COMMON_API Effect::SetOwner : public Effect::EffectBase {
public:
    explicit SetOwner(ValueRef::ValueRefBase<int>* empire_id);
    virtual ~SetOwner();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<int>*    m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a species for empire with id \a empire_id to
  * \a opinion */
class FO_COMMON_API Effect::SetSpeciesEmpireOpinion : public Effect::EffectBase {
public:
    SetSpeciesEmpireOpinion(ValueRef::ValueRefBase<std::string>* species_name,
                            ValueRef::ValueRefBase<int>* empire_id,
                            ValueRef::ValueRefBase<double>* opinion);
    virtual ~SetSpeciesEmpireOpinion();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*  m_species_name;
    ValueRef::ValueRefBase<int>*          m_empire_id;
    ValueRef::ValueRefBase<double>*       m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a opinionated_species for other species
  * \a rated_species to \a opinion */
class FO_COMMON_API Effect::SetSpeciesSpeciesOpinion : public Effect::EffectBase {
public:
    SetSpeciesSpeciesOpinion(ValueRef::ValueRefBase<std::string>* opinionated_species_name,
                             ValueRef::ValueRefBase<std::string>* rated_species_name,
                             ValueRef::ValueRefBase<double>* opinion);
    virtual ~SetSpeciesSpeciesOpinion();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*  m_opinionated_species_name;
    ValueRef::ValueRefBase<std::string>*  m_rated_species_name;
    ValueRef::ValueRefBase<double>*       m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Planet with specified \a type and \a size at the system with
  * specified \a location_id */
class FO_COMMON_API Effect::CreatePlanet : public Effect::EffectBase {
public:
    CreatePlanet(ValueRef::ValueRefBase<PlanetType>* type,
                 ValueRef::ValueRefBase<PlanetSize>* size,
                 ValueRef::ValueRefBase<std::string>* name = 0);
    virtual ~CreatePlanet();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<PlanetType>*     m_type;
    ValueRef::ValueRefBase<PlanetSize>*     m_size;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Building with specified \a type on the \a target Planet. */
class FO_COMMON_API Effect::CreateBuilding : public Effect::EffectBase {
public:
    explicit CreateBuilding(ValueRef::ValueRefBase<std::string>* building_type_name,
                            ValueRef::ValueRefBase<std::string>* name = 0);
    virtual ~CreateBuilding();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_building_type_name;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Ship with specified \a predefined_ship_design_name design
  * from those in the list of PredefinedShipDesignManager, and owned by the
  * empire with the specified \a empire_id */
class FO_COMMON_API Effect::CreateShip : public Effect::EffectBase {
public:
    explicit CreateShip(ValueRef::ValueRefBase<std::string>* predefined_ship_design_name,
                        ValueRef::ValueRefBase<int>* empire_id = 0,
                        ValueRef::ValueRefBase<std::string>* species_name = 0,
                        ValueRef::ValueRefBase<std::string>* ship_name = 0);
    explicit CreateShip(ValueRef::ValueRefBase<int>* ship_design_id,
                        ValueRef::ValueRefBase<int>* empire_id = 0,
                        ValueRef::ValueRefBase<std::string>* species_name = 0,
                        ValueRef::ValueRefBase<std::string>* ship_name = 0);
    virtual ~CreateShip();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_design_name;
    ValueRef::ValueRefBase<int>*            m_design_id;
    ValueRef::ValueRefBase<int>*            m_empire_id;
    ValueRef::ValueRefBase<std::string>*    m_species_name;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Field with specified \a field_type_name FieldType
  * of the specified \a size. */
class FO_COMMON_API Effect::CreateField : public Effect::EffectBase {
public:
    explicit CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                         ValueRef::ValueRefBase<double>* size = 0,
                         ValueRef::ValueRefBase<std::string>* name = 0);
    CreateField(ValueRef::ValueRefBase<std::string>* field_type_name,
                ValueRef::ValueRefBase<double>* x,
                ValueRef::ValueRefBase<double>* y,
                ValueRef::ValueRefBase<double>* size = 0,
                ValueRef::ValueRefBase<std::string>* name = 0);
    virtual ~CreateField();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_field_type_name;
    ValueRef::ValueRefBase<double>*         m_x;
    ValueRef::ValueRefBase<double>*         m_y;
    ValueRef::ValueRefBase<double>*         m_size;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new system with the specified \a colour and at the specified
  * location. */
class FO_COMMON_API Effect::CreateSystem : public Effect::EffectBase {
public:
    CreateSystem(ValueRef::ValueRefBase< ::StarType>* type,
                 ValueRef::ValueRefBase<double>* x,
                 ValueRef::ValueRefBase<double>* y,
                 ValueRef::ValueRefBase<std::string>* name = 0);
    CreateSystem(ValueRef::ValueRefBase<double>* x,
                 ValueRef::ValueRefBase<double>* y,
                 ValueRef::ValueRefBase<std::string>* name = 0);
    virtual ~CreateSystem();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase< ::StarType>*    m_type;
    ValueRef::ValueRefBase<double>*         m_x;
    ValueRef::ValueRefBase<double>*         m_y;
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Destroys the target object.  When executed on objects that contain other
  * objects (such as Fleets and Planets), all contained objects are destroyed
  * as well.  Destroy effects delay the desctruction of their targets until
  * after other all effects have executed, to ensure the source or target of
  * other effects are present when they execute. */
class FO_COMMON_API Effect::Destroy : public Effect::EffectBase {
public:
    Destroy();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Adds the Special with the name \a name to the target object. */
class FO_COMMON_API Effect::AddSpecial : public Effect::EffectBase {
public:
    explicit AddSpecial(const std::string& name, float capacity = 1.0f);
    explicit AddSpecial(ValueRef::ValueRefBase<std::string>* name,
                        ValueRef::ValueRefBase<double>* capacity = 0);
    ~AddSpecial();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_name;
    ValueRef::ValueRefBase<double>*         m_capacity;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes the Special with the name \a name to the target object.  This has
  * no effect if no such Special was already attached to the target object. */
class FO_COMMON_API Effect::RemoveSpecial : public Effect::EffectBase {
public:
    explicit RemoveSpecial(const std::string& name);
    explicit RemoveSpecial(ValueRef::ValueRefBase<std::string>* name);
    ~RemoveSpecial();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API Effect::AddStarlanes : public Effect::EffectBase {
public:
    explicit AddStarlanes(Condition::ConditionBase* other_lane_endpoint_condition);
    virtual ~AddStarlanes();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    Condition::ConditionBase*   m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API Effect::RemoveStarlanes : public Effect::EffectBase {
public:
    explicit RemoveStarlanes(Condition::ConditionBase* other_lane_endpoint_condition);
    virtual ~RemoveStarlanes();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    Condition::ConditionBase*   m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the star type of the target to \a type.  This has no effect on
  * non-System targets. */
class FO_COMMON_API Effect::SetStarType : public Effect::EffectBase {
public:
    explicit SetStarType(ValueRef::ValueRefBase<StarType>* type);
    virtual ~SetStarType();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<StarType>*   m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location of another UniverseObject that matches
  * the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API Effect::MoveTo : public Effect::EffectBase {
public:
    explicit MoveTo(Condition::ConditionBase* location_condition);
    virtual ~MoveTo();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    Condition::ConditionBase*   m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location as though it was moving in orbit of
  * some object or position on the map.  Sign of \a speed indicates CCW / CW
  * rotation.*/
class FO_COMMON_API Effect::MoveInOrbit : public Effect::EffectBase {
public:
    MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                Condition::ConditionBase* focal_point_condition);
    MoveInOrbit(ValueRef::ValueRefBase<double>* speed,
                ValueRef::ValueRefBase<double>* focus_x = 0,
                ValueRef::ValueRefBase<double>* focus_y = 0);

    virtual ~MoveInOrbit();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<double>* m_speed;
    Condition::ConditionBase*       m_focal_point_condition;
    ValueRef::ValueRefBase<double>* m_focus_x;
    ValueRef::ValueRefBase<double>* m_focus_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject a specified distance towards some object or
  * position on the map. */
class FO_COMMON_API Effect::MoveTowards : public Effect::EffectBase {
public:
    MoveTowards(ValueRef::ValueRefBase<double>* speed,
                Condition::ConditionBase* dest_condition);
    MoveTowards(ValueRef::ValueRefBase<double>* speed,
                ValueRef::ValueRefBase<double>* dest_x = 0,
                ValueRef::ValueRefBase<double>* dest_y = 0);

    virtual ~MoveTowards();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<double>* m_speed;
    Condition::ConditionBase*       m_dest_condition;
    ValueRef::ValueRefBase<double>* m_dest_x;
    ValueRef::ValueRefBase<double>* m_dest_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the route of the target fleet to move to an UniverseObject that
  * matches the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API Effect::SetDestination : public Effect::EffectBase {
public:
    explicit SetDestination(Condition::ConditionBase* location_condition);
    virtual ~SetDestination();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    Condition::ConditionBase*   m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets aggression level of the target object. */
class FO_COMMON_API Effect::SetAggression : public Effect::EffectBase {
public:
    explicit SetAggression(bool aggressive);

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    bool    m_aggressive;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Causes the owner empire of the target object to win the game.  If the
  * target object has multiple owners, nothing is done. */
class FO_COMMON_API Effect::Victory : public Effect::EffectBase {
public:
    explicit Victory(const std::string& reason_string); // TODO: Make this a ValueRefBase<std::string>*

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    std::string m_reason_string;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets whether an empire has researched at tech, and how much research
  * progress towards that tech has been completed. */
class FO_COMMON_API Effect::SetEmpireTechProgress : public Effect::EffectBase {
public:
    SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                          ValueRef::ValueRefBase<double>* research_progress);
    SetEmpireTechProgress(ValueRef::ValueRefBase<std::string>* tech_name,
                          ValueRef::ValueRefBase<double>* research_progress,
                          ValueRef::ValueRefBase<int>* empire_id);
    virtual ~SetEmpireTechProgress();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_tech_name;
    ValueRef::ValueRefBase<double>*         m_research_progress;
    ValueRef::ValueRefBase<int>*            m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API Effect::GiveEmpireTech : public Effect::EffectBase {
public:
    explicit GiveEmpireTech(ValueRef::ValueRefBase<std::string>* tech_name,
                            ValueRef::ValueRefBase<int>* empire_id = 0);
    virtual ~GiveEmpireTech();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    ValueRef::ValueRefBase<std::string>*    m_tech_name;
    ValueRef::ValueRefBase<int>*            m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Generates a sitrep message for the empire with id \a recipient_empire_id.
  * The message text is the user string specified in \a message_string with
  * string substitutions into the message text as specified in \a message_parameters
  * which are substituted as string parameters %1%, %2%, %3%, etc. in the order
  * they are specified.  Extra parameters beyond those needed by \a message_string
  * are ignored, and missing parameters are left as blank text. */
class FO_COMMON_API Effect::GenerateSitRepMessage : public Effect::EffectBase {
public:
    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          const std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                          ValueRef::ValueRefBase<int>* recipient_empire_id,
                          EmpireAffiliationType affiliation);
    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          const std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >& message_parameters,
                          EmpireAffiliationType affiliation,
                          Condition::ConditionBase* condition = 0);
    virtual ~GenerateSitRepMessage();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    const std::string&              MessageString() const       { return m_message_string; }
    const std::string&              Icon() const                { return m_icon; }
    const std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>*> >&
                                    MessageParameters() const   { return m_message_parameters; }
    ValueRef::ValueRefBase<int>*    RecipientID() const         { return m_recipient_empire_id; }
    Condition::ConditionBase*       GetCondition() const        { return m_condition; }
    EmpireAffiliationType           Affiliation() const         { return m_affiliation; }

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    std::string                                 m_message_string;
    std::string                                 m_icon;
    std::vector<std::pair<std::string,
        ValueRef::ValueRefBase<std::string>*> > m_message_parameters;
    ValueRef::ValueRefBase<int>*                m_recipient_empire_id;
    Condition::ConditionBase*                   m_condition;
    EmpireAffiliationType                       m_affiliation;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies an overlay texture to Systems. */
class FO_COMMON_API Effect::SetOverlayTexture : public Effect::EffectBase {
public:
    SetOverlayTexture(const std::string& texture, ValueRef::ValueRefBase<double>* size);
    virtual ~SetOverlayTexture();

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name);

private:
    std::string                     m_texture;
    ValueRef::ValueRefBase<double>* m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies a texture to Planets. */
class FO_COMMON_API Effect::SetTexture : public Effect::EffectBase {
public:
    explicit SetTexture(const std::string& texture);

    virtual void        Execute(const ScriptingContext& context) const;
    virtual std::string Description() const;
    virtual std::string Dump() const;

    virtual void        SetTopLevelContent(const std::string& content_name) {}

private:
    std::string m_texture;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

// template implementations
template <class Archive>
void Effect::EffectsGroup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_scope)
        & BOOST_SERIALIZATION_NVP(m_activation)
        & BOOST_SERIALIZATION_NVP(m_stacking_group)
        & BOOST_SERIALIZATION_NVP(m_effects);
}

template <class Archive>
void Effect::EffectBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void Effect::SetMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void Effect::SetShipPartMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_part_name)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void Effect::SetEmpireMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void Effect::SetEmpireStockpile::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_stockpile)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void Effect::SetEmpireCapital::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Effect::SetPlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void Effect::SetPlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void Effect::SetSpecies::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_species_name);
}

template <class Archive>
void Effect::SetOwner::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Effect::CreatePlanet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::CreateBuilding::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_building_type_name)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::CreateShip::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_design_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_species_name)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::CreateField::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_field_type_name)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::CreateSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::Destroy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase);
}

template <class Archive>
void Effect::AddSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_capacity);
}

template <class Archive>
void Effect::RemoveSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void Effect::AddStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void Effect::RemoveStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void Effect::SetStarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void Effect::MoveTo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void Effect::MoveInOrbit::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_focal_point_condition)
        & BOOST_SERIALIZATION_NVP(m_focus_x)
        & BOOST_SERIALIZATION_NVP(m_focus_y);
}

template <class Archive>
void Effect::MoveTowards::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_dest_condition)
        & BOOST_SERIALIZATION_NVP(m_dest_x)
        & BOOST_SERIALIZATION_NVP(m_dest_y);
}

template <class Archive>
void Effect::SetDestination::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void Effect::SetAggression::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_aggressive);
}

template <class Archive>
void Effect::Victory::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_reason_string);
}

template <class Archive>
void Effect::SetEmpireTechProgress::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_research_progress)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Effect::GiveEmpireTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void Effect::GenerateSitRepMessage::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_message_string)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_message_parameters)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition)
        & BOOST_SERIALIZATION_NVP(m_affiliation);
}

template <class Archive>
void Effect::SetOverlayTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void Effect::SetTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture);
}


#endif // _Effect_h_
