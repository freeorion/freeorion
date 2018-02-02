#include "Networking.h"
#include "../util/OptionsDB.h"
#include "../util/i18n.h"

namespace {
    void AddOptions(OptionsDB& db) {
        db.Add("network.discovery.port",    UserStringNop("OPTIONS_DB_NETWORK_DISCOVERY_PORT"), 12345,  RangedValidator<int>(1025, 65535));
        db.Add("network.message.port",      UserStringNop("OPTIONS_DB_NETWORK_MESSAGE_PORT"),   12346,  RangedValidator<int>(1025, 65535));
    }
    bool temp_bool = RegisterOptions(&AddOptions);
}

namespace Networking {
    const std::string DISCOVERY_QUESTION = "Yo, can I play Free-O here, dog?";
    const std::string DISCOVERY_ANSWER = "Word!";
#ifdef FREEORION_OPENBSD
    // Needs to set shorter linger time on OpenBSD to be able to start the session
    const int SOCKET_LINGER_TIME = 1 << (sizeof(unsigned short) * 4 - 1);
#else
    const int SOCKET_LINGER_TIME = 1 << (sizeof(unsigned short) * 8 - 1);
#endif
    const int INVALID_PLAYER_ID = -1;

    int DiscoveryPort()
    { return GetOptionsDB().Get<int>("network.discovery.port"); }
    int MessagePort()
    { return GetOptionsDB().Get<int>("network.message.port"); }

    AuthRoles::AuthRoles(const std::initializer_list<RoleType>& roles) {
       for (RoleType r : roles) {
           m_roles.set(r, true);
       }
    }

    void AuthRoles::SetRole(RoleType role, bool value)
    { m_roles.set(role, value); }

    void AuthRoles::Clear()
    { m_roles = std::bitset<Roles_Count>(); }

    bool AuthRoles::HasRole(RoleType role) const
    { return m_roles.test(role); }

    std::string AuthRoles::Text() const
    { return m_roles.to_string(); }

    void AuthRoles::SetText(const std::string& text)
    { m_roles = std::bitset<Roles_Count>(text); }
}
