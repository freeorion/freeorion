#ifndef _Effect_h_
#define _Effect_h_

#include "EffectAccounting.h"

#include "../util/Export.h"

#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/optional/optional.hpp>

#include <vector>

class UniverseObject;
struct ScriptingContext;

namespace Condition {
    struct ConditionBase;
    typedef std::vector<std::shared_ptr<const UniverseObject>> ObjectSet;
}

namespace ValueRef {
    template <class T>
    struct ValueRefBase;
}

namespace Effect {
class EffectBase;


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
class FO_COMMON_API EffectsGroup {
public:
    EffectsGroup(std::unique_ptr<Condition::ConditionBase>&& scope,
                 std::unique_ptr<Condition::ConditionBase>&& activation,
                 std::vector<std::unique_ptr<EffectBase>>&& effects,
                 const std::string& accounting_label = "",
                 const std::string& stacking_group = "", int priority = 0,
                 const std::string& description = "",
                 const std::string& content_name = "");
    virtual ~EffectsGroup();

    /** execute all effects in group */
    void    Execute(const TargetsCauses& targets_causes,
                    AccountingMap* accounting_map = nullptr,
                    bool only_meter_effects = false,
                    bool only_appearance_effects = false,
                    bool include_empire_meter_effects = false,
                    bool only_generate_sitrep_effects = false) const;

    const std::string&              StackingGroup() const       { return m_stacking_group; }
    Condition::ConditionBase*       Scope() const               { return m_scope.get(); }
    Condition::ConditionBase*       Activation() const          { return m_activation.get(); }
    const std::vector<EffectBase*>  EffectsList() const;
    const std::string&              GetDescription() const;
    const std::string&              AccountingLabel() const     { return m_accounting_label; }
    int                             Priority() const            { return m_priority; }
    std::string                     Dump(unsigned short ntabs = 0) const;
    bool                            HasMeterEffects() const;
    bool                            HasAppearanceEffects() const;
    bool                            HasSitrepEffects() const;

    void                            SetTopLevelContent(const std::string& content_name);
    const std::string&              TopLevelContent() const { return m_content_name; }

    virtual unsigned int            GetCheckSum() const;

protected:
    std::unique_ptr<Condition::ConditionBase>   m_scope;
    std::unique_ptr<Condition::ConditionBase>   m_activation;
    std::string                 m_stacking_group;
    std::vector<std::unique_ptr<EffectBase>>    m_effects;
    std::string                 m_accounting_label;
    int                         m_priority;
    std::string                 m_description;
    std::string                 m_content_name;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Returns a single string which `Dump`s a vector of EffectsGroups. */
FO_COMMON_API std::string Dump(const std::vector<std::shared_ptr<EffectsGroup>>& effects_groups);

/** The base class for all Effects.  When an Effect is executed, the source
  * object (the object to which the Effect or its containing EffectGroup is
  * attached) and the target object are both required.  Note that this means
  * that ValueRefs contained within Effects can refer to values in either the
  * source or target objects. */
class FO_COMMON_API EffectBase {
public:
    virtual ~EffectBase();

    virtual void Execute(const ScriptingContext& context) const = 0;

    virtual void Execute(const ScriptingContext& context, const TargetSet& targets) const;

