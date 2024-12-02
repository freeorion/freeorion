#include "AIWrapper.h"

#include "AIClientApp.h"
#include "../ClientNetworking.h"
#include "../../universe/Fleet.h"
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
#include "../../util/Order.h"
#include "../../Empire/Empire.h"
#include "../../Empire/EmpireManager.h"
#include "../../Empire/Diplomacy.h"
#include "../../Empire/Government.h"
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

#include <utility>
#if !defined(__cpp_lib_integer_comparison_functions)
namespace std {
    inline auto cmp_less_equal(auto&& lhs, auto&& rhs) { return lhs <= rhs; }
}
#endif

namespace py = boost::python;


////////////////////////
// Python AIWrapper //
////////////////////////
namespace {
    // static string to save AI state
    std::string s_save_state_string("");

    /** @brief Return the player name of the client identified by @a player_id
     *
     * @param player_id An client identifier.
     *
     * @return An UTF-8 encoded and NUL terminated string containing the player
     *      name of this client or an empty string the player is not known or
     *      does not exist.
     */
    auto PlayerNameByID(int player_id) -> const std::string&
    {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it != players.end())
            return it->second.name;
        else {
            DebugLogger() << "AIWrapper::PlayerName(" << std::to_string(player_id) << ") - passed an invalid player_id";
            throw std::invalid_argument("AIWrapper::PlayerName : given invalid player_id");
        }
    }

    auto EmpirePlayerID(int empire_id) -> int
    {
        int player_id = AIClientApp::GetApp()->EmpirePlayerID(empire_id);
        if (Networking::INVALID_PLAYER_ID == player_id)
            DebugLogger() << "AIWrapper::EmpirePlayerID(" << empire_id << ") - passed an invalid empire_id";
        return player_id;
    }

    /** @brief Return all player identifiers that are in game
     *
     * @return A vector containing the identifiers of all players.
     */
    auto AllPlayerIDs() -> std::vector<int>
    {
        std::vector<int> player_ids;
        for (auto& entry : AIClientApp::GetApp()->Players())
            player_ids.push_back(entry.first);
        return player_ids;
    }

    /** @brief Return if the player identified by @a player_id is the game
     *      host
     *
     * @param player_id An client identifier.
     *
     * @return True if the player is the game host, false if not.
     */
    auto PlayerIsHost(int player_id) -> bool
    {
        auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it == players.end())
            return false;
        return it->second.host;
    }

    /** @brief Return the empire identifier of the empire @a player_id controls
     *
     * @param player_id An client identifier.
     *
     * @return An empire identifier.
     */
    auto PlayerEmpireID(int player_id) -> int
    {
        const auto& players = AIClientApp::GetApp()->Players();
        auto it = players.find(player_id);
        if (it == players.end())
            return ALL_EMPIRES; // default invalid value
        return it->second.empire_id;
    }

    /** @brief Return all empire identifiers that are in game
     *
     * @return A vector containing the identifiers of all empires.
     */
    auto AllEmpireIDs() -> std::vector<int>
    {
        const auto& players = AIClientApp::GetApp()->Players();
        auto rng = players | range_transform([](const auto& id_pi) { return id_pi.second.empire_id; })
            | range_filter([](int empire_id) { return empire_id != ALL_EMPIRES; });
        std::vector<int> retval;
        retval.reserve(players.size());
        range_copy(rng, std::back_inserter(retval));
        return retval;
    }

    void InitMeterEstimatesAndDiscrepancies() {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        context.ContextUniverse().InitMeterEstimatesAndDiscrepancies(context);
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
        std::vector<Planet*> unowned_planets;
        int player_id = -1;

        ScriptingContext& context = IApp::GetApp()->GetContext();
        Universe& universe = context.ContextUniverse();

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
            unowned_planets.reserve(universe.Objects().size<Planet>());
            universe.InhibitUniverseObjectSignals(true);
            for (auto planet : universe.Objects().allRaw<Planet>()
                 | range_filter([](const auto* p) { return p->Unowned(); }))
            {
                unowned_planets.push_back(planet);
                planet->SetOwner(player_id);
            }
        }

        // update meter estimates with temporary ownership
        universe.UpdateMeterEstimates(context);

        if (pretend_to_own_unowned_planets) {
            // remove temporary ownership added above
            for (auto planet : unowned_planets)
                planet->SetOwner(ALL_EMPIRES);
            universe.InhibitUniverseObjectSignals(false);
        }
    }

    void UpdateResourcePools() {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        const int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateResourcePools : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateResourcePools(context,
                                    empire->TechCostsTimes(context),
                                    empire->PlanetAnnexationCosts(context),
                                    empire->PolicyAdoptionCosts(context),
                                    empire->ProductionCostsTimes(context));
    }

    void UpdateResearchQueue() {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateResearchQueue : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateResearchQueue(context, empire->TechCostsTimes(context));
    }

    void UpdateProductionQueue() {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        const int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "UpdateProductionQueue : couldn't get empire with id " << empire_id;
            return;
        }
        empire->UpdateProductionQueue(context, empire->ProductionCostsTimes(context));
    }

    auto GetUserStringList(const std::string& list_key) -> py::list
    {
        py::list ret_list;
        for (const std::string& string : UserStringList(list_key))
            ret_list.append(string);
        return ret_list;
    }

    template <typename OrderType, typename... Args>
    auto Issue(Args&&... args) -> int
    {
        auto* app = ClientApp::GetApp();
        ScriptingContext& context = app->GetContext();

        if (!OrderType::Check(app->EmpireID(), args..., context))
            return 0;

        app->Orders().IssueOrder<OrderType>(context, app->EmpireID(), std::forward<Args>(args)...);

        return 1;
    }

    auto IssueNewFleetOrder(const std::string& fleet_name, int ship_id) -> int
    {
        std::vector<int> ship_ids{ship_id};
        auto app = ClientApp::GetApp();
        ScriptingContext& context = IApp::GetApp()->GetContext();

        if (!NewFleetOrder::Check(app->EmpireID(), fleet_name, ship_ids,
                                  FleetAggression::FLEET_OBSTRUCTIVE, context))
        { return INVALID_OBJECT_ID; }

        const auto order = app->Orders().IssueOrder<NewFleetOrder>(
            context, app->EmpireID(), fleet_name, ship_ids, FleetAggression::FLEET_OBSTRUCTIVE);

        return order ? order->FleetID() : INVALID_OBJECT_ID;
    }

    auto IssueFleetTransferOrder(int ship_id, int new_fleet_id) -> int
    {
        std::vector<int> ship_ids{ship_id};
        return Issue<FleetTransferOrder>(new_fleet_id, ship_ids);
    }

    auto IssueEnqueueTechOrder(const std::string& tech_name, int position) -> int
    {
        const Tech* tech = GetTech(tech_name);
        if (!tech) {
            ErrorLogger() << "IssueEnqueueTechOrder : passed tech_name that is not the name of a tech.";
            return 0;
        }

        auto* app = AIClientApp::GetApp();
        app->Orders().IssueOrder<ResearchQueueOrder>(app->GetContext(), app->EmpireID(), tech_name, position);

        return 1;
    }

    auto IssueDequeueTechOrder(const std::string& tech_name) -> int
    {
        const Tech* tech = GetTech(tech_name);
        if (!tech) {
            ErrorLogger() << "IssueDequeueTechOrder : passed tech_name that is not the name of a tech.";
            return 0;
        }

        auto* app = AIClientApp::GetApp();
        app->Orders().IssueOrder<ResearchQueueOrder>(app->GetContext(), app->EmpireID(), tech_name);

        return 1;
    }

    auto IssueAdoptPolicyOrder(const std::string& policy_name, const std::string& category, int slot) -> int
    {
        const Policy* policy = GetPolicy(policy_name);
        if (!policy) {
            ErrorLogger() << "IssueAdoptPolicyOrder : passed policy_name, " << policy_name
                          << ", that is not the name of a policy.";
            return 0;
        }
        if (!policy->Category().empty() && policy->Category() != category) {
            ErrorLogger() << "IssueAdoptPolicyOrder : passed policy_name, " << policy_name
                          << ",  and category, " << category << ",  name that are inconsistent";
            return 0;
        }

        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);

        if (!empire) {
            ErrorLogger() << "IssueAdoptPolicyOrder : couldn't get empire with id " << empire_id;
            return 0;
        }
        if (empire->PolicyAdopted(policy_name)) {
            ErrorLogger() << "IssueAdoptPolicyOrder : policy with name " << policy_name
                          << " was already adopted";
            return 0;
        }

        app->Orders().IssueOrder<PolicyOrder>(context, empire_id, policy_name, category, slot);
        return 1;
    }

    auto IssueDeadoptPolicyOrder(const std::string& policy_name) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);

        if (!empire) {
            ErrorLogger() << "IssueDeadoptPolicyOrder : couldn't get empire with id " << empire_id;
            return 0;
        }
        if (!empire->PolicyAdopted(policy_name)) {
            ErrorLogger() << "IssueDeadoptPolicyOrder : policy with name " << policy_name
                          << " was not yet adopted, so can't be un-adopted";
            return 0;
        }

        app->Orders().IssueOrder<PolicyOrder>(context, empire_id, policy_name);
        return 1;
    }

    auto IsProducibleBuilding(const std::string& item_name, int location_id) -> bool
    {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IsProducibleBuilding : couldn't get empire with id " << empire_id;
            return false;
        }
        return empire->ProducibleItem(BuildType::BT_BUILDING, item_name, location_id, context);
    }

    auto IsEnqueuableBuilding(const std::string& item_name, int location_id) -> bool
    {
        ScriptingContext& context = IApp::GetApp()->GetContext();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IsEnqueuableBuilding : couldn't get empire with id " << empire_id;
            return false;
        }
        return empire->EnqueuableItem(BuildType::BT_BUILDING, item_name, location_id, context);
    }

    auto IssueEnqueueBuildingProductionOrder(const std::string& item_name, int location_id) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();

        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        if (!empire->EnqueuableItem(BuildType::BT_BUILDING, item_name, location_id, context)) {
            ErrorLogger() << "IssueEnqueueBuildingProductionOrder : specified item_name and location_id that don't indicate an item that can be enqueued at that location";
            return 0;
        }

        app->Orders().IssueOrder<ProductionQueueOrder>(
            context,
            ProductionQueueOrder::ProdQueueOrderAction::PLACE_IN_QUEUE,
            empire_id,
            ProductionQueue::ProductionItem(BuildType::BT_BUILDING, item_name),
            1, location_id);

        return 1;
    }

    auto IsEnqueuableShip(int design_id, int location_id) -> bool
    {
        const ScriptingContext& context = IApp::GetApp()->GetContext();
        int empire_id = AIClientApp::GetApp()->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IsEnqueuableShip : couldn't get empire with id " << empire_id;
            return false;
        }
        // as of this writing, ships don't have a distinction between producible and enqueuable
        return empire->ProducibleItem(BuildType::BT_SHIP, design_id, location_id, context);
    }

    auto IssueEnqueueShipProductionOrder(int design_id, int location_id) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        const Universe& universe{context.ContextUniverse()};
        int empire_id = app->EmpireID();

        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueEnqueueShipProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        if (!empire->ProducibleItem(BuildType::BT_SHIP, design_id, location_id, context)) {
            ErrorLogger() << "IssueEnqueueShipProductionOrder : specified design_id and location_id that don't indicate a design that can be built at that location";
            return 0;
        }

        app->Orders().IssueOrder<ProductionQueueOrder>(
            context,
            ProductionQueueOrder::ProdQueueOrderAction::PLACE_IN_QUEUE,
            empire_id,
            ProductionQueue::ProductionItem(BuildType::BT_SHIP, design_id, universe),
            1, location_id);

        return 1;
    }

    auto IssueChangeProductionQuantityOrder(int queue_index, int new_quantity, int new_blocksize) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || std::cmp_less_equal(queue.size(), queue_index)) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index outside range of items on queue.";
            return 0;
        }
        if (queue[queue_index].item.build_type != BuildType::BT_SHIP) {
            ErrorLogger() << "IssueChangeProductionQuantityOrder : passed queue_index for a non-ship item.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            app->Orders().IssueOrder<ProductionQueueOrder>(
                context,
                ProductionQueueOrder::ProdQueueOrderAction::SET_QUANTITY_AND_BLOCK_SIZE,
                empire_id, queue_it->uuid, new_quantity, new_blocksize);

        return 1;
    }

    auto IssueRequeueProductionOrder(int old_queue_index, int new_queue_index) -> int
    {
        if (old_queue_index == new_queue_index) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed same old and new indexes... nothing to do.";
            return 0;
        }

        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueRequeueProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (old_queue_index < 0 || std::cmp_less_equal(queue.size(), old_queue_index)) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed old_queue_index outside range of items on queue.";
            return 0;
        }

        // After removing an earlier entry in queue, all later entries are shifted down one queue index, so
        // inserting before the specified item index should now insert before the previous item index.  This
        // also allows moving to the end of the queue, rather than only before the last item on the queue.
        int actual_new_index = new_queue_index;
        if (old_queue_index < new_queue_index)
            actual_new_index = new_queue_index - 1;

        if (new_queue_index < 0 || std::cmp_less_equal(queue.size(), actual_new_index)) {
            ErrorLogger() << "IssueRequeueProductionOrder : passed new_queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(old_queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            app->Orders().IssueOrder<ProductionQueueOrder>(
                context,
                ProductionQueueOrder::ProdQueueOrderAction::MOVE_ITEM_TO_INDEX,
                empire_id, queue_it->uuid, new_queue_index);

        return 1;
    }

    auto IssueDequeueProductionOrder(int queue_index) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueDequeueProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || std::cmp_less_equal(queue.size(), queue_index)) {
            ErrorLogger() << "IssueDequeueProductionOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = queue.find(queue_index);

        if (queue_it != queue.end())
            app->Orders().IssueOrder<ProductionQueueOrder>(
                context,
                ProductionQueueOrder::ProdQueueOrderAction::REMOVE_FROM_QUEUE,
                empire_id, queue_it->uuid);

        return 1;
    }

    auto IssuePauseProductionOrder(int queue_index, bool pause) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssuePauseProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || std::cmp_less_equal(queue.size(), queue_index)) {
            ErrorLogger() << "IssueChangeProductionPauseOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = queue.find(queue_index);

        if (queue_it != queue.end())
            app->Orders().IssueOrder<ProductionQueueOrder>(
                context,
                ProductionQueueOrder::ProdQueueOrderAction::PAUSE_PRODUCTION,
                empire_id, queue_it->uuid);

        return 1;
    }

    auto IssueAllowStockpileProductionOrder(int queue_index, bool use_stockpile) -> int
    {
        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        auto empire = context.GetEmpire(empire_id);
        if (!empire) {
            ErrorLogger() << "IssueAllowStockpileProductionOrder : couldn't get empire with id " << empire_id;
            return 0;
        }

        const ProductionQueue& queue = empire->GetProductionQueue();
        if (queue_index < 0 || std::cmp_less_equal(queue.size(), queue_index)) {
            ErrorLogger() << "IssueChangeProductionStockpileOrder : passed queue_index outside range of items on queue.";
            return 0;
        }

        auto queue_it = empire->GetProductionQueue().find(queue_index);

        if (queue_it != empire->GetProductionQueue().end())
            app->Orders().IssueOrder<ProductionQueueOrder>(
                context,
                ProductionQueueOrder::ProdQueueOrderAction::ALLOW_STOCKPILE_USE,
                empire_id, queue_it->uuid);

        return 1;
    }

    auto IssueCreateShipDesignOrder(const std::string& name, const std::string& description,
                                    const std::string& hull, const py::list parts_list,
                                    const std::string& icon, const std::string& model,
                                    bool name_desc_in_stringtable) -> int
    {
        if (name.empty() || description.empty() || hull.empty()) {
            ErrorLogger() << "IssueCreateShipDesignOrderOrder : passed an empty name, description, or hull.";
            return 0;
        }

        std::vector<std::string> parts;
        int const num_parts = py::len(parts_list);
        parts.reserve(num_parts);
        for (int i = 0; i < num_parts; i++)
            parts.push_back(py::extract<std::string>(parts_list[i]));

        auto* app = AIClientApp::GetApp();
        ScriptingContext& context = app->GetContext();
        int empire_id = app->EmpireID();
        const auto uuid = boost::uuids::random_generator()();

        // create design from stuff chosen in UI
        try {
            auto design = std::make_unique<ShipDesign>(
                std::invalid_argument(""), name, description, context.current_turn,
                empire_id, hull, parts, icon, model, name_desc_in_stringtable, false, uuid);
            app->Orders().IssueOrder<ShipDesignOrder>(context, empire_id, *design);
            return 1;

        } catch (const std::invalid_argument&) {
            ErrorLogger() << "IssueCreateShipDesignOrderOrder failed to create a new ShipDesign object";
            return 0;
        }
    }

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
    auto GetStaticSaveStateString() -> const std::string&
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
        py::def("playerName",
                +[]() -> std::string const& { return AIClientApp::GetApp()->PlayerName(); },
                py::return_value_policy<py::copy_const_reference>(),
                "Returns the name (string) of this AI player.");

        py::def("playerName",               PlayerNameByID,                 py::return_value_policy<py::copy_const_reference>(), "Returns the name (string) of the player with the indicated playerID (int).");

        py::def("playerID",
                +[]() -> int { return AIClientApp::GetApp()->PlayerID(); },
                "Returns the integer id of this AI player.");

        py::def("empirePlayerID",           EmpirePlayerID,                 "Returns the player ID (int) of the player who is controlling the empire with the indicated empireID (int).");
        py::def("allPlayerIDs",             AllPlayerIDs,                   py::return_value_policy<py::return_by_value>(), "Returns an object (intVec) that contains the player IDs of all players in the game.");

        py::def("playerIsAI",
                +[](int player_id) -> bool { return AIClientApp::GetApp()->GetPlayerClientType(player_id) == Networking::ClientType::CLIENT_TYPE_AI_PLAYER; },
                "Returns True (boolean) if the player with the indicated"
                " playerID (int) is controlled by an AI and false (boolean)"
                " otherwise.");

        py::def("playerIsHost",             PlayerIsHost,                   "Returns True (boolean) if the player with the indicated playerID (int) is the host player for the game and false (boolean) otherwise.");

        py::def("empireID",
                +[]() -> int { return AIClientApp::GetApp()->EmpireID(); },
                "Returns the empire ID (int) of this AI player's empire.");

        py::def("playerEmpireID",           PlayerEmpireID,                 "Returns the empire ID (int) of the player with the specified player ID (int).");
        py::def("allEmpireIDs",             AllEmpireIDs,                   py::return_value_policy<py::return_by_value>(), "Returns an object (intVec) that contains the empire IDs of all empires in the game.");

        py::def("getEmpire",
                +[]() -> const Empire* { return AIClientApp::GetApp()->GetEmpire(AIClientApp::GetApp()->EmpireID()); },
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the empire object (Empire) of this AI player");

        py::def("getEmpire",
                +[](int empire_id) -> const Empire* { return AIClientApp::GetApp()->GetEmpire(empire_id); },
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the empire object (Empire) with the specified empire ID (int)");

        py::def("getDiplomaticStatus",
                +[](int empire_id1, int empire_id2) -> const DiplomaticStatus { return AIClientApp::GetApp()->Empires().GetDiplomaticStatus(empire_id1, empire_id2); },
                "Returns the diplomatic status between two empires");

        py::def("getUniverse",
                +[]() -> const Universe& { return ClientApp::GetApp()->GetUniverse(); },
                py::return_value_policy<py::reference_existing_object>(),
                "Returns the universe object (Universe)");

        py::def("currentTurn",
                +[]() -> int { return ClientApp::GetApp()->CurrentTurn(); },
                "Returns the current game turn (int).");

        py::def("getAIDir",
                +[]() -> std::string { return PathToString(GetResourceDir() / FilenameToPath(GetOptionsDB().Get<std::string>("ai-path"))); },
                py::return_value_policy<py::return_by_value>());

        py::def("initMeterEstimatesDiscrepancies",
                InitMeterEstimatesAndDiscrepancies,
                "For all objects and max / target meters, determines discrepancies "
                "between actual meters and what the known universe should produce. "
                "This is used later when updating meter estimates to incorporate "
                "those discrepancies.");
        py::def("updateMeterEstimates",  UpdateMeterEstimates);
        py::def("updateResourcePools",   UpdateResourcePools);
        py::def("updateResearchQueue",   UpdateResearchQueue);
        py::def("updateProductionQueue", UpdateProductionQueue);

        py::def("isProducibleBuilding",
                IsProducibleBuilding,
                "Returns true if the specified building type (string) can be produced "
                "by this client's empire at the specified production location (int). "
                "Being producible means that if the item is on the production queue, "
                "it can be allocated production points that are available at its "
                "location. Being producible does not mean that the building type can "
                "be added to the queue.");
        py::def("isEnqueuableBuilding",
                IsEnqueuableBuilding,
                "Returns true if the specified building type (string) can be enqueued "
                "by this client's empire at the specified production location (int). "
                "Being enqueuable means that the item can be added to the queue, but "
                "does not mean that the item will be allocated production points once "
                " it is added");
        py::def("isEnqueuableShip",
                IsEnqueuableShip,
                "Returns true if the specified ship design (int) can be enqueued by "
                "this client's empire at the specified production location (int). "
                "Enqueued ships should always also be producible, and thus able to "
                "be allocated production points once enqueued, if any are available "
                "at the production location.");

        py::def("issueFleetMoveOrder",
                +[](int fleet_id, int destination_id) -> int { return Issue<FleetMoveOrder>(fleet_id, destination_id, false); },
                "Orders the fleet with indicated fleetID (int) to move to the"
                " system with the indicated destinationID (int). Returns 1 (int)"
                " on success or 0 (int) on failure due to not finding the"
                " indicated fleet or system.");

        py::def("appendFleetMoveOrder",
                +[](int fleet_id, int destination_id) -> int { return Issue<FleetMoveOrder>(fleet_id, destination_id, true); },
                "Orders the fleet with indicated fleetID (int) to append the"
                " system with the indicated destinationID (int) to its possibly"
                " already enqueued route. Returns 1 (int) on success or 0 (int)"
                " on failure due to not finding the indicated fleet or system.");

        py::def("issueRenameOrder",
                +[](int object_id, const std::string& new_name) -> int { return Issue<RenameOrder>(object_id, new_name); },
                "Orders the renaming of the object with indicated objectID (int)"
                " to the new indicated name (string). Returns 1 (int) on success"
                " or 0 (int) on failure due to this AI player not being able to"
                " rename the indicated object (which this player must fully own,"
                " and which must be a fleet, ship or planet).");

        py::def("issueScrapOrder",
                +[](int object_id) -> int { return Issue<ScrapOrder>(object_id); },
                "Orders the ship or building with the indicated objectID (int)"
                " to be scrapped. Returns 1 (int) on success or 0 (int) on"
                " failure due to not finding a ship or building with the"
                " indicated ID, or if the indicated ship or building is not"
                " owned by this AI client's empire.");

        py::def("issueNewFleetOrder",                   IssueNewFleetOrder, "Orders a new fleet to be created with the indicated name (string) and containing the indicated shipIDs (IntVec). The ships must be located in the same system and must all be owned by this player. Returns the new fleets id (int) on success or 0 (int) on failure due to one of the noted conditions not being met.");
        py::def("issueFleetTransferOrder",              IssueFleetTransferOrder, "Orders the ship with ID shipID (int) to be transferred to the fleet with ID newFleetID. Returns 1 (int) on success, or 0 (int) on failure due to not finding the fleet or ship, or the client's empire not owning either, or the two not being in the same system (or either not being in a system) or the ship already being in the fleet.");

        py::def("issueColonizeOrder",
                +[](int ship_id, int planet_id) -> int { return Issue<ColonizeOrder>(ship_id, planet_id); },
                "Orders the ship with ID shipID (int) to colonize the planet"
                " with ID planetID (int). Returns 1 (int) on success or 0 (int)"
                " on failure due to not finding the indicated ship or planet,"
                " this client's player not owning the indicated ship, the"
                " planet already being colonized, or the planet and ship not"
                " being in the same system.");

        py::def("issueInvadeOrder",
                +[](int ship_id, int planet_id) -> int { return Issue<InvadeOrder>(ship_id, planet_id); });

        py::def("issueBombardOrder",
                +[](int ship_id, int planet_id) -> int { return Issue<BombardOrder>(ship_id, planet_id); });

        py::def("issueAggressionOrder",
                +[](int object_id, bool aggressive) -> int { return Issue<AggressiveOrder>(object_id, aggressive ? FleetAggression::FLEET_AGGRESSIVE : FleetAggression::FLEET_DEFENSIVE); });

        py::def("issueGiveObjectToEmpireOrder",
                +[](int object_id, int recipient_id) -> int { return Issue<GiveObjectToEmpireOrder>(object_id, recipient_id); });

        py::def("issueChangeFocusOrder",
                +[](int planet_id, const std::string& focus) -> int { return Issue<ChangeFocusOrder>(planet_id, focus); },
                "Orders the planet with ID planetID (int) to use focus setting"
                " focus (string). Returns 1 (int) on success or 0 (int) on"
                " failure if the planet can't be found or isn't owned by this"
                " player, or if the specified focus is not valid on the planet.");

        py::def("issueEnqueueTechOrder",                IssueEnqueueTechOrder, "Orders the tech with name techName (string) to be added to the tech queue at position (int) on the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech can't be enqueued by this player's empire.");
        py::def("issueDequeueTechOrder",                IssueDequeueTechOrder, "Orders the tech with name techName (string) to be removed from the queue. Returns 1 (int) on success or 0 (int) on failure if the indicated tech can't be found. Will return 1 (int) but do nothing if the indicated tech isn't on this player's empire's tech queue.");
        py::def("issueAdoptPolicyOrder",                IssueAdoptPolicyOrder, "Orders the policy with name policyName (string) to be adopted by the empire in the category categoryName (string) and slot slot (int). Returns 1 (int) on success or 0 (int) on failure if the indicated policy can't be found. Will return 1 (int) but do nothing if the indicated policy can't be enqueued by this player's empire in the requested category and slot.");
        py::def("issueDeadoptPolicyOrder",              IssueDeadoptPolicyOrder, "Orders the policy with name policyName (string) to be de-adopted by the empire. Returns 1 (int) on success or 0 (int) on failure if the indicated policy was not already adopted.");
        py::def("issueEnqueueBuildingProductionOrder",  IssueEnqueueBuildingProductionOrder, "Orders the building with name (string) to be added to the production queue at the location of the planet with id locationID. Returns 1 (int) on success or 0 (int) on failure if there is no such building or it is not available to this player's empire, or if the building can't be produced at the specified location.");
        py::def("issueEnqueueShipProductionOrder",      IssueEnqueueShipProductionOrder, "Orders the ship design with ID designID (int) to be added to the production queue at the location of the planet with id locationID (int). Returns 1 (int) on success or 0 (int) on failure there is no such ship design or it not available to this player's empire, or if the design can't be produced at the specified location.");
        py::def("issueChangeProductionQuantityOrder",   IssueChangeProductionQuantityOrder);
        py::def("issueRequeueProductionOrder",          IssueRequeueProductionOrder, "Orders the item on the production queue at index oldQueueIndex (int) to be moved to index newQueueIndex (int). Returns 1 (int) on success or 0 (int) on failure if the old and new queue indices are equal, if either queue index is less than 0 or greater than the largest indexed item on the queue.");
        py::def("issueDequeueProductionOrder",          IssueDequeueProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be removed form the production queue. Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        py::def("issuePauseProductionOrder",            IssuePauseProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be paused (or unpaused). Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        py::def("issueAllowStockpileProductionOrder",   IssueAllowStockpileProductionOrder, "Orders the item on the production queue at index queueIndex (int) to be enabled (or disabled) to use the imperial stockpile. Returns 1 (int) on success or 0 (int) on failure if the queue index is less than 0 or greater than the largest indexed item on the queue.");
        py::def("issueCreateShipDesignOrder",           IssueCreateShipDesignOrder, "Orders the creation of a new ship design with the name (string), description (string), hull (string), parts vector partsVec (StringVec), graphic (string) and model (string). model should be left as an empty string as of this writing. There is currently no easy way to find the id of the new design, though the client's empire should have the new design after this order is issued successfully. Returns 1 (int) on success or 0 (int) on failure if any of the name, description, hull or graphic are empty strings, if the design is invalid (due to not following number and type of slot requirements for the hull) or if creating the design fails for some reason.");

        py::def("sendChatMessage",
                +[](int recipient, const std::string& message) { AIClientApp::GetApp()->Networking().SendMessage(PlayerChatMessage(message, {recipient}, false)); },
                "Sends the indicated message (string) to the player with the"
                " indicated recipientID (int) or to all players if"
                " recipientID is -1.");

        py::def("sendDiplomaticMessage",    SendDiplomaticMessage);

        py::def("setSaveStateString",       SetStaticSaveStateString,       "Sets the save state string (string). This is a persistant storage space for the AI script to retain state information when the game is saved and reloaded. Any AI state information to be saved should be stored in a single string (likely using Python's pickle module) and stored using this function when the prepareForSave() Python function is called.");
        py::def("getSaveStateString",       GetStaticSaveStateString,       py::return_value_policy<py::copy_const_reference>(), "Returns the previously-saved state string (string). Can be used to retrieve the last-set save state string at any time, although this string is also passed to the resumeLoadedGame(savedStateString) Python function when a game is loaded, so this function isn't necessary to use if resumeLoadedGame stores the passed string. ");

        py::def("userString",
                +[](const std::string& key) -> const std::string& { return UserString(key); },
                py::return_value_policy<py::copy_const_reference>());
        py::def("userStringExists",
                +[](const std::string& key) -> bool { return UserStringExists(key); });
        py::def("userStringList",           &GetUserStringList);

        py::def("getGalaxySetupData",
                +[]() -> const GalaxySetupData& { return AIClientApp::GetApp()->GetGalaxySetupData(); },
                py::return_value_policy<py::copy_const_reference>());

        py::scope().attr("INVALID_GAME_TURN") = INVALID_GAME_TURN;
    }
}
