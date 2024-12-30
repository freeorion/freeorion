#ifndef _Effect_h_
#define _Effect_h_


#include <compare>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/container/flat_map.hpp>
#include "ConstantsFwd.h"
#include "Condition.h"
#include "EnumsFwd.h"
#include "../util/Enum.h"
#include "../util/Export.h"


class UniverseObject;
struct ScriptingContext;

//! Types of in-game things that might contain an EffectsGroup, or "cause"
//! effects to occur
FO_ENUM(
    (EffectsCauseType),
    ((INVALID_EFFECTS_GROUP_CAUSE_TYPE, -1))
    ((ECT_UNKNOWN_CAUSE))
    ((ECT_INHERENT))
    ((ECT_TECH))
    ((ECT_BUILDING))
    ((ECT_FIELD))
    ((ECT_SPECIAL))
    ((ECT_SPECIES))
    ((ECT_SHIP_PART))
    ((ECT_SHIP_HULL))
    ((ECT_POLICY))
)

#if !defined(CONSTEXPR_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTEXPR_STRING constexpr
#  else
#    define CONSTEXPR_STRING
#  endif
#endif

namespace Effect {
    struct AccountingInfo;
    class EffectsGroup;

    using TargetSet = std::vector<UniverseObject*>;
    /** Effect accounting information for all meters of all objects that are
      * acted on by effects. */
    using AccountingMap = std::unordered_map<int, boost::container::flat_map<MeterType, std::vector<AccountingInfo>>>;

    /** Description of cause of an effect: the general cause type, and the
      * specific cause.  eg. Building and a particular BuildingType. */
    struct FO_COMMON_API EffectCause {
        CONSTEXPR_STRING EffectCause() = default;

        CONSTEXPR_STRING explicit EffectCause(EffectsCauseType cause_type_) noexcept(noexcept(std::string{})) :
            cause_type(cause_type_)
        {}

        template <typename S1, typename S2>
        EffectCause(EffectsCauseType cause_type_, S1&& specific_cause_, S2&& custom_label_ = "") 
            noexcept(noexcept(std::string{std::declval<S1>()}) && noexcept(std::string{std::declval<S2>()})) :
            cause_type(cause_type_),
            specific_cause(std::forward<S1>(specific_cause_)),
            custom_label(std::forward<S2>(custom_label_))
        {}

        EffectsCauseType cause_type = EffectsCauseType::INVALID_EFFECTS_GROUP_CAUSE_TYPE;  ///< general type of effect cause, eg. tech, building, special...
        std::string      specific_cause; ///< name of specific cause, eg. "Wonder Farm", "Antenna Mk. VI"
        std::string      custom_label;   ///< script-specified accounting label for this effect cause
    };

    /** Combination of targets and cause for an effects group. */
    struct TargetsAndCause {
        TargetsAndCause() = default;
        template <typename S1, typename S2>
        TargetsAndCause(EffectsCauseType ect, S1&& specific_cause, S2&& custom_label) :
            effect_cause(ect, std::forward<S1>(specific_cause), std::forward<S2>(custom_label))
        {}

        TargetSet target_set;
        EffectCause effect_cause;
    };

    /** Combination of an EffectsGroup and the id of a source object. */
    struct SourcedEffectsGroup {
        constexpr SourcedEffectsGroup() = default;
        constexpr SourcedEffectsGroup(int source_object_id_, const EffectsGroup* effects_group_) noexcept :
            source_object_id(source_object_id_),
            effects_group(effects_group_)
        {}
        constexpr auto operator<=>(const SourcedEffectsGroup&) const noexcept = default;
        int source_object_id = INVALID_OBJECT_ID;
        const EffectsGroup* effects_group = nullptr;
    };

    /** Map from (effects group and source object) to target set of for
      * that effects group with that source object.  A multimap is used
      * so that a single source object can have multiple instances of the
      * same effectsgroup.  This is useful when a Ship has multiple copies
      * of the same effects group due to having multiple copies of the same
      * ship part in its design. */
    typedef std::vector<std::pair<SourcedEffectsGroup, TargetsAndCause>> SourcesEffectsTargetsAndCausesVec;

    /** The base class for all Effects.  When an Effect is executed, the source
    * object (the object to which the Effect or its containing EffectGroup is
    * attached) and the target object are both required.  Note that this means
    * that ValueRefs contained within Effects can refer to values in either the
    * source or target objects. */
    class FO_COMMON_API Effect {
    public:
        virtual ~Effect() = default;

        virtual void Execute(ScriptingContext& context) const = 0;

        virtual void Execute(ScriptingContext& context, const TargetSet& targets) const;

        virtual void Execute(ScriptingContext& context,
                             const TargetSet& targets,
                             AccountingMap* accounting_map,
                             const EffectCause& effect_cause,
                             bool only_meter_effects = false,
                             bool only_appearance_effects = false,
                             bool include_empire_meter_effects = false,
                             bool only_generate_sitrep_effects = false) const;

        [[nodiscard]] virtual bool operator==(const Effect& rhs) const;

        [[nodiscard]] virtual std::string Dump(uint8_t ntabs = 0) const = 0;

        [[nodiscard]] virtual bool IsMeterEffect() const noexcept { return false; }
        [[nodiscard]] virtual bool IsEmpireMeterEffect() const noexcept { return false; }
        [[nodiscard]] virtual bool IsAppearanceEffect() const noexcept { return false; }
        [[nodiscard]] virtual bool IsSitrepEffect() const noexcept { return false; }
        [[nodiscard]] virtual bool IsConditionalEffect() const noexcept { return false; }

