#include "Order.h"

#include "AppInterface.h"
#include "../universe/Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OrderSet.h"
#include "../universe/Predicates.h"
#include "../universe/Species.h"
#include "../universe/Building.h"
#include "../universe/Planet.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../universe/UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <vector>

const Tech* GetTech(const std::string& name);

#define DEBUG_CREATE_FLEET_ORDER 0
#define DEBUG_FLEET_MOVE_ORDER   0
#if DEBUG_CREATE_FLEET_ORDER || DEBUG_FLEET_MOVE_ORDER
#  include <iostream>
#endif


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
Order::Order() :
    m_empire(ALL_EMPIRES),
    m_executed(false)
{}

void Order::ValidateEmpireID() const {
    if (!(Empires().Lookup(EmpireID())))
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
    const UniverseObject* obj = GetUniverseObject(object);
    if (!obj) {
        Logger().errorStream() << "RenameOrder::RenameOrder() : Attempted to rename nonexistant object with id " << object;
        return;
    }

    if (m_name.empty()) {
        Logger().errorStream() << "RenameOrder::RenameOrder() : Attempted to name an object \"\".";
        // make order do nothing
        m_object = INVALID_OBJECT_ID;
        return;
    }
}

void RenameOrder::ExecuteImpl() const {
    ValidateEmpireID();

    UniverseObject* obj = GetUniverseObject(m_object);

    if (!obj) {
        Logger().errorStream() << "Attempted to rename nonexistant object with id " << m_object;
        return;
    }

    // verify that empire specified in order owns specified object
    if (!obj->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire specified in rename order does not own specified object.";
        return;
    }

    // disallow the name "", since that denotes an unknown object
    if (m_name == "") {
        Logger().errorStream() << "Name \"\" specified in rename order is invalid.";
        return;
    }

    obj->Rename(m_name);
}


////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder() :
    Order()
{}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id, const std::vector<int>& ship_ids) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(system_id),
    m_new_id( new_id ),
    m_ship_ids(ship_ids)
{
#if DEBUG_CREATE_FLEET_ORDER
    std::cerr << "NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id, int ship_id) : \n" << std::endl 
              << "    m_empire=" << EmpireID() << std::endl
              << "    m_fleet_name=" << m_fleet_name << std::endl
              << "    m_system_id=" << m_system_id << std::endl
              << "    m_position=(" << m_position.first << " " << m_position.second << ")" << std::endl
              << "    m_new_id=" << m_new_id << std::endl
              << "    m_ship_ids.size()=" << m_ship_ids.size() << std::endl
              << std::endl;
#endif
}

void NewFleetOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    if (m_system_id == INVALID_OBJECT_ID) {
        Logger().errorStream() << "Empire attempted to create a new fleet outside a system";
        return;
    }
    System* system = GetSystem(m_system_id);
    if (!system) {
        Logger().errorStream() << "Empire attempted to create a new fleet in a nonexistant system";
        return;
    }

    // verify ship validity
    std::vector<int> validated_ships;
    for (unsigned int i = 0; i < m_ship_ids.size(); ++i) {
        // verify that empire is not trying to take ships from somebody else's fleet
        const Ship* ship = GetShip(m_ship_ids[i]);
        if (!ship) {
            Logger().errorStream() << "Empire attempted to create a new fleet with an invalid ship";
            continue;
        }
        if (!ship->OwnedBy(EmpireID())) {
            Logger().errorStream() << "Empire attempted to create a new fleet with ships from another's fleet.";
            continue;
        }
        if (ship->SystemID() != m_system_id) {
            Logger().errorStream() << "Empire attempted to make a new fleet from ship in the wrong system";
            continue;
        }
        validated_ships.push_back(m_ship_ids[i]);
    }
    if (validated_ships.empty())
        return;


    // create fleet
    Fleet* fleet = new Fleet(m_fleet_name, system->X(), system->Y(), EmpireID());
    fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
    // an ID is provided to ensure consistancy between server and client universes
    universe.InsertID(fleet, m_new_id);
    system->Insert(fleet);
    fleet->SetAggressive(false);    // TODO: Copy from old fleet of ships?


    // add ship(s) to fleet
    for (unsigned int i = 0; i < validated_ships.size(); ++i)
        fleet->AddShip(validated_ships[i]);
}


