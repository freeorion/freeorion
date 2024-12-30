#ifndef _Effects_h_
#define _Effects_h_


#include <boost/optional/optional.hpp>
#include "Effect.h"
#include "../util/Export.h"


namespace Condition {
    using ObjectSet = std::vector<const UniverseObject*>;
}

namespace ValueRef {
    template <typename T>
    struct ValueRef;
}

namespace Effect {
/** Does nothing when executed. Useful for triggering side-effects of effect
  * execution without modifying the gamestate. */
class FO_COMMON_API NoOp final : public Effect {
public:
    NoOp() = default;

    void                       Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;
    void                       SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t     GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;
};

/** Sets the meter of the given kind to \a value.  The max value of the meter
  * is set if \a max == true; otherwise the current value of the meter is set.
  * If the target of the Effect does not have the requested meter, nothing is
  * done. */
class FO_COMMON_API SetMeter final : public Effect {
public:
    SetMeter(MeterType meter,
             std::unique_ptr<ValueRef::ValueRef<double>>&& value,
             boost::optional<std::string> accounting_label = boost::none);
    SetMeter(MeterType meter,
             std::unique_ptr<ValueRef::ValueRef<double>>&& value,
             const char* accounting_label) :
        SetMeter(meter, std::move(value), std::string{accounting_label})
    {}

    [[nodiscard]] bool operator==(const Effect& rhs) const override;

    void Execute(ScriptingContext& context) const override;

    void Execute(ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    [[nodiscard]] std::string        Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool               IsMeterEffect() const noexcept override { return true; }
    void                             SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] MeterType          GetMeterType() const noexcept { return m_meter; };
    [[nodiscard]] const std::string& AccountingLabel() const noexcept { return m_accounting_label; }
    [[nodiscard]] uint32_t           GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    MeterType m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value;
    std::string m_accounting_label;
};

/** Sets the indicated meter on all ship parts in the indicated subset.  This
  * has no effect on non-Ship targets.  If slot_type is specified, only parts
  * that can mount in the indicated slot type (internal or external) are
  * affected (this is not the same at the slot type in which the part is
  * actually located, as a part might be mountable in both types, and
  * located in a different type than specified, and would be matched). */
class FO_COMMON_API SetShipPartMeter final : public Effect {
public:
    /** Affects the \a meter_type meter that belongs to part(s) named \a
        part_name. */
    SetShipPartMeter(MeterType meter_type,
                     std::unique_ptr<ValueRef::ValueRef<std::string>>&& part_name,
                     std::unique_ptr<ValueRef::ValueRef<double>>&& value);

    [[nodiscard]] bool operator==(const Effect& rhs) const override;

    void Execute(ScriptingContext& context) const override;
    void Execute(ScriptingContext& context, const TargetSet& targets) const override;
    void Execute(ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool         IsMeterEffect() const noexcept override { return true; }
    void                       SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] const ValueRef::ValueRef<std::string>* GetPartName() const noexcept { return m_part_name.get(); }
    [[nodiscard]] MeterType    GetMeterType() const noexcept { return m_meter; }
    [[nodiscard]] uint32_t     GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_part_name;
    MeterType                                           m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_value;
};

/** Sets the indicated meter on the empire with the indicated id to the
  * indicated value.  If \a meter is not a valid meter for empires,
  * does nothing. */
class FO_COMMON_API SetEmpireMeter final : public Effect {
public:
    SetEmpireMeter(std::string& meter, std::unique_ptr<ValueRef::ValueRef<double>>&& value);

    SetEmpireMeter(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id, std::string& meter,
                   std::unique_ptr<ValueRef::ValueRef<double>>&& value);

    [[nodiscard]] bool operator==(const Effect& rhs) const override;

    void Execute(ScriptingContext& context) const override;
    void Execute(ScriptingContext& context, const TargetSet& targets) const override;
    void Execute(ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool         IsMeterEffect() const noexcept override { return true; }
    [[nodiscard]] bool         IsEmpireMeterEffect() const noexcept override { return true; }
    void                       SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t     GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>>    m_empire_id;
    std::string                                 m_meter;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value;
};

/** Sets the empire stockpile of the target's owning empire to \a value.  If
  * the target does not have exactly one owner, nothing is done. */
class FO_COMMON_API SetEmpireStockpile final : public Effect {
public:
    SetEmpireStockpile(ResourceType stockpile,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& value);
    SetEmpireStockpile(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                       ResourceType stockpile,
                       std::unique_ptr<ValueRef::ValueRef<double>>&& value);

