#ifndef _Networking_h_
#define _Networking_h_

#include <string>
#include <bitset>
#include <cstdint>

#include "../util/Enum.h"
#include "../util/Export.h"

namespace Networking {
    FO_COMMON_API extern const std::string DISCOVERY_QUESTION;
    FO_COMMON_API extern const std::string DISCOVERY_ANSWER;
    FO_COMMON_API extern const int SOCKET_LINGER_TIME;
    inline constexpr int INVALID_PLAYER_ID = -1;
    inline constexpr int NO_TEAM_ID = -1;

    FO_COMMON_API int DiscoveryPort();
    FO_COMMON_API int MessagePort();

    FO_ENUM(
        (ClientType),
        ((INVALID_CLIENT_TYPE, -1))
        ((CLIENT_TYPE_AI_PLAYER))
        ((CLIENT_TYPE_HUMAN_PLAYER))
        ((CLIENT_TYPE_HUMAN_OBSERVER))
        ((CLIENT_TYPE_HUMAN_MODERATOR))
        ((NUM_CLIENT_TYPES))
    )

    constexpr auto is_ct = [](const auto& thing, const Networking::ClientType ct) noexcept {
        if constexpr (requires { thing == ct; })
            return thing == ct;
        else if constexpr (requires { thing.client_type == ct; })
            return thing.client_type == ct;
        else if constexpr (requires { thing->client_type == ct; })
            return thing && thing->client_type == ct;
        else if constexpr (requires { thing.GetClientType() == ct; })
            return thing.GetClientType() == ct;
        else if constexpr (requires { thing->GetClientType() == ct; })
            return thing && thing->GetClientType() == ct;
        else
            static_assert(requires { thing == ct; });
    };

    constexpr auto is_ai = [](const auto& thing) noexcept
    { return is_ct(thing, Networking::ClientType::CLIENT_TYPE_AI_PLAYER); };

    constexpr auto is_human = [](const auto& thing) noexcept
    { return is_ct(thing, Networking::ClientType::CLIENT_TYPE_HUMAN_PLAYER); };

    static constexpr auto is_ai_or_human = [](const auto& thing) noexcept
    { return is_ai(thing) || is_human(thing); };

    static constexpr auto is_mod = [](const auto& thing) noexcept
    { return is_ct(thing, Networking::ClientType::CLIENT_TYPE_HUMAN_MODERATOR); };

    static constexpr auto is_obs = [](const auto& thing) noexcept
    { return is_ct(thing, Networking::ClientType::CLIENT_TYPE_HUMAN_OBSERVER); };

    static constexpr auto is_mod_or_obs = [](const auto& thing) noexcept
    { return is_mod(thing) || is_obs(thing); };

    constexpr auto is_invalid = [](const auto& thing) noexcept
    { return is_ct(thing, Networking::ClientType::INVALID_CLIENT_TYPE); };

    enum class RoleType : uint8_t {
        ROLE_HOST = 0,              ///< allows save and load games, edit other player settings, stop server
        ROLE_CLIENT_TYPE_MODERATOR, ///< allows have a client type Moderator
        ROLE_CLIENT_TYPE_PLAYER,    ///< allows have a client type Player
        ROLE_CLIENT_TYPE_OBSERVER,  ///< allows have a client type Observer
        ROLE_GALAXY_SETUP,          ///< allows change galaxy and AI settings in lobby

        Roles_Count
    };

    class FO_COMMON_API AuthRoles {
    public:
        constexpr AuthRoles() = default;
        explicit AuthRoles(std::initializer_list<RoleType> roles) {
            for (RoleType r : roles)
                m_roles.set(std::size_t(r), true);
        }

        void SetRole(RoleType role, bool value = true) { m_roles.set(std::size_t(role), value); }
        void Clear() noexcept { m_roles.reset(); }

        [[nodiscard]] bool HasRole(RoleType role) const { return m_roles.test(std::size_t(role)); }
        [[nodiscard]] std::string Text() const { return m_roles.to_string(); }
        void SetText(const std::string& text) { m_roles = std::bitset<std::size_t(RoleType::Roles_Count)>(text); }

    private:
        std::bitset<int(RoleType::Roles_Count)> m_roles;
    };
}

#endif
