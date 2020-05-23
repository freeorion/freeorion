#include "AIWrapper.h"

#include "AIClientApp.h"
#include "../ClientNetworking.h"
#include "../../universe/Planet.h"
#include "../../universe/ShipDesign.h"
#include "../../universe/Tech.h"
#include "../../universe/Universe.h"
#include "../../util/AppInterface.h"
#include "../../util/Directories.h"
#include "../../util/Logger.h"
#include "../../util/i18n.h"
#include "../../util/MultiplayerCommon.h"
#include "../../util/OptionsDB.h"
#include "../../util/OrderSet.h"
#include "../../util/Order.h"
#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"
#include "../../Empire/Diplomacy.h"
#include "../../python/SetWrapper.h"
#include "../../python/CommonWrappers.h"

#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/python/list.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/scope.hpp>
#include <boost/uuid/random_generator.hpp>

using boost::python::class_;
using boost::python::def;
using boost::python::no_init;
using boost::noncopyable;
using boost::python::return_value_policy;
using boost::python::copy_const_reference;
using boost::python::reference_existing_object;
using boost::python::return_by_value;
using boost::python::return_internal_reference;

using boost::python::vector_indexing_suite;
using boost::python::map_indexing_suite;

using boost::python::object;
using boost::python::import;
using boost::python::exec;
using boost::python::dict;
using boost::python::list;
using boost::python::extract;


////////////////////////
// Python AIWrapper //
////////////////////////
namespace {
    // static string to save AI state
    static std::string s_save_state_string("");

    const std::string& PlayerName()
    { return AIClientApp::GetApp()->PlayerName(); }

