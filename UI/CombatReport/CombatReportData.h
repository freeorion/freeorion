#ifndef COMBATREPORTDATA_H
#define COMBATREPORTDATA_H


#include <GG/Clr.h>
#include "../util/Export.h"

#include <memory>
#include <vector>


struct WeaponFireEvent;
struct CombatParticipantState;
class Empire;
class UniverseObject;

FO_COMMON_API extern const int INVALID_OBJECT_ID;
FO_COMMON_API extern const int ALL_EMPIRES;

// Data on what happened on single object in the combat
struct ParticipantSummary {
    int     object_id = INVALID_OBJECT_ID;
    int     empire_id = ALL_EMPIRES;
    float   current_health = 0.0f;
    float   max_health = 0.0f;

    ParticipantSummary() = default;
    ParticipantSummary(int object_id_, int empire_id_, const CombatParticipantState& state);
};

// A summary of what happened to a side in the combat (an empire or neutral)
struct CombatSummary {
public:
    // Should be auto_ptr, but don't have c++11
    typedef std::shared_ptr<ParticipantSummary> ParticipantSummaryPtr;
    // Participant summaries have vectors inside them,
    // so we don't want to have to copy them around while sorting
    // therefore we store them with pointers
    typedef std::vector<ParticipantSummaryPtr> UnitSummaries;

    Empire*         empire = nullptr;
    UnitSummaries   unit_summaries;
    float           total_current_health = 0.0f;
    float           total_max_health = 0.0f;
    float           max_max_health = 0.0f;
    float           max_current_health = 0.0f;

    CombatSummary() = default;
    CombatSummary(int empire_id);

    GG::Clr         SideColor() const;
    std::string     SideName() const;
    unsigned int    DestroyedUnits() const;
    void            AddUnit(int unit_id, const CombatParticipantState& state);  // Adds a summary of a unit to the summary of its side and aggregates its data.
    void            Sort();                                                     // Sorts the units of this side in some sensible fashion
};


#endif
