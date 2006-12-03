// -*- C++ -*-
#ifndef _Networking_h_
#define _Networking_h_

#include <GG/Enum.h>

#include <string>


/** the states the server may be in at various points during its execution*/
enum ServerState {
    SERVER_IDLE,         ///< there is no game yet and no one has send a HOST_*_GAME Message yet; this is the initial state
    SERVER_MP_LOBBY,     ///< the host and possibly other players are in the multiplayer lobby, preparing to start a game
    SERVER_GAME_SETUP,   ///< a HOST_*_GAME Message has been received, and a game is being set up (the server is waiting for all players to join)
    SERVER_WAITING,      ///< a game is in progress and currently the server is waiting for players to finish their turns
    SERVER_PROCESSING,   ///< the server is processing a turn
    SERVER_DISCONNECT,   ///< the server has encountered a disconnect error and is dealing with it
    SERVER_DYING         ///< the server is ending its execution
};

namespace Networking {
    extern const std::string DISCOVERY_QUESTION;
    extern const std::string DISCOVERY_ANSWER;
    extern const int DISCOVERY_PORT;
    extern const int MESSAGE_PORT;
    extern const int SOCKET_LINGER_TIME;
    extern const int HOST_PLAYER_ID;
}

namespace GG {
    GG_ENUM_MAP_BEGIN(ServerState)
    GG_ENUM_MAP_INSERT(SERVER_IDLE)
    GG_ENUM_MAP_INSERT(SERVER_MP_LOBBY)
    GG_ENUM_MAP_INSERT(SERVER_GAME_SETUP)
    GG_ENUM_MAP_INSERT(SERVER_WAITING)
    GG_ENUM_MAP_INSERT(SERVER_PROCESSING)
    GG_ENUM_MAP_INSERT(SERVER_DISCONNECT)
    GG_ENUM_MAP_INSERT(SERVER_DYING)
    GG_ENUM_MAP_END
}
GG_ENUM_STREAM_IN(ServerState)
GG_ENUM_STREAM_OUT(ServerState)

#endif
