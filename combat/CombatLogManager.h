#ifndef _CombatLogManager_h_
#define _CombatLogManager_h_

#include "CombatSystem.h"

#include "../util/Export.h"

#include <boost/optional/optional.hpp>
#include <boost/scoped_ptr.hpp>

// A snapshot of the state of a participant of the combat
// at it's end
struct FO_COMMON_API CombatParticipantState {
    float current_health;
    float max_health;

    CombatParticipantState();
    CombatParticipantState(const UniverseObject& object);
private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

struct FO_COMMON_API CombatLog {
    CombatLog();
    CombatLog(const CombatInfo& combat_info);

    int                         turn;
    int                         system_id;
    std::set<int>               empire_ids;
    std::set<int>               object_ids;
    std::set<int>               damaged_object_ids;
    std::set<int>               destroyed_object_ids;
    std::vector<CombatEventPtr> combat_events;
    std::map<int, CombatParticipantState> participant_states;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

BOOST_CLASS_VERSION ( CombatLog, 1 );

/** Stores and retreives combat logs. */
class FO_COMMON_API CombatLogManager {
public:
    /** \name Accessors */ //@{
    /** Return the requested combat log or boost::none.*/
    boost::optional<const CombatLog&>  GetLog(int log_id) const;

    /** Return true if there are partial logs.*/
    bool HasIncompleteLogs() const;

    /** Return the ids of all incomplete logs.*/
    std::vector<int> IncompleteLogIDs() const;
    //@}

    /** \name Mutators */ //@{
    int     AddLog(const CombatLog& log);   // adds log, returns unique log id
    void    Clear();
    //@}

    static CombatLogManager& GetCombatLogManager();

private:
    CombatLogManager();
    ~CombatLogManager();

    class CombatLogManagerImpl;
    // TODO use C++11 unique_ptr
    boost::scoped_ptr<CombatLogManagerImpl> const pimpl;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** returns the singleton combat log manager */
FO_COMMON_API CombatLogManager&   GetCombatLogManager();

/** Returns the CombatLog with the indicated id, or an empty log if there
  * is no avaiable log with that id. */
FO_COMMON_API boost::optional<const CombatLog&> GetCombatLog(int log_id);

#endif // _CombatLogManager_h_
