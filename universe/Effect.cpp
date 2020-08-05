#include "Effect.h"

#include "Enums.h"
#include "ObjectMap.h"
#include "UniverseObject.h"

Effect::EffectCause::EffectCause(EffectsCauseType cause_type_, std::string specific_cause_,
                                 std::string custom_label_) :
    cause_type(cause_type_),
    specific_cause(std::move(specific_cause_)),
    custom_label(std::move(custom_label_))
{
    //DebugLogger() << "EffectCause(" << cause_type << ", " << specific_cause << ", " << custom_label << ")";
}

Effect::AccountingInfo::AccountingInfo(int source_id_, EffectsCauseType cause_type_,
                                       float meter_change_, float running_meter_total_,
                                       std::string&& specific_cause_, std::string&& custom_label_) :
    EffectCause(cause_type_, std::move(specific_cause_), std::move(custom_label_)),
    source_id(source_id_),
    meter_change(meter_change_),
    running_meter_total(running_meter_total_)
{}

bool Effect::AccountingInfo::operator==(const AccountingInfo& rhs) const {
    return
        cause_type == rhs.cause_type &&
        specific_cause == rhs.specific_cause &&
        custom_label == rhs.custom_label &&
        source_id == rhs.source_id &&
        meter_change == rhs.meter_change &&
        running_meter_total == rhs.running_meter_total;
}

Effect::TargetsAndCause::TargetsAndCause(TargetSet target_set_, EffectCause effect_cause_) :
    target_set(std::move(target_set_)),
    effect_cause(std::move(effect_cause_))
{}

Effect::SourcedEffectsGroup::SourcedEffectsGroup(int source_object_id_, const EffectsGroup* effects_group_) :
    source_object_id(source_object_id_),
    effects_group(effects_group_)
{}

bool Effect::SourcedEffectsGroup::operator<(const SourcedEffectsGroup& right) const {
    return (this->source_object_id < right.source_object_id ||
            ((this->source_object_id == right.source_object_id) && this->effects_group < right.effects_group));
}

