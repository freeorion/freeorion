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
#include "../universe/Universe.h"
#include "../universe/UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <vector>


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
Order::Order() :
    m_empire(ALL_EMPIRES),
    m_executed(false)
{}

void Order::ValidateEmpireID() const {
    if (!(GetEmpire(EmpireID())))
        throw std::runtime_error("Invalid empire ID specified for order.");
}

void Order::Execute() const {
    ExecuteImpl();
    m_executed = true;
}

bool Order::Undo() const
{ return UndoImpl(); }

bool Order::Executed() const
{ return m_executed; }

bool Order::UndoImpl() const
{ return false; }

////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder() :
    Order(),
    m_object(INVALID_OBJECT_ID)
{}

RenameOrder::RenameOrder(int empire, int object, const std::string& name) :
    Order(empire),
    m_object(object),
    m_name(name)
{
    TemporaryPtr<const UniverseObject> obj = GetUniverseObject(object);
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
    ValidateEmpireID();

    TemporaryPtr<UniverseObject> obj = GetUniverseObject(m_object);

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
    if (m_name == "") {
        ErrorLogger() << "Name \"\" specified in rename order is invalid.";
        return;
    }

    obj->Rename(m_name);
}

////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder() :
    Order(),
    m_fleet_names(),
    m_system_id(INVALID_OBJECT_ID)
{}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name, int fleet_id,
                             int system_id, const std::vector<int>& ship_ids,
                             bool aggressive) :
    Order(empire),
    m_fleet_names(),
    m_system_id(system_id),
    m_fleet_ids(),
    m_ship_id_groups(),
    m_aggressives()
{
    m_fleet_names.push_back(fleet_name);
    m_fleet_ids.push_back(fleet_id);
    m_ship_id_groups.push_back(ship_ids);
    m_aggressives.push_back(aggressive);
}


NewFleetOrder::NewFleetOrder(int empire, const std::vector<std::string>& fleet_names,
                             const std::vector<int>& fleet_ids, int system_id,
                             const std::vector<std::vector<int> >& ship_id_groups,
                             const std::vector<bool>& aggressives) :
    Order(empire),
    m_fleet_names(fleet_names),
    m_system_id(system_id),
    m_fleet_ids(fleet_ids),
    m_ship_id_groups(ship_id_groups),
    m_aggressives(aggressives)
{}

