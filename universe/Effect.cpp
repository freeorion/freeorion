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


