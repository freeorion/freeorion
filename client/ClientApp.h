// -*- C++ -*-
#ifndef _ClientApp_h_
#define _ClientApp_h_

#ifndef _ClientEmpire_h_
#include "../Empire/ClientEmpireManager.h"
#endif 

#ifndef _ClientNetworkCore_h_
#include "../network/ClientNetworkCore.h"
#endif

#ifndef _Message_h_
#include "../network/Message.h"
#endif

#ifndef _OrderSet_h_
#include "../util/OrderSet.h"
#endif

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif 

#ifndef _AppInterface_h_
#include "../util/AppInterface.h"
#endif

#include <string>

namespace log4cpp {class Category;};

class CombatModule;
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
    const std::string&   PlayerName() const;       ///< returns the player name of this client
    int                  PlayerID() const;         ///< returns the player ID of this client
    int                  EmpireID() const;         ///< returns the empire ID of this client
    int                  CurrentTurn() const;      ///< returns the current game turn

    /** returns the orders message containing all orders issued so far in the turn; if \a save_game_data is true, additional 
        client-side data may also be included */
    virtual Message      TurnOrdersMessage(bool save_game_data = false) const;
    //@}

    /** \name Mutators */ //@{   
    virtual void         StartTurn();   ///< encodes order sets and sends turn orders message

    Universe&            GetUniverse();      ///< returns client's local copy of Universe
    ClientEmpireManager& Empires();          ///< returns the set of known Empires
    OrderSet&            Orders();           ///< returns Order set for this client's player
    ClientNetworkCore&   NetworkCore();      ///< returns the network core object for this client's player
    MultiplayerLobbyWnd* MultiplayerLobby(); ///< returns the multiplayer lobby window, or 0 if none exists
    CombatModule*        CurrentCombat();    ///< returns this client's currently executing Combat; may be 0
    //@}

    /** handles an incoming message from the server with the appropriate action or response */
    static void          HandleMessage(const Message& msg);

    /** handles the situation in which this client is disconnected from the server unexpectedly */
    static void          HandleServerDisconnect();

    /** returns a universe object ID which can be used for new objects created by the client.
        Can return UniverseObject::INVALID_OBJECT_ID if an ID cannot be created. */
    static int           GetNewObjectID();

    static ClientApp*    GetApp(); ///< returns the singleton ClientApp object

protected:
    /** handles universe and empire data update */
    void UpdateTurnData( const XMLDoc &diff );

    MultiplayerLobbyWnd* m_multiplayer_lobby_wnd;

    Universe                m_universe;
    ClientEmpireManager     m_empires;
    CombatModule*           m_current_combat;
    OrderSet                m_orders;
    ClientNetworkCore       m_network_core;
    std::string             m_player_name;
    int                     m_player_id;
    int                     m_empire_id;
    int                     m_current_turn;

private:
    const ClientApp& operator=(const ClientApp&); // disabled
    ClientApp(const ClientApp&); // disabled
   
    virtual void HandleMessageImpl(const Message& msg) = 0;
    virtual void HandleServerDisconnectImpl() {}

    static ClientApp* s_app; ///< a ClientApp pointer to the singleton instance of the app
};

#endif // _ClientApp_h_
