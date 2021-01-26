#include "CombatReportData.h"
#include "../../combat/CombatLogManager.h"
#include "../../universe/UniverseObject.h"
#include "../../util/AppInterface.h"
#include "../../util/i18n.h"
#include "../../util/Logger.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"

#include <GG/ClrConstants.h>

ParticipantSummary::ParticipantSummary(int object_id_, int empire_id_, const CombatParticipantState& state) :
    object_id(object_id_),
    empire_id(empire_id_),
    current_health(state.current_health),
    max_health(state.max_health)
{}

CombatSummary::CombatSummary(int empire_id) :
    empire(GetEmpire(empire_id))
{}

GG::Clr CombatSummary::SideColor() const {
    if (empire)
        return empire->Color();
    else
        return GG::CLR_WHITE;
}

std::string CombatSummary::SideName() const {
    if (empire)
        return empire->Name();
    else
        return UserString("NEUTRAL");
}

unsigned int CombatSummary::DestroyedUnits() const {
    unsigned count = 0;
    for (const ParticipantSummaryPtr& summary : unit_summaries) {
        if (summary->current_health <= 0.0f && summary->max_health > 0.0f)
            ++count;
    }
    return count;
}

void CombatSummary::AddUnit(int unit_id, const CombatParticipantState& state) {
    unit_summaries.push_back(std::make_shared<ParticipantSummary>(unit_id, empire ? empire->EmpireID() : ALL_EMPIRES, state));

    total_current_health += std::max(unit_summaries.back()->current_health, 0.0f);
    total_max_health += std::max(unit_summaries.back()->max_health, 0.0f);

    max_current_health = std::max(max_current_health, unit_summaries.back()->current_health);
    max_max_health = std::max(max_max_health, unit_summaries.back()->max_health);
}

void CombatSummary::Sort() {
    // First by max health, then by current health.
    // This will group similar ships, then order them by current health
    std::sort(unit_summaries.begin(), unit_summaries.end(), [](const auto& one, const auto& two) {
        if (one->max_health > two->max_health)
            return true;
        else if (two->max_health > one->max_health)
            return false;
        else if (one->current_health > two->current_health)
            return true;
        else
            return false;
    });
}