    /** @brief Return the player name of the client identified by @a player_id
     *
     * @param player_id An client identifier.
     *
     * @return An UTF-8 encoded and NUL terminated string containing the player
     *      name of this client or an empty string the player is not known or
     *      does not exist.
     */
    const std::string& PlayerNameByID(int player_id) {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it != players.end())
            return it->second.name;
        else {
            DebugLogger() << "AIWrapper::PlayerName(" << std::to_string(player_id) << ") - passed an invalid player_id";
            throw std::invalid_argument("AIWrapper::PlayerName : given invalid player_id");
        }
    }

    int PlayerID()
    { return AIClientApp::GetApp()->PlayerID(); }

    int EmpirePlayerID(int empire_id) {
        int player_id = AIClientApp::GetApp()->EmpirePlayerID(empire_id);
        if (Networking::INVALID_PLAYER_ID == player_id)
            DebugLogger() << "AIWrapper::EmpirePlayerID(" << empire_id << ") - passed an invalid empire_id";
        return player_id;
    }

    /** @brief Return all player identifiers that are in game
     *
     * @return A vector containing the identifiers of all players.
     */
    std::vector<int> AllPlayerIDs() {
        std::vector<int> player_ids;
        for (auto& entry : AIClientApp::GetApp()->Players())
            player_ids.push_back(entry.first);
        return player_ids;
    }

    /** @brief Return if the player identified by @a player_id is an AI
     *
     * @param player_id An client identifier.
     *
     * @return True if the player is an AI, false if not.
     */
    bool PlayerIsAI(int player_id)
    { return AIClientApp::GetApp()->GetPlayerClientType(player_id) == Networking::CLIENT_TYPE_AI_PLAYER; }

    /** @brief Return if the player identified by @a player_id is the game
     *      host
     *
     * @param player_id An client identifier.
     *
     * @return True if the player is the game host, false if not.
     */
    bool PlayerIsHost(int player_id) {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it == players.end())
            return false;
        return it->second.host;
    }

    int EmpireID()
    { return AIClientApp::GetApp()->EmpireID(); }

    /** @brief Return the empire identifier of the empire @a player_id controls
     *
     * @param player_id An client identifier.
     *
     * @return An empire identifier.
     */
    int PlayerEmpireID(int player_id) {
        for (auto& entry : AIClientApp::GetApp()->Players()) {
            if (entry.first == player_id)
                return entry.second.empire_id;
        }
        return ALL_EMPIRES; // default invalid value
    }

    /** @brief Return all empire identifiers that are in game
     *
     * @return A vector containing the identifiers of all empires.
     */
    std::vector<int> AllEmpireIDs() {
        std::vector<int> empire_ids;
        for (auto& entry : AIClientApp::GetApp()->Players()) {
            auto empire_id = entry.second.empire_id;
            if (empire_id != ALL_EMPIRES)
                empire_ids.push_back(empire_id);
        }
        return empire_ids;
    }

    /** @brief Return the ::Empire this client controls
     *
     * @return A pointer to the Empire instance this client has the control
     *      over.
     */
    const Empire* PlayerEmpire()
    { return AIClientApp::GetApp()->GetEmpire(AIClientApp::GetApp()->EmpireID()); }

    const Empire* GetEmpireByID(int empire_id)
    { return AIClientApp::GetApp()->GetEmpire(empire_id); }

    const GalaxySetupData& PythonGalaxySetupData()
    { return AIClientApp::GetApp()->GetGalaxySetupData(); }

    void InitMeterEstimatesAndDiscrepancies() {
        Universe& universe = AIClientApp::GetApp()->GetUniverse();
        universe.InitMeterEstimatesAndDiscrepancies();
    }

    /** @brief Set ::Universe ::Meter instances to their estimated values as
     *      if the next turn processing phase were done
     *
     * @param pretend_to_own_unowned_planets When set to true pretend during
     *      calculation that this clients Empire owns all known uncolonized
     *      planets.  The unowned planets MAX ::Meter values will contain the
     *      estimated value for those planets.
     */
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
            player_id = AIClientApp::GetApp()->PlayerID();

            // get all planets the player knows about that aren't yet colonized
            // (aren't owned by anyone).  Add this the current player's
            // ownership to all, while remembering which planets this is done
            // to.
            universe.InhibitUniverseObjectSignals(true);
            for (auto& planet : universe.Objects().all<Planet>()) {
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

    boost::python::list GetUserStringList(const std::string& list_key) {
        boost::python::list ret_list;
        for (const std::string& string : UserStringList(list_key))
            ret_list.append(string);
        return ret_list;
    }

    //! Return the canonical AI directory path
    //!
    //! The value depends on the ::OptionsDB `resource.path` and `ai-path` keys.
    //!
    //! @return
    //! The canonical path pointing to the directory containing all python AI
    //! scripts.
    std::string GetAIDir()
    { return (GetResourceDir() / GetOptionsDB().Get<std::string>("ai-path")).string(); }

    OrderSet& IssuedOrders()
    { return AIClientApp::GetApp()->Orders(); }

    template<typename OrderType, typename... Args>
    int Issue(Args &&... args) {
        auto app = ClientApp::GetApp();

        if (!OrderType::Check(app->EmpireID(), std::forward<Args>(args)...))
            return 0;

        app->Orders().IssueOrder(std::make_shared<OrderType>(app->EmpireID(), std::forward<Args>(args)...));

        return 1;
    }

    int IssueNewFleetOrder(const std::string& fleet_name, int ship_id) {
        std::vector<int> ship_ids{ship_id};
        auto app = ClientApp::GetApp();
        if (!NewFleetOrder::Check(app->EmpireID(), fleet_name, ship_ids, false))
            return 0;

        auto order = std::make_shared<NewFleetOrder>(app->EmpireID(), fleet_name, ship_ids, false);
        app->Orders().IssueOrder(order);

        return order->FleetID();
    }

    int IssueFleetTransferOrder(int ship_id, int new_fleet_id) {
        std::vector<int> ship_ids{ship_id};
        return Issue<FleetTransferOrder>(new_fleet_id, ship_ids);
    }

    int IssueRenameOrder(int object_id, const std::string& new_name)
    { return Issue<RenameOrder>(object_id, new_name); }

    int IssueScrapOrder(int object_id)
    { return Issue<ScrapOrder>(object_id); }

    int IssueChangeFocusOrder(int planet_id, const std::string& focus)
    { return Issue<ChangeFocusOrder>(planet_id, focus); }

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
        if (!empire) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        if (!empire->ProducibleItem(BT_BUILDING, item_name, location_id)) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : specified item_name and location_id that don't indicate an item that can be built at that location";
            return 0;
        }

        if (!empire->EnqueuableItem(BT_BUILDING, item_name, location_id)) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : specified item_name and location_id that don't indicate an item that can be enqueued at that location";
            return 0;
        }

        auto item = ProductionQueue::ProductionItem(BT_BUILDING, item_name);

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::PLACE_IN_QUEUE,
                                                   empire_id, item, 1, location_id));

        return 1;
    }

    int IssueEnqueueShipProductionOrder(int design_id, int location_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueEnqueueShipProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        if (!empire->ProducibleItem(BT_SHIP, design_id, location_id)) {
            ErrorLogger() << "IssueEnqueueShipProductionOrder : specified design_id and location_id that don't indicate a design that can be built at that location";
            return 0;
        }

        auto item = ProductionQueue::ProductionItem(BT_SHIP, design_id);

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::PLACE_IN_QUEUE,
                                                   empire_id, item, 1, location_id));

        return 1;
    }

    int IssueChangeProductionQuantityOrder(int queue_index, int new_quantity, int new_blocksize) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index outside range of items on queue.";
            return 0;
        }
        if (queue[queue_index].item.build_type != BT_SHIP) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index for a non-ship item.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            AIClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::SET_QUANTITY_AND_BLOCK_SIZE,
                                                       empire_id, queue_it->uuid,
                                                       new_quantity, new_blocksize));

        return 1;
    }

    int IssueRequeueProductionOrder(int old_queue_index, int new_queue_index) {
        if (old_queue_index == new_queue_index) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed same old and new indexes... nothing to do.";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueRequeueProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

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

        auto queue_it = empire->GetProductionQueue().find(old_queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::MOVE_ITEM_TO_INDEX,
                                                   empire_id, queue_it->uuid, new_queue_index));

        return 1;
    }

    int IssueDequeueProductionOrder(int queue_index) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueDequeueProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueDequeueProductionOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            AIClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::REMOVE_FROM_QUEUE,
                                                       empire_id, queue_it->uuid));

        return 1;
    }

    int IssuePauseProductionOrder(int queue_index, bool pause) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssuePauseProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionPauseOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            AIClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::PAUSE_PRODUCTION,
                                                       empire_id, queue_it->uuid));

        return 1;
    }

    int IssueAllowStockpileProductionOrder(int queue_index, bool use_stockpile) {
        int empire_id = AIClientApp::GetApp()->EmpireID();
        Empire* empire = AIClientApp::GetApp()->GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueAllowStockpileProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || static_cast<int>(queue.size()) <= queue_index) {
            ErrorLogger() << "IssueChangeProductionStockpileOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            AIClientApp::GetApp()->Orders().IssueOrder(
                std::make_shared<ProductionQueueOrder>(ProductionQueueOrder::ALLOW_STOCKPILE_USE,
                                                       empire_id, queue_it->uuid));

        return 1;
    }

    int IssueCreateShipDesignOrder(const std::string& name, const std::string& description,
                                   const std::string& hull, const boost::python::list parts_list,
                                   const std::string& icon, const std::string& model,
                                   bool name_desc_in_stringtable)
    {
        if (name.empty() || description.empty() || hull.empty()) {
            ErrorLogger() << "IssueCreateShipDesignOrderOrder : passed an empty name, description, or hull.";
            return 0;
        }

        std::vector<std::string> parts;
        int const num_parts = boost::python::len(parts_list);
        for (int i = 0; i < num_parts; i++)
            parts.push_back(boost::python::extract<std::string>(parts_list[i]));

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

    int IssueFleetMoveOrder(int fleet_id, int destination_id)
    { return Issue<FleetMoveOrder>(fleet_id, destination_id); }

    int AppendFleetMoveOrder(int fleet_id, int destination_id)
    { return Issue<FleetMoveOrder>(fleet_id, destination_id, true); }

    int IssueColonizeOrder(int ship_id, int planet_id)
    { return Issue<ColonizeOrder>(ship_id, planet_id); }

    int IssueInvadeOrder(int ship_id, int planet_id)
    { return Issue<InvadeOrder>(ship_id, planet_id); }

    int IssueBombardOrder(int ship_id, int planet_id)
    { return Issue<BombardOrder>(ship_id, planet_id); }

    int IssueAggressionOrder(int object_id, bool aggressive)
    { return Issue<AggressiveOrder>(object_id, aggressive); }

    int IssueGiveObjectToEmpireOrder(int object_id, int recipient_id)
    { return Issue<GiveObjectToEmpireOrder>(object_id, recipient_id); }

    void SendPlayerChatMessage(int recipient_player_id, const std::string& message_text)
    { AIClientApp::GetApp()->Networking().SendMessage(PlayerChatMessage(message_text, {recipient_player_id}, false)); }

    void SendDiplomaticMessage(const DiplomaticMessage& diplo_message) {
        AIClientApp* app = AIClientApp::GetApp();
        if (!app) return;

        int sender_player_id = app->PlayerID();
        if (sender_player_id == Networking::INVALID_PLAYER_ID) return;

        int recipient_player_id = app->EmpirePlayerID(diplo_message.RecipientEmpireID());
        if (recipient_player_id == Networking::INVALID_PLAYER_ID) return;

        app->Networking().SendMessage(DiplomacyMessage(diplo_message));
    }
}

