#ifndef AI_INTERFACE
#define AI_INTERFACE

#include "../universe/Universe.h"

#include <string>

class AIClientApp;
class Empire;
class Tech;

/* AI logic modules implement this class, and AIClientApps contain one, and call it to generate orders */
class AIBase
{
public:
    virtual ~AIBase();

    virtual void                GenerateOrders();                                           ///< Called when the server has sent a new turn update.  AI should review the new gamestate and send orders for this turn.
    virtual void                HandleChatMessage(int sender_id, const std::string& msg);   ///< Called when another player sends a chat message to this player.  AI can respond or ignore.
    virtual void                StartNewGame();                                             ///< Called when a new game (not loaded) is started.  AI should clear its state and prepare to start a new game
    virtual void                ResumeLoadedGame(const std::string& save_state_string);     ///< Called when a game is loaded from save.  AI should extract any state information stored in \a save_state_string so as to be able to continue generating orders when asked to do so
    virtual const std::string&  GetSaveStateString();                                       ///< Called when the server is saving the game.  AI should store any state information it will need to resume at a later time, and return this information in the save_state_string
};

/* Public interface providing relatively easy-to use and somewhat conveniently grouped-together functions that
   a class that implements AIBase can call to get information from the AIClientApp about the gamestate, and which
   can be used to interat with the gamestate by issueing orders, ending the AI player's turn, or sending message
   to other players. */
namespace AIInterface
{
    /** Gamestate Accessors */ //@{
    const std::string&      PlayerName();                   ///< returns the player name of this client
    const std::string&      PlayerName(int player_id);      ///< returns the name of player with \a player_id

    int                     PlayerID();                     ///< returns the player ID of this client
    int                     EmpirePlayerID(int empire_id);  ///< returns ID of player controlling empire with id \a empire_id
    std::vector<int>        AllPlayerIDs();                 ///< returns vector containing IDs of all players in game

    bool                    PlayerIsAI(int player_id);      ///< returns true iff the player with id \a player_id is an AI
    bool                    PlayerIsHost(int player_id);    ///< returns true iff the player with id \a player_id is the game host

    int                     EmpireID();                     ///< returns the empire ID of this client
    int                     PlayerEmpireID(int player_id);  ///< returns ID of empire controlled by player with id \a player_id
    std::vector<int>        AllEmpireIDs();                 ///< returns vector containing IDs of all empires in game

    const Empire*           GetEmpire();                    ///< returns empire of this client's player
    const Empire*           GetEmpire(int empire_id);       ///< returns empire with id \a empire_id

    const Universe&         GetUniverse();                  ///< returns Universe known to this player

    const Tech*             GetTech(const std::string& tech_name);  ///< returns Tech with name \a name

    int                     CurrentTurn();                  ///< returns the current game turn
    //@}

    /** Order-Giving */ //@{
    int                     IssueFleetMoveOrder(int fleet_id, int destination_id);
    int                     IssueRenameOrder(int object_id, const std::string& new_name);
    int                     IssueNewFleetOrder(const std::string& fleet_name, const std::vector<int>& ship_ids);
    int                     IssueFleetTransferOrder();
    int                     IssueFleetColonizeOrder(int ship_id, int planet_id);
    int                     IssueDeleteFleetOrder();
    int                     IssueChangeFocusOrder(int planet_id, FocusType focus_type, bool primary);
    int                     IssueEnqueueTechOrder(const std::string& tech_name, int position);
    int                     IssueDequeueTechOrder(const std::string& tech_name);
    int                     IssueEnqueueProductionOrder(BuildType build_type, const std::string& item_name, int location_id);
    int                     IssueEnqueueProductionOrder(BuildType build_type, int design_id, int location_id);
    int                     IssueRequeueProductionOrder(int old_queue_index, int new_queue_index);
    int                     IssueDequeueProductionOrder(int queue_index);

    void                    SendPlayerChatMessage(int recipient_player_id, const std::string& message_text);

    void                    DoneTurn();        ///< AI player is done submitting orders for this turn
    //@}

    /** Logging */ //@{
    void                    LogOutput(const std::string& log_text);     ///< output text to as DEBUG
    void                    ErrorOutput(const std::string& log_text);   ///< output text to as ERROR
    //@}
};

#endif