////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() :
    Order(),
    m_fleet(INVALID_OBJECT_ID),
    m_start_system(INVALID_OBJECT_ID),
    m_dest_system(INVALID_OBJECT_ID)
{}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet_id, int start_system_id, int dest_system_id) :
    Order(empire),
    m_fleet(fleet_id),
    m_start_system(start_system_id),
    m_dest_system(dest_system_id)
{
    // perform sanity checks
    const Fleet* fleet = GetFleet(FleetID());
    if (!fleet) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    const System* destination_system = GetSystem(DestinationSystemID());
    if (!destination_system) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id " << DestinationSystemID() << " but no such system exists / is known to exist";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }

    std::pair<std::list<int>, double> short_path = GetUniverse().ShortestPath(m_start_system, m_dest_system, empire);

    m_route.clear();
    std::copy(short_path.first.begin(), short_path.first.end(), std::back_inserter(m_route));

    // ensure a zero-length (invalid) route is not requested / sent to a fleet
    if (m_route.empty())
        m_route.push_back(m_start_system);

#if DEBUG_FLEET_MOVE_ORDER
    std::cerr << "FleetMoveOrder(int empire, int fleet, int start_syste, int dest_system) : " << std::endl
              << "    m_empire=" << EmpireID() << std::endl
              << "    m_fleet=" << m_fleet << std::endl
              << "    m_start_system=" << m_start_system << std::endl
              << "    m_dest_system=" << m_dest_system << std::endl
              << std::endl;
#endif
}

void FleetMoveOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Fleet* fleet = GetFleet(FleetID());
    if (!fleet) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    const System* destination_system = GetEmpireKnownSystem(DestinationSystemID(), EmpireID());
    if (!destination_system) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id " << DestinationSystemID() << " but no such system is known to that empire";
        return;
    }

    // reject empty routes
    if (m_route.empty()) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet to move on empty route";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }


    // verify fleet route first system
    int fleet_sys_id = fleet->SystemID();
    if (fleet_sys_id != INVALID_OBJECT_ID) {
        // fleet is in a system.  Its move path should also start from that system.
        if (fleet_sys_id != m_start_system) {
            Logger().errorStream() << "Empire with id " << EmpireID() << " ordered a fleet to move from a system with id " << m_start_system <<
                                     " that it is not at.  Fleet is located at system with id " << fleet_sys_id;
            return;
        }
    } else {
        // fleet is not in a system.  Its move path should start from the next system it is moving to.
        int next_system = fleet->NextSystemID();
        if (next_system != m_start_system) {
            Logger().errorStream() << "Empire with id " << EmpireID() << " ordered a fleet to move starting from a system with id " << m_start_system <<
                                     ", but the fleet's next destination is system with id " << next_system;
            return;
        }
    }


    // convert list of ids to list of System
    std::list<int> route_list;
    std::copy(m_route.begin(), m_route.end(), std::back_inserter(route_list));


    // validate route.  Only allow travel between systems connected in series by starlanes known to this fleet's owner.

    // check destination validity: disallow movement that's out of range
    std::pair<int, int> eta = fleet->ETA(fleet->MovePath(route_list));
    if (eta.first == Fleet::ETA_NEVER || eta.first == Fleet::ETA_OUT_OF_RANGE) {
        Logger().debugStream() << "FleetMoveOrder::ExecuteImpl rejected out of range move order";
        return;
    }

    fleet->SetRoute(route_list);
}


////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder() :
    Order(),
    m_fleet_from(INVALID_OBJECT_ID),
    m_fleet_to(INVALID_OBJECT_ID)
{}

FleetTransferOrder::FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships) : 
    Order(empire),
    m_fleet_from(fleet_from),
    m_fleet_to(fleet_to),
    m_add_ships(ships)
{}

