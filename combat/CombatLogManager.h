#ifndef _CombatLogManager_h_
#define _CombatLogManager_h_

#include "CombatSystem.h"
#include "../util/Export.h"
#include <boost/optional/optional.hpp>
#include <memory>


// A snapshot of the state of a participant of the combat
// at it's end
struct FO_COMMON_API CombatParticipantState {
    float current_health = 0.0f;
    float max_health = 0.0f;

    CombatParticipantState() = default;
    CombatParticipantState(const UniverseObject& object);
};

struct FO_COMMON_API CombatLog {
    CombatLog() = default;
    CombatLog(const CombatInfo& combat_info);

    int                         turn = INVALID_GAME_TURN;
    int                         system_id = INVALID_OBJECT_ID;
    std::set<int>               empire_ids;
    std::set<int>               object_ids;
    std::set<int>               damaged_object_ids;
    std::set<int>               destroyed_object_ids;
    std::vector<CombatEventPtr> combat_events;
    std::map<int, CombatParticipantState> participant_states;
};


/** Stores and retreives combat logs. */
class FO_COMMON_API CombatLogManager {
public:
    CombatLogManager() = default;
    CombatLogManager& operator=(CombatLogManager&& rhs) noexcept;

    /** Return the requested combat log or boost::none.*/
    [[nodiscard]] boost::optional<const CombatLog&> GetLog(int log_id) const;

    /** Return the ids of all incomplete logs, if any. */
    [[nodiscard]] std::vector<int> IncompleteLogIDs() const { return {m_incomplete_logs.begin(), m_incomplete_logs.end()}; }

    int AddNewLog(CombatLog log);      // adds log, returns unique log id

    /** Replace incomplete log with \p id with \p log. An incomplete log is a
        partially downloaded log where only the log id is known.*/
    void CompleteLog(int id, CombatLog log);
    void Clear();

    static constexpr int INVALID_COMBAT_LOG_ID = -1;

private:
    std::unordered_map<int, CombatLog> m_logs;
    //! Set of logs ids that do not have bodies and need to be fetched from the server
    std::set<int>                      m_incomplete_logs;
    std::atomic<int>                   m_latest_log_id = 0;

    template <typename Archive>
    friend void serialize(Archive&, CombatLogManager&, const unsigned int);

    template <typename Archive>
    friend void SerializeIncompleteLogs(Archive&, CombatLogManager&, const unsigned int);
};


/** returns the singleton combat log manager */
[[nodiscard]] FO_COMMON_API CombatLogManager& GetCombatLogManager();

/** Returns the CombatLog with the indicated id, or an empty log if there
  * is no avaiable log with that id. */
[[nodiscard]] FO_COMMON_API boost::optional<const CombatLog&> GetCombatLog(int log_id);


#endif
