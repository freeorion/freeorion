#include "AIInterface.h"

#include "../util/MultiplayerCommon.h"
#include "../network/ClientNetworkCore.h"
#include "../client/AI/AIClientApp.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"


#include "../util/OrderSet.h"

#include <stdexcept>
#include <string>
#include <map>

//////////////////////////////////
//          AI Base             //
//////////////////////////////////
void AIBase::GenerateOrders()
{
    AIInterface::DoneTurn();
}

void AIBase::HandleChatMessage(int sender_id, const std::string& msg)
{}

//////////////////////////////////
//        AI Interface          //
//////////////////////////////////
namespace AIInterface {
    const Universe& GetUniverse()
    {
        return AIClientApp::GetApp()->GetUniverse();
    }

    const std::string& PlayerName()
    {
        return AIClientApp::GetApp()->PlayerName();
    }

    const std::string& PlayerName(int player_id)
    {
        const std::map<int, PlayerInfo>& players = AIClientApp::GetApp()->Players();
        std::map<int, PlayerInfo>::const_iterator it = players.find(player_id);
        if (it != players.end())
            return it->second.name;
        else
            throw std::invalid_argument("AIInterface::PlayerName : given invalid player_id");
    }

    int PlayerID()
    {
        return AIClientApp::GetApp()->PlayerID();
    }

    int EmpireID()
    {
        return AIClientApp::GetApp()->EmpireID();
    }

    const Empire* GetEmpire()
    {
        return AIClientApp::GetApp()->Empires().Lookup(AIClientApp::GetApp()->EmpireID());
    }

    const Empire* GetEmpire(int empire_id)
    {
        return AIClientApp::GetApp()->Empires().Lookup(empire_id);
    }

    int EmpirePlayerID(int empire_id)
    {
        return AIClientApp::GetApp()->GetEmpirePlayerID(empire_id);
    }

    const Empire* GetPlayerEmpire(int player_id)
    {
        return AIClientApp::GetApp()->GetPlayerEmpire(player_id);
    }

    int CurrentTurn()
    {
        return AIClientApp::GetApp()->CurrentTurn();
    }

    int IssueFleetMoveOrder(int fleet_id, int destination_id)
    {
        Universe& universe = AIClientApp::GetApp()->GetUniverse();
        
        const Fleet* fleet = universe.Object<Fleet>(fleet_id);
        if (!fleet) {
            Logger().errorStream() << "AIInterface::IssueFleetMoveOrder : passed an invalid fleet_id";
            return 0;
        }
        
        int empire_id = AIClientApp::GetApp()->EmpireID();
        if (!fleet->WhollyOwnedBy(empire_id)) {
            Logger().errorStream() << "AIInterface::IssueFleetMoveOrder : passed fleet_id of fleet not owned only by player";
            return 0;
        }

        int start_id = fleet->SystemID();
        if (start_id == UniverseObject::INVALID_OBJECT_ID)
            start_id = fleet->NextSystemID();

        AIClientApp::GetApp()->Orders().IssueOrder(new FleetMoveOrder(empire_id, fleet_id, start_id, destination_id));

        return 1;
    }

    int IssueRenameOrder()
    {
        return 0;
    }

    int IssueNewFleetOrder(const std::string fleet_name, const std::vector<int>& ship_ids)
    {
        if (ship_ids.empty()) {
            Logger().errorStream() << "AIInterface::IssueNewFleetOrder : passed empty vector of ship_ids";
            return 0;
        }

        Universe& universe = AIClientApp::GetApp()->GetUniverse();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Ship* ship = 0;
        
        // make sure all ships exist and are owned just by this player       
        for (std::vector<int>::const_iterator it = ship_ids.begin(); it != ship_ids.end(); ++it) {
            ship = universe.Object<Ship>(*it);
            if (!ship) {
                Logger().errorStream() << "AIInterface::IssueNewFleetOrder : passed an invalid ship_id";
                return 0;
            }
            if (!ship->WhollyOwnedBy(empire_id)) {
                Logger().errorStream() << "AIInterface::IssueNewFleetOrder : passed ship_id of ship not owned only by player";
                return 0;
            }
        }

        // make sure all ships are at the same location
        System* system = ship->GetSystem();
        int system_id = ship->SystemID();
        double ship_x = ship->X();
        double ship_y = ship->Y();
        if (system_id != UniverseObject::INVALID_OBJECT_ID) {
            // ships are located in a system: can just check that all ships have same system id as first ship
            std::vector<int>::const_iterator it = ship_ids.begin();
            for (++it; it != ship_ids.end(); ++it) {
                Ship* ship2 = universe.Object<Ship>(*it);
                if (ship2->SystemID() != system_id) {
                    Logger().errorStream() << "AIInterface::IssueNewFleetOrder : passed ship_ids of ships at different locations";
                    return 0;
                }
            }
        } else {
            // ships are located in deep space: need to check their exact locations
            std::vector<int>::const_iterator it = ship_ids.begin();
            for (++it; it != ship_ids.end(); ++it) {
                Ship* ship2 = universe.Object<Ship>(*it);
                if ((ship2->X() != ship_x) || (ship2->Y() != ship_y)) {
                    Logger().errorStream() << "AIInterface::IssueNewFleetOrder : passed ship_ids of ships at different locations";
                    return 0;
                }
            }
        }

        int new_fleet_id = ClientApp::GetNewObjectID();
        if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) 
            throw std::runtime_error("Couldn't get new object ID when transferring ship to new fleet");

        if (system)
            AIClientApp::GetApp()->Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, system->ID(), ship_ids));
        else
            AIClientApp::GetApp()->Orders().IssueOrder(new NewFleetOrder(empire_id, fleet_name, new_fleet_id, ship_x, ship_y, ship_ids));
        
        return 1;
    }

    int IssueFleetTransferOrder()
    {
        return 0;
    }

    int IssueFleetColonizeOrder()
    {
        return 0;
    }

    int IssueDeleteFleetOrder()
    {
        return 0;
    }

    int IssueChangeFocusOrder()
    {
        return 0;
    }

    int IssueResearchQueueOrder()
    {
        return 0;
    }

    int IssueProductionQueueOrder()
    {
        return 0;
    }

    void SendPlayerChatMessage(int recipient_player_id, const std::string& message_text)
    {
        if (recipient_player_id == -1)
            AIClientApp::GetApp()->NetworkCore().SendMessage(GlobalChatMessage(PlayerID(), message_text));
        else
            AIClientApp::GetApp()->NetworkCore().SendMessage(SingleRecipientChatMessage(PlayerID(), recipient_player_id, message_text));
    }

    void DoneTurn()
    {
        Logger().debugStream() << "AIInterface::DoneTurn()";
        AIClientApp::GetApp()->StartTurn(); // encodes order sets and sends turn orders message.  "done" the turn for the client, but "starts" the turn for the server
    }

    void SaveState()
    {}

    void LoadState()
    {}

    void LogOutput(const std::string& log_text)
    {
        Logger().debugStream() << "AI Log : " << log_text;
    }
} // namespace AIInterface