void FleetTransferOrder::ExecuteImpl() const {
    ValidateEmpireID();

    ObjectMap& objects = GetUniverse().Objects();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = GetFleet(SourceFleet());
    Fleet* target_fleet = GetFleet(DestinationFleet());

    if (!source_fleet || !target_fleet) {
        Logger().errorStream() << "Empire attempted to move ships to or from a nonexistant fleet";
        return;
    }

    // verify that both fleets are in the same system
    if (source_fleet->SystemID() == INVALID_OBJECT_ID) {
        Logger().errorStream() << "Empire attempted to transfer ships to/from fleet(s) not in a system";
        return;
    }
    if (source_fleet->SystemID() != target_fleet->SystemID()) {
        Logger().errorStream() << "Empire attempted to transfer ships to/from fleets in different systems";
        return;
    }

    // verify that empire is not trying to take ships from somebody else's fleet
    if (!source_fleet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to merge ships from another's fleet.";
        return;
    }

    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided the server
    // verifies IDs of the Empires issuing the orders.
    if (!target_fleet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to merge ships into another's fleet.";
        return;
    }


    // iterate down the ship vector and add each one to the fleet
    // after first verifying that it is a valid ship id
    std::vector<int>::const_iterator itr = m_add_ships.begin();
    while (itr != m_add_ships.end()) {
        // find the ship, verify that ID is valid
        int curr = (*itr);
        Ship* a_ship = GetShip(curr);
        if (!a_ship) {
            Logger().errorStream() << "Illegal ship id specified in fleet merge order.";
            return;
        }

        // figure out what fleet this ship is coming from -- verify its the one we
        // said it comes from
        if (a_ship->FleetID() != SourceFleet()) {
            Logger().errorStream() << "Ship in merge order is not in specified source fleet.";
            return;
        }

        // send the ship to its new fleet
        //a_ship->SetFleetID(DestinationFleet());  // redundant: AddShip resets the Fleet ID of ships it adds
        //source_fleet->RemoveShip(curr);  // redundant: AddShip calls RemoveShip on ship's old fleet
        target_fleet->AddShip(curr);

        itr++;
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

    Ship* ship = GetShip(m_ship);
    if (!ship) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl couldn't get ship with id " << m_ship;
        return;
    }
    if (!ship->CanColonize()) { // verifies that species exists and can colonize and that ship design can colonize
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl got ship that can't colonize";
        return;
    }
    if (!ship->OwnedBy(empire_id)) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return;
    }
    const ShipDesign* design = ship->Design();
    if (!design) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl couldn't find ship's design!";
        return;
    }
    double colonist_capacity = design->ColonyCapacity();

    Planet* planet = GetPlanet(m_planet);
    if (!planet) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->CurrentMeterValue(METER_POPULATION) > 0.0) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given planet that already has population";
        return;
    }
    if (!planet->Unowned() && planet->Owner() != empire_id) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given planet that owned by another empire";
        return;
    }
    if (planet->OwnedBy(empire_id) && colonist_capacity == 0.0) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given planet that is already owned by empire and colony ship with zero capcity";
        return;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(m_planet, empire_id) < VIS_PARTIAL_VISIBILITY) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given planet that empire has insufficient visibility of";
        return;
    }
    if (colonist_capacity > 0.0 && planet->EnvironmentForSpecies(ship->SpeciesName()) < PE_HOSTILE) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl nonzero colonist capacity and planet that ship's species can't colonize";
        return;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given id of ship not in a system";
        return;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return;
    }
    if (planet->IsAboutToBeColonized()) {
        Logger().errorStream() << "ColonizeOrder::ExecuteImpl given id planet that is already being colonized";
        return;
    }

    planet->SetIsAboutToBeColonized(true);
    ship->SetColonizePlanet(m_planet);
}

bool ColonizeOrder::UndoImpl() const {
    Planet* planet = GetPlanet(m_planet);
    if (!planet) {
        Logger().errorStream() << "ColonizeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }
    if (!planet->IsAboutToBeColonized()) {
        Logger().errorStream() << "ColonizeOrder::UndoImpl planet is not about to be colonized...";
        return false;
    }

    Ship* ship = GetShip(m_ship);
    if (!ship) {
        Logger().errorStream() << "ColonizeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedColonizePlanet() != m_planet) {
        Logger().errorStream() << "ColonizeOrder::UndoImpl ship is not about to colonize planet";
        return false;
    }

    planet->SetIsAboutToBeColonized(false);
    ship->ClearColonizePlanet();

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

    Ship* ship = GetShip(m_ship);
    if (!ship) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl couldn't get ship with id " << m_ship;
        return;
    }
    if (!ship->HasTroops()) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl got ship that can't invade";
        return;
    }
    if (!ship->OwnedBy(empire_id)) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl got ship that isn't owned by the order-issuing empire";
        return;
    }

    Planet* planet = GetPlanet(m_planet);
    if (!planet) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl couldn't get planet with id " << m_planet;
        return;
    }
    if (planet->Unowned() && planet->CurrentMeterValue(METER_POPULATION) == 0.0) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given unpopulated planet";
        return;
    }
    if (planet->CurrentMeterValue(METER_SHIELD) > 0.0) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given planet with shield > 0";
        return;
    }
    if (planet->OwnedBy(empire_id)) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given planet that is already owned by the order-issuing empire";
        return;
    }
    if (!planet->Unowned() && Empires().GetDiplomaticStatus(planet->Owner(), empire_id) != DIPLO_WAR) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given planet owned by an empire not at war with order-issuing empire";
        return;
    }
    if (GetUniverse().GetObjectVisibilityByEmpire(m_planet, empire_id) < VIS_PARTIAL_VISIBILITY) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given planet that empire has insufficient visibility of";
        return;
    }

    int ship_system_id = ship->SystemID();
    if (ship_system_id == INVALID_OBJECT_ID) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given id of ship not in a system";
        return;
    }
    int planet_system_id = planet->SystemID();
    if (ship_system_id != planet_system_id) {
        Logger().errorStream() << "InvadeOrder::ExecuteImpl given ids of ship and planet not in the same system";
        return;
    }

    // note: multiple ships, from same or different empires, can invade the same planet on the same turn

    planet->SetIsAboutToBeInvaded(true);
    ship->SetInvadePlanet(m_planet);
}

