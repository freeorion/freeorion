#ifndef _EmpireManager_h_
#define _EmpireManager_h_

#include "Diplomacy.h"
#include "../universe/EnumsFwd.h"
#include "../util/Export.h"
#include "../util/Serialize.h"

#include <GG/Clr.h>

#include <boost/filesystem.hpp>
#include <boost/serialization/access.hpp>
#include <boost/signals2/signal.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

class Empire;
class UniverseObject;

/** Maintains all of the Empire objects that exist in the application. */
class FO_COMMON_API EmpireManager {
public:
    /// Iterator over Empires
    typedef std::map<int, Empire*>::iterator iterator;

    /// Const Iterator over Empires
    typedef std::map<int, Empire*>::const_iterator const_iterator;

    /** \name Structors */ //@{
    virtual ~EmpireManager();

    const EmpireManager& operator=(EmpireManager& rhs); ///< assignment operator (move semantics)
    //@}

    /** \name Accessors */ //@{
    /** Returns the empire whose ID is \a ID, or 0 if none exists. */
    const Empire*       GetEmpire(int id) const;
    /** Return the empire source or nullptr if the empire or source doesn't exist.*/
    std::shared_ptr<const UniverseObject> GetSource(int id) const;
    const std::string&  GetEmpireName(int id) const;

    const_iterator      begin() const;
    const_iterator      end() const;

    int                 NumEmpires() const;

    DiplomaticStatus            GetDiplomaticStatus(int empire1, int empire2) const;
    std::set<int>               GetEmpireIDsWithDiplomaticStatusWithEmpire(int empire_id,
                                                                           DiplomaticStatus diplo_status) const;
    bool                        DiplomaticMessageAvailable(int sender_id, int recipient_id) const;
    const DiplomaticMessage&    GetDiplomaticMessage(int sender_id, int recipient_id) const;

    std::string         Dump() const;
    //@}

    /** \name Mutators */ //@{
    /** Returns the empire whose ID is \a id, or 0 if none exists. */
    Empire*     GetEmpire(int id);

    iterator    begin();
    iterator    end();

    void        BackPropagateMeters();

    void        SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status);
    void        HandleDiplomaticMessage(const DiplomaticMessage& message);
    void        SetDiplomaticMessage(const DiplomaticMessage& message);
    void        RemoveDiplomaticMessage(int sender_id, int recipient_id);

    void        ResetDiplomacy();

    /** Creates and inserts an empire with the specified properties and returns
      * a pointer to it.  This will only set up the data in Empire.  It is the
      * caller's responsibility to make sure that universe updates planet
      * ownership. */
    Empire*     CreateEmpire(int empire_id, const std::string& name, const std::string& player_name,
                             const GG::Clr& color, bool authenticated);

    /** Removes and deletes all empires from the manager. */
    void        Clear();
    //@}

    typedef boost::signals2::signal<void (int, int)>  DiploSignalType;

    mutable DiploSignalType DiplomaticStatusChangedSignal;
    mutable DiploSignalType DiplomaticMessageChangedSignal;

private:
    EmpireManager();

    std::string DumpDiplomacy() const;

    /** Adds the given empire to the manager. */
    void        InsertEmpire(Empire* empire);
    void        GetDiplomaticMessagesToSerialize(std::map<std::pair<int, int>, DiplomaticMessage>& messages,
                                                 int encoding_empire) const;

    std::map<int, Empire*>                          m_empire_map;
    std::map<std::pair<int, int>, DiplomaticStatus> m_empire_diplomatic_statuses;
    std::map<std::pair<int, int>, DiplomaticMessage>m_diplomatic_messages;

    friend class ClientApp;
    friend class ServerApp;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
};

extern template FO_COMMON_API void EmpireManager::serialize<freeorion_bin_oarchive>(freeorion_bin_oarchive&, const unsigned int);
extern template FO_COMMON_API void EmpireManager::serialize<freeorion_bin_iarchive>(freeorion_bin_iarchive&, const unsigned int);
extern template FO_COMMON_API void EmpireManager::serialize<freeorion_xml_oarchive>(freeorion_xml_oarchive&, const unsigned int);
extern template FO_COMMON_API void EmpireManager::serialize<freeorion_xml_iarchive>(freeorion_xml_iarchive&, const unsigned int);

/** The colors that are available for use for empires in the game. */
FO_COMMON_API const std::vector<GG::Clr>& EmpireColors();

/** Initialize empire colors from \p path */
FO_COMMON_API void InitEmpireColors(const boost::filesystem::path& path);

#endif // _EmpireManager_h_
