#ifndef _Effect_h_
#define _Effect_h_


#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <boost/container/flat_map.hpp>
#include <GG/Enum.h>
#include "EnumsFwd.h"
#include "../util/Export.h"


FO_COMMON_API extern const int INVALID_OBJECT_ID;

class UniverseObject;
struct ScriptingContext;

namespace Condition {
    struct Condition;
}

//! Types of in-game things that might contain an EffectsGroup, or "cause"
//! effects to occur
GG_ENUM(EffectsCauseType,
    INVALID_EFFECTS_GROUP_CAUSE_TYPE = -1,
    ECT_UNKNOWN_CAUSE,
    ECT_INHERENT,
    ECT_TECH,
    ECT_BUILDING,
    ECT_FIELD,
    ECT_SPECIAL,
    ECT_SPECIES,
    ECT_SHIP_PART,
    ECT_SHIP_HULL,
    ECT_POLICY
)

namespace Effect {
    struct AccountingInfo;
    class EffectsGroup;

    typedef std::vector<std::shared_ptr<UniverseObject>> TargetSet;
    /** Effect accounting information for all meters of all objects that are
      * acted on by effects. */
    typedef std::unordered_map<int, boost::container::flat_map<MeterType, std::vector<AccountingInfo>>> AccountingMap;

    /** Description of cause of an effect: the general cause type, and the
      * specific cause.  eg. Building and a particular BuildingType. */
    struct FO_COMMON_API EffectCause {
        explicit EffectCause() = default;
        EffectCause(EffectsCauseType cause_type_, std::string specific_cause_,
                    std::string custom_label_ = "");
        EffectsCauseType    cause_type = EffectsCauseType::INVALID_EFFECTS_GROUP_CAUSE_TYPE;  ///< general type of effect cause, eg. tech, building, special...
        std::string         specific_cause; ///< name of specific cause, eg. "Wonder Farm", "Antenna Mk. VI"
        std::string         custom_label;   ///< script-specified accounting label for this effect cause
    };

    /** Combination of targets and cause for an effects group. */
    struct TargetsAndCause {
        explicit TargetsAndCause() = default;
        TargetsAndCause(TargetSet target_set_, EffectCause effect_cause_);
        TargetSet target_set;
        EffectCause effect_cause;
    };

    /** Combination of an EffectsGroup and the id of a source object. */
    struct SourcedEffectsGroup {
        explicit SourcedEffectsGroup() = default;
        SourcedEffectsGroup(int source_object_id_, const EffectsGroup* effects_group_);
        bool operator<(const SourcedEffectsGroup& right) const;
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
        virtual ~Effect();

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

        virtual std::string     Dump(unsigned short ntabs = 0) const = 0;

        virtual bool            IsMeterEffect() const { return false; }
        virtual bool            IsEmpireMeterEffect() const { return false; }
        virtual bool            IsAppearanceEffect() const { return false; }
        virtual bool            IsSitrepEffect() const { return false; }
        virtual bool            IsConditionalEffect() const { return false; }

        // TODO: source-invariant?

        virtual void            SetTopLevelContent(const std::string& content_name) = 0;
        virtual unsigned int    GetCheckSum() const;
    };

    /** Accounting information about what the causes are and changes produced
      * by effects groups acting on meters of objects. */
    struct FO_COMMON_API AccountingInfo : public EffectCause {
        explicit AccountingInfo() = default;
        AccountingInfo(int source_id_, EffectsCauseType cause_type_, float meter_change_,
                       float running_meter_total_, std::string&& specific_cause_ = "",
                       std::string&& custom_label_ = "");

        bool operator==(const AccountingInfo& rhs) const;

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

        /** execute all effects in group */
        void Execute(ScriptingContext& source_context,
                     const TargetsAndCause& targets_cause,
                     AccountingMap* accounting_map = nullptr,
                     bool only_meter_effects = false,
                     bool only_appearance_effects = false,
                     bool include_empire_meter_effects = false,
                     bool only_generate_sitrep_effects = false) const;

        const std::string&              StackingGroup() const       { return m_stacking_group; }
        Condition::Condition*           Scope() const               { return m_scope.get(); }
        Condition::Condition*           Activation() const          { return m_activation.get(); }
        const std::vector<Effect*>      EffectsList() const;
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
        std::unique_ptr<Condition::Condition>   m_scope;
        std::unique_ptr<Condition::Condition>   m_activation;
        std::string                             m_stacking_group;
        std::vector<std::unique_ptr<Effect>>    m_effects;
        std::string                             m_accounting_label;
        int                                     m_priority; // constructor sets this, so don't need a default value here
        std::string                             m_description;
        std::string                             m_content_name;
    };

    /** Returns a single string which `Dump`s a vector of EffectsGroups. */
    FO_COMMON_API std::string Dump(const std::vector<std::shared_ptr<EffectsGroup>>& effects_groups);
}

#endif