bool InvadeOrder::UndoImpl() const {
    Planet* planet = GetPlanet(m_planet);
    if (!planet) {
        Logger().errorStream() << "InvadeOrder::UndoImpl couldn't get planet with id " << m_planet;
        return false;
    }

    Ship* ship = GetShip(m_ship);
    if (!ship) {
        Logger().errorStream() << "InvadeOrder::UndoImpl couldn't get ship with id " << m_ship;
        return false;
    }
    if (ship->OrderedInvadePlanet() != m_planet) {
        Logger().errorStream() << "InvadeOrder::UndoImpl ship is not about to invade planet";
        return false;
    }

    planet->SetIsAboutToBeInvaded(false);
    ship->ClearInvadePlanet();

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

    Fleet* fleet = GetFleet(FleetID());

    if (!fleet) {
        Logger().errorStream() << "Illegal fleet id specified in fleet delete order.";
        return;
    }

    if (!fleet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to issue deletion order to another's fleet.";
        return;
    }

    if (!fleet->Empty())
        return;

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

    Planet* planet = GetPlanet(PlanetID());

    if (!planet) {
        Logger().errorStream() << "Illegal planet id specified in change planet focus order.";
        return;
    }

    if (!planet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to issue change planet focus to another's planet.";
        return;
    }

    planet->SetFocus(m_focus);

    Empire* empire = Empires().Lookup(EmpireID());
    empire->UpdateResearchQueue();
    empire->UpdateProductionQueue();
}

////////////////////////////////////////////////
// ResearchQueueOrder
////////////////////////////////////////////////
ResearchQueueOrder::ResearchQueueOrder() :
    Order(),
    m_position(-1),
    m_remove(false)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(-1),
    m_remove(true)
{}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, int position) :
    Order(empire),
    m_tech_name(tech_name),
    m_position(position),
    m_remove(false)
{}

void ResearchQueueOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Empire* empire = Empires().Lookup(EmpireID());
    if (!empire) return;
    if (m_remove)
        empire->RemoveTechFromQueue(m_tech_name);
    else
        empire->PlaceTechInQueue(m_tech_name, m_position);
}

////////////////////////////////////////////////
// ProductionQueueOrder
////////////////////////////////////////////////
ProductionQueueOrder::ProductionQueueOrder() :
    Order(),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(INVALID_OBJECT_ID),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, BuildType build_type, const std::string& item, int number, int location) :
    Order(empire),
    m_build_type(build_type),
    m_item_name(item),
    m_design_id(INVALID_OBJECT_ID),
    m_number(number),
    m_location(location),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{
    if (m_build_type == BT_SHIP) {
        Logger().errorStream() << "Attempted to construct a ProductionQueueOrder for a BT_SHIP with a name, not a design id";
    }
}