namespace FreeOrionPython {
    const std::string& GetStaticSaveStateString()
    { return s_save_state_string; }

    void SetStaticSaveStateString(const std::string& new_state_string)
    { s_save_state_string = new_state_string; }

    void ClearStaticSaveStateString()
    { s_save_state_string.clear(); }

    /** Expose game client to Python.
     *
     * CallPolicies:
     *
     * return_value_policy<copy_const_reference>        when returning a relatively small object, such as a string,
     *                                                  that is returned by const reference or pointer
     *
     * return_value_policy<return_by_value>             when returning either a simple data type or a temporary object
     *                                                  in a function that will go out of scope after being returned
     *
     * return_value_policy<reference_existing_object>   when returning an object from a non-member function
     */
    void WrapAI()
    {
        def("playerName",               PlayerName,                     return_value_policy<copy_const_reference>(), "Returns the name (string) of this AI player.");
        def("playerName",               PlayerNameByID,                 return_value_policy<copy_const_reference>(), "Returns the name (string) of the player with the indicated playerID (int).");

        def("playerID",                 PlayerID,                       "Returns the integer id of this AI player.");
        def("empirePlayerID",           EmpirePlayerID,                 "Returns the player ID (int) of the player who is controlling the empire with the indicated empireID (int).");
        def("allPlayerIDs",             AllPlayerIDs,                   return_value_policy<return_by_value>(), "Returns an object (intVec) that contains the player IDs of all players in the game.");

        def("playerIsAI",               PlayerIsAI,                     "Returns True (boolean) if the player with the indicated playerID (int) is controlled by an AI and false (boolean) otherwise.");
        def("playerIsHost",             PlayerIsHost,                   "Returns True (boolean) if the player with the indicated playerID (int) is the host player for the game and false (boolean) otherwise.");

        def("empireID",                 EmpireID,                       "Returns the empire ID (int) of this AI player's empire.");
        def("playerEmpireID",           PlayerEmpireID,                 "Returns the empire ID (int) of the player with the specified player ID (int).");
        def("allEmpireIDs",             AllEmpireIDs,                   return_value_policy<return_by_value>(), "Returns an object (intVec) that contains the empire IDs of all empires in the game.");

        def("getEmpire",                PlayerEmpire,                   return_value_policy<reference_existing_object>(), "Returns the empire object (Empire) of this AI player");
        def("getEmpire",                GetEmpireByID,                  return_value_policy<reference_existing_object>(), "Returns the empire object (Empire) with the specified empire ID (int)");

        def("getUniverse",              GetUniverse,       return_value_policy<reference_existing_object>(), "Returns the universe object (Universe)");

        def("currentTurn",              CurrentTurn,       "Returns the current game turn (int).");

        def("getAIDir",                 GetAIDir,                       return_value_policy<return_by_value>());

        def("initMeterEstimatesDiscrepancies",      InitMeterEstimatesAndDiscrepancies);
        def("updateMeterEstimates",                 UpdateMeterEstimates);
        def("updateResourcePools",                  UpdateResourcePools);
        def("updateResearchQueue",                  UpdateResearchQueue);
        def("updateProductionQueue",                UpdateProductionQueue);

        def("issueFleetMoveOrder",                  IssueFleetMoveOrder, "Orders the fleet with indicated fleetID (int) to move to the system with the indicated destinationID (int). Returns 1 (int) on success or 0 (int) on failure due to not finding the indicated fleet or system.");
        def("appendFleetMoveOrder",                 AppendFleetMoveOrder, "Orders the fleet with indicated fleetID (int) to append the system with the indicated destinationID (int) to its possibly already enqueued route. Returns 1 (int) on success or 0 (int) on failure due to not finding the indicated fleet or system.");
        def("issueRenameOrder",                     IssueRenameOrder, "Orders the renaming of the object with indicated objectID (int) to the new indicated name (string). Returns 1 (int) on success or 0 (int) on failure due to this AI player not being able to rename the indicated object (which this player must fully own, and which must be a fleet, ship or planet).");
        def("issueScrapOrder",                      IssueScrapOrder, "Orders the ship or building with the indicated objectID (int) to be scrapped. Returns 1 (int) on success or 0 (int) on failure due to not finding a ship or building with the indicated ID, or if the indicated ship or building is not owned by this AI client's empire.");
        def("issueNewFleetOrder",                   IssueNewFleetOrder, "Orders a new fleet to be created with the indicated name (string) and containing the indicated shipIDs (IntVec). The ships must be located in the same system and must all be owned by this player. Returns the new fleets id (int) on success or 0 (int) on failure due to one of the noted conditions not being met.");
        def("issueFleetTransferOrder",              IssueFleetTransferOrder, "Orders the ship with ID shipID (int) to be transferred to the fleet with ID newFleetID. Returns 1 (int) on success, or 0 (int) on failure due to not finding the fleet or ship, or the client's empire not owning either, or the two not being in the same system (or either not being in a system) or the ship already being in the fleet.");
        def("issueColonizeOrder",                   IssueColonizeOrder, "Orders the ship with ID shipID (int) to colonize the planet with ID planetID (int). Returns 1 (int) on success or 0 (int) on failure due to not finding the indicated ship or planet, this client's player not owning the indicated ship, the planet already being colonized, or the planet and ship not being in the same system.");
        def("issueInvadeOrder",                     IssueInvadeOrder);
        def("issueBombardOrder",                    IssueBombardOrder);
        def("issueAggressionOrder",                 IssueAggressionOrder);
        def("issueGiveObjectToEmpireOrder",         IssueGiveObjectToEmpireOrder);
        def("issueChangeFocusOrder",                IssueChangeFocusOrder, "Orders the planet with ID planetID (int) to use focus setting focus (string). Returns 1 (int) on success or 0 (int) on failure if the planet can't be found or isn't owned by this player, or if the specified focus is not valid on the planet.");
        def("issueEnqueueTechOrder",                IssueEnqueueTechOrder, "Orders the tech with name techName (string) to be added to the tech queue at position (int) on the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech can't be enqueued by this player's empire.");
        def("issueDequeueTechOrder",                IssueDequeueTechOrder, "Orders the tech with name techName (string) to be removed from the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech isn't on this player's empire's tech queue.");
        def("issueEnqueueBuildingProductionOrder",  IssueEnqueueBuildingProductionOrder, "Orders the building with name (string) to be added to the production queue at the location of the planet with id locationID. Returns 1 (int) on success or 0 (int) on failure if there is no such building or it is not available to this player's empire, or if the building can't be produced at the specified location.");
        def("issueEnqueueShipProductionOrder",      IssueEnqueueShipProductionOrder, "Orders the ship design with ID designID (int) to be added to the production queue at the location of the planet with id locationID (int). Returns 1 (int) on success or 0 (int) on failure there is no such ship design or it not available to this player's empire, or if the design can't be produced at the specified location.");
        def("issueChangeProductionQuantityOrder",   IssueChangeProductionQuantityOrder);
        def("issueRequeueProductionOrder",          IssueRequeueProductionOrder, "Orders the item on the production queue at index oldQueueIndex (int) to be moved to index newQueueIndex (int). Returns 1 (int) on success or 0 (int) on failure if the old and new queue indices are equal, if either queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issueDequeueProductionOrder",          IssueDequeueProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be removed form the production queue. Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issuePauseProductionOrder",            IssuePauseProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be paused (or unpaused). Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issueAllowStockpileProductionOrder",   IssueAllowStockpileProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be enabled (or disabled) to use the imperial stockpile. Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        def("issueCreateShipDesignOrder",           IssueCreateShipDesignOrder, "Orders the creation of a new ship design with the name (string), description (string), hull (string), parts vector partsVec (StringVec), graphic (string) and model (string). model should be left as an empty string as of this writing. There is currently no easy way to find the id of the new design, though the client's empire should have the new design after this order is issued successfully. Returns 1 (int) on success or 0 (int) on failure if any of the name, description, hull or graphic are empty strings, if the design is invalid (due to not following number and type of slot requirements for the hull) or if creating the design fails for some reason.");

        class_<OrderSet, noncopyable>("OrderSet", no_init)
            .def(map_indexing_suite<OrderSet>())
            .add_property("size",       &OrderSet::size)
        ;

        class_<Order, boost::noncopyable>("Order", no_init)
            .add_property("empireID",   &Order::EmpireID)
            .add_property("executed",   &Order::Executed)
        ;

        def("getOrders",                IssuedOrders,                   return_value_policy<reference_existing_object>(), "Returns the orders the client empire has issued (OrderSet).");

        def("sendChatMessage",          SendPlayerChatMessage,          "Sends the indicated message (string) to the player with the indicated recipientID (int) or to all players if recipientID is -1.");
        def("sendDiplomaticMessage",    SendDiplomaticMessage);

        def("setSaveStateString",       SetStaticSaveStateString,       "Sets the save state string (string). This is a persistant storage space for the AI script to retain state information when the game is saved and reloaded. Any AI state information to be saved should be stored in a single string (likely using Python's pickle module) and stored using this function when the prepareForSave() Python function is called.");
        def("getSaveStateString",       GetStaticSaveStateString,       return_value_policy<copy_const_reference>(), "Returns the previously-saved state string (string). Can be used to retrieve the last-set save state string at any time, although this string is also passed to the resumeLoadedGame(savedStateString) Python function when a game is loaded, so this function isn't necessary to use if resumeLoadedGame stores the passed string. ");

        def("userString",               make_function(&UserString,      return_value_policy<copy_const_reference>()));
        def("userStringExists",         make_function(&UserStringExists,return_value_policy<return_by_value>()));
        def("userStringList",           &GetUserStringList);

        def("getGalaxySetupData",       PythonGalaxySetupData, return_value_policy<copy_const_reference>());

        boost::python::scope().attr("INVALID_GAME_TURN") = INVALID_GAME_TURN;
    }
}
