#ifndef _Networking_h_
#define _Networking_h_

#include <string>
#include <bitset>

#include "../util/Export.h"

namespace Networking {
    FO_COMMON_API extern const std::string DISCOVERY_QUESTION;
    FO_COMMON_API extern const std::string DISCOVERY_ANSWER;
    FO_COMMON_API extern const int SOCKET_LINGER_TIME;
    FO_COMMON_API extern const int INVALID_PLAYER_ID;
    FO_COMMON_API extern const int NO_TEAM_ID;

    FO_COMMON_API int DiscoveryPort();
    FO_COMMON_API int MessagePort();

    enum class ClientType : int {
        INVALID_CLIENT_TYPE = -1,
        CLIENT_TYPE_AI_PLAYER,
        CLIENT_TYPE_HUMAN_PLAYER,
        CLIENT_TYPE_HUMAN_OBSERVER,
        CLIENT_TYPE_HUMAN_MODERATOR,
        NUM_CLIENT_TYPES
    };
    FO_COMMON_API std::ostream& operator<<(std::ostream& os, ClientType client_type);

    enum class RoleType : size_t {  // index in bitset so not enum class
        ROLE_HOST = 0,              ///< allows save and load games, edit other player settings, stop server
        ROLE_CLIENT_TYPE_MODERATOR, ///< allows have a client type Moderator
        ROLE_CLIENT_TYPE_PLAYER,    ///< allows have a client type Player
        ROLE_CLIENT_TYPE_OBSERVER,  ///< allows have a client type Observer
        ROLE_GALAXY_SETUP,          ///< allows change galaxy and AI settings in lobby

        Roles_Count
    };

    class FO_COMMON_API AuthRoles {
    public:
        AuthRoles() = default;

        AuthRoles(const std::initializer_list<RoleType>& roles);

        void SetRole(RoleType role, bool value = true);
        void Clear();

        bool HasRole(RoleType role) const;

        std::string Text() const;
        void SetText(const std::string& text);
    private:
        std::bitset<int(RoleType::Roles_Count)> m_roles;
    };

}

#endif
