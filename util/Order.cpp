#include "Order.h"

#include "Logger.h"
#include "OrderSet.h"
#include "../universe/Fleet.h"
#include "../universe/Predicates.h"
#include "../universe/Species.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/Ship.h"
#include "../universe/ShipDesign.h"
#include "../universe/System.h"
#include "../universe/Pathfinder.h"
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../universe/Enums.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"

#include <boost/uuid/nil_generator.hpp>

#include <fstream>
#include <vector>


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
Empire* Order::GetValidatedEmpire() const {
    auto empire = GetEmpire(EmpireID());
    if (!empire)
        throw std::runtime_error("Invalid empire ID specified for order.");
    return empire;
}

void Order::Execute() const {
    if (m_executed)
        return;

    ExecuteImpl();
    m_executed = true;
}

bool Order::Undo() const {
    auto undone =  UndoImpl();
    if (undone)
        m_executed = false;
    return undone;
}

bool Order::Executed() const
{ return m_executed; }

bool Order::UndoImpl() const
{ return false; }

////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder(int empire, int object, const std::string& name) :
    Order(empire),
    m_object(object),
    m_name(name)
{
    if (!Check(empire, object, name)) {
        m_object = INVALID_OBJECT_ID;
        return;
    }
}

bool RenameOrder::Check(int empire, int object, const std::string& new_name) {
    // disallow the name "", since that denotes an unknown object
    if (new_name.empty()) {
        ErrorLogger() << "RenameOrder::Check() : passed an empty new_name.";
        return false;
    }

    auto obj = GetUniverseObject(object);

    if (!obj) {
        ErrorLogger() << "RenameOrder::Check() : passed an invalid object.";
        return false;
    }

    // verify that empire specified in order owns specified object
    if (!obj->OwnedBy(empire)) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object << " is"
                      << " not owned by empire " << empire << ".";
        return false;
    }

    if (obj->Name() == new_name) {
        ErrorLogger() << "RenameOrder::Check() : Object " << object
                      << " should renamed to the same name.";
        return false;
    }

    return true;
}

void RenameOrder::ExecuteImpl() const {
    if (!Check(EmpireID(), m_object, m_name))
        return;

    GetValidatedEmpire();

    auto obj = GetUniverseObject(m_object);

    obj->Rename(m_name);
}

////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name,
                             const std::vector<int>& ship_ids,
                             bool aggressive) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_fleet_id(INVALID_OBJECT_ID),
    m_ship_ids(ship_ids),
    m_aggressive(aggressive)
{
    if (!Check(empire, fleet_name, ship_ids, aggressive))
        return;
}

bool NewFleetOrder::Check(int empire, const std::string& fleet_name, const std::vector<int>& ship_ids, bool aggressive) {
    if (ship_ids.empty()) {
        ErrorLogger() << "Empire attempted to create a new fleet without ships";
        return false;
    }

    int system_id = INVALID_OBJECT_ID;

    for (int ship_id : ship_ids) {
        // verify that empire is not trying to take ships from somebody else's fleet
        auto ship = GetShip(ship_id);
        if (!ship) {
            ErrorLogger() << "Empire attempted to create a new fleet with an invalid ship";
            return false;
        }
        if (!ship->OwnedBy(empire)) {
            ErrorLogger() << "Empire attempted to create a new fleet with ships from another's fleet.";
            return false;
        }
        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "Empire to create a new fleet with traveling ships.";
            return false;
        }

        if (system_id == INVALID_OBJECT_ID)
            system_id = ship->SystemID();

        if (ship->SystemID() != system_id) {
            ErrorLogger() << "Empire attempted to make a new fleet from ship in the wrong system";
            return false;
        }
    }

    if (system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire attempted to create a new fleet outside a system";
        return false;
    }
    auto system = GetSystem(system_id);
    if (!system) {
        ErrorLogger() << "Empire attempted to create a new fleet in a nonexistant system";
        return false;
    }

    return true;
}

void NewFleetOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_fleet_name, m_ship_ids, m_aggressive))
        return;

    GetUniverse().InhibitUniverseObjectSignals(true);

    // validate specified ships
    std::vector<std::shared_ptr<Ship>> validated_ships;
    for (int ship_id : m_ship_ids)
        validated_ships.push_back(GetShip(ship_id));

    int system_id = validated_ships[0]->SystemID();
    auto system = GetSystem(system_id);

    std::shared_ptr<Fleet> fleet;
    if (m_fleet_id == INVALID_OBJECT_ID) {
        // create fleet
        fleet = GetUniverse().InsertNew<Fleet>(m_fleet_name, system->X(), system->Y(), EmpireID());
        m_fleet_id = fleet->ID();
    } else {
        fleet = GetUniverse().InsertByEmpireWithID<Fleet>(
            EmpireID(), m_fleet_id, m_fleet_name, system->X(), system->Y(), EmpireID());
    }

    if (!fleet) {
        ErrorLogger() << "Unable to create fleet.";
        return;
    }

    fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
    fleet->SetAggressive(m_aggressive);

    // an ID is provided to ensure consistancy between server and client universes
    GetUniverse().SetEmpireObjectVisibility(EmpireID(), fleet->ID(), VIS_FULL_VISIBILITY);

    system->Insert(fleet);

    // new fleet will get same m_arrival_starlane as fleet of the first ship in the list.
    auto first_ship = validated_ships[0];
    auto first_fleet = GetFleet(first_ship->FleetID());
    if (first_fleet)
        fleet->SetArrivalStarlane(first_fleet->ArrivalStarlane());

    std::unordered_set<std::shared_ptr<Fleet>> modified_fleets;
    // remove ships from old fleet(s) and add to new
    for (auto& ship : validated_ships) {
        if (auto old_fleet = GetFleet(ship->FleetID())) {
            modified_fleets.insert(old_fleet);
            old_fleet->RemoveShips({ship->ID()});
        }
        ship->SetFleetID(fleet->ID());
    }
    fleet->AddShips(m_ship_ids);

    if (m_fleet_name.empty())
        fleet->Rename(fleet->GenerateFleetName());

    GetUniverse().InhibitUniverseObjectSignals(false);

    std::vector<std::shared_ptr<Fleet>> created_fleets{fleet};
    system->FleetsInsertedSignal(created_fleets);
    system->StateChangedSignal();

    // Signal changed state of modified fleets and remove any empty fleets.
    for (auto& modified_fleet : modified_fleets) {
        if (!modified_fleet->Empty())
            modified_fleet->StateChangedSignal();
        else {
            if (auto modified_fleet_system = GetSystem(modified_fleet->SystemID()))
                modified_fleet_system->Remove(modified_fleet->ID());

            GetUniverse().Destroy(modified_fleet->ID());
        }
    }

}

////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, int dest_system_id,
                               bool append) :
    Order(empire_id),
    m_fleet(fleet_id),
    m_dest_system(dest_system_id),
    m_route(),
    m_append(append)
{
    if (!Check(empire_id, fleet_id, dest_system_id))
        return;

    auto fleet = GetFleet(FleetID());

    int start_system = fleet->SystemID();
    if (start_system == INVALID_OBJECT_ID)
        start_system = fleet->NextSystemID();
    if (append && !fleet->TravelRoute().empty())
        start_system = fleet->TravelRoute().back();

    auto short_path = GetPathfinder()->ShortestPath(start_system, m_dest_system, EmpireID());

    std::copy(short_path.first.begin(), short_path.first.end(), std::back_inserter(m_route));

    // ensure a zero-length (invalid) route is not requested / sent to a fleet
    if (m_route.empty())
        m_route.push_back(start_system);
}

bool FleetMoveOrder::Check(int empire_id, int fleet_id, int dest_system_id, bool append) {
    auto fleet = GetFleet(fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet with id " << fleet_id << " to move, but no such fleet exists";
        return false;
    }

    if (!fleet->OwnedBy(empire_id) ) {
        ErrorLogger() << "Empire with id " << empire_id << " order to move but does not own fleet with id " << fleet_id;
        return false;
    }

    int start_system = fleet->SystemID();
    if (start_system == INVALID_OBJECT_ID)
        start_system = fleet->NextSystemID();

    auto dest_system = GetEmpireKnownSystem(dest_system_id, empire_id);
    if (!dest_system) {
        ErrorLogger() << "Empire with id " << empire_id << " ordered fleet to move to system with id " << dest_system_id << " but no such system is known to that empire";
        return false;
    }

    // verify fleet route first system
    if (append && !fleet->TravelRoute().empty()) {
        // We should append and there is something to append to
        int last_system = fleet->TravelRoute().back();
        if (last_system != start_system) {
            ErrorLogger() << "Empire with id " << empire_id
                          << " ordered a fleet to continue from system with id " << start_system
                          << ", but the fleet's current route won't lead there, it leads to system " << last_system;
            return false;
        }
    }

    return true;
}

void FleetMoveOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_fleet, m_dest_system))
        return;

    // convert list of ids to list of System
    std::list<int> route_list;

    auto fleet = GetFleet(FleetID());

    if (m_append && !fleet->TravelRoute().empty()){
        route_list = fleet->TravelRoute();
        route_list.erase(--route_list.end());// Remove the last one since it is the first one of the other
    }

    std::copy(m_route.begin(), m_route.end(), std::back_inserter(route_list));

    // check destination validity: disallow movement that's out of range
    auto eta = fleet->ETA(fleet->MovePath(route_list));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl rejected out of range move order";
        return;
    }

    DebugLogger() << [fleet, route_list]() {
        std::stringstream ss;
        ss << "FleetMoveOrder::ExecuteImpl Setting route of fleet " << fleet->ID() << " to ";
        for (int waypoint : route_list)
            ss << " " << std::to_string(waypoint);
        return ss.str();
    }();

    fleet->SetRoute(route_list);
}

////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder(int empire, int dest_fleet,
                                       const std::vector<int>& ships) :
    Order(empire),
    m_dest_fleet(dest_fleet),
    m_add_ships(ships)
{
    if (!Check(empire, dest_fleet, ships))
        return;
}

bool FleetTransferOrder::Check(int empire_id, int dest_fleet_id, const std::vector<int>& ship_ids) {
    auto fleet = GetFleet(dest_fleet_id);
    if (!fleet) {
        ErrorLogger() << "Empire attempted to move ships to a nonexistant fleet";
        return false;
    }
    // check that destination fleet is owned by empire
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueFleetTransferOrder : passed fleet_id "<< dest_fleet_id << " of fleet not owned by player";
        return false;
    }

    if (fleet->SystemID() == INVALID_OBJECT_ID) {
        ErrorLogger() << "IssueFleetTransferOrder : new fleet is not in a system";
        return false;
    }

    bool invalid_ships {false};

    for (auto ship : Objects().FindObjects<Ship>(ship_ids)) {
        if (!ship) {
            ErrorLogger() << "IssueFleetTransferOrder : passed an invalid ship_id";
            invalid_ships = true;
            break;
        }

        if (!ship->OwnedBy(empire_id)) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship_id of ship not owned by player";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() == INVALID_OBJECT_ID) {
            ErrorLogger() << "IssueFleetTransferOrder : ship is not in a system";
            invalid_ships = true;
            break;
        }

        if (ship->SystemID() != fleet->SystemID()) {
            ErrorLogger() << "IssueFleetTransferOrder : passed ship is not in the same system as the target fleet";
            invalid_ships = true;
            break;
        }
    }

    return !invalid_ships;
}

void FleetTransferOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), DestinationFleet(), m_add_ships))
        return;

    // look up the destination fleet
    auto target_fleet = GetFleet(DestinationFleet());

    // check that all ships are in the same system
    auto ships = Objects().FindObjects<Ship>(m_add_ships);

    GetUniverse().InhibitUniverseObjectSignals(true);

    // remove from old fleet(s)
    std::set<std::shared_ptr<Fleet>> modified_fleets;
    for (auto& ship : ships) {
        if (auto source_fleet = GetFleet(ship->FleetID())) {
            source_fleet->RemoveShips({ship->ID()});
            modified_fleets.insert(source_fleet);
        }
        ship->SetFleetID(target_fleet->ID());
    }

    // add to new fleet
    std::vector<int> validated_ship_ids;
    validated_ship_ids.reserve(m_add_ships.size());

    for (auto& ship : ships)
        validated_ship_ids.push_back(ship->ID());

    target_fleet->AddShips(validated_ship_ids);

    GetUniverse().InhibitUniverseObjectSignals(false);

    // signal change to fleet states
    modified_fleets.insert(target_fleet);

    for (auto& modified_fleet : modified_fleets) {
        if (!modified_fleet->Empty())
            modified_fleet->StateChangedSignal();
        else {
            if (auto system = GetSystem(modified_fleet->SystemID()))
                system->Remove(modified_fleet->ID());

            GetUniverse().Destroy(modified_fleet->ID());
        }
    }
}

////////////////////////////////////////////////
// ColonizeOrder
////////////////////////////////////////////////
ColonizeOrder::ColonizeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if (!Check(empire, ship, planet))
        return;
}

