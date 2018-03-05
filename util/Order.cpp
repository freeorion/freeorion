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
    auto obj = GetUniverseObject(object);
    if (!obj) {
        ErrorLogger() << "RenameOrder::RenameOrder() : Attempted to rename nonexistant object with id " << object;
        return;
    }

    if (m_name.empty()) {
        ErrorLogger() << "RenameOrder::RenameOrder() : Attempted to name an object \"\".";
        // make order do nothing
        m_object = INVALID_OBJECT_ID;
        return;
    }
}

void RenameOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    auto obj = GetUniverseObject(m_object);

    if (!obj) {
        ErrorLogger() << "Attempted to rename nonexistant object with id " << m_object;
        return;
    }

    // verify that empire specified in order owns specified object
    if (!obj->OwnedBy(EmpireID())) {
        ErrorLogger() << "Empire (" << EmpireID()
                      << ") specified in rename order does not own specified object which is owned by "
                      << obj->Owner() << ".";
        return;
    }

    // disallow the name "", since that denotes an unknown object
    if (m_name.empty()) {
        ErrorLogger() << "Name \"\" specified in rename order is invalid.";
        return;
    }

    obj->Rename(m_name);
}

////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name,
                             int system_id, const std::vector<int>& ship_ids,
                             bool aggressive) :
    NewFleetOrder(empire, std::vector<std::string>(1, fleet_name),
                  system_id, std::vector<std::vector<int>>(1, ship_ids),
                  std::vector<bool>(1, aggressive) )
{}

NewFleetOrder::NewFleetOrder(int empire, const std::vector<std::string>& fleet_names,
                             int system_id,
                             const std::vector<std::vector<int>>& ship_id_groups,
                             const std::vector<bool>& aggressives) :
    Order(empire),
    m_fleet_names(fleet_names),
    m_system_id(system_id),
    m_fleet_ids(std::vector<int>(m_fleet_names.size(), INVALID_OBJECT_ID)),
    m_ship_id_groups(ship_id_groups),
    m_aggressives(aggressives)
{}

void NewFleetOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    if (m_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire attempted to create a new fleet outside a system";
        return;
    }
    auto system = GetSystem(m_system_id);
    if (!system) {
        ErrorLogger() << "Empire attempted to create a new fleet in a nonexistant system";
        return;
    }

    if (m_fleet_names.empty())
        return;
    if (m_fleet_names.size() != m_fleet_ids.size()
        || m_fleet_names.size() != m_ship_id_groups.size()
        || m_fleet_names.size() != m_aggressives.size())
    {
        ErrorLogger() << "NewFleetOrder has inconsistent data container sizes...";
        return;
    }

    GetUniverse().InhibitUniverseObjectSignals(true);
    std::vector<std::shared_ptr<Fleet>> created_fleets;
    created_fleets.reserve(m_ship_id_groups.size());

    std::unordered_set<std::shared_ptr<Fleet>> modified_fleets;

    // create fleet for each group of ships
    for (int i = 0; i < static_cast<int>(m_ship_id_groups.size()); ++i) {
        const auto& fleet_name =    m_fleet_names[i];
        int         fleet_id =      m_fleet_ids[i];
        const auto& ship_ids =      m_ship_id_groups[i];
        bool        aggressive =    m_aggressives[i];

        if (ship_ids.empty())
            continue;   // nothing to do...

        // validate specified ships
        std::vector<std::shared_ptr<Ship>> validated_ships;
        std::vector<int>                   validated_ships_ids;
        for (int ship_id : ship_ids) {
            // verify that empire is not trying to take ships from somebody else's fleet
            auto ship = GetShip(ship_id);
            if (!ship) {
                ErrorLogger() << "Empire attempted to create a new fleet with an invalid ship";
                continue;
            }
            if (!ship->OwnedBy(EmpireID())) {
                ErrorLogger() << "Empire attempted to create a new fleet with ships from another's fleet.";
                continue;
            }
            if (ship->SystemID() != m_system_id) {
                ErrorLogger() << "Empire attempted to make a new fleet from ship in the wrong system";
                continue;
            }
            validated_ships.push_back(ship);
            validated_ships_ids.push_back(ship->ID());
        }
        if (validated_ships.empty())
            continue;

        std::shared_ptr<Fleet> fleet;
        if (fleet_id == INVALID_OBJECT_ID) {
            // create fleet
            fleet = GetUniverse().InsertNew<Fleet>(fleet_name, system->X(), system->Y(), EmpireID());
            m_fleet_ids[i] = fleet->ID();
        } else {
            fleet = GetUniverse().InsertByEmpireWithID<Fleet>(
                EmpireID(), fleet_id, fleet_name, system->X(), system->Y(), EmpireID());
        }

        if (!fleet) {
            ErrorLogger() << "Unable to create fleet.";
            return;
        }

        fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
        fleet->SetAggressive(aggressive);

        // an ID is provided to ensure consistancy between server and client universes
        GetUniverse().SetEmpireObjectVisibility(EmpireID(), fleet->ID(), VIS_FULL_VISIBILITY);

        system->Insert(fleet);

        // new fleet will get same m_arrival_starlane as fleet of the first ship in the list.
        auto first_ship = validated_ships[0];
        auto first_fleet = GetFleet(first_ship->FleetID());
        if (first_fleet)
            fleet->SetArrivalStarlane(first_fleet->ArrivalStarlane());

        // remove ships from old fleet(s) and add to new
        for (auto& ship : validated_ships) {
            if (auto old_fleet = GetFleet(ship->FleetID())) {
                modified_fleets.insert(old_fleet);
                old_fleet->RemoveShip(ship->ID());
            }
            ship->SetFleetID(fleet->ID());
        }
        fleet->AddShips(validated_ships_ids);

        if (fleet_name.empty())
            fleet->Rename(fleet->GenerateFleetName());

        created_fleets.push_back(fleet);
    }

    GetUniverse().InhibitUniverseObjectSignals(false);

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
FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, int start_system_id, int dest_system_id,
                               std::vector<int> route, bool append) :
    Order(empire_id),
    m_fleet(fleet_id),
    m_start_system(start_system_id),
    m_dest_system(dest_system_id),
    m_route(route),
    m_append(append)
{
    // perform sanity checks
    auto fleet = GetFleet(FleetID());
    if (!fleet) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    auto destination_system = GetSystem(DestinationSystemID());
    if (!destination_system) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id "
                      << DestinationSystemID() << " but no such system exists / is known to exist";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()) ) {
        ErrorLogger() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }

    if (m_route.empty()) {
        auto short_path = GetPathfinder()->ShortestPath(m_start_system, m_dest_system, EmpireID());

        m_route.clear();
        std::copy(short_path.first.begin(), short_path.first.end(), std::back_inserter(m_route));

        // ensure a zero-length (invalid) route is not requested / sent to a fleet
        if (m_route.empty())
            m_route.push_back(m_start_system);
    }
}

FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, int start_system_id, int dest_system_id, bool append) :
    FleetMoveOrder(empire_id, fleet_id, start_system_id, dest_system_id, std::vector<int>(), append)
{}

FleetMoveOrder::FleetMoveOrder(int empire_id, int fleet_id, std::vector<int> route) :
    FleetMoveOrder(empire_id, fleet_id, *route.begin(), *route.rbegin(), route, false)
{}

void FleetMoveOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    auto fleet = GetFleet(FleetID());
    if (!fleet) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    auto destination_system = GetEmpireKnownSystem(DestinationSystemID(), EmpireID());
    if (!destination_system) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id " << DestinationSystemID() << " but no such system is known to that empire";
        return;
    }

    // reject empty routes
    if (m_route.empty()) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet to move on empty route";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()) ) {
        ErrorLogger() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }


    // verify fleet route first system
    int fleet_sys_id = fleet->SystemID();
    if (!m_append || fleet->TravelRoute().empty()) {
        if (fleet_sys_id != INVALID_OBJECT_ID) {
            // fleet is in a system.  Its move path should also start from that system.
            if (fleet_sys_id != m_start_system) {
                ErrorLogger() << "Empire with id " << EmpireID()
                              << " ordered a fleet to move from a system with id " << m_start_system
                              << " that it is not at.  Fleet is located at system with id " << fleet_sys_id;
                return;
            }
        } else {
            // fleet is not in a system.  Its move path should start from the next system it is moving to.
            int next_system = fleet->NextSystemID();
            if (next_system != m_start_system) {
                ErrorLogger() << "Empire with id " << EmpireID()
                              << " ordered a fleet to move starting from a system with id " << m_start_system
                              << ", but the fleet's next destination is system with id " << next_system;
                return;
            }
        }
    } else {
        // We should append and there is something to append to
        int last_system = fleet->TravelRoute().back();
        if (last_system != m_start_system) {
            ErrorLogger() << "Empire with id " << EmpireID()
                          << " ordered a fleet to continue from system with id " << m_start_system
                          << ", but the fleet's current route won't lead there, it leads to system " << last_system;
            return;
        }
    }


    // convert list of ids to list of System
    std::list<int> route_list;

    if (m_append && !fleet->TravelRoute().empty()){
        route_list = fleet->TravelRoute();
        route_list.erase(--route_list.end());// Remove the last one since it is the first one of the other
    }

    std::copy(m_route.begin(), m_route.end(), std::back_inserter(route_list));


    // validate route.  Only allow travel between systems connected in series by starlanes known to this fleet's owner.

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
{}

void FleetTransferOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    // look up the destination fleet
    auto target_fleet = GetFleet(DestinationFleet());
    if (!target_fleet) {
        ErrorLogger() << "Empire attempted to move ships to a nonexistant fleet";
        return;
    }
    // check that destination fleet is owned by empire
    if (!target_fleet->OwnedBy(EmpireID())) {
        ErrorLogger() << "Empire attempted to move ships to a fleet it does not own";
        return;
    }
    // verify that fleet is in a system
    if (target_fleet->SystemID() == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire attempted to transfer ships to/from fleet(s) not in a system";
        return;
    }

    // check that all ships are in the same system
    auto ships = Objects().FindObjects<Ship>(m_add_ships);

    std::vector<std::shared_ptr<Ship>>  validated_ships;
    validated_ships.reserve(m_add_ships.size());
    std::vector<int>                    validated_ship_ids;
    validated_ship_ids.reserve(m_add_ships.size());

    for (auto& ship : ships) {
        if (!ship->OwnedBy(EmpireID()))
            continue;
        if (ship->SystemID() != target_fleet->SystemID())
            continue;
        if (ship->FleetID() == target_fleet->ID())
            continue;
        validated_ships.push_back(ship);
        validated_ship_ids.push_back(ship->ID());
    }
    if (validated_ships.empty())
        return;

    GetUniverse().InhibitUniverseObjectSignals(true);

    // remove from old fleet(s)
    std::set<std::shared_ptr<Fleet>> modified_fleets;
    for (auto& ship : validated_ships) {
        if (auto source_fleet = GetFleet(ship->FleetID())) {
            source_fleet->RemoveShip(ship->ID());
            modified_fleets.insert(source_fleet);
        }
        ship->SetFleetID(target_fleet->ID());
    }

    // add to new fleet
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
{}

void ColonizeOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl couldn't get ship with id " << m_ship;
        return;
    }
    if (!ship->CanColonize()) { // verifies that species exists and can colonize and that ship can colonize
        ErrorLogger() << "ColonizeOrder::ExecuteImpl got ship that can't colonize";
        return;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return;
    }

    float colonist_capacity = ship->ColonyCapacity();

    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->InitialMeterValue(METER_POPULATION) > 0.0f) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given planet that already has population";
        return;
    }
    if (!planet->Unowned() && planet->Owner() != empire_id) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given planet that owned by another empire";
        return;
    }
    if (planet->OwnedBy(empire_id) && colonist_capacity == 0.0f) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given planet that is already owned by empire and colony ship with zero capcity";
        return;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(m_planet, empire_id) < VIS_PARTIAL_VISIBILITY) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given planet that empire has insufficient visibility of";
        return;
    }
    if (colonist_capacity > 0.0f && planet->EnvironmentForSpecies(ship->SpeciesName()) < PE_HOSTILE) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl nonzero colonist capacity, " << colonist_capacity
                      << ", and planet " << planet->Name() << " of type, " << planet->Type() << ", that ship's species, "
                      << ship->SpeciesName() << ", can't colonize";
        return;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given id of ship not in a system";
        return;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return;
    }
    if (planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl given id planet that is already being colonized";
        return;
    }

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
{}

void InvadeOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl couldn't get ship with id " << m_ship;
        return;
    }
    if (!ship->HasTroops()) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl got ship that can't invade";
        return;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return;
    }

    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->Unowned() && planet->InitialMeterValue(METER_POPULATION) == 0.0) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given unpopulated planet";
        return;
    }
    if (planet->InitialMeterValue(METER_SHIELD) > 0.0) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet with shield > 0";
        return;
    }
    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return;
    }
    if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(m_planet, empire_id) < VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
        //return;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given id of ship not in a system";
        return;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return;
    }

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
{}

void BombardOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    auto ship = GetShip(m_ship);
    if (!ship) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get ship with id " << m_ship;
        return;
    }
    if (!ship->CanBombard()) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that can't bombard";
        return;
    }
    if (!ship->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return;
    }

    auto planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "BombardOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->OwnedBy(empire_id)) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return;
    }
    if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(m_planet, empire_id) < VIS_BASIC_VISIBILITY) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given planet that empire reportedly has insufficient visibility of, but will be allowed to proceed pending investigation";
        //return;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given id of ship not in a system";
        return;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        ErrorLogger() << "BombardOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return;
    }

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
{}

void ChangeFocusOrder::ExecuteImpl() const {
    GetValidatedEmpire();

    auto planet = GetPlanet(PlanetID());

    if (!planet) {
        ErrorLogger() << "Illegal planet id specified in change planet focus order.";
        return;
    }

    if (!planet->OwnedBy(EmpireID())) {
        ErrorLogger() << "Empire attempted to issue change planet focus to another's planet.";
        return;
    }

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
        if (empire_known_design_ids.find(m_design_id) != empire_known_design_ids.end()) {
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
{}

void ScrapOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    if (auto ship = GetShip(m_object_id)) {
        if (ship->SystemID() != INVALID_OBJECT_ID && ship->OwnedBy(empire_id)) {
            ship->SetOrderedScrapped(true);
            //DebugLogger() << "ScrapOrder::ExecuteImpl empire: " << empire_id
            //                       << " on ship: " << ship->ID() << " at system: " << ship->SystemID()
            //                       << " ... ordered scrapped?: " << ship->OrderedScrapped();
        }
    } else if (std::shared_ptr<Building> building = GetBuilding(m_object_id)) {
        int planet_id = building->PlanetID();
        if (auto planet = GetPlanet(planet_id)) {
            if (building->OwnedBy(empire_id) && planet->OwnedBy(empire_id))
                building->SetOrderedScrapped(true);
        }
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
{}

void AggressiveOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();
    if (auto fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id))
            fleet->SetAggressive(m_aggression);
    }
}

/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
GiveObjectToEmpireOrder::GiveObjectToEmpireOrder(int empire, int object_id, int recipient) :
    Order(empire),
    m_object_id(object_id),
    m_recipient_empire_id(recipient)
{}

void GiveObjectToEmpireOrder::ExecuteImpl() const {
    GetValidatedEmpire();
    int empire_id = EmpireID();

    if (auto fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id))
            fleet->SetGiveToEmpire(m_recipient_empire_id);

    } else if (auto planet = GetPlanet(m_object_id)) {
        if (planet->OwnedBy(empire_id))
            planet->SetGiveToEmpire(m_recipient_empire_id);
    }
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
