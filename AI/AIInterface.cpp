#include "AIInterface.h"

#include "../network/ClientNetworking.h"
#include "../client/AI/AIClientApp.h"

#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Planet.h"
#include "../universe/Fleet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Tech.h"
#include "../universe/Enums.h"
#include "../Empire/Empire.h"

#include "../util/Directories.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../util/Order.h"
#include "../util/OrderSet.h"

#include <boost/timer.hpp>
#include <boost/python/str.hpp>
#include <boost/uuid/random_generator.hpp>


#include <map>
#include <stdexcept>
#include <string>


using boost::python::object;
using boost::python::str;

namespace {
    const float DUMMY_FLOAT = 0.0f;
}

//////////////////////////////////
//          AI Base             //
//////////////////////////////////
AIBase::~AIBase()
{}

void AIBase::GenerateOrders()
{ AIInterface::DoneTurn(); }

void AIBase::HandleChatMessage(int sender_id, const std::string& msg)
{}

void AIBase::HandleDiplomaticMessage(const DiplomaticMessage& msg)
{}

void AIBase::HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u)
{}

void AIBase::StartNewGame()
{}

void AIBase::ResumeLoadedGame(const std::string& save_state_string)
{}

const std::string& AIBase::GetSaveStateString() const {
    static std::string default_state_string("AIBase default save state string");
    DebugLogger() << "AIBase::GetSaveStateString() returning: " << default_state_string;
    return default_state_string;
}

void AIBase::SetAggression(int aggr)
{ m_aggression = aggr; }


//////////////////////////////////
//        AI Interface          //
//////////////////////////////////
namespace AIInterface {
    const std::string& PlayerName()
    { return AIClientApp::GetApp()->PlayerName(); }