ProductionQueueOrder::ProductionQueueOrder(int empire, BuildType build_type, int design_id, int number, int location) :
    Order(empire),
    m_build_type(build_type),
    m_item_name(""),
    m_design_id(design_id),
    m_number(number),
    m_location(location),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{
    if (m_build_type == BT_BUILDING) {
        Logger().errorStream() << "Attempted to construct a ProductionQueueOrder for a BT_BUILDING with a design id, not a name";
    }
}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(INVALID_OBJECT_ID),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(new_quantity),
    m_new_index(INVALID_INDEX)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_index) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(INVALID_OBJECT_ID),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(new_index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(INVALID_OBJECT_ID),
    m_number(0),
    m_location(INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{}

void ProductionQueueOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Empire* empire = Empires().Lookup(EmpireID());
    if (m_build_type == BT_BUILDING)
        empire->PlaceBuildInQueue(BT_BUILDING, m_item_name, m_number, m_location);
    else if (m_build_type == BT_SHIP)
        empire->PlaceBuildInQueue(BT_SHIP, m_design_id, m_number, m_location);
    else if (m_new_quantity != INVALID_QUANTITY)
        empire->SetBuildQuantity(m_index, m_new_quantity);
    else if (m_new_index != INVALID_INDEX)
        empire->MoveBuildWithinQueue(m_index, m_new_index);
    else if (m_index != INVALID_INDEX)
        empire->RemoveBuildFromQueue(m_index);
    else
        Logger().errorStream() << "Malformed ProductionQueueOrder.";
}

////////////////////////////////////////////////
// ShipDesignOrder
////////////////////////////////////////////////
ShipDesignOrder::ShipDesignOrder() :
    Order(),
    m_design_id(INVALID_OBJECT_ID),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_design_id(existing_design_id_to_remember),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_design_id(design_id_to_erase),
    m_delete_design_from_empire(true),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design) :
    Order(empire),
    m_design_id(new_design_id),
    m_delete_design_from_empire(false),
    m_create_new_design(true),
    m_name(ship_design.Name()),
    m_description(ship_design.Description()),
    m_designed_by_empire_id(ship_design.DesignedByEmpire()),
    m_designed_on_turn(ship_design.DesignedOnTurn()),
    m_hull(ship_design.Hull()),
    m_parts(ship_design.Parts()),
    m_is_monster(ship_design.IsMonster()),
    m_icon(ship_design.Icon()),
    m_3D_model(ship_design.Model()),
    m_name_desc_in_stringtable(ship_design.LookupInStringtable())
{}

void ShipDesignOrder::ExecuteImpl() const {
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    Empire* empire = Empires().Lookup(EmpireID());
    if (m_delete_design_from_empire) {
        // player is ordering empire to forget about a particular design
        if (!empire->ShipDesignKept(m_design_id)) {
            Logger().errorStream() << "Tried to remove a ShipDesign that the empire wasn't remembering";
            return;
        }
        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        // player is creating a new design
        if (m_designed_by_empire_id != EmpireID()) {
            Logger().errorStream() << "Tried to create a new ShipDesign designed by another empire";
            return;
        }

        // check if a design with this ID already exists
        if (universe.GetShipDesign(m_design_id)) {
            Logger().errorStream() << "Tried to create a new ShipDesign with an id of an already-existing ShipDesign";
            return;
        }
        ShipDesign* new_ship_design = new ShipDesign(m_name, m_description, m_designed_by_empire_id,
                                                     m_designed_on_turn, m_hull, m_parts,
                                                     m_icon, m_3D_model, m_name_desc_in_stringtable,
                                                     m_is_monster);

        universe.InsertShipDesignID(new_ship_design, m_design_id);
        universe.SetEmpireKnowledgeOfShipDesign(m_design_id, EmpireID());
        empire->AddShipDesign(m_design_id);

    } else if (!m_create_new_design && !m_delete_design_from_empire) {
        // player is order empire to retain a particular design, so that is can
        // be used to construct ships by that empire.

        // TODO: consider removing this order, so that an empire needs to use
        // espionage or trade to gain access to a ship design made by another
        // player

        // check if empire is already remembering the design
        if (empire->ShipDesignKept(m_design_id)) {
            Logger().errorStream() << "Tried to remember a ShipDesign that was already being remembered";
            return;
        }

        // check if the empire can see any objects that have this design (thus enabling it to be copied)
        const std::set<int>& empire_known_design_ids = universe.EmpireKnownShipDesignIDs(EmpireID());
        if (empire_known_design_ids.find(m_design_id) != empire_known_design_ids.end()) {
            empire->AddShipDesign(m_design_id);
        } else {
            Logger().errorStream() << "Tried to remember a ShipDesign that this empire hasn't seen";
            return;
        }

    } else {
        Logger().errorStream() << "Malformed ShipDesignOrder.";
        return;
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

    if (Ship* ship = GetShip(m_object_id)) {
        if (ship->SystemID() != INVALID_OBJECT_ID && ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(true);
    } else if (Building* building = GetBuilding(m_object_id)) {
        int planet_id = building->PlanetID();
        if (const Planet* planet = GetPlanet(planet_id)) {
            if (building->OwnedBy(empire_id) && planet->OwnedBy(empire_id))
                building->SetOrderedScrapped(true);
        }
    }
}

bool ScrapOrder::UndoImpl() const {
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (Ship* ship = GetShip(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (Building* building = GetBuilding(m_object_id)) {
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
    if (Fleet* fleet = GetFleet(m_object_id)) {
        if (fleet->OwnedBy(empire_id))
            fleet->SetAggressive(m_aggression);
    }
}