bool ColonizeOrder::Check(int empire_id, int ship_id, int planet_id) {
    auto ship = GetShip(ship_id);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::Check() : passed an invalid ship_id " << ship_id;
        return false;
    }
    auto fleet = GetFleet(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "ColonizeOrder::Check() : ship with passed ship_id has invalid fleet_id";
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : empire does not own fleet of passed ship";
        return 0;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::Check() : got ship that isn't owned by the order-issuing empire";
        return false;
    }

    if (!ship->CanColonize()) { // verifies that species exists and can colonize and that ship can colonize
        ErrorLogger() << "ColonizeOrder::Check() : got ship that can't colonize";
        return false;
    }

    auto planet = GetPlanet(planet_id);
    float colonist_capacity = ship->ColonyCapacity();
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::Check() : couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->InitialMeterValue(METER_POPULATION) > 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that already has population";
        return false;
    }
    if (!planet->Unowned() && planet->Owner() != empire_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that owned by another empire";
        return false;
    }
    if (planet->OwnedBy(empire_id) && colonist_capacity == 0.0f) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that is already owned by empire and colony ship with zero capcity";
        return false;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) < VIS_PARTIAL_VISIBILITY) {
        ErrorLogger() << "ColonizeOrder::Check() : given planet that empire has insufficient visibility of";
        return false;
    }
    if (colonist_capacity > 0.0f && planet->EnvironmentForSpecies(ship->SpeciesName()) < PE_HOSTILE) {
        ErrorLogger() << "ColonizeOrder::Check() : nonzero colonist capacity, " << colonist_capacity
                      << ", and planet " << planet->Name() << " of type, " << planet->Type() << ", that ship's species, "
                      << ship->SpeciesName() << ", can't colonize";
        return false;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "ColonizeOrder::Check() : given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "ColonizeOrder::Check() : given ids of ship and planet not in the same system";
        return false;
    }
    if (planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::Check() : given id planet that is already being colonized";
        return false;
    }

    return true;
}

void ColonizeOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_ship, m_planet))
        return;

    auto ship = GetShip(m_ship);
    auto planet = GetPlanet(m_planet);

    planet->SetIsAboutToBeColonized(true);
    ship->SetColonizePlanet(m_planet);

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool ColonizeOrder::UndoImpl() const {
    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }
    if (!planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::UndoImpl planet is not about to be colonized...";
        return false;
    }

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedColonizePlanet() != m_planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl ship is not about to colonize planet";
        return false;
    }

    planet->SetIsAboutToBeColonized(false);
    ship->ClearColonizePlanet();

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// InvadeOrder
////////////////////////////////////////////////
InvadeOrder::InvadeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if(!Check(empire, ship, planet))
        return;
}

bool InvadeOrder::Check(int empire_id, int ship_id, int planet_id) {
    // make sure ship_id is a ship...
    auto ship = GetShip(ship_id);
    if (!ship) {
        ErrorLogger() << "IssueInvadeOrder : passed an invalid ship_id";
        return false;
    }

    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder : empire does not own passed ship";
        return false;
    }
    if (!ship->HasTroops()) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl got ship that can't invade";
        return false;
    }

    // get fleet of ship
    auto fleet = GetFleet(ship->FleetID());
    if (!fleet) {
        ErrorLogger() << "IssueInvadeOrder : ship with passed ship_id has invalid fleet_id";
        return false;
    }

    // make sure player owns ship and its fleet
    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueInvadeOrder : empire does not own fleet of passed ship";
        return false;
    }

    auto planet = GetPlanet(planet_id);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl couldn't get planet with id " << planet_id;
        return false;
    }

    if (ship->SystemID() != planet->SystemID()) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return false;
    }

    if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) < VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
        return false;
    }

    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return false;
    }

    if (planet->Unowned() && planet->InitialMeterValue(METER_POPULATION) == 0.0) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given unpopulated planet";
        return false;
    }

    if (planet->InitialMeterValue(METER_SHIELD) > 0.0) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet with shield > 0";
        return false;
    }

    if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return false;
    }

    return true;
}

void InvadeOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if(!Check(EmpireID(), m_ship, m_planet))
        return;

    auto ship = GetShip(m_ship);
    auto planet = GetPlanet(m_planet);

    // note: multiple ships, from same or different empires, can invade the same planet on the same turn
    DebugLogger() << "InvadeOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to invade planet " << m_planet << " " << planet->Name();
    planet->SetIsAboutToBeInvaded(true);
    ship->SetInvadePlanet(m_planet);

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool InvadeOrder::UndoImpl() const {
    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedInvadePlanet() != m_planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl ship is not about to invade planet";
        return false;
    }

    planet->SetIsAboutToBeInvaded(false);
    ship->ClearInvadePlanet();

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// BombardOrder
////////////////////////////////////////////////
BombardOrder::BombardOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
    if(!Check(empire, ship, planet))
        return;
}

