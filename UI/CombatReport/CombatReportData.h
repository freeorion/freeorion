#ifndef COMBATREPORTDATA_H
#define COMBATREPORTDATA_H


#include <GG/Clr.h>

#include <memory>
#include <vector>


struct WeaponFireEvent;
struct CombatParticipantState;
class Empire;
class UniverseObject;

// Data on what happened on single object in the combat
struct ParticipantSummary {
    int                         object_id;
    int                         empire_id;
    // These appear to be never used
    /* std::vector<WeaponFireEvent*>   attacks; */
    /* std::vector<WeaponFireEvent*>   attacks_against; */
    float                       current_health;
    float                       max_health;

    ParticipantSummary();
    ParticipantSummary(int object_id, int empire_id, const CombatParticipantState& state);
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

    Empire*         empire;
    UnitSummaries   unit_summaries;
    float           total_current_health;
    float           total_max_health;
    float           max_max_health;
    float           max_current_health;

    CombatSummary();
    CombatSummary(int empire_id);

    GG::Clr         SideColor() const;
    std::string     SideName() const;
    unsigned int    DestroyedUnits() const;
    void            AddUnit(int unit_id, const CombatParticipantState& state);  // Adds a summary of a unit to the summary of its side and aggregates its data.
    void            Sort();                                                     // Sorts the units of this side in some sensible fashion
};

#endif // COMBATREPORTDATA_H
