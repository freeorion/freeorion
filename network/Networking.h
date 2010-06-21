// -*- C++ -*-
#ifndef _Networking_h_
#define _Networking_h_

#include <GG/Enum.h>

#include <string>


namespace Networking {
    extern const std::string DISCOVERY_QUESTION;
    extern const std::string DISCOVERY_ANSWER;
    extern const int DISCOVERY_PORT;
    extern const int MESSAGE_PORT;
    extern const int SOCKET_LINGER_TIME;
    extern const int HOST_PLAYER_ID;
    extern const int INVALID_PLAYER_ID;

    enum ClientType {
        INVALID_CLIENT_TYPE = -1,
        CLIENT_TYPE_AI_PLAYER,
        CLIENT_TYPE_HUMAN_PLAYER,
        CLIENT_TYPE_HUMAN_OBSERVER,
        NUM_CLIENT_TYPES
    };
}

#endif
