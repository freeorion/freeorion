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
using EmpireColor = std::array<uint8_t, 4>;
using DiploStatusMap = boost::container::flat_map<std::pair<int, int>, DiplomaticStatus>;

/** Maintains all of the Empire objects that exist in the application. */
class FO_COMMON_API EmpireManager final {
public:
    using container_type = std::map<int, std::shared_ptr<Empire>>;
    using iterator = container_type::iterator;
    using const_container_type = std::map<int, std::shared_ptr<const Empire>>;
    using const_iterator = const_container_type::const_iterator;

    EmpireManager() = default;
    EmpireManager& operator=(EmpireManager&& other) noexcept;
    ~EmpireManager() = default;

    [[nodiscard]] const auto&                           EmpireIDs() const noexcept { return m_empire_ids; }
    [[nodiscard]] const const_container_type&           GetEmpires() const noexcept { return m_const_empire_map; }
    [[nodiscard]] std::shared_ptr<const Empire>         GetEmpire(int id) const;  //!< Returns the empire whose ID is \a id, or nullptr if none exist
    [[nodiscard]] std::shared_ptr<const UniverseObject> GetSource(int id, const ObjectMap& objects) const;  //!< Return the empire source or nullptr if the empire or source doesn't exist

    [[nodiscard]] int                       NumEmpires() const noexcept { return static_cast<int>(m_const_empire_map.size()); }
    [[nodiscard]] int                       NumEliminatedEmpires() const;

    [[nodiscard]] const auto&               CapitalIDs() const noexcept { return m_capital_ids; }

    [[nodiscard]] const auto&               GetDiplomaticStatuses() const noexcept { return m_empire_diplomatic_statuses; }
    [[nodiscard]] DiplomaticStatus          GetDiplomaticStatus(int empire1, int empire2) const;

    [[nodiscard]] boost::container::flat_set<int> GetEmpireIDsWithDiplomaticStatusWithEmpire(
        int empire_id, DiplomaticStatus diplo_status) const
    { return GetEmpireIDsWithDiplomaticStatusWithEmpire(empire_id, diplo_status, m_empire_diplomatic_statuses); }

    [[nodiscard]] static boost::container::flat_set<int> GetEmpireIDsWithDiplomaticStatusWithEmpire(
        int empire_id, DiplomaticStatus diplo_status, const DiploStatusMap& statuses);

    [[nodiscard]] bool                      DiplomaticMessageAvailable(int sender_id, int recipient_id) const;
    [[nodiscard]] const DiplomaticMessage&  GetDiplomaticMessage(int sender_id, int recipient_id) const;

    [[nodiscard]] std::string               Dump() const;

    [[nodiscard]] std::shared_ptr<Empire>   GetEmpire(int id);  //!< Returns the empire whose ID is \a id, or nullptr if none exist
    [[nodiscard]] const container_type&     GetEmpires() noexcept { return m_empire_map; }

    [[nodiscard]] const_iterator            begin() const noexcept { return m_const_empire_map.begin(); }
    [[nodiscard]] const_iterator            end() const noexcept { return m_const_empire_map.end(); }
    [[nodiscard]] iterator                  begin() noexcept { return m_empire_map.begin(); }
    [[nodiscard]] iterator                  end() noexcept { return m_empire_map.end(); }

    [[nodiscard]] std::size_t               SizeInMemory() const;

    void BackPropagateMeters() noexcept;

    void SetDiplomaticStatus(int empire1, int empire2, DiplomaticStatus status);
    void HandleDiplomaticMessage(const DiplomaticMessage& message);
    void SetDiplomaticMessage(const DiplomaticMessage& message);
    void RemoveDiplomaticMessage(int sender_id, int recipient_id);

    void ResetDiplomacy();

    void RefreshCapitalIDs();

    /** Creates and inserts an empire with the specified properties and returns
      * a pointer to it.  This will only set up the data in Empire.  It is the
      * caller's responsibility to make sure that universe updates planet
      * ownership. */
    void CreateEmpire(int empire_id, std::string name, std::string player_name,
                      EmpireColor color, bool authenticated);

    /** Removes and deletes all empires from the manager. */
    void Clear() noexcept;

    typedef boost::signals2::signal<void (int, int)> DiploSignalType;

    mutable DiploSignalType DiplomaticStatusChangedSignal;
    mutable DiploSignalType DiplomaticMessageChangedSignal;

private:
    [[nodiscard]] std::string DumpDiplomacy() const;

    void InsertEmpire(std::shared_ptr<Empire>&& empire); //!< Adds the given empire to the manager

    void GetDiplomaticMessagesToSerialize(std::map<std::pair<int, int>, DiplomaticMessage>& messages,
                                          int encoding_empire) const;

    boost::container::flat_set<int>                  m_empire_ids;
    boost::container::flat_set<int>                  m_capital_ids;
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
[[nodiscard]] FO_COMMON_API const std::vector<std::array<uint8_t, 4>>& EmpireColors();

/** Initialize empire colors from \p path */
FO_COMMON_API void InitEmpireColors(const boost::filesystem::path& path);


#endif
