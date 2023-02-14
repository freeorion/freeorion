#include "Effect.h"

#include "Enums.h"
#include "ObjectMap.h"
#include "UniverseObject.h"

bool Effect::AccountingInfo::operator==(const AccountingInfo& rhs) const noexcept {
    return
        cause_type == rhs.cause_type &&
        specific_cause == rhs.specific_cause &&
        custom_label == rhs.custom_label &&
        source_id == rhs.source_id &&
        meter_change == rhs.meter_change &&
        running_meter_total == rhs.running_meter_total;
}

Effect::SourcedEffectsGroup::SourcedEffectsGroup(int source_object_id_, const EffectsGroup* effects_group_) :
    source_object_id(source_object_id_),
    effects_group(effects_group_)
{}

bool Effect::SourcedEffectsGroup::operator<(const SourcedEffectsGroup& right) const {
    return (this->source_object_id < right.source_object_id ||
            ((this->source_object_id == right.source_object_id) && this->effects_group < right.effects_group));
}

