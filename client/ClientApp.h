// -*- C++ -*-
#ifndef _ClientApp_h_
#define _ClientApp_h_

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif 

#ifndef _ClientEmpire_h_
#include "../Empire/ClientEmpireManager.h"
#endif 

#ifndef _OrderSet_h_
#include "../util/OrderSet.h"
#endif

#ifndef _ClientNetworkCore_h_
#include "../network/ClientNetworkCore.h"
#endif

#include <string>

namespace log4cpp { class Category;};

class CombatModule;
class Message;
class MultiplayerLobbyWnd;

/** the abstract base class for the application framework classes AIClientApp and HumanClientApp.  The static functions
    are designed to give both types of client (which are very different) a unified interface.  This allows code in either
    type of client app to handle Messages and gain access to the data structures common to both apps, without worrying 
    about which type of app the code is being run in.  This allows for a more modular design.*/
class ClientApp
{
public:
    /** \name Structors */ //@{   
    ClientApp();
    virtual ~ClientApp();
    //@}
   
    /** \name Accessors */ //@{   
    const std::string&   PlayerName() const {return m_player_name;}   ///< returns the player name of this client
    int                  PlayerID() const {return m_player_id;}       ///< returns the player ID of this client
    //@}

    /** handles an incoming message from the server with the appropriate action or response */
    static void                   HandleMessage(const Message& msg);

    static MultiplayerLobbyWnd*   MultiplayerLobby(); ///< returns the multiplayer lobby window, or 0 if none exists
    static Universe&              GetUniverse();       ///< returns client's local copy of Universe
    static ClientEmpireManager&   Empire();         ///< returns this client's player's Empire
    static CombatModule*          CurrentCombat();  ///< returns this client's currently executing Combat; may be 0
    static OrderSet&              Orders();         ///< returns Order set for this client's player
    static ClientNetworkCore&     NetworkCore();    ///< returns the network core object for this client's player
    

protected:

    Universe       m_universe;

    MultiplayerLobbyWnd* m_multiplayer_lobby_wnd;

    ClientEmpireManager  m_empire;
    CombatModule*        m_current_combat;
    OrderSet             m_orders;
    ClientNetworkCore    m_network_core;
   
    std::string          m_player_name;
    int                  m_player_id;
   
private:
    const ClientApp& operator=(const ClientApp&); // disabled
    ClientApp(const ClientApp&); // disabled
   
    /** handles an incoming message from the server with the appropriate action or response */
    virtual void HandleMessageImpl(const Message& msg) = 0;

    static ClientApp* s_app; ///< a ClientApp pointer to the singleton instance of the app
};

#endif // _ClientApp_h_

