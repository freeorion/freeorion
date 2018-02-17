#include "AIInterface.h"

#include "../util/MultiplayerCommon.h"
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

const std::string& AIBase::GetSaveStateString() {
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

    const Universe& GetUniverse()
    { return AIClientApp::GetApp()->GetUniverse(); }

    const Tech* GetTech(const std::string& name)
    { return TechManager::GetTechManager().GetTech(name); }

    int CurrentTurn()
    { return AIClientApp::GetApp()->CurrentTurn(); }

    object GetOptionsDBOptionStr(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? str(GetOptionsDB().Get<std::string>(option)) : str(); }

    object GetOptionsDBOptionInt(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? object(GetOptionsDB().Get<int>(option)) : object(); }

    object GetOptionsDBOptionBool(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? object(GetOptionsDB().Get<bool>(option)) : object(); }

    object GetOptionsDBOptionDouble(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? object(GetOptionsDB().Get<double>(option)) : object(); }

    std::string GetAIDir()
    { return (GetResourceDir() / GetOptionsDB().Get<std::string>("ai-path")).string(); }

    const GalaxySetupData&  GetGalaxySetupData()
    { return AIClientApp::GetApp()->GetGalaxySetupData(); }

    void InitTurn() {
        //boost::timer turn_init_timer;

        //InitMeterEstimatesAndDiscrepancies();
        //UpdateMeterEstimates();
        //UpdateResourcePools();

        //DebugLogger() << "AIInterface::InitTurn time: " << (turn_init_timer.elapsed() * 1000.0);
    }

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

    int IssueFleetMoveOrder(int fleet_id, int destination_id) {
        auto fleet = GetFleet(fleet_id);
        if (!fleet) {
            ErrorLogger() << "IssueFleetMoveOrder : passed an invalid fleet_id";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        if (!fleet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetMoveOrder : passed fleet_id of fleet not owned by player";
            return 0;
        }

        int start_id = fleet->SystemID();
        if (start_id == INVALID_OBJECT_ID)
            start_id = fleet->NextSystemID();

        if (destination_id != INVALID_OBJECT_ID && destination_id == start_id)
            DebugLogger() << "AIInterface::IssueFleetMoveOrder : pass destination system id (" << destination_id << ") that fleet is already in";

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<FleetMoveOrder>(empire_id, fleet_id, start_id, destination_id));

        return 1;
    }

    int IssueRenameOrder(int object_id, const std::string& new_name) {
        if (new_name.empty()) {
            ErrorLogger() << "IssueRenameOrder : passed an empty new name";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        auto obj = GetUniverseObject(object_id);

        if (!obj) {
            ErrorLogger() << "IssueRenameOrder : passed an invalid object_id";
            return 0;
        }
        if (!obj->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueRenameOrder : passed object_id of object not owned by player";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<RenameOrder>(empire_id, object_id, new_name));

        return 1;
    }

    int IssueScrapOrder(const std::vector<int>& object_ids) {
            if (object_ids.empty()) {
                ErrorLogger() << "IssueScrapOrder : passed empty vector of object_ids";
                return 0;
            }

            int empire_id = AIClientApp::GetApp()->EmpireID();

            // make sure all objects exist and are owned just by this player
            for (int object_id : object_ids) {
                auto obj = GetUniverseObject(object_id);

                if (!obj) {
                    ErrorLogger() << "IssueScrapOrder : passed an invalid object_id";
                    return 0;
                }

                if (!obj->OwnedBy(empire_id)) {
                    ErrorLogger() << "IssueScrapOrder : passed object_id of object not owned by player";
                    return 0;
                }

                AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ScrapOrder>(empire_id, object_id));
            }

            return 1;
    }

    int IssueScrapOrder(int object_id) {
        std::vector<int> object_ids;
        object_ids.push_back(object_id);
        return IssueScrapOrder(object_ids);
    }

    int IssueNewFleetOrder(const std::string& fleet_name, const std::vector<int>& ship_ids) {
        if (ship_ids.empty()) {
            ErrorLogger() << "IssueNewFleetOrder : passed empty vector of ship_ids";
            return 0;
        }

        if (fleet_name.empty()) {
            ErrorLogger() << "IssueNewFleetOrder : tried to create a nameless fleet";
            return 0;
        }

        int empire_id = AIClientApp::GetApp()->EmpireID();
        std::shared_ptr<const Ship> ship;

        // make sure all ships exist and are owned just by this player
        for (int ship_id : ship_ids) {
            ship = GetShip(ship_id);
            if (!ship) {
                ErrorLogger() << "IssueNewFleetOrder : passed an invalid ship_id";
                return 0;
            }
            if (!ship->OwnedBy(empire_id)) {
                ErrorLogger() << "IssueNewFleetOrder : passed ship_id of ship not owned by player";
                return 0;
            }
        }

        // make sure all ships are at a system, and that all are at the same system
        int system_id = ship->SystemID();
        if (system_id == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueNewFleetOrder : passed ship_ids of ships at different locations";
            return 0;
        }

        auto it = ship_ids.begin();
        for (++it; it != ship_ids.end(); ++it) {
            auto ship2 = GetShip(*it);
            if (ship2->SystemID() != system_id) {
                ErrorLogger() << "IssueNewFleetOrder : passed ship_ids of ships at different locations";
                return 0;
            }
        }

        auto order = std::make_shared<NewFleetOrder>(empire_id, fleet_name, system_id, ship_ids);
        AIClientApp::GetApp()->Orders().IssueOrder(order);

        return *order->FleetIDs().begin();
    }

    int IssueNewFleetOrder(const std::string& fleet_name, int ship_id) {
        std::vector<int> ship_ids;
        ship_ids.push_back(ship_id);
        return IssueNewFleetOrder(fleet_name, ship_ids);
    }

    int IssueFleetTransferOrder(int ship_id, int new_fleet_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "IssueFleetTransferOrder : passed an invalid ship_id " << ship_id;
            return 0;
        }
        int ship_sys_id = ship->SystemID();
        if (ship_sys_id == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueFleetTransferOrder : ship is not in a system";
            return 0;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship_id of ship not owned by player";
            return 0;
        }

        auto fleet = GetFleet(new_fleet_id);
        if (!fleet) {
            ErrorLogger() << "IssueFleetTransferOrder : passed an invalid new_fleet_id " << new_fleet_id;
            return 0;
        }
        int fleet_sys_id = fleet->SystemID();
        if (fleet_sys_id == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueFleetTransferOrder : new fleet is not in a system";
            return 0;
        }
        if (!fleet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetTransferOrder : passed fleet_id "<< new_fleet_id << " of fleet not owned by player";
            return 0;
        }

        if (fleet_sys_id != ship_sys_id) {
            ErrorLogger() << "IssueFleetTransferOrder : new fleet in system " << fleet_sys_id << " and ship in system "<< ship_sys_id <<  " are not in the same system";
            return 0;
        }

        std::vector<int> ship_ids;
        ship_ids.push_back(ship_id);
        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<FleetTransferOrder>(empire_id, new_fleet_id, ship_ids));

        return 1;
    }

    int IssueColonizeOrder(int ship_id, int planet_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        // make sure ship_id is a ship...
        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "IssueColonizeOrder : passed an invalid ship_id";
            return 0;
        }

        // get fleet of ship
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet) {
            ErrorLogger() << "IssueColonizeOrder : ship with passed ship_id has invalid fleet_id";
            return 0;
        }

        // make sure player owns ship and its fleet
        if (!fleet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueColonizeOrder : empire does not own fleet of passed ship";
            return 0;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueColonizeOrder : empire does not own passed ship";
            return 0;
        }

        // verify that planet exists and is un-occupied.
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "IssueColonizeOrder : no planet with passed planet_id";
            return 0;
        }
        if ((!planet->Unowned()) && !( planet->OwnedBy(empire_id) && planet->InitialMeterValue(METER_POPULATION) == 0.0f)) {
            ErrorLogger() << "IssueColonizeOrder : planet with passed planet_id "<<planet_id<<" is already owned, or colonized by own empire";
            return 0;
        }

        // verify that planet is in same system as the fleet
        if (planet->SystemID() != fleet->SystemID()) {
            ErrorLogger() << "IssueColonizeOrder : fleet and planet are not in the same system";
            return 0;
        }
        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueColonizeOrder : ship is not in a system";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<ColonizeOrder>(empire_id, ship_id, planet_id));

        return 1;
    }

    int IssueInvadeOrder(int ship_id, int planet_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        // make sure ship_id is a ship...
        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "IssueInvadeOrder : passed an invalid ship_id";
            return 0;
        }

        // get fleet of ship
        auto fleet = GetFleet(ship->FleetID());
        if (!fleet) {
            ErrorLogger() << "IssueInvadeOrder : ship with passed ship_id has invalid fleet_id";
            return 0;
        }

        // make sure player owns ship and its fleet
        if (!fleet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueInvadeOrder : empire does not own fleet of passed ship";
            return 0;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueInvadeOrder : empire does not own passed ship";
            return 0;
        }

        // verify that planet exists and is occupied by another empire
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "IssueInvadeOrder : no planet with passed planet_id";
            return 0;
        }
        bool owned_by_invader = planet->OwnedBy(empire_id);
        bool unowned = planet->Unowned();
        bool populated = planet->InitialMeterValue(METER_POPULATION) > 0.0f;
        bool visible = GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) >= VIS_PARTIAL_VISIBILITY;
        bool vulnerable = planet->InitialMeterValue(METER_SHIELD) <= 0.0f;
        float shields = planet->InitialMeterValue(METER_SHIELD);
        std::string this_species = planet->SpeciesName();
        //bool being_invaded = planet->IsAboutToBeInvaded();
        bool invadable = !owned_by_invader && vulnerable && (populated || !unowned) && visible ;// && !being_invaded; a 'being_invaded' check prevents AI from invading with multiple ships at once, which is important
        if (!invadable) {
            ErrorLogger() << "IssueInvadeOrder : planet with passed planet_id " << planet_id
                          << " and species " << this_species << " is not invadable due to one or more of: owned by invader empire, "
                          << "not visible to invader empire, has shields above zero, or is already being invaded.";
            if (!unowned)
                ErrorLogger() << "IssueInvadeOrder : planet (id " << planet_id << ") is not unowned";
            if (!visible)
                ErrorLogger() << "IssueInvadeOrder : planet (id " << planet_id << ") is not visible";
            if (!vulnerable)
                ErrorLogger() << "IssueInvadeOrder : planet (id " << planet_id << ") is not vulnerable, shields at "<<shields;
            return 0;
        }

        // verify that planet is in same system as the fleet
        if (planet->SystemID() != fleet->SystemID()) {
            ErrorLogger() << "IssueInvadeOrder : fleet and planet are not in the same system";
            return 0;
        }
        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueInvadeOrder : ship is not in a system";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<InvadeOrder>(empire_id, ship_id, planet_id));

        return 1;
    }

    int IssueBombardOrder(int ship_id, int planet_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        // make sure ship_id is a ship...
        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "IssueBombardOrder : passed an invalid ship_id";
            return 0;
        }
        if (ship->TotalWeaponsDamage() <= 0.0f) {   // this will test the current meter values. potential issue if some local change sets these to zero even though they will be nonzero on server when bombard is processed before effects application / meter update
            ErrorLogger() << "IssueBombardOrder : ship can't attack / bombard";
            return 0;
        }
        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueBombardOrder : ship isn't owned by the order-issuing empire";
            return 0;
        }


        // verify that planet exists and is occupied by another empire
        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "IssueBombardOrder : no planet with passed planet_id";
            return 0;
        }
        if (planet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueBombardOrder : planet is already owned by the order-issuing empire";
            return 0;
        }
        if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
            ErrorLogger() << "IssueBombardOrder : planet owned by an empire not at war with order-issuing empire";
            return 0;
        }
        if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) < VIS_BASIC_VISIBILITY) {
            ErrorLogger() << "IssueBombardOrder : planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
            //return;
        }


        int ship_system_id = ship->SystemID();
        if (ship_system_id == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueBombardOrder : given id of ship not in a system";
            return 0;
        }
        int planet_system_id = planet->SystemID();
        if (ship_system_id != planet_system_id) {
            ErrorLogger() << "IssueBombardOrder : given ids of ship and planet not in the same system";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(std::make_shared<BombardOrder>(empire_id, ship_id, planet_id));

        return 1;
    }

    int IssueAggressionOrder(int object_id, bool aggressive) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        auto fleet = GetFleet(object_id);
        if (!fleet) {
            ErrorLogger() << "IssueAggressionOrder : no fleet with passed id";
            return 0;
        }
        if (!fleet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueAggressionOrder : passed object_id of object not owned by player";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<AggressiveOrder>(empire_id, object_id, aggressive));

        return 1;
    }

    int IssueGiveObjectToEmpireOrder(int object_id, int recipient_id) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        if (GetEmpire(recipient_id) == 0) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : given invalid recipient empire id";
            return 0;
        }

        if (Empires().GetDiplomaticStatus(empire_id, recipient_id) != DIPLO_PEACE) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : attempting to give to empire not at peace";
            return 0;
        }

        auto obj = GetUniverseObject(object_id);
        if (!obj) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed invalid object id";
            return 0;
        }

        if (!obj->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object not owned by player";
            return 0;
        }

        if (obj->ObjectType() != OBJ_FLEET && obj->ObjectType() != OBJ_PLANET) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object that is not a fleet or planet";
            return 0;
        }

        auto system = GetSystem(obj->SystemID());
        if (!system) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : couldn't get system of object";
            return 0;
        }

        // can only give to empires with something present to receive the gift
        bool recipient_has_something_here = false;
        auto system_objects = Objects().FindObjects<const UniverseObject>(system->ObjectIDs());
        for (auto& system_object : system_objects) {
            if (system_object->Owner() == recipient_id) {
                recipient_has_something_here = true;
                break;
            }
        }
        if (!recipient_has_something_here) {
            ErrorLogger() << "IssueGiveObjectToEmpireOrder : recipient empire has nothing in system";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<GiveObjectToEmpireOrder>(empire_id, object_id, recipient_id));

        return 1;
    }

    int IssueChangeFocusOrder(int planet_id, const std::string& focus) {
        int empire_id = AIClientApp::GetApp()->EmpireID();

        auto planet = GetPlanet(planet_id);
        if (!planet) {
            ErrorLogger() << "IssueChangeFocusOrder : no planet with passed planet_id "<<planet_id;
            return 0;
        }
        if (!planet->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueChangeFocusOrder : empire does not own planet with passed planet_id";
            return 0;
        }
        if (false) {    // todo: verify that focus is valid for specified planet
            ErrorLogger() << "IssueChangeFocusOrder : invalid focus specified";
            return 0;
        }

        AIClientApp::GetApp()->Orders().IssueOrder(
            std::make_shared<ChangeFocusOrder>(empire_id, planet_id, focus));

        return 1;
    }

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
        AIClientApp::GetApp()->StartTurn(); // encodes order sets and sends turn orders message.  "done" the turn for the client, but "starts" the turn for the server
    }

    void LogOutput(const std::string& log_text)
    { DebugLogger() << log_text; }

    void ErrorOutput(const std::string& error_text)
    { ErrorLogger() << error_text; }
} // namespace AIInterface
