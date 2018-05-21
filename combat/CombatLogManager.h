#ifndef _CombatLogManager_h_
#define _CombatLogManager_h_

#include "CombatSystem.h"

#include "../util/Export.h"
#include "../util/Serialize.h"

#include <boost/optional/optional.hpp>
#include <boost/serialization/nvp.hpp>

#include <memory>


// A snapshot of the state of a participant of the combat
// at it's end
struct FO_COMMON_API CombatParticipantState {
    float current_health = 0.0f;
    float max_health = 0.0f;

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

BOOST_CLASS_VERSION(CombatLog, 1);

/** Stores and retreives combat logs. */
class FO_COMMON_API CombatLogManager {
public:
    /** \name Accessors */ //@{
    /** Return the requested combat log or boost::none.*/
    boost::optional<const CombatLog&>  GetLog(int log_id) const;

    /** Return the ids of all incomplete logs or boost::none if they are all complete.*/
    boost::optional<std::vector<int>> IncompleteLogIDs() const;
    //@}

    /** \name Mutators */ //@{
    int  AddNewLog(const CombatLog& log);   // adds log, returns unique log id
    /** Replace incomplete log with \p id with \p log. An incomplete log is a
        partially downloaded log where only the log id is known.*/
    void CompleteLog(int id, const CombatLog& log);
    void Clear();

    /** Serialize log headers so that the receiving LogManager can then request
        complete logs in the background.*/
    template <class Archive>
    void SerializeIncompleteLogs(Archive& ar, const unsigned int version);
    //@}

    static CombatLogManager& GetCombatLogManager();

private:
    CombatLogManager();
    ~CombatLogManager();

    class Impl;

    std::unique_ptr<Impl> const m_impl;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};


extern template
FO_COMMON_API void CombatLogManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void CombatLogManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void CombatLogManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive& ar, const unsigned int version);
extern template
FO_COMMON_API void CombatLogManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive& ar, const unsigned int version);


/** returns the singleton combat log manager */
FO_COMMON_API CombatLogManager& GetCombatLogManager();

/** Returns the CombatLog with the indicated id, or an empty log if there
  * is no avaiable log with that id. */
FO_COMMON_API boost::optional<const CombatLog&> GetCombatLog(int log_id);

#endif // _CombatLogManager_h_