void NewFleetOrder::ExecuteImpl() const {
    ValidateEmpireID();

    if (m_system_id == INVALID_OBJECT_ID) {
        ErrorLogger() << "Empire attempted to create a new fleet outside a system";
        return;
    }
    TemporaryPtr<System> system = GetSystem(m_system_id);
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
    std::vector<TemporaryPtr<Fleet> > created_fleets;
    created_fleets.reserve(m_ship_id_groups.size());


    // create fleet for each group of ships
    for (int i = 0; i < static_cast<int>(m_ship_id_groups.size()); ++i) {
        const std::string&      fleet_name =    m_fleet_names[i];
        int                     fleet_id =      m_fleet_ids[i];
        const std::vector<int>& ship_ids =      m_ship_id_groups[i];
        bool                    aggressive =    m_aggressives[i];

        if (ship_ids.empty())
            continue;   // nothing to do...

        // validate specified ships
        std::vector<TemporaryPtr<Ship> >    validated_ships;
        std::vector<int>                    validated_ships_ids;
        for (int ship_id : ship_ids) {
            // verify that empire is not trying to take ships from somebody else's fleet
            TemporaryPtr<Ship> ship = GetShip(ship_id);
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

        // create fleet
        TemporaryPtr<Fleet> fleet = GetUniverse().CreateFleet(fleet_name, system->X(), system->Y(),
                                                              EmpireID(), fleet_id);
        fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
        fleet->SetAggressive(aggressive);

        // an ID is provided to ensure consistancy between server and client universes
        GetUniverse().SetEmpireObjectVisibility(EmpireID(), fleet->ID(), VIS_FULL_VISIBILITY);

        system->Insert(fleet);

        // new fleet will get same m_arrival_starlane as fleet of the first ship in the list.
        TemporaryPtr<Ship> firstShip = validated_ships[0];
        TemporaryPtr<Fleet> firstFleet = GetFleet(firstShip->FleetID());
        if (firstFleet)
            fleet->SetArrivalStarlane(firstFleet->ArrivalStarlane());

        // remove ships from old fleet(s) and add to new
        for (TemporaryPtr<Ship> ship : validated_ships) {
            if (TemporaryPtr<Fleet> old_fleet = GetFleet(ship->FleetID()))
                old_fleet->RemoveShip(ship->ID());
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
}

////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() :
    Order(),
    m_fleet(INVALID_OBJECT_ID),
    m_start_system(INVALID_OBJECT_ID),
    m_dest_system(INVALID_OBJECT_ID),
    m_append(false)
{}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet_id, int start_system_id, int dest_system_id, bool append) :
    Order(empire),
    m_fleet(fleet_id),
    m_start_system(start_system_id),
    m_dest_system(dest_system_id),
    m_append(append)
{
    // perform sanity checks
    TemporaryPtr<const Fleet> fleet = GetFleet(FleetID());
    if (!fleet) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    TemporaryPtr<const System> destination_system = GetSystem(DestinationSystemID());
    if (!destination_system) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id " << DestinationSystemID() << " but no such system exists / is known to exist";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()) ) {
        ErrorLogger() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }

    std::pair<std::list<int>, double> short_path = GetUniverse().ShortestPath(m_start_system, m_dest_system, empire);

    m_route.clear();
    std::copy(short_path.first.begin(), short_path.first.end(), std::back_inserter(m_route));

    // ensure a zero-length (invalid) route is not requested / sent to a fleet
    if (m_route.empty())
        m_route.push_back(m_start_system);
}

void FleetMoveOrder::ExecuteImpl() const {
    ValidateEmpireID();

    TemporaryPtr<Fleet> fleet = GetFleet(FleetID());
    if (!fleet) {
        ErrorLogger() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    TemporaryPtr<const System> destination_system = GetEmpireKnownSystem(DestinationSystemID(), EmpireID());
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

    if(m_append && !fleet->TravelRoute().empty()){
        route_list = fleet->TravelRoute();
        route_list.erase(--route_list.end());// Remove the last one since it is the first one of the other
    }

    std::copy(m_route.begin(), m_route.end(), std::back_inserter(route_list));


    // validate route.  Only allow travel between systems connected in series by starlanes known to this fleet's owner.

    // check destination validity: disallow movement that's out of range
    std::pair<int, int> eta = fleet->ETA(fleet->MovePath(route_list));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE) {
        DebugLogger() << "FleetMoveOrder::ExecuteImpl rejected out of range move order";
        return;
    }

    std::string waypoints;
    for (int waypoint : route_list) {
        waypoints += std::string(" ") + boost::lexical_cast<std::string>(waypoint);
    }
    DebugLogger() << "FleetMoveOrder::ExecuteImpl Setting route of fleet " << fleet->ID() << " to " << waypoints;

    fleet->SetRoute(route_list);
}

////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder() :
    Order(),
    m_dest_fleet(INVALID_OBJECT_ID)
{}

FleetTransferOrder::FleetTransferOrder(int empire, int dest_fleet, const std::vector<int>& ships) :
    Order(empire),
    m_dest_fleet(dest_fleet),
    m_add_ships(ships)
{}

void FleetTransferOrder::ExecuteImpl() const {
    ValidateEmpireID();

    // look up the destination fleet
    TemporaryPtr<Fleet> target_fleet = GetFleet(DestinationFleet());
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
    std::vector<TemporaryPtr<Ship> > ships = Objects().FindObjects<Ship>(m_add_ships);

    std::vector<TemporaryPtr<Ship> > validated_ships;
    validated_ships.reserve(m_add_ships.size());
    std::vector<int>                 validated_ship_ids;
    validated_ship_ids.reserve(m_add_ships.size());

    for (TemporaryPtr<Ship> ship : ships) {
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
    std::set<TemporaryPtr<Fleet> > modified_fleets;
    for (TemporaryPtr<Ship> ship : validated_ships) {
        if (TemporaryPtr<Fleet> source_fleet = GetFleet(ship->FleetID())) {
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

    for (TemporaryPtr<Fleet> modified_fleet : modified_fleets) {
        if (!modified_fleet->Empty())
            modified_fleet->StateChangedSignal();
        // if modified fleet is empty, it should be immently destroyed, so that updating it now is redundant
    }
}

////////////////////////////////////////////////
// ColonizeOrder
////////////////////////////////////////////////
ColonizeOrder::ColonizeOrder() :
    Order(),
    m_ship(INVALID_OBJECT_ID),
    m_planet(INVALID_OBJECT_ID)
{}

ColonizeOrder::ColonizeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{}

void ColonizeOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->CurrentMeterValue(METER_POPULATION) > 0.0f) {
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
        ErrorLogger() << "ColonizeOrder::ExecuteImpl nonzero colonist capacity and planet that ship's species can't colonize";
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

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool ColonizeOrder::UndoImpl() const {
    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "ColonizeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }
    if (!planet->IsAboutToBeColonized()) {
        ErrorLogger() << "ColonizeOrder::UndoImpl planet is not about to be colonized...";
        return false;
    }

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// InvadeOrder
////////////////////////////////////////////////
InvadeOrder::InvadeOrder() :
    Order(),
    m_ship(INVALID_OBJECT_ID),
    m_planet(INVALID_OBJECT_ID)
{}

InvadeOrder::InvadeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{}

void InvadeOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->Unowned() && planet->CurrentMeterValue(METER_POPULATION) == 0.0) {
        ErrorLogger() << "InvadeOrder::ExecuteImpl given unpopulated planet";
        return;
    }
    if (planet->CurrentMeterValue(METER_SHIELD) > 0.0) {
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

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool InvadeOrder::UndoImpl() const {
    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "InvadeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// BombardOrder
////////////////////////////////////////////////
BombardOrder::BombardOrder() :
    Order(),
    m_ship(INVALID_OBJECT_ID),
    m_planet(INVALID_OBJECT_ID)
{}

BombardOrder::BombardOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{}

void BombardOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
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

    // note: multiple ships, from same or different empires, can invade the same planet on the same turn
    DebugLogger() << "BombardOrder::ExecuteImpl set for ship " << m_ship << " "
                           << ship->Name() << " to bombard planet " << m_planet << " " << planet->Name();
    planet->SetIsAboutToBeBombarded(true);
    ship->SetBombardPlanet(m_planet);

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();
}

bool BombardOrder::UndoImpl() const {
    TemporaryPtr<Planet> planet = GetPlanet(m_planet);
    if (!planet) {
        ErrorLogger() << "BombardOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    TemporaryPtr<Ship> ship = GetShip(m_ship);
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

    if (TemporaryPtr<Fleet> fleet = GetFleet(ship->FleetID()))
        fleet->StateChangedSignal();

    return true;
}

////////////////////////////////////////////////
// DeleteFleetOrder
////////////////////////////////////////////////
DeleteFleetOrder::DeleteFleetOrder() :
    Order(),
    m_fleet(-1)
{}

DeleteFleetOrder::DeleteFleetOrder(int empire, int fleet) :
    Order(empire),
    m_fleet(fleet)
{}

void DeleteFleetOrder::ExecuteImpl() const {
    ValidateEmpireID();

    TemporaryPtr<Fleet> fleet = GetFleet(FleetID());

    if (!fleet) {
        ErrorLogger() << "Illegal fleet id specified in fleet delete order: " << FleetID();
        return;
    }

    if (!fleet->OwnedBy(EmpireID())) {
        ErrorLogger() << "Empire attempted to issue deletion order to another's fleet.";
        return;
    }

    if (!fleet->Empty())
        return; // should be no ships to delete

    TemporaryPtr<System> system = GetSystem(fleet->SystemID());
    if (system)
        system->Remove(fleet->ID());

    GetUniverse().Destroy(FleetID());
}

////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder() :
    Order(),
    m_planet(INVALID_OBJECT_ID),
    m_focus()
{}

ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, const std::string& focus) :
    Order(empire),
    m_planet(planet),
    m_focus(focus)
{}

void ChangeFocusOrder::ExecuteImpl() const {
    ValidateEmpireID();

    TemporaryPtr<Planet> planet = GetPlanet(PlanetID());

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
ResearchQueueOrder::ResearchQueueOrder() :
    Order(),
    m_position(INVALID_INDEX),
    m_remove(false),
    m_pause(INVALID_PAUSE_RESUME)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(INVALID_INDEX),
    m_remove(true),
    m_pause(INVALID_PAUSE_RESUME)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, int position) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(position),
    m_remove(false),
    m_pause(INVALID_PAUSE_RESUME)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, bool pause, float dummy) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(INVALID_INDEX),
    m_remove(false),
    m_pause(pause ? PAUSE : RESUME)
{}

void ResearchQueueOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Empire* empire = GetEmpire(EmpireID());
    if (!empire) return;
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
ProductionQueueOrder::ProductionQueueOrder() :
    Order(),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, const ProductionQueue::ProductionItem& item,
                                           int number, int location, int pos) :
    Order(empire),
    m_item(item),
    m_number(number),
    m_location(location),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(pos),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, int new_blocksize) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(new_quantity),
    m_new_blocksize(new_blocksize),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(new_quantity),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int rally_point_id, bool dummy1, bool dummy2) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(rally_point_id),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_index) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(new_index),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(INVALID_PAUSE_RESUME)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, bool pause, float dummy) :
    Order(empire),
    m_item(),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_blocksize(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX),
    m_rally_point_id(INVALID_OBJECT_ID),
    m_pause(pause ? PAUSE : RESUME)
{}

void ProductionQueueOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Empire* empire = GetEmpire(EmpireID());
    try {
        if (m_item.build_type == BT_BUILDING || m_item.build_type == BT_SHIP) {
            empire->PlaceProductionOnQueue(m_item, m_number, m_location, m_new_index);

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
    Order(),
    m_design_id(INVALID_OBJECT_ID),
    m_update_name_or_description(false),
    m_delete_design_from_empire(false),
    m_create_new_design(false),
    m_move_design(false),
    m_designed_on_turn(0),
    m_is_monster(false),
    m_name_desc_in_stringtable(false),
    m_design_id_after(INVALID_OBJECT_ID)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_design_id(existing_design_id_to_remember),
    m_update_name_or_description(false),
    m_delete_design_from_empire(false),
    m_create_new_design(false),
    m_move_design(false),
    m_designed_on_turn(0),
    m_is_monster(false),
    m_name_desc_in_stringtable(false),
    m_design_id_after(INVALID_OBJECT_ID)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_design_id(design_id_to_erase),
    m_update_name_or_description(false),
    m_delete_design_from_empire(true),
    m_create_new_design(false),
    m_move_design(false),
    m_designed_on_turn(0),
    m_is_monster(false),
    m_name_desc_in_stringtable(false),
    m_design_id_after(INVALID_OBJECT_ID)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design) :
    Order(empire),
    m_design_id(new_design_id),
    m_update_name_or_description(false),
    m_delete_design_from_empire(false),
    m_create_new_design(true),
    m_move_design(false),
    m_name(ship_design.Name()),
    m_description(ship_design.Description()),
    m_designed_on_turn(ship_design.DesignedOnTurn()),
    m_hull(ship_design.Hull()),
    m_parts(ship_design.Parts()),
    m_is_monster(ship_design.IsMonster()),
    m_icon(ship_design.Icon()),
    m_3D_model(ship_design.Model()),
    m_name_desc_in_stringtable(ship_design.LookupInStringtable()),
    m_design_id_after(INVALID_OBJECT_ID)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id, const std::string& new_name/* = ""*/, const std::string& new_description/* = ""*/) :
    Order(empire),
    m_design_id(existing_design_id),
    m_update_name_or_description(true),
    m_delete_design_from_empire(false),
    m_create_new_design(false),
    m_move_design(false),
    m_name(new_name),
    m_description(new_description),
    m_designed_on_turn(0),
    m_is_monster(false),
    m_name_desc_in_stringtable(false),
    m_design_id_after(INVALID_OBJECT_ID)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id, int design_id_after) :
    Order(empire),
    m_design_id(design_id),
    m_update_name_or_description(false),
    m_delete_design_from_empire(false),
    m_create_new_design(false),
    m_move_design(true),
    m_designed_on_turn(0),
    m_is_monster(false),
    m_name_desc_in_stringtable(false),
    m_design_id_after(design_id_after)
{}

void ShipDesignOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    Empire* empire = GetEmpire(EmpireID());
    if (m_delete_design_from_empire) {
        // player is ordering empire to forget about a particular design
        if (!empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Tried to remove a ShipDesign that the empire wasn't remembering";
            return;
        }
        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        // check if a design with this ID already exists
        if (universe.GetShipDesign(m_design_id)) {
            ErrorLogger() << "Tried to create a new ShipDesign with an id of an already-existing ShipDesign";
            return;
        }
        ShipDesign* new_ship_design = new ShipDesign(m_name, m_description,
                                                     m_designed_on_turn, EmpireID(), m_hull, m_parts,
                                                     m_icon, m_3D_model, m_name_desc_in_stringtable,
                                                     m_is_monster);

        universe.InsertShipDesignID(new_ship_design, m_design_id);
        universe.SetEmpireKnowledgeOfShipDesign(m_design_id, EmpireID());
        empire->AddShipDesign(m_design_id);

    } else if (m_update_name_or_description) {
        // player is ordering empire to rename a design
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        std::set<int>::iterator design_it = empire_known_design_ids.find(m_design_id);
        if (design_it == empire_known_design_ids.end()) {
            ErrorLogger() << "Tried to rename/redescribe a ShipDesign that this empire hasn't seen";
            return;
        }
        const ShipDesign* design = GetShipDesign(*design_it);
        if (!design) {
            ErrorLogger() << "Tried to rename/redescribe a ShipDesign that doesn't exist (but this empire has seen it)!";
            return;
        }
        if (design->DesignedByEmpire() != EmpireID()) {
            ErrorLogger() << "Tried to rename/redescribe a ShipDesign that isn't owned by this empire!";
            return;
        }
        GetUniverse().RenameShipDesign(m_design_id, m_name, m_description);

    } else if (m_move_design) {
        //Move an existing design from its current location to just before the after_design
        if (!empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Tried to move a ShipDesign that the empire wasn't remembering";
            return;
        }
        if (m_design_id == m_design_id_after)
            return;

        empire->RemoveShipDesign(m_design_id);
        empire->AddShipDesign(m_design_id, m_design_id_after);
        DebugLogger() << "Move Ship Design " << m_design_id << " to before " << m_design_id_after;
    } else {
        // player is ordering empire to retain a particular design, so that is can
        // be used to construct ships by that empire.

        // TODO: consider removing this order, so that an empire needs to use
        // espionage or trade to gain access to a ship design made by another
        // player

        // check if empire is already remembering the design
        if (empire->ShipDesignKept(m_design_id)) {
            ErrorLogger() << "Tried to remember a ShipDesign that was already being remembered";
            return;
        }

        // check if the empire can see any objects that have this design (thus enabling it to be copied)
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        if (empire_known_design_ids.find(m_design_id) != empire_known_design_ids.end()) {
            empire->AddShipDesign(m_design_id);
        } else {
            ErrorLogger() << "Tried to remember a ShipDesign that this empire hasn't seen";
            return;
        }

    }
}