        // TODO: source-invariant?

        virtual void SetTopLevelContent(const std::string& content_name) = 0;
        [[nodiscard]] virtual uint32_t GetCheckSum() const;

        //! Makes a clone of this Effect in a new owning pointer. Required for Boost.Python, which
        //! doesn't supports move semantics for returned values.
        [[nodiscard]] virtual std::unique_ptr<Effect> Clone() const = 0;
    };

    /** Accounting information about what the causes are and changes produced
      * by effects groups acting on meters of objects. */
    struct FO_COMMON_API AccountingInfo : public EffectCause {
        AccountingInfo() = default;

        AccountingInfo(float meter_change_, float running_meter_total_)
            noexcept(noexcept(EffectCause{EffectsCauseType::ECT_UNKNOWN_CAUSE})) :
            EffectCause(EffectsCauseType::ECT_UNKNOWN_CAUSE),
            source_id(INVALID_OBJECT_ID),
            meter_change(meter_change_),
            running_meter_total(running_meter_total_)
        {}

        AccountingInfo(int source_id_, EffectsCauseType cause_type_, float meter_change_,
                       float running_meter_total_)
            noexcept(noexcept(EffectCause{std::declval<EffectsCauseType>()})) :
            EffectCause(cause_type_),
            source_id(source_id_),
            meter_change(meter_change_),
            running_meter_total(running_meter_total_)
        {}

        template <typename S1, typename S2 = const char*>
        AccountingInfo(int source_id_, EffectsCauseType cause_type_, float meter_change_,
                       float running_meter_total_, S1&& specific_cause_, S2&& custom_label_ = "")
            noexcept(noexcept(EffectCause{std::declval<EffectsCauseType>(), std::declval<S1>(), std::declval<S2>()})) :
            EffectCause(cause_type_, std::forward<S1>(specific_cause_), std::forward<S2>(custom_label_)),
            source_id(source_id_),
            meter_change(meter_change_),
            running_meter_total(running_meter_total_)
        {}

        [[nodiscard]] bool operator==(const AccountingInfo& rhs) const noexcept;

        int     source_id = INVALID_OBJECT_ID;  ///< source object of effect
        float   meter_change = 0.0f;            ///< net change on meter due to this effect, as best known by client's empire
        float   running_meter_total = 0.0f;     ///< meter total as of this effect.
    };

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
        EffectsGroup(std::unique_ptr<Condition::Condition>&& scope,
                     std::unique_ptr<Condition::Condition>&& activation,
                     std::vector<std::unique_ptr<Effect>>&& effects,
                     std::string accounting_label = "",
                     std::string stacking_group = "",
                     int priority = 0,
                     std::string description = "",
                     std::string content_name = "");
        virtual ~EffectsGroup();

        EffectsGroup(EffectsGroup&& rhs) = default;

        [[nodiscard]] bool operator==(const EffectsGroup& rhs) const;

        /** execute all effects in group */
        void Execute(ScriptingContext& source_context,
                     const TargetsAndCause& targets_cause,
                     AccountingMap* accounting_map = nullptr,
                     bool only_meter_effects = false,
                     bool only_appearance_effects = false,
                     bool include_empire_meter_effects = false,
                     bool only_generate_sitrep_effects = false) const;

        [[nodiscard]] auto& StackingGroup() const noexcept   { return m_stacking_group; }
        [[nodiscard]] auto* Scope() const noexcept           { return m_scope.get(); }
        [[nodiscard]] auto* Activation() const noexcept      { return m_activation.get(); }
        [[nodiscard]] auto& Effects() const noexcept         { return m_effects; }
        [[nodiscard]] auto& GetDescription() const noexcept  { return m_description; }
        [[nodiscard]] auto& AccountingLabel() const noexcept { return m_accounting_label; }
        [[nodiscard]] int   Priority() const noexcept        { return m_priority; }
        [[nodiscard]] bool  HasMeterEffects() const noexcept      { return m_has_meter_effects; }
        [[nodiscard]] bool  HasAppearanceEffects() const noexcept { return m_has_appearance_effects; }
        [[nodiscard]] bool  HasSitrepEffects() const noexcept     { return m_has_sitrep_effects; }

        void SetTopLevelContent(std::string content_name);

        [[nodiscard]] auto& TopLevelContent() const noexcept { return m_content_name; }

        [[nodiscard]] std::string      Dump(uint8_t ntabs = 0) const;
        [[nodiscard]] virtual uint32_t GetCheckSum() const;

    protected:
        std::unique_ptr<Condition::Condition>   m_scope;
        std::unique_ptr<Condition::Condition>   m_activation;
        std::string                             m_stacking_group;
        std::vector<std::unique_ptr<Effect>>    m_effects;
        std::string                             m_accounting_label;
        const int                               m_priority; // constructor sets this, so don't need a default value here
        std::string                             m_description;
        std::string                             m_content_name;
        const bool                              m_has_meter_effects;
        const bool                              m_has_appearance_effects;
        const bool                              m_has_sitrep_effects;
    };

    /** Returns a single string which `Dump`s a vector of EffectsGroups. */
    FO_COMMON_API std::string Dump(const std::vector<EffectsGroup>& effects_groups);
}

#endif
