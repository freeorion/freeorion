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
    const int SOCKET_LINGER_TIME = 1 << (sizeof(unsigned short) * 8 - 1);
    const int INVALID_PLAYER_ID = -1;

    int DiscoveryPort()
    { return GetOptionsDB().Get<int>("network.discovery.port"); }
    int MessagePort()
    { return GetOptionsDB().Get<int>("network.message.port"); }
}
