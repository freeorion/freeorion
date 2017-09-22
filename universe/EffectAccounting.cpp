#include "EffectAccounting.h"

#include "UniverseObject.h"
#include "ObjectMap.h"
#include "Enums.h"

Effect::EffectCause::EffectCause() :
    cause_type(INVALID_EFFECTS_GROUP_CAUSE_TYPE),
    specific_cause(),
    custom_label()
{}

Effect::EffectCause::EffectCause(EffectsCauseType cause_type_, const std::string& specific_cause_,
                                 const std::string& custom_label_) :
    cause_type(cause_type_),
    specific_cause(specific_cause_),
    custom_label(custom_label_)
{
    //DebugLogger() << "EffectCause(" << cause_type << ", " << specific_cause << ", " << custom_label << ")";
}

Effect::AccountingInfo::AccountingInfo() :
    EffectCause(),
    source_id(INVALID_OBJECT_ID),
    meter_change(0.0),
    running_meter_total(0.0)
{}

bool Effect::AccountingInfo::operator==(const Effect::AccountingInfo& rhs) const {
    return
        cause_type == rhs.cause_type &&
        specific_cause == rhs.specific_cause &&
        custom_label == rhs.custom_label &&
        source_id == rhs.source_id &&
        meter_change == rhs.meter_change &&
        running_meter_total == rhs.running_meter_total;
}

Effect::TargetsAndCause::TargetsAndCause() :
    target_set(),
    effect_cause()
{}

Effect::TargetsAndCause::TargetsAndCause(const TargetSet& target_set_, const EffectCause& effect_cause_) :
    target_set(target_set_),
    effect_cause(effect_cause_)
{}

Effect::SourcedEffectsGroup::SourcedEffectsGroup() :
    source_object_id(INVALID_OBJECT_ID)
{}

Effect::SourcedEffectsGroup::SourcedEffectsGroup(int source_object_id_, const std::shared_ptr<EffectsGroup>& effects_group_) :
    source_object_id(source_object_id_),
    effects_group(effects_group_)
{}

bool Effect::SourcedEffectsGroup::operator<(const SourcedEffectsGroup& right) const {
    return (this->source_object_id < right.source_object_id ||
            ((this->source_object_id == right.source_object_id) && this->effects_group < right.effects_group));
}