    const std::string& PlayerName(int player_id) {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it != players.end())
            return it->second.name;
        else {
            DebugLogger() << "AIInterface::PlayerName(" << std::to_string(player_id) << ") - passed an invalid player_id";
            throw std::invalid_argument("AIInterface::PlayerName : given invalid player_id");
        }
    }

    int PlayerID()
    { return AIClientApp::GetApp()->PlayerID(); }

    int EmpireID()
    { return AIClientApp::GetApp()->EmpireID(); }

    int PlayerEmpireID(int player_id) {
        for (auto& entry : AIClientApp::GetApp()->Players()) {
            if (entry.first == player_id)
                return entry.second.empire_id;
        }
        return ALL_EMPIRES; // default invalid value
    }

    std::vector<int>  AllEmpireIDs() {
        std::vector<int> empire_ids;
        for (auto& entry : AIClientApp::GetApp()->Players())
            empire_ids.push_back(entry.second.empire_id);
        return empire_ids;
    }

    const Empire* GetEmpire()
    { return AIClientApp::GetApp()->GetEmpire(AIClientApp::GetApp()->EmpireID()); }

    const Empire* GetEmpire(int empire_id)
    { return AIClientApp::GetApp()->GetEmpire(empire_id); }

    int EmpirePlayerID(int empire_id) {
        int player_id = AIClientApp::GetApp()->EmpirePlayerID(empire_id);
        if (-1 == player_id)
            DebugLogger() << "AIInterface::EmpirePlayerID(" << std::to_string(empire_id) << ") - passed an invalid empire_id";
        return player_id;
    }

    std::vector<int> AllPlayerIDs() {
        std::vector<int> player_ids;
        for (auto& entry : AIClientApp::GetApp()->Players())
            player_ids.push_back(entry.first);
        return player_ids;
    }

    bool PlayerIsAI(int player_id)
    { return AIClientApp::GetApp()->GetPlayerClientType(player_id) == Networking::CLIENT_TYPE_AI_PLAYER; }

    bool PlayerIsHost(int player_id) {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it == players.end())
            return false;
        return it->second.host;
    }

    const GalaxySetupData&  GetGalaxySetupData()
    { return AIClientApp::GetApp()->GetGalaxySetupData(); }

    void InitMeterEstimatesAndDiscrepancies() {
        Universe& universe = AIClientApp::GetApp()->GetUniverse();
        universe.InitMeterEstimatesAndDiscrepancies();
    }

    void UpdateMeterEstimates(bool pretend_to_own_unowned_planets) {
        std::vector<std::shared_ptr<Planet>> unowned_planets;
        int player_id = -1;
        Universe& universe = AIClientApp::GetApp()->GetUniverse();
        if (pretend_to_own_unowned_planets) {
            // Add this player ownership to all planets that the player can see
            // but which aren't currently colonized.  This way, any effects the
            // player knows about that would act on those planets if the player
            // colonized them include those planets in their scope.  This lets
            // effects from techs the player knows alter the max population of
            // planet that is displayed to the player, even if those effects
            // have a condition that causes them to only act on planets the
            // player owns (so as to not improve enemy planets if a player
            // reseraches a tech that should only benefit him/herself).
            player_id = AIInterface::PlayerID();

            // get all planets the player knows about that aren't yet colonized
            // (aren't owned by anyone).  Add this the current player's
            // ownership to all, while remembering which planets this is done
            // to.
            universe.InhibitUniverseObjectSignals(true);
            for (auto& planet : universe.Objects().FindObjects<Planet>()) {
                 if (planet->Unowned()) {
                     unowned_planets.push_back(planet);
                     planet->SetOwner(player_id);
                 }
            }
        }

        // update meter estimates with temporary ownership
        universe.UpdateMeterEstimates();

        if (pretend_to_own_unowned_planets) {
            // remove temporary ownership added above
            for (auto& planet : unowned_planets)
                planet->SetOwner(ALL_EMPIRES);
            universe.InhibitUniverseObjectSignals(false);
        }
    }

    void UpdateResourcePools() {
//        for (auto& entry : AIClientApp::GetApp()->Empires())
//            entry.second->UpdateResourcePools();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = ::GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateResourcePools : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateResourcePools();
    }

    void UpdateResearchQueue() {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = ::GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateResearchQueue : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateResearchQueue();
    }

    void UpdateProductionQueue() {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = ::GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateProductionQueue : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateProductionQueue();
    }

    OrderSet& IssuedOrders()
    { return AIClientApp::GetApp()->Orders(); }

    int IssueEnqueueTechOrder(const std::string& tech_name, int position) {
        const Tech* tech = GetTech(tech_name);
        if (!tech) {
            ErrorLogger() << "IssueEnqueueTechOrder : passed tech_name that is not the name of a tech.";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ResearchQueueOrder>(empire_id, tech_name, position));

        return 1;
    }

    int IssueDequeueTechOrder(const std::string& tech_name) {
        const Tech* tech = GetTech(tech_name);
        if (!tech) {
            ErrorLogger() << "IssueDequeueTechOrder : passed tech_name that is not the name of a tech.";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ResearchQueueOrder>(empire_id, tech_name));

        return 1;
    }

    int IssueEnqueueBuildingProductionOrder(const std::string& item_name, int location_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        if (!empire->ProducibleItem(BT_BUILDING, item_name, location_id)) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : specified item_name and location_id that don't indicate an item that can be built at that location";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, ProductionQueue::ProductionItem(BT_BUILDING, item_name), 1, location_id));

        return 1;
    }

    int IssueEnqueueShipProductionOrder(int design_id, int location_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        if (!empire->ProducibleItem(BT_SHIP, design_id, location_id)) {
            ErrorLogger() << "IssueEnqueueShipProductionOrder : specified design_id and location_id that don't indicate a design that can be built at that location";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, ProductionQueue::ProductionItem(BT_SHIP, design_id), 1, location_id));

        return 1;
    }

    int IssueChangeProductionQuantityOrder(int queue_index, int new_quantity, int new_blocksize) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index outside range of items on queue.";
            return 0;
        }
        if (queue[queue_index].item.build_type != BT_SHIP) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index for a non-ship item.";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, queue_index, new_quantity, new_blocksize));

        return 1;
    }

    int IssueRequeueProductionOrder(int old_queue_index, int new_queue_index) {
        if (old_queue_index == new_queue_index) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed same old and new indexes... nothing to do.";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (old_queue_index < 0 || static_cast<int>(queue.size()) <= old_queue_index) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed old_queue_index outside range of items on queue.";
            return 0;
        }

        // After removing an earlier entry in queue, all later entries are shifted down one queue index, so
        // inserting before the specified item index should now insert before the previous item index.  This
        // also allows moving to the end of the queue, rather than only before the last item on the queue.
        int actual_new_index = new_queue_index;
        if (old_queue_index < new_queue_index)
            actual_new_index = new_queue_index - 1;

        if (new_queue_index < 0 || static_cast<int>(queue.size()) <= actual_new_index) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed new_queue_index outside range of items on queue.";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, old_queue_index, new_queue_index));

        return 1;
    }

    int IssueDequeueProductionOrder(int queue_index) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueDequeueProductionOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, queue_index));

        return 1;
    }

    int IssuePauseProductionOrder(int queue_index, bool pause) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionPauseOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, queue_index, pause, DUMMY_FLOAT));

        return 1;
    }

    int IssueAllowStockpileProductionOrder(int queue_index, bool use_stockpile) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionStockpileOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(empire_id, queue_index, use_stockpile, DUMMY_FLOAT, DUMMY_FLOAT));

        return 1;
    }

    int IssueCreateShipDesignOrder(const std::string& name, const std::string& description,
                                   const std::string& hull, const std::vector<std::string> parts,
                                   const std::string& icon, const std::string& model, bool name_desc_in_stringtable)
    {
        if (name.empty() || description.empty() || hull.empty()) {
            ErrorLogger() << "IssueCreateShipDesignOrderOrder : passed an empty name, description, or hull.";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        int current_turn = CurrentTurn();

        const auto uuid = boost::uuids::random_generator()();

        // create design from stuff chosen in UI
        ShipDesign* design;
        try {
            design = new ShipDesign(std::invalid_argument(""), name, description, current_turn, ClientApp::GetApp()->EmpireID(),
                                    hull, parts, icon, model, name_desc_in_stringtable, false, uuid);

        } catch (const std::invalid_argument&) {
            ErrorLogger() << "IssueCreateShipDesignOrderOrder failed to create a new ShipDesign object";
            return 0;
        }

        auto order = std::make_shared<ShipDesignOrder>(empire_id, *design);
        AIClientApp::GetApp()->Orders().IssueOrder(order);
        delete design;

        return 1;
    }

    void SendPlayerChatMessage(int recipient_player_id, const std::string& message_text) {
        AIClientApp::GetApp()->Networking().SendMessage(PlayerChatMessage(message_text, recipient_player_id));
    }

    void SendDiplomaticMessage(const DiplomaticMessage& diplo_message) {
        AIClientApp* app = AIClientApp::GetApp();
        if (!app) return;
        int sender_player_id = app->PlayerID();
        if (sender_player_id == Networking::INVALID_PLAYER_ID) return;
        int recipient_empire_id = diplo_message.RecipientEmpireID();
        int recipient_player_id = app->EmpirePlayerID(recipient_empire_id);
        if (recipient_player_id == Networking::INVALID_PLAYER_ID) return;
        app->Networking().SendMessage(DiplomacyMessage(diplo_message));
    }

    void DoneTurn() {
        DebugLogger() << "AIInterface::DoneTurn()";
        AIClientApp* app = AIClientApp::GetApp();
        app->StartTurn(app->GetAI()->GetSaveStateString()); // encodes order sets and sends turn orders message.  "done" the turn for the client, but "starts" the turn for the server
    }

    void LogOutput(const std::string& log_text)
    { DebugLogger() << log_text; }

    void ErrorOutput(const std::string& error_text)
    { ErrorLogger() << error_text; }
} // namespace AIInterface
