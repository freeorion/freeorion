// -*- C++ -*-
#ifndef _Networking_h_
#define _Networking_h_

#include <string>

#include "../util/Export.h"

namespace Networking {
    FO_COMMON_API extern const std::string DISCOVERY_QUESTION;
    FO_COMMON_API extern const std::string DISCOVERY_ANSWER;
    FO_COMMON_API extern const int DISCOVERY_PORT;
    FO_COMMON_API extern const int MESSAGE_PORT;
    FO_COMMON_API extern const int SOCKET_LINGER_TIME;
    FO_COMMON_API extern const int INVALID_PLAYER_ID;

    enum ClientType {
        INVALID_CLIENT_TYPE = -1,
        CLIENT_TYPE_AI_PLAYER,
        CLIENT_TYPE_HUMAN_PLAYER,
        CLIENT_TYPE_HUMAN_OBSERVER,
        CLIENT_TYPE_HUMAN_MODERATOR,
        NUM_CLIENT_TYPES
    };
}

#endif