    void Execute(const TargetsCauses& targets_causes,
                 AccountingMap* accounting_map,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const;

    virtual void Execute(const ScriptingContext& context,
                         const TargetSet& targets,
                         AccountingMap* accounting_map,
                         const EffectCause& effect_cause,
                         bool only_meter_effects = false,
                         bool only_appearance_effects = false,
                         bool include_empire_meter_effects = false,
                         bool only_generate_sitrep_effects = false) const;

    virtual std::string     Dump(unsigned short ntabs = 0) const = 0;
    virtual bool            IsMeterEffect() const { return false; }
    virtual bool            IsEmpireMeterEffect() const { return false; }
    virtual bool            IsAppearanceEffect() const { return false; }
    virtual bool            IsSitrepEffect() const { return false; }
    virtual bool            IsConditionalEffect() const { return false; }
    virtual void            SetTopLevelContent(const std::string& content_name) = 0;
    virtual unsigned int    GetCheckSum() const;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Does nothing when executed. Useful for triggering side-effects of effect
  * execution without modifying the gamestate. */
class FO_COMMON_API NoOp final : public EffectBase {
public:
    NoOp();

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override {}
    unsigned int    GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the meter of the given kind to \a value.  The max value of the meter
  * is set if \a max == true; otherwise the current value of the meter is set.
  * If the target of the Effect does not have the requested meter, nothing is
  * done. */
class FO_COMMON_API SetMeter final : public EffectBase {
public:

    SetMeter(MeterType meter,
             std::unique_ptr<ValueRef::ValueRefBase<double>>&& value,
             const boost::optional<std::string>& accounting_label = boost::none);

    void Execute(const ScriptingContext& context) const override;

    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(const ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string         Dump(unsigned short ntabs = 0) const override;
    bool                IsMeterEffect() const override { return true; }
    void                SetTopLevelContent(const std::string& content_name) override;
    MeterType           GetMeterType() const { return m_meter; };
    const std::string&  AccountingLabel() const { return m_accounting_label; }
    unsigned int        GetCheckSum() const override;

private:
    MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_value;
    std::string m_accounting_label;

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
class FO_COMMON_API SetShipPartMeter final : public EffectBase {
public:
    /** Affects the \a meter_type meter that belongs to part(s) named \a
        part_name. */
    SetShipPartMeter(MeterType meter_type,
                     std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& part_name,
                     std::unique_ptr<ValueRef::ValueRefBase<double>>&& value);

    void Execute(const ScriptingContext& context) const override;
    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;
    void Execute(const ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string     Dump(unsigned short ntabs = 0) const override;
    bool            IsMeterEffect() const override { return true; }
    void            SetTopLevelContent(const std::string& content_name) override;
    const           ValueRef::ValueRefBase<std::string>* GetPartName() const { return m_part_name.get(); }
    MeterType       GetMeterType() const { return m_meter; }
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_part_name;
    MeterType                                               m_meter;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the indicated meter on the empire with the indicated id to the
  * indicated value.  If \a meter is not a valid meter for empires,
  * does nothing. */
class FO_COMMON_API SetEmpireMeter final : public EffectBase {
public:
    SetEmpireMeter(const std::string& meter, std::unique_ptr<ValueRef::ValueRefBase<double>>&& value);

    SetEmpireMeter(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id, const std::string& meter,
                   std::unique_ptr<ValueRef::ValueRefBase<double>>&& value);

    void Execute(const ScriptingContext& context) const override;
    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;
    void Execute(const ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string     Dump(unsigned short ntabs = 0) const override;
    bool            IsMeterEffect() const override { return true; }
    bool            IsEmpireMeterEffect() const override { return true; }
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<int>>    m_empire_id;
    std::string                                     m_meter;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the empire stockpile of the target's owning empire to \a value.  If
  * the target does not have exactly one owner, nothing is done. */
class FO_COMMON_API SetEmpireStockpile final : public EffectBase {
public:
    SetEmpireStockpile(ResourceType stockpile,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& value);
    SetEmpireStockpile(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                       ResourceType stockpile,
                       std::unique_ptr<ValueRef::ValueRefBase<double>>&& value);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<int>>    m_empire_id;
    ResourceType                                    m_stockpile;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_value;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Makes the target planet the capital of its owner's empire.  If the target
  * object is not a planet, does not have an owner, or has more than one owner
  * the effect does nothing. */
class FO_COMMON_API SetEmpireCapital final : public EffectBase {
public:
    explicit SetEmpireCapital();
    explicit SetEmpireCapital(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<int>> m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets.  Note that changing the
    type of a PT_ASTEROID or PT_GASGIANT planet will also change its size to SZ_TINY or SZ_HUGE, respectively.
    Similarly, changing type to PT_ASTEROID or PT_GASGIANT will also cause the size to change to SZ_ASTEROID or
    SZ_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetType final : public EffectBase {
public:
    explicit SetPlanetType(std::unique_ptr<ValueRef::ValueRefBase<PlanetType>>&& type);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<PlanetType>> m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the planet size of the target to \a size.  This has no effect on non-
  * Planet targets.  Note that changing the size of a PT_ASTEROID or PT_GASGIANT
  * planet will also change its type to PT_BARREN.  Similarly, changing size to
  * SZ_ASTEROID or SZ_GASGIANT will also cause the type to change to PT_ASTEROID
  * or PT_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetSize final : public EffectBase {
public:
    explicit SetPlanetSize(std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>>&& size);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>> m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the species on the target to \a species_name.  This works on planets
  * and ships, but has no effect on other objects. */
class FO_COMMON_API SetSpecies final : public EffectBase {
public:
    explicit SetSpecies(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>> m_species_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets empire \a empire_id as the owner of the target.  This has no effect if
  * \a empire_id was already the owner of the target object. */
class FO_COMMON_API SetOwner final : public EffectBase {
public:
    explicit SetOwner(std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<int>> m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a species for empire with id \a empire_id to
  * \a opinion */
class FO_COMMON_API SetSpeciesEmpireOpinion final : public EffectBase {
public:
    SetSpeciesEmpireOpinion(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
                            std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
                            std::unique_ptr<ValueRef::ValueRefBase<double>>&& opinion);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_species_name;
    std::unique_ptr<ValueRef::ValueRefBase<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the opinion of Species \a opinionated_species for other species
  * \a rated_species to \a opinion */
class FO_COMMON_API SetSpeciesSpeciesOpinion final : public EffectBase {
public:
    SetSpeciesSpeciesOpinion(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& opinionated_species_name,
                             std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& rated_species_name,
                             std::unique_ptr<ValueRef::ValueRefBase<double>>&& opinion);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_opinionated_species_name;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_rated_species_name;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_opinion;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Planet with specified \a type and \a size at the system with
  * specified \a location_id */
class FO_COMMON_API CreatePlanet final : public EffectBase {
public:
    CreatePlanet(std::unique_ptr<ValueRef::ValueRefBase<PlanetType>>&& type,
                 std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>>&& size,
                 std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                 std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<PlanetType>>     m_type;
    std::unique_ptr<ValueRef::ValueRefBase<PlanetSize>>     m_size;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_name;
    std::vector<std::unique_ptr<EffectBase>>                m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Building with specified \a type on the \a target Planet. */
class FO_COMMON_API CreateBuilding final : public EffectBase {
public:
    CreateBuilding(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& building_type_name,
                   std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                   std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_building_type_name;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_name;
    std::vector<std::unique_ptr<EffectBase>>                m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Ship with specified \a predefined_ship_design_name design
  * from those in the list of PredefinedShipDesignManager, and owned by the
  * empire with the specified \a empire_id */
class FO_COMMON_API CreateShip final : public EffectBase {
public:
    CreateShip(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& predefined_ship_design_name,
               std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
               std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
               std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& ship_name,
               std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    CreateShip(std::unique_ptr<ValueRef::ValueRefBase<int>>&& ship_design_id,
               std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id,
               std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& species_name,
               std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& ship_name,
               std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_design_name;
    std::unique_ptr<ValueRef::ValueRefBase<int>>            m_design_id;
    std::unique_ptr<ValueRef::ValueRefBase<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_species_name;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_name;
    std::vector<std::unique_ptr<EffectBase>>                m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new Field with specified \a field_type_name FieldType
  * of the specified \a size. */
class FO_COMMON_API CreateField final : public EffectBase {
public:
    CreateField(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& field_type_name,
                         std::unique_ptr<ValueRef::ValueRefBase<double>>&& size,
                         std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                         std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    CreateField(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& field_type_name,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& size,
                std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_field_type_name;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_x;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_y;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_size;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_name;
    std::vector<std::unique_ptr<EffectBase>>                m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates a new system with the specified \a colour and at the specified
  * location. */
class FO_COMMON_API CreateSystem final : public EffectBase {
public:
    CreateSystem(std::unique_ptr<ValueRef::ValueRefBase< ::StarType>>&& type,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                 std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                 std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    CreateSystem(std::unique_ptr<ValueRef::ValueRefBase<double>>&& x,
                 std::unique_ptr<ValueRef::ValueRefBase<double>>&& y,
                 std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                 std::vector<std::unique_ptr<EffectBase>>&& effects_to_apply_after);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase< ::StarType>>    m_type;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_x;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_y;
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_name;
    std::vector<std::unique_ptr<EffectBase>>                m_effects_to_apply_after;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Destroys the target object.  When executed on objects that contain other
  * objects (such as Fleets and Planets), all contained objects are destroyed
  * as well.  Destroy effects delay the desctruction of their targets until
  * after other all effects have executed, to ensure the source or target of
  * other effects are present when they execute. */
class FO_COMMON_API Destroy final : public EffectBase {
public:
    Destroy();

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override {}
    unsigned int GetCheckSum() const override;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Adds the Special with the name \a name to the target object. */
class FO_COMMON_API AddSpecial final : public EffectBase {
public:
    explicit AddSpecial(const std::string& name, float capacity = 1.0f);
    explicit AddSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name,
                        std::unique_ptr<ValueRef::ValueRefBase<double>>&& capacity = nullptr);

    void Execute(const ScriptingContext& context) const override;

    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const ValueRef::ValueRefBase<std::string>* GetSpecialName() const { return m_name.get(); }
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_capacity;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes the Special with the name \a name to the target object.  This has
  * no effect if no such Special was already attached to the target object. */
class FO_COMMON_API RemoveSpecial final : public EffectBase {
public:
    explicit RemoveSpecial(const std::string& name);
    explicit RemoveSpecial(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& name);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>> m_name;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Creates starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API AddStarlanes final : public EffectBase {
public:
    explicit AddStarlanes(std::unique_ptr<Condition::ConditionBase>&& other_lane_endpoint_condition);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<Condition::ConditionBase> m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Removes starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API RemoveStarlanes final : public EffectBase {
public:
    explicit RemoveStarlanes(std::unique_ptr<Condition::ConditionBase>&& other_lane_endpoint_condition);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<Condition::ConditionBase> m_other_lane_endpoint_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the star type of the target to \a type.  This has no effect on
  * non-System targets. */
class FO_COMMON_API SetStarType final : public EffectBase {
public:
    explicit SetStarType(std::unique_ptr<ValueRef::ValueRefBase<StarType>>&& type);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<StarType>> m_type;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location of another UniverseObject that matches
  * the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API MoveTo final : public EffectBase {
public:
    explicit MoveTo(std::unique_ptr<Condition::ConditionBase>&& location_condition);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<Condition::ConditionBase> m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject to a location as though it was moving in orbit of
  * some object or position on the map.  Sign of \a speed indicates CCW / CW
  * rotation.*/
class FO_COMMON_API MoveInOrbit final : public EffectBase {
public:
    MoveInOrbit(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                std::unique_ptr<Condition::ConditionBase>&& focal_point_condition);
    MoveInOrbit(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& focus_x = nullptr,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& focus_y = nullptr);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_speed;
    std::unique_ptr<Condition::ConditionBase>       m_focal_point_condition;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_focus_x;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_focus_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Moves an UniverseObject a specified distance towards some object or
  * position on the map. */
class FO_COMMON_API MoveTowards final : public EffectBase {
public:
    MoveTowards(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                std::unique_ptr<Condition::ConditionBase>&& dest_condition);
    MoveTowards(std::unique_ptr<ValueRef::ValueRefBase<double>>&& speed,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& dest_x = nullptr,
                std::unique_ptr<ValueRef::ValueRefBase<double>>&& dest_y = nullptr);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_speed;
    std::unique_ptr<Condition::ConditionBase>       m_dest_condition;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_dest_x;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_dest_y;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets the route of the target fleet to move to an UniverseObject that
  * matches the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API SetDestination final : public EffectBase {
public:
    explicit SetDestination(std::unique_ptr<Condition::ConditionBase>&& location_condition);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<Condition::ConditionBase> m_location_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets aggression level of the target object. */
class FO_COMMON_API SetAggression final : public EffectBase {
public:
    explicit SetAggression(bool aggressive);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override {}
    unsigned int    GetCheckSum() const override;

private:
    bool m_aggressive;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Causes the owner empire of the target object to win the game.  If the
  * target object has multiple owners, nothing is done. */
class FO_COMMON_API Victory final : public EffectBase {
public:
    explicit Victory(const std::string& reason_string); // TODO: Make this a ValueRefBase<std::string>*

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override {}
    unsigned int    GetCheckSum() const override;

private:
    std::string m_reason_string;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets whether an empire has researched at tech, and how much research
  * progress towards that tech has been completed. */
class FO_COMMON_API SetEmpireTechProgress final : public EffectBase {
public:
    SetEmpireTechProgress(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& tech_name,
                          std::unique_ptr<ValueRef::ValueRefBase<double>>&& research_progress,
                          std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id = nullptr);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_tech_name;
    std::unique_ptr<ValueRef::ValueRefBase<double>>         m_research_progress;
    std::unique_ptr<ValueRef::ValueRefBase<int>>            m_empire_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

class FO_COMMON_API GiveEmpireTech final : public EffectBase {
public:
    explicit GiveEmpireTech(std::unique_ptr<ValueRef::ValueRefBase<std::string>>&& tech_name,
                            std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id = nullptr);

    void            Execute(const ScriptingContext& context) const override;
    std::string     Dump(unsigned short ntabs = 0) const override;
    void            SetTopLevelContent(const std::string& content_name) override;
    unsigned int    GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<std::string>>    m_tech_name;
    std::unique_ptr<ValueRef::ValueRefBase<int>>            m_empire_id;

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
class FO_COMMON_API GenerateSitRepMessage final : public EffectBase {
public:
    using MessageParams =  std::vector<std::pair<
        std::string, std::unique_ptr<ValueRef::ValueRefBase<std::string>>>>;

    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          MessageParams&& message_parameters,
                          std::unique_ptr<ValueRef::ValueRefBase<int>>&& recipient_empire_id,
                          EmpireAffiliationType affiliation,
                          const std::string label = "",
                          bool stringtable_lookup = true);
    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          MessageParams&& message_parameters,
                          EmpireAffiliationType affiliation,
                          std::unique_ptr<Condition::ConditionBase>&& condition,
                          const std::string label = "",
                          bool stringtable_lookup = true);
    GenerateSitRepMessage(const std::string& message_string, const std::string& icon,
                          MessageParams&& message_parameters,
                          EmpireAffiliationType affiliation,
                          const std::string& label = "",
                          bool stringtable_lookup = true);

    void                Execute(const ScriptingContext& context) const override;
    bool                IsSitrepEffect() const override     { return true; }
    std::string         Dump(unsigned short ntabs = 0) const override;
    void                SetTopLevelContent(const std::string& content_name) override;
    const std::string&  MessageString() const               { return m_message_string; }
    const std::string&  Icon() const                        { return m_icon; }

    std::vector<std::pair<std::string, ValueRef::ValueRefBase<std::string>* >> MessageParameters() const;

    ValueRef::ValueRefBase<int>*    RecipientID() const     { return m_recipient_empire_id.get(); }
    Condition::ConditionBase*       GetCondition() const    { return m_condition.get(); }
    EmpireAffiliationType           Affiliation() const     { return m_affiliation; }
    unsigned int                    GetCheckSum() const override;

private:
    std::string             m_message_string;
    std::string             m_icon;
    std::vector<std::pair<std::string, std::unique_ptr<ValueRef::ValueRefBase<std::string>>>>
                            m_message_parameters;
    std::unique_ptr<ValueRef::ValueRefBase<int>>
                            m_recipient_empire_id;
    std::unique_ptr<Condition::ConditionBase>
                            m_condition;
    EmpireAffiliationType   m_affiliation;
    std::string             m_label;
    bool                    m_stringtable_lookup;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies an overlay texture to Systems. */
class FO_COMMON_API SetOverlayTexture final : public EffectBase {
public:
    SetOverlayTexture(const std::string& texture, std::unique_ptr<ValueRef::ValueRefBase<double>>&& size);
    SetOverlayTexture(const std::string& texture, ValueRef::ValueRefBase<double>* size);

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    bool IsAppearanceEffect() const override { return true; }
    void SetTopLevelContent(const std::string& content_name) override;
    unsigned int GetCheckSum() const override;

private:
    std::string m_texture;
    std::unique_ptr<ValueRef::ValueRefBase<double>> m_size;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Applies a texture to Planets. */
class FO_COMMON_API SetTexture final : public EffectBase {
public:
    explicit SetTexture(const std::string& texture);

    void Execute(const ScriptingContext& context) const override;

    std::string Dump(unsigned short ntabs = 0) const override;
    bool IsAppearanceEffect() const override { return true; }
    void SetTopLevelContent(const std::string& content_name) override {}
    unsigned int GetCheckSum() const override;

private:
    std::string m_texture;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Sets visibility of an object for an empire, independent of standard
  * visibility mechanics. */
class FO_COMMON_API SetVisibility final : public EffectBase {
public:
    SetVisibility(std::unique_ptr<ValueRef::ValueRefBase<Visibility>> vis,
                  EmpireAffiliationType affiliation,
                  std::unique_ptr<ValueRef::ValueRefBase<int>>&& empire_id = nullptr,
                  std::unique_ptr<Condition::ConditionBase>&& of_objects = nullptr);    // if not specified, acts on target. if specified, acts on all matching objects

    void Execute(const ScriptingContext& context) const override;
    std::string Dump(unsigned short ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    ValueRef::ValueRefBase<Visibility>* GetVisibility() const
    { return m_vis.get(); }

    ValueRef::ValueRefBase<int>* EmpireID() const
    { return m_empire_id.get(); }

    EmpireAffiliationType Affiliation() const
    { return m_affiliation; }

    Condition::ConditionBase* OfObjectsCondition() const
    { return m_condition.get(); }

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<ValueRef::ValueRefBase<Visibility>> m_vis;
    std::unique_ptr<ValueRef::ValueRefBase<int>> m_empire_id;
    EmpireAffiliationType m_affiliation;
    std::unique_ptr<Condition::ConditionBase> m_condition;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** Executes a set of effects if an execution-time condition is met, or an
  * alterative set of effects if the condition is not met. */
class FO_COMMON_API Conditional final : public EffectBase {
public:
    Conditional(std::unique_ptr<Condition::ConditionBase>&& target_condition,
                std::vector<std::unique_ptr<EffectBase>>&& true_effects,
                std::vector<std::unique_ptr<EffectBase>>&& false_effects);

    void Execute(const ScriptingContext& context) const override;
    /** Note: executes all of the true or all of the false effects on each
        target, without considering any of the only_* type flags. */

    void Execute(const ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(const ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    std::string Dump(unsigned short ntabs = 0) const override;

    bool IsMeterEffect() const override;
    bool IsAppearanceEffect() const override;
    bool IsSitrepEffect() const override;
    bool IsConditionalEffect() const override { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    unsigned int GetCheckSum() const override;

private:
    std::unique_ptr<Condition::ConditionBase> m_target_condition; // condition to apply to each target object to determine which effects to execute
    std::vector<std::unique_ptr<EffectBase>> m_true_effects;      // effects to execute if m_target_condition matches target object
    std::vector<std::unique_ptr<EffectBase>> m_false_effects;     // effects to execute if m_target_condition does not match target object

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


// template implementations
template <class Archive>
void EffectsGroup::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_NVP(m_scope)
        & BOOST_SERIALIZATION_NVP(m_activation)
        & BOOST_SERIALIZATION_NVP(m_stacking_group)
        & BOOST_SERIALIZATION_NVP(m_effects)
        & BOOST_SERIALIZATION_NVP(m_description)
        & BOOST_SERIALIZATION_NVP(m_content_name);
}

template <class Archive>
void EffectBase::serialize(Archive& ar, const unsigned int version)
{}

template <class Archive>
void NoOp::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase);
}

template <class Archive>
void SetMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value)
        & BOOST_SERIALIZATION_NVP(m_accounting_label);
}

template <class Archive>
void SetShipPartMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_part_name)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireMeter::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_meter)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireStockpile::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_stockpile)
        & BOOST_SERIALIZATION_NVP(m_value);
}

template <class Archive>
void SetEmpireCapital::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void SetPlanetType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void SetPlanetSize::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void SetSpecies::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_species_name);
}

template <class Archive>
void SetOwner::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void CreatePlanet::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateBuilding::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_building_type_name)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateShip::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_design_name)
        & BOOST_SERIALIZATION_NVP(m_design_id)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_species_name)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateField::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_field_type_name)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_size)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void CreateSystem::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type)
        & BOOST_SERIALIZATION_NVP(m_x)
        & BOOST_SERIALIZATION_NVP(m_y)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_effects_to_apply_after);
}

template <class Archive>
void Destroy::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase);
}

template <class Archive>
void AddSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name)
        & BOOST_SERIALIZATION_NVP(m_capacity);
}

template <class Archive>
void RemoveSpecial::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_name);
}

template <class Archive>
void AddStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void RemoveStarlanes::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_other_lane_endpoint_condition);
}

template <class Archive>
void SetStarType::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_type);
}

template <class Archive>
void MoveTo::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void MoveInOrbit::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_focal_point_condition)
        & BOOST_SERIALIZATION_NVP(m_focus_x)
        & BOOST_SERIALIZATION_NVP(m_focus_y);
}

template <class Archive>
void MoveTowards::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_speed)
        & BOOST_SERIALIZATION_NVP(m_dest_condition)
        & BOOST_SERIALIZATION_NVP(m_dest_x)
        & BOOST_SERIALIZATION_NVP(m_dest_y);
}

template <class Archive>
void SetDestination::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_location_condition);
}

template <class Archive>
void SetAggression::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_aggressive);
}

template <class Archive>
void Victory::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_reason_string);
}

template <class Archive>
void SetEmpireTechProgress::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_research_progress)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void GiveEmpireTech::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_tech_name)
        & BOOST_SERIALIZATION_NVP(m_empire_id);
}

template <class Archive>
void GenerateSitRepMessage::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_message_string)
        & BOOST_SERIALIZATION_NVP(m_icon)
        & BOOST_SERIALIZATION_NVP(m_message_parameters)
        & BOOST_SERIALIZATION_NVP(m_recipient_empire_id)
        & BOOST_SERIALIZATION_NVP(m_condition)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_label)
        & BOOST_SERIALIZATION_NVP(m_stringtable_lookup);
}

template <class Archive>
void SetOverlayTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture)
        & BOOST_SERIALIZATION_NVP(m_size);
}

template <class Archive>
void SetTexture::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_texture);
}

template <class Archive>
void SetVisibility::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_vis)
        & BOOST_SERIALIZATION_NVP(m_empire_id)
        & BOOST_SERIALIZATION_NVP(m_affiliation)
        & BOOST_SERIALIZATION_NVP(m_condition);
}

template <class Archive>
void Conditional::serialize(Archive& ar, const unsigned int version)
{
    ar  & BOOST_SERIALIZATION_BASE_OBJECT_NVP(EffectBase)
        & BOOST_SERIALIZATION_NVP(m_target_condition)
        & BOOST_SERIALIZATION_NVP(m_true_effects)
        & BOOST_SERIALIZATION_NVP(m_false_effects);
}
} // namespace Effect

#endif // _Effect_h_
