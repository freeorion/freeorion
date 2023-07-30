#ifndef _ClientAppFixture_h_
#define _ClientAppFixture_h_

#include "client/ClientApp.h"

class ClientAppFixture : public ClientApp {
public:
    ClientAppFixture();

    bool PingLocalHostServer();
    bool ConnectToLocalHostServer();
    bool ConnectToServer(const std::string& ip_address);
    void DisconnectFromServer();

    void HostSPGame(unsigned int num_AIs);
    void JoinGame();

    bool ProcessMessages(const boost::posix_time::ptime& start_time, int max_seconds);
    bool HandleMessage(Message& msg);
    void SaveGame();
    void UpdateLobby();

    unsigned int GetLobbyAICount() const;

    int EffectsProcessingThreads() const override;
protected:
    std::set<int>        m_ai_empires;             ///< Ids of AI empires in game.
    std::set<int>        m_ai_waiting;             ///< Ids of AI empires not yet send orders.
    boost::uuids::uuid   m_cookie;                 ///< Cookie from server login.
    MultiplayerLobbyData m_lobby_data;             ///< Lobby data.
    bool                 m_lobby_updated = false;  ///< Did player get updated lobby.
    bool                 m_game_started = false;   ///< Is server started the game?
    bool                 m_turn_done = false;      ///< Is server processed turn?
    bool                 m_save_completed = false; ///< Is server saved game?
};

constexpr static int MAX_WAITING_SEC = 120;

#endif
