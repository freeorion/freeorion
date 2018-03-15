#ifndef _ClientAppFixture_h_
#define _ClientAppFixture_h_

#include "client/ClientApp.h"

class ClientAppFixture : public ClientApp {
public:
    ClientAppFixture();

    bool PingLocalHostServer();

    bool ConnectToLocalHostServer();

    void HostSPGame(unsigned int num_AIs);

    bool ProcessMessages(const boost::posix_time::ptime& start_time, int max_seconds);

    bool HandleMessage(Message& msg);

    void SendTurnOrders();

    int EffectsProcessingThreads() const;
protected:
    bool          m_game_started; ///< Is server started the game?
    std::set<int> m_ai_players; ///< Ids of AI players in game.
    std::set<int> m_ai_waiting; ///< Ids of AI players not yet send orders.
};

#endif // _ClientAppFixture_h_