////////////////////////////////////////////////
// ScrapOrder
////////////////////////////////////////////////
ScrapOrder::ScrapOrder() :
    Order(),
    m_object_id(INVALID_OBJECT_ID)
{}

ScrapOrder::ScrapOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

void ScrapOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (TemporaryPtr<Ship> ship = GetShip(m_object_id)) {
        if (ship->SystemID() != INVALID_OBJECT_ID && ship->OwnedBy(empire_id)) {
            ship->SetOrderedScrapped(true);
            //DebugLogger() << "ScrapOrder::ExecuteImpl empire: " << empire_id
            //                       << " on ship: " << ship->ID() << " at system: " << ship->SystemID()
            //                       << " ... ordered scrapped?: " << ship->OrderedScrapped();
        }
    } else if (TemporaryPtr<Building> building = GetBuilding(m_object_id)) {
        int planet_id = building->PlanetID();
        if (TemporaryPtr<const Planet> planet = GetPlanet(planet_id)) {
            if (building->OwnedBy(empire_id) && planet->OwnedBy(empire_id))
                building->SetOrderedScrapped(true);
        }
    }
}

bool ScrapOrder::UndoImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (TemporaryPtr<Ship> ship = GetShip(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (TemporaryPtr<Building> building = GetBuilding(m_object_id)) {
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
AggressiveOrder::AggressiveOrder() :
    Order(),
    m_object_id(INVALID_OBJECT_ID),
    m_aggression(false)
{}

AggressiveOrder::AggressiveOrder(int empire, int object_id, bool aggression/* = true*/) :
    Order(empire),
    m_object_id(object_id),
    m_aggression(aggression)
{}

void AggressiveOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();
    if (TemporaryPtr<Fleet> fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id))
            fleet->SetAggressive(m_aggression);
    }
}

