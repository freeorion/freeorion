// -*- C++ -*-
#ifndef _ClientApp_h_
#define _ClientApp_h_

#include "../Empire/EmpireManager.h"
#include "../network/ClientNetworking.h"
#include "../universe/Universe.h"
#include "../util/OrderSet.h"
#include "../util/AppInterface.h"
#include "../util/MultiplayerCommon.h"

/** The abstract base class for the application framework classes AIClientApp
  * and HumanClientApp.  The static functions are designed to give both types
  * of client (which are very different) a unified interface.  This allows code
  * in either type of client app to handle Messages and gain access to the data
  * structures common to both apps, without worrying about which type of app the
  * code is being run in.*/
class ClientApp : public IApp {
public:
    /** \name Structors */ //@{
    ClientApp();
    virtual ~ClientApp();
    //@}

    /** \name Accessors */ //@{
    int                     PlayerID() const;         ///< returns the player ID of this client
    int                     EmpireID() const;         ///< returns the empire ID of this client
    int                     CurrentTurn() const;      ///< returns the current game turn

    int                     EmpirePlayerID(int empire_id) const;///< returns the player ID for the player playing the empire with ID \a empire_id

    const std::map<int, PlayerInfo>&Players() const;            ///< returns the map, indexed by player ID, of PlayerInfo structs containing info about players in the game
    const std::map<int, Message::PlayerStatus>&
                                    PlayerStatus() const;       ///< returns the map, indexed by player ID, of the latest known PlayerStatus for each player in the game

    const Universe&                 GetUniverse() const;        ///< returns client's local copy of Universe
    const GalaxySetupData&          GetGalaxySetupData() const; ///< returns the settings used in creating the current Universe
    const OrderSet&                 Orders() const;             ///< returns Order set for this client's player
    const CombatOrderSet&           CombatOrders() const;       ///< returns CombatOrder set for this client's player
    const ClientNetworking&         Networking() const;         ///< returns the networking object for this client's player
    const Networking::ClientType    GetEmpireClientType(int empire_id) const;   ///< returns the networking client type for the given empire_id
    const Networking::ClientType    GetPlayerClientType(int player_id) const;   ///< returns the networking client type for the given player_id
    const Networking::ClientType    GetClientType() const;                      ///< returns the networking client type for this client's player

    std::string                     GetVisibleObjectName(TemporaryPtr<const UniverseObject> object);
    //@}

    /** \name Mutators */ //@{
    virtual void                StartTurn();        ///< encodes order sets and sends turn orders message
    virtual void                SendCombatSetup();  ///< encodes and sends combat setup orders message
    virtual void                StartCombatTurn();  ///< encodes combat order sets and sends combat turn orders message

    Universe&                   GetUniverse();  ///< returns client's local copy of Universe
    GalaxySetupData&            GetGalaxySetupData();
    EmpireManager&              Empires();      ///< returns the set of known Empires
    TemporaryPtr<UniverseObject>GetUniverseObject(int object_id);
    ObjectMap&                  EmpireKnownObjects(int empire_id); ///< returns the server's map for known objects of specified empire. */
    TemporaryPtr<UniverseObject>EmpireKnownObject(int object_id, int empire_id);
    OrderSet&                   Orders();       ///< returns Order set for this client's player
    CombatOrderSet&             CombatOrders(); ///< returns CombatOrder set for this client's player
    ClientNetworking&           Networking();   ///< returns the networking object for this client's player
    std::map<int, PlayerInfo>&  Players();      ///< returns the map, indexed by player ID, of PlayerInfo structs containing info about players in the game
    std::map<int, Message::PlayerStatus>&
                                PlayerStatus(); ///< returns the map, indexed by player ID, of the latest known PlayerStatus for each player in the game

    void SetEmpireID(int id);                   ///< sets the empire ID of this client
    void SetCurrentTurn(int turn);              ///< sets the current game turn
    void SetSinglePlayerGame(bool sp = true);   ///< sets whether the current game is single player (sp = true) or multiplayer (sp = false)
    void SetPlayerStatus(int player_id, Message::PlayerStatus status);

    /** returns a universe object ID which can be used for new objects created by the client.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    int                     GetNewObjectID();

    /** returns a design ID which can be used for a new design to uniquely identify it.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    int                     GetNewDesignID();

    /** Emitted when a player is eliminated; in many places in the code, empires
        are refered to by ID.  This allows such places to listen for
        notification that one of these IDs has become invalidated.*/
    mutable boost::signals2::signal<void (int)> EmpireEliminatedSignal;

    static ClientApp*       GetApp(); ///< returns the singleton ClientApp object

protected:
    Universe                    m_universe;
    GalaxySetupData             m_galaxy_setup_data;
    EmpireManager               m_empires;
    OrderSet                    m_orders;
    CombatOrderSet              m_combat_orders;
    ClientNetworking            m_networking;
    int                         m_empire_id;
    int                         m_current_turn;
    std::map<int, PlayerInfo>   m_player_info;      ///< indexed by player id, contains info about all players in the game
    std::map<int, Message::PlayerStatus>
                                m_player_status;    ///< indexed by player id, the last known PlayerStatus for each player

private:
    const ClientApp& operator=(const ClientApp&); // disabled
    ClientApp(const ClientApp&); // disabled
};

#endif // _ClientApp_h_