bool BombardOrder::Check(int empire_id, int ship_id, int planet_id) {
    auto ship = GetShip(ship_id);
    if (!ship) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get ship with id " << ship_id;
        return false;
    }
    if (!ship->CanBombard()) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that can't bombard";
        return false;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return false;
    }
    if (ship->TotalWeaponsDamage() <= 0.0f) {   // this will test the current meter values. potential issue if some local change sets these to zero even though they will be nonzero on server when bombard is processed before effects application / meter update
        ErrorLogger() << "IssueBombardOrder : ship can't attack / bombard";
        return false;
    }

    auto planet = GetPlanet(planet_id);
    if (!planet) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get planet with id " << planet_id;
        return false;
    }
    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return false;
    }
    if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return false;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(planet_id, empire_id) < VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given id of ship not in a system";
        return false;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return false;
    }

    return true;
}

void BombardOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if(!Check(EmpireID(), m_ship, m_planet))
        return;

    auto ship = GetShip(m_ship);
    auto planet = GetPlanet(m_planet);

    // note: multiple ships, from same or different empires, can bombard the same planet on the same turn
    DebugLogger() << "BombardOrder::ExecuteImpl set for ship " << m_ship << " "
                  << ship->Name() << " to bombard planet " << m_planet << " "
                  << planet->Name();
    planet->SetIsAboutToBeBombarded(true);
    ship->SetBombardPlanet(m_planet);

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool BombardOrder::UndoImpl() const {
    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedBombardPlanet() != m_planet) {
        ErrorLogger() << "BombardOrder::UndoImpl ship is not about to bombard planet";
        return false;
    }

    planet->SetIsAboutToBeBombarded(false);
    ship->ClearBombardPlanet();

    if (auto fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, const std::string& focus) :
    Order(empire),
    m_planet(planet),
    m_focus(focus)
{
    if (!Check(empire, planet, focus))
        return;
}

bool ChangeFocusOrder::Check(int empire_id, int planet_id, const std::string& focus) {
    auto planet = GetPlanet(planet_id);

    if (!planet) {
        ErrorLogger() << "Illegal planet id specified in change planet focus order.";
        return false;
    }

    if (!planet->OwnedBy(empire_id)) {
        ErrorLogger() << "Empire attempted to issue change planet focus to another's planet.";
        return false;
    }

    if (false) {    // todo: verify that focus is valid for specified planet
        ErrorLogger() << "IssueChangeFocusOrder : invalid focus specified";
        return false;
    }

    return true;
}

void ChangeFocusOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_planet, m_focus))
        return;

    auto planet = GetPlanet(m_planet);

    planet->SetFocus(m_focus);
}

////////////////////////////////////////////////
// ResearchQueueOrder
////////////////////////////////////////////////
ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name) :
    Order(empire),
    m_tech_name(tech_name),
    m_remove(true)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, int position) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(position)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, bool pause, float dummy) :
    Order(empire),
    m_tech_name(tech_name),
    m_pause(pause ? PAUSE : RESUME)
{}

void ResearchQueueOrder::ExecuteImpl() const {
    auto empire = GetValidatedEmpire();

    if (m_remove) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: removing from queue tech: " << m_tech_name;
        empire->RemoveTechFromQueue(m_tech_name);
    } else if (m_pause == PAUSE) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: pausing tech: " << m_tech_name;
        empire->PauseResearch(m_tech_name);
    } else if (m_pause == RESUME) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: unpausing tech: " << m_tech_name;
        empire->ResumeResearch(m_tech_name);
    } else if (m_position != INVALID_INDEX) {
        DebugLogger() << "ResearchQueueOrder::ExecuteImpl: adding tech to queue: " << m_tech_name;
        empire->PlaceTechInQueue(m_tech_name, m_position);
    } else {
        ErrorLogger() << "ResearchQueueOrder::ExecuteImpl: Malformed";
    }
}

