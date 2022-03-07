#ifndef _EmpireManager_h_
#define _EmpireManager_h_

#include "Diplomacy.h"
#include "../universe/EnumsFwd.h"
#include "../util/AppInterface.h"
#include "../util/Export.h"

#include <boost/filesystem.hpp>
#include <boost/signals2/signal.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

class Empire;
class UniverseObject;
typedef std::array<unsigned char, 4> EmpireColor;

/** Maintains all of the Empire objects that exist in the application. */
class FO_COMMON_API EmpireManager {
public:
    using container_type = std::map<int, std::shared_ptr<Empire>>;
    using iterator = container_type::iterator;
    using const_container_type = std::map<int, std::shared_ptr<const Empire>>;
    using const_iterator = const_container_type::const_iterator;

    using DiploStatusMap = std::map<std::pair<int, int>, DiplomaticStatus>;

    EmpireManager() = default;
    EmpireManager& operator=(EmpireManager&& other) noexcept;
    ~EmpireManager() = default;

    [[nodiscard]] std::vector<int>                      EmpireIDs() const;
    [[nodiscard]] const const_container_type&           GetEmpires() const;
    [[nodiscard]] std::shared_ptr<const Empire>         GetEmpire(int id) const;  //!< Returns the empire whose ID is \a id, or nullptr if none exist
    [[nodiscard]] const std::string&                    GetEmpireName(int id) const;
    [[nodiscard]] std::shared_ptr<const UniverseObject> GetSource(int id, const ObjectMap& objects) const;  //!< Return the empire source or nullptr if the empire or source doesn't exist

    [[nodiscard]] int                       NumEmpires() const;
    [[nodiscard]] int                       NumEliminatedEmpires() const;

    [[nodiscard]] const DiploStatusMap&     GetDiplomaticStatuses() const;
    [[nodiscard]] DiplomaticStatus          GetDiplomaticStatus(int empire1, int empire2) const;
    [[nodiscard]] std::set<int>             GetEmpireIDsWithDiplomaticStatusWithEmpire(
        int empire_id, DiplomaticStatus diplo_status) const
    { return GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, diplo_status, m_empire_diplomatic_statuses); }
    [[nodiscard]] static std::set<int>      GetEmpireIDsWithDiplomaticStatusWithEmpire(
        int empire_id, DiplomaticStatus diplo_status, const DiploStatusMap& statuses);
    [[nodiscard]] bool                      DiplomaticMessageAvailable(int sender_id, int recipient_id) const;
    [[nodiscard]] const DiplomaticMessage&  GetDiplomaticMessage(int sender_id, int recipient_id) const;

    [[nodiscard]] std::string               Dump() const;

    [[nodiscard]] std::shared_ptr<Empire>   GetEmpire(int id);  //!< Returns the empire whose ID is \a id, or nullptr if none exist
    [[nodiscard]] const container_type&     GetEmpires();

    [[nodiscard]] const_iterator            begin() const;
    [[nodiscard]] const_iterator            end() const;
    [[nodiscard]] iterator                  begin();
    [[nodiscard]] iterator                  end();

    void BackPropagateMeters();

    void SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status);
    void HandleDiplomaticMessage(const DiplomaticMessage& message);
    void SetDiplomaticMessage(const DiplomaticMessage& message);
    void RemoveDiplomaticMessage(int sender_id, int recipient_id);

    void ResetDiplomacy();

    /** Creates and inserts an empire with the specified properties and returns
      * a pointer to it.  This will only set up the data in Empire.  It is the
      * caller's responsibility to make sure that universe updates planet
      * ownership. */
    void CreateEmpire(int empire_id, std::string name, std::string player_name,
                      const EmpireColor& color, bool authenticated);

    /** Removes and deletes all empires from the manager. */
    void Clear();

    typedef boost::signals2::signal<void (int, int)> DiploSignalType;

    mutable DiploSignalType DiplomaticStatusChangedSignal;
    mutable DiploSignalType DiplomaticMessageChangedSignal;

private:
    [[nodiscard]] std::string DumpDiplomacy() const;

    void InsertEmpire(std::shared_ptr<Empire>&& empire); //!< Adds the given empire to the manager

    void GetDiplomaticMessagesToSerialize(std::map<std::pair<int, int>, DiplomaticMessage>& messages,
                                          int encoding_empire) const;

    container_type                                   m_empire_map;
    const_container_type                             m_const_empire_map;
    DiploStatusMap                                   m_empire_diplomatic_statuses;
    std::map<std::pair<int, int>, DiplomaticMessage> m_diplomatic_messages;

    friend class ClientApp;
    friend class ServerApp;

    template <typename Archive>
    friend void serialize(Archive&, EmpireManager&, unsigned int const);
};

/** The colors that are available for use for empires in the game. */
[[nodiscard]] FO_COMMON_API const std::vector<std::array<unsigned char, 4>>& EmpireColors();

/** Initialize empire colors from \p path */
FO_COMMON_API void InitEmpireColors(const boost::filesystem::path& path);


#endif
