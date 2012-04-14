#include "EffectAccounting.h"

#include "UniverseObject.h"

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
    //Logger().debugStream() << "EffectCause(" << cause_type << ", " << specific_cause << ", " << custom_label << ")";
}

Effect::AccountingInfo::AccountingInfo() :
    EffectCause(),
    source_id(INVALID_OBJECT_ID),
    meter_change(0.0),
    running_meter_total(0.0)
{}

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

Effect::SourcedEffectsGroup::SourcedEffectsGroup(int source_object_id_, const boost::shared_ptr<const EffectsGroup>& effects_group_) :
    source_object_id(source_object_id_),
    effects_group(effects_group_)
{}

bool Effect::SourcedEffectsGroup::operator<(const SourcedEffectsGroup& right) const {
    return (this->source_object_id < right.source_object_id ||
        (this->source_object_id == right.source_object_id) && this->effects_group < right.effects_group);
}

