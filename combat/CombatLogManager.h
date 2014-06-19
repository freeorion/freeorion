// -*- C++ -*-
#ifndef _CombatLogManager_h_
#define _CombatLogManager_h_

#include "CombatSystem.h"

#include "../util/Export.h"

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

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


/** Stores and retreives combat logs. */
class FO_COMMON_API CombatLogManager {
public:
    /** \name Accessors */ //@{
    std::map<int, CombatLog>::const_iterator    begin() const;
    std::map<int, CombatLog>::const_iterator    end() const;
    std::map<int, CombatLog>::const_iterator    find(int log_id) const;
    bool                                        LogAvailable(int log_id) const; // returns whether a log with the indicated id is available
    const CombatLog&                            GetLog(int log_id) const;       // returns requested combat log, or an empty default log if no log with the requested id exists
    //@}

    /** \name Mutators */ //@{
    int     AddLog(const CombatLog& log);   // adds log, returns unique log id
    void    RemoveLog(int log_id);
    void    Clear();
    //@}

    static CombatLogManager& GetCombatLogManager();

private:
    CombatLogManager();

    void GetLogsToSerialize(std::map<int, CombatLog>& logs, int encoding_empire) const;
    void SetLog(int log_id, const CombatLog& log);

    std::map<int, CombatLog>    m_logs;
    int                         m_latest_log_id;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

/** returns the singleton combat log manager */
FO_COMMON_API CombatLogManager&   GetCombatLogManager();

/** Returns the CombatLog with the indicated id, or an empty log if there
  * is no avaiable log with that id. */
FO_COMMON_API const CombatLog&    GetCombatLog(int log_id);

/** Returns true if a CombatLog with the indicated id is available. */
FO_COMMON_API bool                CombatLogAvailable(int log_id);

template <class Archive>
void CombatLogManager::serialize(Archive& ar, const unsigned int version)
{
    std::map<int, CombatLog> logs;

    if (Archive::is_saving::value) {
        GetLogsToSerialize(logs, GetUniverse().EncodingEmpire());
    }

    ar  & BOOST_SERIALIZATION_NVP(logs)
        & BOOST_SERIALIZATION_NVP(m_latest_log_id);

    if (Archive::is_loading::value) {
        // copy new logs, but don't erase old ones
        for (std::map<int, CombatLog>::const_iterator it = logs.begin(); it != logs.end(); ++it)
            this->SetLog(it->first, it->second);
    }
}


#endif // _CombatLogManager_h_