////////////////////////////////////////////////
// ProductionQueueOrder
////////////////////////////////////////////////
ProductionQueueOrder::ProductionQueueOrder(int empire, const ProductionQueue::ProductionItem& item,
                                           int number, int location, int pos) :
    Order(empire),
    m_item(item),
    m_number(number),
    m_location(location),
    m_new_index(pos)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, int new_blocksize) :
    Order(empire),
    m_index(index),
    m_new_quantity(new_quantity),
    m_new_blocksize(new_blocksize)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy) :
    Order(empire),
    m_index(index),
    m_new_quantity(new_quantity)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int rally_point_id, bool dummy1, bool dummy2) :
    Order(empire),
    m_index(index),
    m_rally_point_id(rally_point_id)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_index) :
    Order(empire),
    m_index(index),
    m_new_index(new_index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index) :
    Order(empire),
    m_index(index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, bool pause, float dummy) :
    Order(empire),
    m_index(index),
    m_pause(pause ? PAUSE : RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, float dummy1) :
    Order(empire),
    m_index(index),
    m_split_incomplete(index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, float dummy1, float dummy2) :
    Order(empire),
    m_index(index),
    m_dupe(index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, bool allow_use_imperial_pp, float dummy, float dummy2) :
    Order(empire),
    m_index(index),
    m_use_imperial_pp(allow_use_imperial_pp ? USE_IMPERIAL_PP : DONT_USE_IMPERIAL_PP)
{}

void ProductionQueueOrder::ExecuteImpl() const {
    auto empire = GetValidatedEmpire();

    try {
        if (m_item.build_type == BT_BUILDING || m_item.build_type == BT_SHIP || m_item.build_type == BT_STOCKPILE) {
            DebugLogger() << "ProductionQueueOrder place " << m_item.Dump();
            empire->PlaceProductionOnQueue(m_item, m_number, 1, m_location, m_new_index);

        } else if (m_split_incomplete != INVALID_SPLIT_INCOMPLETE) {
            DebugLogger() << "ProductionQueueOrder splitting incomplete from item";
            empire->SplitIncompleteProductionItem(m_index);

        } else if (m_dupe != INVALID_SPLIT_INCOMPLETE) {
            DebugLogger() << "ProductionQueueOrder duplicating item";
            empire->DuplicateProductionItem(m_index);

        } else if (m_new_blocksize != INVALID_QUANTITY) {
            DebugLogger() << "ProductionQueueOrder quantity " << m_new_quantity << " Blocksize " << m_new_blocksize;
            empire->SetProductionQuantityAndBlocksize(m_index, m_new_quantity, m_new_blocksize);

        } else if (m_new_quantity != INVALID_QUANTITY) {
            DebugLogger() << "ProductionQueueOrder quantity " << m_new_quantity;
            empire->SetProductionQuantity(m_index, m_new_quantity);

        } else if (m_new_index != INVALID_INDEX) {
            DebugLogger() << "ProductionQueueOrder moving item in queue";
            empire->MoveProductionWithinQueue(m_index, m_new_index);

        } else if (m_rally_point_id != INVALID_OBJECT_ID) {
            DebugLogger() << "ProductionQueueOrder setting rally point to id: " << m_rally_point_id;
            empire->SetProductionRallyPoint(m_index, m_rally_point_id);

        } else if (m_index != INVALID_INDEX) {
            if (m_pause == PAUSE) {
                DebugLogger() << "ProductionQueueOrder: pausing production";
                empire->PauseProduction(m_index);

            } else if (m_pause == RESUME) {
                DebugLogger() << "ProductionQueueOrder: unpausing production";
                empire->ResumeProduction(m_index);

            } else if (m_use_imperial_pp == USE_IMPERIAL_PP) {
                DebugLogger() << "ProductionQueueOrder: allow use of imperial PP stockpile";
                empire->AllowUseImperialPP(m_index, true); 

            } else if (m_use_imperial_pp == DONT_USE_IMPERIAL_PP) {
                DebugLogger() << "ProductionQueueOrder: disallow use of imperial PP stockpile";
                empire->AllowUseImperialPP(m_index, false);

            } else /*if (m_pause == INVALID_PAUSE_RESUME)*/ {
                DebugLogger() << "ProductionQueueOrder: removing item from index " << m_index;
                empire->RemoveProductionFromQueue(m_index);
            }
        } else {
            ErrorLogger() << "ProductionQueueOrder: Malformed";
        }
    } catch (const std::exception& e) {
        ErrorLogger() << "Build order execution threw exception: " << e.what();
    }
}

////////////////////////////////////////////////
// ShipDesignOrder
////////////////////////////////////////////////
ShipDesignOrder::ShipDesignOrder() :
    m_uuid(boost::uuids::nil_generator()())
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_design_id(existing_design_id_to_remember),
    m_uuid(boost::uuids::nil_generator()())
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_design_id(design_id_to_erase),
    m_uuid(boost::uuids::nil_generator()()),
    m_delete_design_from_empire(true)
{}