    [[nodiscard]] bool operator==(const Effect& rhs) const override;

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>>    m_empire_id;
    ResourceType                                m_stockpile;
    std::unique_ptr<ValueRef::ValueRef<double>> m_value;
};

/** Makes the target planet the capital of its owner's empire.  If the target
  * object is not a planet, does not have an owner, or has more than one owner
  * the effect does nothing. */
class FO_COMMON_API SetEmpireCapital final : public Effect {
public:
    SetEmpireCapital();
    explicit SetEmpireCapital(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    [[nodiscard]] bool operator==(const Effect& rhs) const override;

    void        Execute(ScriptingContext& context) const override;
    std::string Dump(uint8_t ntabs = 0) const override;
    void        SetTopLevelContent(const std::string& content_name) override;
    uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Sets the planet type of the target to \a type.  This has no effect on non-Planet targets.  Note that changing the
    type of a PlanetType::PT_ASTEROID or PlanetType::PT_GASGIANT planet will also change its size to PlanetSize::SZ_TINY or PlanetSize::SZ_HUGE, respectively.
    Similarly, changing type to PlanetType::PT_ASTEROID or PlanetType::PT_GASGIANT will also cause the size to change to PlanetSize::SZ_ASTEROID or
    PlanetSize::SZ_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetType final : public Effect {
public:
    explicit SetPlanetType(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<PlanetType>> m_type;
};

/** Sets the original planet type of the target to \a type.  This has no effect on non-Planet targets.
    This does not change the planet itself, it only affects game effects that compare the current type
    to the original type. Typically effects that trigger during universe creation will set both
    current and original type, while later effect will only modify the current type. */
class FO_COMMON_API SetOriginalType final : public Effect {
public:
    explicit SetOriginalType(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<PlanetType>> m_type;
};

/** Sets the planet size of the target to \a size.  This has no effect on non-
  * Planet targets.  Note that changing the size of a PlanetType::PT_ASTEROID or PlanetType::PT_GASGIANT
  * planet will also change its type to PlanetType::PT_BARREN.  Similarly, changing size to
  * PlanetSize::SZ_ASTEROID or PlanetSize::SZ_GASGIANT will also cause the type to change to PlanetType::PT_ASTEROID
  * or PlanetType::PT_GASGIANT, respectively. */
class FO_COMMON_API SetPlanetSize final : public Effect {
public:
    explicit SetPlanetSize(std::unique_ptr<ValueRef::ValueRef<PlanetSize>>&& size);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<PlanetSize>> m_size;
};

/** Sets the species on the target to \a species_name.  This works on planets
  * and ships, but has no effect on other objects. */
class FO_COMMON_API SetFocus final : public Effect {
public:
    explicit SetFocus(std::unique_ptr<ValueRef::ValueRef<std::string>>&& focus);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_focus_name;
};

/** Sets the species on the target to \a species_name.  This works on planets
  * and ships, but has no effect on other objects. */
class FO_COMMON_API SetSpecies final : public Effect {
public:
    explicit SetSpecies(std::unique_ptr<ValueRef::ValueRef<std::string>>&& species);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_species_name;
};

/** Sets empire \a empire_id as the owner of the target.  This has no effect if
  * \a empire_id was already the owner of the target object. */
class FO_COMMON_API SetOwner final : public Effect {
public:
    explicit SetOwner(std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
};

/** Sets the opinion of Species \a species for empire with id \a empire_id to
  * \a opinion */
class FO_COMMON_API SetSpeciesEmpireOpinion final : public Effect {
public:
    SetSpeciesEmpireOpinion(std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
                            std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
                            std::unique_ptr<ValueRef::ValueRef<double>>&& opinion,
                            bool target_opinion);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool IsMeterEffect() const noexcept override { return true; }
    [[nodiscard]] bool IsEmpireMeterEffect() const noexcept override { return true; }
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_species_name;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_opinion;
    bool                                                m_target;
};

/** Sets the opinion of Species \a opinionated_species for other species
  * \a rated_species to \a opinion */
class FO_COMMON_API SetSpeciesSpeciesOpinion final : public Effect {
public:
    SetSpeciesSpeciesOpinion(std::unique_ptr<ValueRef::ValueRef<std::string>>&& opinionated_species_name,
                             std::unique_ptr<ValueRef::ValueRef<std::string>>&& rated_species_name,
                             std::unique_ptr<ValueRef::ValueRef<double>>&& opinion,
                             bool target_opinion);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool IsMeterEffect() const noexcept override { return true; }
    [[nodiscard]] bool IsEmpireMeterEffect() const noexcept override { return true; }
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_opinionated_species_name;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_rated_species_name;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_opinion;
    bool                                                m_target;
};

/** Creates a new Planet with specified \a type and \a size at the system with
  * specified \a location_id */
class FO_COMMON_API CreatePlanet final : public Effect {
public:
    CreatePlanet(std::unique_ptr<ValueRef::ValueRef<PlanetType>>&& type,
                 std::unique_ptr<ValueRef::ValueRef<PlanetSize>>&& size,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                 std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<PlanetType>>     m_type;
    std::unique_ptr<ValueRef::ValueRef<PlanetSize>>     m_size;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::vector<std::unique_ptr<Effect>>                m_effects_to_apply_after;
};

/** Creates a new Building with specified \a type on the \a target Planet. */
class FO_COMMON_API CreateBuilding final : public Effect {
public:
    CreateBuilding(std::unique_ptr<ValueRef::ValueRef<std::string>>&& building_type_name,
                   std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                   std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_building_type_name;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::vector<std::unique_ptr<Effect>>                m_effects_to_apply_after;
};

/** Creates a new Ship with specified \a predefined_ship_design_name design
  * from those in the list of PredefinedShipDesignManager, and owned by the
  * empire with the specified \a empire_id */
class FO_COMMON_API CreateShip final : public Effect {
public:
    CreateShip(std::unique_ptr<ValueRef::ValueRef<std::string>>&& predefined_ship_design_name,
               std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
               std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
               std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_name,
               std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    CreateShip(std::unique_ptr<ValueRef::ValueRef<int>>&& ship_design_id,
               std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id,
               std::unique_ptr<ValueRef::ValueRef<std::string>>&& species_name,
               std::unique_ptr<ValueRef::ValueRef<std::string>>&& ship_name,
               std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_design_name;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_design_id;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_species_name;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::vector<std::unique_ptr<Effect>>                m_effects_to_apply_after;
};

/** Creates a new Field with specified \a field_type_name FieldType
  * of the specified \a size. */
class FO_COMMON_API CreateField final : public Effect {
public:
    CreateField(std::unique_ptr<ValueRef::ValueRef<std::string>>&& field_type_name,
                std::unique_ptr<ValueRef::ValueRef<double>>&& size,
                std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    CreateField(std::unique_ptr<ValueRef::ValueRef<std::string>>&& field_type_name,
                std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                std::unique_ptr<ValueRef::ValueRef<double>>&& size,
                std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_field_type_name;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_x;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_y;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_size;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::vector<std::unique_ptr<Effect>>                m_effects_to_apply_after;
};

/** Creates a new system with the specified \a colour and at the specified
  * location. */
class FO_COMMON_API CreateSystem final : public Effect {
public:
    CreateSystem(std::unique_ptr<ValueRef::ValueRef< ::StarType>>&& type,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                 std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    CreateSystem(std::unique_ptr<ValueRef::ValueRef<double>>&& x,
                 std::unique_ptr<ValueRef::ValueRef<double>>&& y,
                 std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                 std::vector<std::unique_ptr<Effect>>&& effects_to_apply_after);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef< ::StarType>>    m_type;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_x;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_y;
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_name;
    std::vector<std::unique_ptr<Effect>>                m_effects_to_apply_after;
};

/** Destroys the target object.  When executed on objects that contain other
  * objects (such as Fleets and Planets), all contained objects are destroyed
  * as well.  Destroy effects delay the desctruction of their targets until
  * after other all effects have executed, to ensure the source or target of
  * other effects are present when they execute. */
class FO_COMMON_API Destroy final : public Effect {
public:
    Destroy() = default;

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;
};

/** Adds the Special with the name \a name to the target object. */
class FO_COMMON_API AddSpecial final : public Effect {
public:
    explicit AddSpecial(std::string& name, float capacity = 1.0f);
    explicit AddSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name,
                        std::unique_ptr<ValueRef::ValueRef<double>>&& capacity = nullptr);

    void Execute(ScriptingContext& context) const override;

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    const ValueRef::ValueRef<std::string>* GetSpecialName() const { return m_name.get(); }
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
    std::unique_ptr<ValueRef::ValueRef<double>> m_capacity;
};

/** Removes the Special with the name \a name to the target object.  This has
  * no effect if no such Special was already attached to the target object. */
class FO_COMMON_API RemoveSpecial final : public Effect {
public:
    explicit RemoveSpecial(std::string& name);
    explicit RemoveSpecial(std::unique_ptr<ValueRef::ValueRef<std::string>>&& name);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_name;
};

/** Creates starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API AddStarlanes final : public Effect {
public:
    explicit AddStarlanes(std::unique_ptr<Condition::Condition>&& other_lane_endpoint_condition);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<Condition::Condition> m_other_lane_endpoint_condition;
};

/** Removes starlane(s) between the target system and systems that match
  * \a other_lane_endpoint_condition */
class FO_COMMON_API RemoveStarlanes final : public Effect {
public:
    explicit RemoveStarlanes(std::unique_ptr<Condition::Condition>&& other_lane_endpoint_condition);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<Condition::Condition> m_other_lane_endpoint_condition;
};

/** Sets the star type of the target to \a type.  This has no effect on
  * non-System targets. */
class FO_COMMON_API SetStarType final : public Effect {
public:
    explicit SetStarType(std::unique_ptr<ValueRef::ValueRef<StarType>>&& type);

    void                       Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string  Dump(uint8_t ntabs = 0) const override;
    void                       SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t     GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<StarType>> m_type;
};

/** Moves an UniverseObject to a location of another UniverseObject that matches
  * the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API MoveTo final : public Effect {
public:
    explicit MoveTo(std::unique_ptr<Condition::Condition>&& location_condition);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<Condition::Condition> m_location_condition;
};

/** Moves an UniverseObject to a location as though it was moving in orbit of
  * some object or position on the map.  Sign of \a speed indicates CCW / CW
  * rotation.*/
class FO_COMMON_API MoveInOrbit final : public Effect {
public:
    MoveInOrbit(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                std::unique_ptr<Condition::Condition>&& focal_point_condition);
    MoveInOrbit(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                std::unique_ptr<ValueRef::ValueRef<double>>&& focus_x = nullptr,
                std::unique_ptr<ValueRef::ValueRef<double>>&& focus_y = nullptr);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<double>> m_speed;
    std::unique_ptr<Condition::Condition>       m_focal_point_condition;
    std::unique_ptr<ValueRef::ValueRef<double>> m_focus_x;
    std::unique_ptr<ValueRef::ValueRef<double>> m_focus_y;
};

/** Moves an UniverseObject a specified distance towards some object or
  * position on the map. */
class FO_COMMON_API MoveTowards final : public Effect {
public:
    MoveTowards(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                std::unique_ptr<Condition::Condition>&& dest_condition);
    MoveTowards(std::unique_ptr<ValueRef::ValueRef<double>>&& speed,
                std::unique_ptr<ValueRef::ValueRef<double>>&& dest_x = nullptr,
                std::unique_ptr<ValueRef::ValueRef<double>>&& dest_y = nullptr);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<double>> m_speed;
    std::unique_ptr<Condition::Condition>       m_dest_condition;
    std::unique_ptr<ValueRef::ValueRef<double>> m_dest_x;
    std::unique_ptr<ValueRef::ValueRef<double>> m_dest_y;
};

/** Sets the route of the target fleet to move to an UniverseObject that
  * matches the condition \a location_condition.  If multiple objects match the
  * condition, then one is chosen.  If no objects match the condition, then
  * nothing is done. */
class FO_COMMON_API SetDestination final : public Effect {
public:
    explicit SetDestination(std::unique_ptr<Condition::Condition>&& location_condition);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<Condition::Condition> m_location_condition;
};

/** Sets aggression level of the target object. */
class FO_COMMON_API SetAggression final : public Effect {
public:
    explicit SetAggression(FleetAggression aggression);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    FleetAggression m_aggression;
};

/** Causes the owner empire of the target object to win the game.  If the
  * target object has multiple owners, nothing is done. */
class FO_COMMON_API Victory final : public Effect {
public:
    explicit Victory(std::string reason_string); // TODO: Make this a ValueRef<std::string>*

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::string m_reason_string;
};

/** Sets whether an empire has researched at tech, and how much research
  * progress towards that tech has been completed. */
class FO_COMMON_API SetEmpireTechProgress final : public Effect {
public:
    SetEmpireTechProgress(std::unique_ptr<ValueRef::ValueRef<std::string>>&& tech_name,
                          std::unique_ptr<ValueRef::ValueRef<double>>&& research_progress,
                          std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>>    m_tech_name;
    std::unique_ptr<ValueRef::ValueRef<double>>         m_research_progress;
    std::unique_ptr<ValueRef::ValueRef<int>>            m_empire_id;
};

class FO_COMMON_API GiveEmpireContent final : public Effect {
public:
    explicit GiveEmpireContent(std::unique_ptr<ValueRef::ValueRef<std::string>>&& tech_name,
                               UnlockableItemType unlock_type,
                               std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t    GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<std::string>> m_content_name;
    UnlockableItemType                               m_unlock_type;
    std::unique_ptr<ValueRef::ValueRef<int>>         m_empire_id;
};

/** Generates a sitrep message for the empire with id \a recipient_empire_id.
  * The message text is the user string specified in \a message_string with
  * string substitutions into the message text as specified in \a message_parameters
  * which are substituted as string parameters %1%, %2%, %3%, etc. in the order
  * they are specified.  Extra parameters beyond those needed by \a message_string
  * are ignored, and missing parameters are left as blank text. */
class FO_COMMON_API GenerateSitRepMessage final : public Effect {
public:
    using MessageParams =  std::vector<std::pair<
        std::string, std::unique_ptr<ValueRef::ValueRef<std::string>>>>;

    GenerateSitRepMessage(std::string message_string, std::string icon,
                          MessageParams&& message_parameters,
                          std::unique_ptr<ValueRef::ValueRef<int>>&& recipient_empire_id,
                          EmpireAffiliationType affiliation,
                          std::string label = "",
                          bool stringtable_lookup = true);
    GenerateSitRepMessage(std::string message_string, std::string icon,
                          MessageParams&& message_parameters,
                          EmpireAffiliationType affiliation,
                          std::unique_ptr<Condition::Condition>&& condition,
                          std::string label = "",
                          bool stringtable_lookup = true);
    GenerateSitRepMessage(std::string message_string, std::string icon,
                          MessageParams&& message_parameters,
                          EmpireAffiliationType affiliation,
                          std::string label = "",
                          bool stringtable_lookup = true);

    void                      Execute(ScriptingContext& context) const override;
    [[nodiscard]] bool        IsSitrepEffect() const noexcept override { return true; }
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void                      SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] const std::string& MessageString() const noexcept { return m_message_string; }
    [[nodiscard]] const std::string& Icon() const noexcept          { return m_icon; }

    std::vector<std::pair<std::string, const ValueRef::ValueRef<std::string>*>> MessageParameters() const;

    [[nodiscard]] ValueRef::ValueRef<int>* RecipientID() const noexcept  { return m_recipient_empire_id.get(); }
    [[nodiscard]] Condition::Condition*    GetCondition() const noexcept { return m_condition.get(); }
    [[nodiscard]] EmpireAffiliationType    Affiliation() const noexcept  { return m_affiliation; }
    [[nodiscard]] uint32_t                 GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::string                              m_message_string;
    std::string                              m_icon;
    std::vector<std::pair<std::string, std::unique_ptr<ValueRef::ValueRef<std::string>>>>
                                             m_message_parameters;
    std::unique_ptr<ValueRef::ValueRef<int>> m_recipient_empire_id;
    std::unique_ptr<Condition::Condition>    m_condition;
    EmpireAffiliationType                    m_affiliation;
    std::string                              m_label;
    bool                                     m_stringtable_lookup;
};

/** Applies an overlay texture to Systems. */
class FO_COMMON_API SetOverlayTexture final : public Effect {
public:
    SetOverlayTexture(std::string& texture, std::unique_ptr<ValueRef::ValueRef<double>>&& size);
    SetOverlayTexture(std::string& texture, ValueRef::ValueRef<double>* size);

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool IsAppearanceEffect() const noexcept override { return true; }
    void SetTopLevelContent(const std::string& content_name) override;
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::string m_texture;
    std::unique_ptr<ValueRef::ValueRef<double>> m_size;
};

/** Applies a texture to Planets. */
class FO_COMMON_API SetTexture final : public Effect {
public:
    explicit SetTexture(auto&& texture) :
        m_texture(std::forward<decltype(texture)>(texture))
    {}

    void Execute(ScriptingContext& context) const override;

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    [[nodiscard]] bool IsAppearanceEffect() const noexcept override { return true; }
    void SetTopLevelContent(const std::string& content_name) override {}
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::string m_texture;
};

/** Sets visibility of an object for an empire, independent of standard
  * visibility mechanics. */
class FO_COMMON_API SetVisibility final : public Effect {
public:
    SetVisibility(std::unique_ptr<ValueRef::ValueRef<Visibility>> vis,
                  EmpireAffiliationType affiliation,
                  std::unique_ptr<ValueRef::ValueRef<int>>&& empire_id = nullptr,
                  std::unique_ptr<Condition::Condition>&& of_objects = nullptr);    // if not specified, acts on target. if specified, acts on all matching objects

    void Execute(ScriptingContext& context) const override;
    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;
    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] ValueRef::ValueRef<Visibility>* GetVisibility() const noexcept { return m_vis.get(); }
    [[nodiscard]] ValueRef::ValueRef<int>* EmpireID() const noexcept { return m_empire_id.get(); }
    [[nodiscard]] EmpireAffiliationType Affiliation() const noexcept { return m_affiliation; }
    [[nodiscard]] Condition::Condition* OfObjectsCondition() const noexcept  { return m_condition.get(); }
    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<ValueRef::ValueRef<Visibility>> m_vis;
    std::unique_ptr<ValueRef::ValueRef<int>> m_empire_id;
    EmpireAffiliationType m_affiliation;
    std::unique_ptr<Condition::Condition> m_condition;
};

/** Executes a set of effects if an execution-time condition is met, or an
  * alterative set of effects if the condition is not met. */
class FO_COMMON_API Conditional final : public Effect {
public:
    Conditional(std::unique_ptr<Condition::Condition>&& target_condition,
                std::vector<std::unique_ptr<Effect>>&& true_effects,
                std::vector<std::unique_ptr<Effect>>&& false_effects);

    void Execute(ScriptingContext& context) const override;
    /** Note: executes all of the true or all of the false effects on each
        target, without considering any of the only_* type flags. */

    void Execute(ScriptingContext& context, const TargetSet& targets) const override;

    void Execute(ScriptingContext& context,
                 const TargetSet& targets,
                 AccountingMap* accounting_map,
                 const EffectCause& effect_cause,
                 bool only_meter_effects = false,
                 bool only_appearance_effects = false,
                 bool include_empire_meter_effects = false,
                 bool only_generate_sitrep_effects = false) const override;

    [[nodiscard]] std::string Dump(uint8_t ntabs = 0) const override;

    [[nodiscard]] bool IsMeterEffect() const noexcept override;
    [[nodiscard]] bool IsAppearanceEffect() const noexcept override;
    [[nodiscard]] bool IsSitrepEffect() const noexcept override;
    [[nodiscard]] bool IsConditionalEffect() const noexcept override { return true; }

    void SetTopLevelContent(const std::string& content_name) override;

    [[nodiscard]] uint32_t GetCheckSum() const override;

    [[nodiscard]] std::unique_ptr<Effect> Clone() const override;

private:
    std::unique_ptr<Condition::Condition> m_target_condition; // condition to apply to each target object to determine which effects to execute
    std::vector<std::unique_ptr<Effect>> m_true_effects;      // effects to execute if m_target_condition matches target object
    std::vector<std::unique_ptr<Effect>> m_false_effects;     // effects to execute if m_target_condition does not match target object
};
}


#endif
