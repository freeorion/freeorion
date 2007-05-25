#ifndef AI_INTERFACE
#define AI_INTERFACE

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Condition.h"
#include "../util/Order.h"

#include <string>

class AIClientApp;
class Empire;

/* AI logic modules implement this class, and AIClientApps contain one, and call it to generate orders */
class AIBase
{
public:
    virtual void GenerateOrders();  ///< The server has sent a new turn update.  AI should review the new gamestate and send orders for this turn.
    virtual void HandleChatMessage(int sender_id, const std::string& msg);  ///< another player has sent a chat message to this player.  AI can respond or ignore.
};

/* Public interface providing functions that an AI client program can call to get information about the
   gamestate and to interact with it, such as by issuing orders, ending its turn, or sending message to
   other players. */
namespace AIInterface
{
    /** Gamestate Accessors */ //@{
    const Universe&         GetUniverse();                  ///< returns Universe known to this player

    const std::string&      PlayerName();                   ///< returns the player name of this client
    const std::string&      PlayerName(int player_id);      ///< returns the name of player with \a player_id

    int                     PlayerID();                     ///< returns the player ID of this client
    int                     EmpireID();                     ///< returns the empire ID of this client
    const Empire*           GetEmpire();                    ///< returns empire of this client's player
    const Empire*           GetEmpire(int empire_id);       ///< returns empire with id \a empire_id
    const Empire*           GetPlayerEmpire(int player_id); ///< returns empire of player with id \a player_id
    int                     EmpirePlayerID(int empire_id);  ///< returns ID of player controlling empire with id \a empire_id

    int                     CurrentTurn();                  ///< returns the current game turn
    //@}

    /** Order-Giving */ //@{
    int IssueFleetMoveOrder(int fleet_id, int destination_id);
    int IssueRenameOrder();
    int IssueNewFleetOrder(const std::string fleet_name, const std::vector<int>& ship_ids);
    int IssueFleetTransferOrder();
    int IssueFleetColonizeOrder();
    int IssueDeleteFleetOrder();
    int IssueChangeFocusOrder();
    int IssueResearchQueueOrder();
    int IssueProductionQueueOrder();

    void SendPlayerChatMessage(int recipient_player_id, const std::string& message_text);

    void DoneTurn();        ///< AI player is done submitting orders for this turn
    //@}

    /** AI State storage and retrieval */ //@{
    void SaveState();
    void LoadState();
    //@}
};

#endif