ShipDesignOrder::ShipDesignOrder(int empire, const ShipDesign& ship_design) :
    Order(empire),
    m_design_id(INVALID_DESIGN_ID),
    m_uuid(ship_design.UUID()),
    m_create_new_design(true),
    m_name(ship_design.Name(false)),
    m_description(ship_design.Description(false)),
    m_designed_on_turn(ship_design.DesignedOnTurn()),
    m_hull(ship_design.Hull()),
    m_parts(ship_design.Parts()),
    m_is_monster(ship_design.IsMonster()),
    m_icon(ship_design.Icon()),
    m_3D_model(ship_design.Model()),
    m_name_desc_in_stringtable(ship_design.LookupInStringtable())
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id, const std::string& new_name/* = ""*/, const std::string& new_description/* = ""*/) :
    Order(empire),
    m_design_id(existing_design_id),
    m_uuid(boost::uuids::nil_generator()()),
    m_update_name_or_description(true),
    m_name(new_name),
    m_description(new_description)
{}

void ShipDesignOrder::ExecuteImpl() const {
    auto empire = GetValidatedEmpire();

    Universe& universe = GetUniverse();

    if (m_delete_design_from_empire) {
        // player is ordering empire to forget about a particular design
        if (!empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remove a ShipDesign id = " << m_design_id
                          << " that the empire wasn't remembering";
            return;
        }
        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        // check if a design with this ID already exists
        if (const auto existing = universe.GetShipDesign(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to create a new ShipDesign with an id, " << m_design_id
                          << " of an already-existing ShipDesign " << existing->Name();

            return;
        }

        ShipDesign* new_ship_design;
        try {
            new_ship_design = new ShipDesign(std::invalid_argument(""), m_name, m_description,
                                             m_designed_on_turn, EmpireID(), m_hull, m_parts,
                                             m_icon, m_3D_model, m_name_desc_in_stringtable,
                                             m_is_monster, m_uuid);
        } catch (const std::invalid_argument&) {
            ErrorLogger() << "Couldn't create ship design.";
            return;
        }

        if (m_design_id == INVALID_DESIGN_ID) {
            // On the client create a new design id
            universe.InsertShipDesign(new_ship_design);
            m_design_id = new_ship_design->ID();
        } else {
            // On the server use the design id passed from the client
            universe.InsertShipDesignID(new_ship_design, EmpireID(), m_design_id);
        }

        universe.SetEmpireKnowledgeOfShipDesign(m_design_id, EmpireID());
        empire->AddShipDesign(m_design_id);

    } else if (m_update_name_or_description) {
        // player is ordering empire to rename a design
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        auto design_it = empire_known_design_ids.find(m_design_id);
        if (design_it == empire_known_design_ids.end()) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that this empire hasn't seen";
            return;
        }
        const ShipDesign* design = GetShipDesign(*design_it);
        if (!design) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that doesn't exist (but this empire has seen it)!";
            return;
        }
        if (design->DesignedByEmpire() != EmpireID()) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to rename/redescribe a ShipDesign id = " << m_design_id
                          << " that isn't owned by this empire!";
            return;
        }
        GetUniverse().RenameShipDesign(m_design_id, m_name, m_description);

    } else {
        // player is ordering empire to retain a particular design, so that is can
        // be used to construct ships by that empire.

        // TODO: consider removing this order, so that an empire needs to use
        // espionage or trade to gain access to a ship design made by another
        // player

        // check if empire is already remembering the design
        if (empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remember a ShipDesign id = " << m_design_id
                          << " that was already being remembered";
            return;
        }

        // check if the empire can see any objects that have this design (thus enabling it to be copied)
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        if (empire_known_design_ids.count(m_design_id)) {
            empire->AddShipDesign(m_design_id);
        } else {
            ErrorLogger() << "Empire, " << EmpireID() << ", tried to remember a ShipDesign id = " << m_design_id
                          << " that this empire hasn't seen";
            return;
        }

    }
}

////////////////////////////////////////////////
// ScrapOrder
////////////////////////////////////////////////
ScrapOrder::ScrapOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{
    if (!Check(empire, object_id))
        return;
}

