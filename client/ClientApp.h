// -*- C++ -*-
#ifndef _ClientApp_h_
#define _ClientApp_h_

#include "../Empire/EmpireManager.h"
#include "../network/ClientNetworking.h"
#include "../network/Message.h"
#include "../universe/Universe.h"
#include "../util/AppInterface.h"
#include "../util/OrderSet.h"

#include <string>

namespace log4cpp {class Category;};

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
    const std::string&      PlayerName() const;  ///< returns the player name of this client
    int                     PlayerID() const;    ///< returns the player ID of this client
    int                     EmpireID() const;    ///< returns the empire ID of this client
    int                     CurrentTurn() const; ///< returns the current game turn

    const Universe&         GetUniverse() const; ///< returns client's local copy of Universe
    const EmpireManager&    Empires() const;     ///< returns the set of known Empires
    const OrderSet&         Orders() const;      ///< returns Order set for this client's player
    const ClientNetworking& Networking() const;  ///< returns the networking object for this client's player
    //@}

    /** \name Mutators */ //@{
    virtual void            StartTurn();         ///< encodes order sets and sends turn orders message

    Universe&               GetUniverse();       ///< returns client's local copy of Universe
    EmpireManager&          Empires();           ///< returns the set of known Empires
    OrderSet&               Orders();            ///< returns Order set for this client's player
    ClientNetworking&       Networking();        ///< returns the networking object for this client's player

    /** returns a universe object ID which can be used for new objects created by the client.
        Can return UniverseObject::INVALID_OBJECT_ID if an ID cannot be created. */
    int                     GetNewObjectID();
    //@}

    static ClientApp*       GetApp();            ///< returns the singleton ClientApp object

protected:
    /** \name Mutators */ //@{
    void SetPlayerName(const std::string& name);        ///< sets the player name of this client
    void SetPlayerID(int id);                           ///< sets the player ID of this client
    void SetEmpireID(int id);                           ///< sets the empire ID of this client
    void SetCurrentTurn(int turn);                      ///< sets the current game turn
    int& EmpireIDRef();                                 ///< returns the empire ID of this client
    int& CurrentTurnRef();                              ///< returns the current game turn
    //@}

private:
    const ClientApp& operator=(const ClientApp&); // disabled
    ClientApp(const ClientApp&); // disabled

    Universe                m_universe;
    EmpireManager           m_empires;
    OrderSet                m_orders;
    ClientNetworking        m_networking;
    std::string             m_player_name;
    int                     m_player_id;
    int                     m_empire_id;
    int                     m_current_turn;

    static ClientApp* s_app; ///< a ClientApp pointer to the singleton instance of the app
};

#endif // _ClientApp_h_