/////////////////////////////////////////////////////
// GiveObjectToEmpireOrder
/////////////////////////////////////////////////////
GiveObjectToEmpireOrder::GiveObjectToEmpireOrder() :
    Order(),
    m_object_id(INVALID_OBJECT_ID),
    m_recipient_empire_id(ALL_EMPIRES)
{}

GiveObjectToEmpireOrder::GiveObjectToEmpireOrder(int empire, int object_id, int recipient) :
    Order(empire),
    m_object_id(object_id),
    m_recipient_empire_id(recipient)
{}

void GiveObjectToEmpireOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (TemporaryPtr<Fleet> fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id))
            fleet->SetGiveToEmpire(m_recipient_empire_id);

    } else if (TemporaryPtr<Planet> planet = GetPlanet(m_object_id)) {
        if (planet->OwnedBy(empire_id))
            planet->SetGiveToEmpire(m_recipient_empire_id);
    }
}

bool GiveObjectToEmpireOrder::UndoImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (TemporaryPtr<Fleet> fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id)) {
            fleet->ClearGiveToEmpire();
            return true;
        }
    } else if (TemporaryPtr<Planet> planet = GetPlanet(m_object_id)) {
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
ForgetOrder::ForgetOrder() :
    Order(),
    m_object_id(INVALID_OBJECT_ID)
{}

ForgetOrder::ForgetOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

void ForgetOrder::ExecuteImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    DebugLogger() << "ForgetOrder::ExecuteImpl empire: " << empire_id
                  << " for object: " << m_object_id;

    GetUniverse().ForgetKnownObject(empire_id, m_object_id);
}