bool ScrapOrder::Check(int empire_id, int object_id) {
    auto obj = GetUniverseObject(object_id);

    if (!obj) {
        ErrorLogger() << "IssueScrapOrder : passed an invalid object_id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueScrapOrder : passed object_id of object not owned by player";
        return false;
    }

    if (obj->ObjectType() != OBJ_SHIP && obj->ObjectType() != OBJ_BUILDING) {
        ErrorLogger() << "ScrapOrder::Check : passed object that is not a ship or building";
        return false;
    }

    auto ship = GetShip(object_id);
    if (ship && ship->SystemID() == INVALID_OBJECT_ID) {
        ErrorLogger() << "ScrapOrder::Check : can scrap a traveling ship";
    }

    return true;
}

void ScrapOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_object_id))
        return;

    if (auto ship = GetShip(m_object_id)) {
        ship->SetOrderedScrapped(true);
    } else if (std::shared_ptr<Building> building = GetBuilding(m_object_id)) {
        building->SetOrderedScrapped(true);
    }
}

bool ScrapOrder::UndoImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    if (auto ship = GetShip(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (auto building = GetBuilding(m_object_id)) {
        if (building->OwnedBy(empire_id))
            building->SetOrderedScrapped(false);
    } else {
        return false;
    }
    return true;
}

////////////////////////////////////////////////
// AggressiveOrder
////////////////////////////////////////////////
AggressiveOrder::AggressiveOrder(int empire, int object_id, bool aggression/* = true*/) :
    Order(empire),
    m_object_id(object_id),
    m_aggression(aggression)
{
    if (!Check(empire, object_id, aggression))
        return;
}

bool AggressiveOrder::Check(int empire_id, int object_id, bool aggression) {
    auto fleet = GetFleet(object_id);
    if (!fleet) {
        ErrorLogger() << "IssueAggressionOrder : no fleet with passed id";
        return false;
    }

    if (!fleet->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueAggressionOrder : passed object_id of object not owned by player";
        return false;
    }

    return true;
}

void AggressiveOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_object_id, m_aggression))
        return;

    auto fleet = GetFleet(m_object_id);

    fleet->SetAggressive(m_aggression);
}

/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
GiveObjectToEmpireOrder::GiveObjectToEmpireOrder(int empire, int object_id, int recipient) :
    Order(empire),
    m_object_id(object_id),
    m_recipient_empire_id(recipient)
{
    if (!Check(empire, object_id, recipient))
        return;
}

bool GiveObjectToEmpireOrder::Check(int empire_id, int object_id, int recipient_empire_id) {
    if (GetEmpire(recipient_empire_id) == 0) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : given invalid recipient empire id";
        return false;
    }

    if (Empires().GetDiplomaticStatus(empire_id, recipient_empire_id) != DIPLO_PEACE) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : attempting to give to empire not at peace";
        return false;
    }

    auto obj = GetUniverseObject(object_id);
    if (!obj) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed invalid object id";
        return false;
    }

    if (!obj->OwnedBy(empire_id)) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object not owned by player";
        return false;
    }

    auto system = GetSystem(obj->SystemID());
    if (!system) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : couldn't get system of object";
        return false;
    }

    if (obj->ObjectType() != OBJ_FLEET && obj->ObjectType() != OBJ_PLANET) {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : passed object that is not a fleet or planet";
        return false;
    }

    auto system_objects = Objects().FindObjects<const UniverseObject>(system->ObjectIDs());
    if (std::any_of(system_objects.begin(), system_objects.end(),
                     [recipient_empire_id](const std::shared_ptr<const UniverseObject> o){ return o->Owner() == recipient_empire_id; }))
    {
        ErrorLogger() << "IssueGiveObjectToEmpireOrder : recipient empire has nothing in system";
        return false;
    }

    return true;
}

void GiveObjectToEmpireOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (!Check(EmpireID(), m_object_id, m_recipient_empire_id))
        return;

    if (auto fleet = GetFleet(m_object_id))
        fleet->SetGiveToEmpire(m_recipient_empire_id);
    else if (auto planet = GetPlanet(m_object_id))
        planet->SetGiveToEmpire(m_recipient_empire_id);
}

bool GiveObjectToEmpireOrder::UndoImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    if (auto fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id)) {
            fleet->ClearGiveToEmpire();
            return true;
        }
    } else if (auto planet = GetPlanet(m_object_id)) {
        if (planet->OwnedBy(empire_id)) {
            planet->ClearGiveToEmpire();
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////
// ForgetOrder
////////////////////////////////////////////////
ForgetOrder::ForgetOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

void ForgetOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    DebugLogger() << "ForgetOrder::ExecuteImpl empire: " << empire_id
                  << " for object: " << m_object_id;

    GetUniverse().ForgetKnownObject(empire_id, m_object_id);
}
