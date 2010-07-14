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

void Order::ValidateEmpireID() const
{
    if (!(Empires().Lookup(EmpireID())))
        throw std::runtime_error("Invalid empire ID specified for order.");
}

void Order::Execute() const
{
    ExecuteImpl();
    m_executed = true;
}

bool Order::Undo() const
{
    return UndoImpl();
}

bool Order::Executed() const
{
    return m_executed;
}

bool Order::UndoImpl() const
{
    return false;
}


////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder() : 
    Order(),
    m_object(UniverseObject::INVALID_OBJECT_ID)
{}

RenameOrder::RenameOrder(int empire, int object, const std::string& name) :
    Order(empire),
    m_object(object),
    m_name(name)
{
    const UniverseObject* obj = GetMainObjectMap().Object(object);
    if (!obj) {
        Logger().errorStream() << "RenameOrder::RenameOrder() : Attempted to rename nonexistant object with id " << object;
        return;
    }

    if (m_name == "") {
        Logger().errorStream() << "RenameOrder::RenameOrder() : Attempted to name an object \"\".";
        // make order do nothing
        m_object = UniverseObject::INVALID_OBJECT_ID;
        return;
    }
}

void RenameOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    UniverseObject* obj = GetObject(m_object);

    if (!obj) {
        Logger().errorStream() << "Attempted to rename nonexistant object with id " << m_object;
        return;
    }

    // verify that empire specified in order owns specified object
    if (!obj->WhollyOwnedBy(EmpireID())) {
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

void NewFleetOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    if (m_system_id == UniverseObject::INVALID_OBJECT_ID) {
        Logger().errorStream() << "Empire attempted to create a new fleet outside a system";
        return;
    }
    System* system = GetObject<System>(m_system_id);
    if (!system) {
        Logger().errorStream() << "Empire attempted to create a new fleet in a nonexistant system";
        return;
    }


    // create fleet
    Fleet* fleet = new Fleet(m_fleet_name, system->X(), system->Y(), EmpireID());
    fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);
    // an ID is provided to ensure consistancy between server and client universes
    universe.InsertID(fleet, m_new_id);
    system->Insert(fleet);


    // add ship(s) to fleet
    for (unsigned int i = 0; i < m_ship_ids.size(); ++i) {
        // verify that empire is not trying to take ships from somebody else's fleet
        const Ship* ship = GetObject<Ship>(m_ship_ids[i]);
        if (!ship) {
            Logger().errorStream() << "Empire attempted to create a new fleet with an invalid ship";
            return;
        }
        if (!ship->OwnedBy(EmpireID())) {
            Logger().errorStream() << "Empire attempted to create a new fleet with ships from another's fleet.";
            return;
        }
        fleet->AddShip(m_ship_ids[i]);
    }
}


////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() :
    Order(),
    m_fleet(UniverseObject::INVALID_OBJECT_ID),
    m_start_system(UniverseObject::INVALID_OBJECT_ID),
    m_dest_system(UniverseObject::INVALID_OBJECT_ID)
{}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet_id, int start_system_id, int dest_system_id) :
    Order(empire),
    m_fleet(fleet_id),
    m_start_system(start_system_id),
    m_dest_system(dest_system_id)
{
    const Universe& universe = GetUniverse();
    const ObjectMap& main_object_map = GetMainObjectMap();

    // perform sanity checks
    const Fleet* fleet = GetObject<Fleet>(FleetID());
    if (!fleet) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    const System* destination_system = main_object_map.Object<System>(DestinationSystemID());
    if (!destination_system) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet to move to system with id " << DestinationSystemID() << " but no such system exists / is known to exist";
        return;
    }

    // verify that empire specified in order owns specified fleet
    if ( !fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }

    std::pair<std::list<int>, double> short_path = universe.ShortestPath(m_start_system, m_dest_system, empire);

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

void FleetMoveOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Fleet* fleet = GetObject<Fleet>(FleetID());
    if (!fleet) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " ordered fleet with id " << FleetID() << " to move, but no such fleet exists";
        return;
    }

    const System* destination_system = GetEmpireKnownObject<System>(DestinationSystemID(), EmpireID());
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
    if ( !fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire with id " << EmpireID() << " order to move but does not own fleet with id " << FleetID();
        return;
    }


    // verify fleet route first system
    int fleet_sys_id = fleet->SystemID();
    if (fleet_sys_id != UniverseObject::INVALID_OBJECT_ID) {
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
    m_fleet_from(UniverseObject::INVALID_OBJECT_ID),
    m_fleet_to(UniverseObject::INVALID_OBJECT_ID)
{}

FleetTransferOrder::FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships) : 
    Order(empire),
    m_fleet_from(fleet_from),
    m_fleet_to(fleet_to),
    m_add_ships(ships)
{}

void FleetTransferOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    ObjectMap& objects = GetUniverse().Objects();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = objects.Object<Fleet>(SourceFleet());
    Fleet* target_fleet = objects.Object<Fleet>(DestinationFleet());

    if (!source_fleet || !target_fleet) {
        Logger().errorStream() << "Empire attempted to move ships to or from a nonexistant fleet";
        return;
    }

    // verify that empire is not trying to take ships from somebody else's fleet
    if ( !source_fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire attempted to merge ships from another's fleet.";
        return;
    }

    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided the server
    // verifies IDs of the Empires issuing the orders.
    if ( !target_fleet->OwnedBy(EmpireID()) ) {
        Logger().errorStream() << "Empire attempted to merge ships into another's fleet.";
        return;
    }


    // iterate down the ship vector and add each one to the fleet
    // after first verifying that it is a valid ship id
    std::vector<int>::const_iterator itr = m_add_ships.begin();
    while (itr != m_add_ships.end()) {
        // find the ship, verify that ID is valid
        int curr = (*itr);
        Ship* a_ship = objects.Object<Ship>(curr);
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
// FleetColonizeOrder
////////////////////////////////////////////////
FleetColonizeOrder::FleetColonizeOrder() : 
    Order(),
    m_ship(UniverseObject::INVALID_OBJECT_ID),
    m_planet(UniverseObject::INVALID_OBJECT_ID)
{}

FleetColonizeOrder::FleetColonizeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{}

void FleetColonizeOrder::ServerExecute() const
{
    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();


    Planet* planet = objects.Object<Planet>(m_planet);
    if (!planet) {
        Logger().errorStream() << "Empire attempted to colonize a nonexistant planet";
        return;
    }
    const Ship* ship = objects.Object<Ship>(m_ship);
    if (!ship) {
        Logger().errorStream() << "FleetColonizeOrder::ServerExecute couldn't find ship with id " << m_ship;
        return;
    }


    // get species to colonize with: species of ship
    const std::string& species_name = ship->SpeciesName();
    if (species_name.empty()) {
        Logger().errorStream() << "FleetColonizeOrder::ServerExecute ship has no species";
        return;
    }
    const Species* species = GetSpecies(species_name);
    if (!species) {
        Logger().errorStream() << "FleetColonizeOrder::ServerExecute couldn't get species with name: " << species_name;
        return;
    }


    // get colonist capacity of ship: sum of capacities of parts
    double colonist_capacity = 0.0;
    const ShipDesign* design = ship->Design();
    if (!design) {
        Logger().errorStream() << "FleetColonizeOrder::ServerExecute couldn't find ship's design!";
        return;
    }
    const std::vector<std::string>& parts = design->Parts();
    for (std::vector<std::string>::const_iterator it = parts.begin(); it != parts.end(); ++it) {
        const std::string& part_name = *it;
        if (part_name.empty())
            continue;
        const PartType* part_type = GetPartType(part_name);
        if (!part_type) {
            Logger().errorStream() << "FleetColonizeOrder::ServerExecute couldn't find ship part type: " << part_name;
            continue;
        }
        if (part_type->Class() == PC_COLONY) {
            colonist_capacity += boost::get<double>(part_type->Stats());
        }
    }
    if (colonist_capacity <= 0.0) {
        Logger().debugStream() << "colonize order executed by ship with zero colonist capacity!";
        return;
    }


    // all checks passed.  proceed with colonization.


    planet->Reset();
    planet->SetSpecies(species_name);   // do this BEFORE destroying the ship, since species_name is a const reference to Ship::m_species_name

    universe.Destroy(m_ship);


    // find a focus to give planets by default.  use first defined available focus.
    // the planet's AvailableFoci function should return a vector of all names of
    // available foci.
    std::vector<std::string> available_foci = planet->AvailableFoci();
    if (!available_foci.empty())
        planet->SetFocus(*available_foci.begin());

    planet->GetMeter(METER_POPULATION)->SetCurrent(colonist_capacity);
    planet->GetMeter(METER_FARMING)->SetCurrent(planet->GetMeter(METER_POPULATION)->Current());
    planet->GetMeter(METER_HEALTH)->SetCurrent(20.0);

    planet->AddOwner(EmpireID());

    const std::set<int>& planet_buildings = planet->Buildings();
    for (std::set<int>::const_iterator it = planet_buildings.begin(); it != planet_buildings.end(); ++it) {
        if (Building* building = objects.Object<Building>(*it)) {
            building->ClearOwners();
            building->AddOwner(EmpireID()); // TODO: only add owner if empire has visibility of building.  Need to add a check for this every turn... maybe doesn't need to be done during colonization at all?
        } else {
            Logger().errorStream() << "FleetColonizeOrder::ServerExecute couldn't find building with id: " << *it;
        }
    }

    //Logger().debugStream() << "colonizing planet " << planet->Name() << " by empire " << EmpireID() << " meters:";
    //for (MeterType meter_type = MeterType(0); meter_type != NUM_METER_TYPES; meter_type = MeterType(meter_type + 1))
    //    if (const Meter* meter = planet->GetMeter(meter_type))
    //        Logger().debugStream() << "type: " << boost::lexical_cast<std::string>(meter_type) << " val: " << meter->Initial();
}

void FleetColonizeOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();

    // look up the ship and fleet in question
    Ship* ship = objects.Object<Ship>(m_ship);
    if (!ship) {
        Logger().errorStream() << "Empire attempted to colonize with a nonexistant ship";
        return;
    }

    Fleet* fleet = objects.Object<Fleet>(ship->FleetID());
    if (!fleet) {
        Logger().errorStream() << "Empire attempte to colonize with a ship that somehow doesn't have a fleet...?";
        return;
    }

    // verify that empire issuing order owns specified fleet
    if (!fleet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to issue colonize order to another's fleet.";
        return;
    }

    // verify that planet exists and is un-occupied.
    Planet* planet = objects.Object<Planet>(m_planet);
    if (!planet) {
        Logger().errorStream() << "Colonization order issued with invalid planet id.";
        return;
    }

    if (!planet->Unowned()) {
        Logger().errorStream() << "Colonization order issued for owned planet.";
        return;
    }

    // verify that planet is in same system as the fleet
    if (planet->SystemID() != fleet->SystemID() || planet->SystemID() == UniverseObject::INVALID_OBJECT_ID) {
        Logger().errorStream() << "Fleet specified in colonization order is not in specified system.";
        return;
    }

    planet->SetIsAboutToBeColonized(true);

    m_colony_fleet_id = fleet->ID(); // record the fleet in which the colony ship started
    m_colony_fleet_name = fleet->Name();

    // Remove colony ship from fleet; if colony ship is only ship in fleet, destroy the fleet.
    // This leaves the ship in existence, and in its starting system, but not in any fleet;
    // this situation will be resolved by either ServerExecute() or UndoImpl().
    if (fleet->NumShips() == 1) {
        universe.Destroy(m_colony_fleet_id);
    } else {
        fleet->RemoveShip(m_ship);
    }
}

bool FleetColonizeOrder::UndoImpl() const
{
    // Note that this function does double duty: it serves as a normal client-side undo, but must also
    // serve as a server-side undo, when more than one empire tries to colonize the same planet at the
    // same time.

    Universe& universe = GetUniverse();
    ObjectMap& objects = universe.Objects();

    Planet* planet = objects.Object<Planet>(m_planet);
    if (!planet) {
        Logger().errorStream() << "Attempting to undo a fleet colonize order with an invalid planet id";
        return false;
    }
    Fleet* fleet = objects.Object<Fleet>(m_colony_fleet_id);
    // not having a fleet is OK - it may have been removed if the colony ship was the last ship in the fleet
    Ship* ship = objects.Object<Ship>(m_ship);
    if (!ship) {
        Logger().errorStream() << "Attempting to under a fleet colonize order with an invalid ship id";
        return false;
    }

    // if the fleet from which the colony ship came no longer exists or has moved, recreate it
    if (!fleet || fleet->SystemID() != ship->SystemID()) {
        System* system = objects.Object<System>(planet->SystemID());

        int         new_fleet_id =      m_colony_fleet_id;
        std::string new_fleet_name =    m_colony_fleet_name;

        if (fleet && (fleet->SystemID() != ship->SystemID())) {
            // fleet still exists, but it or ship are no longer in the same system, so 
            // need to create a new fleet for the ship in the system it is in

            // new id for new fleet
            new_fleet_id = GetNewObjectID();
            // name for new fleet
            std::vector<int> ship_ids;  ship_ids.push_back(m_ship);
            new_fleet_name = Fleet::GenerateFleetName(ship_ids, new_fleet_id);  // potential bug?!  Client and Server might be using different languge files, meaning the client will see fleet name in its set language on the turn of the undo, but will see a different language fleet name on the next turn which is permanently set by the server undo
        }

        fleet = new Fleet(new_fleet_name, system->X(), system->Y(), EmpireID());
        if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID) {
            Logger().errorStream() << "FleetColonizeOrder::UndoImpl(): Unable to obtain a new fleet ID";
            return false;
        }
        fleet->GetMeter(METER_STEALTH)->SetCurrent(Meter::LARGE_VALUE);

        universe.InsertID(fleet, new_fleet_id);
        fleet->AddShip(ship->ID());
        system->Insert(fleet);
    } else {
        fleet->AddShip(ship->ID());
    }

    planet->SetIsAboutToBeColonized(false);

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

void DeleteFleetOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Fleet* fleet = GetObject<Fleet>(FleetID());

    if (!fleet) {
        Logger().errorStream() << "Illegal fleet id specified in fleet delete order.";
        return;
    }

    if (!fleet->OwnedBy(EmpireID())) {
        Logger().errorStream() << "Empire attempted to issue deletion order to another's fleet.";
        return;
    }

    // this needs to be a no-op, instead of an exception case, because of its interaction with cancelled colonize orders
    // that cause a fleet to be deleted, then silently reconsistuted
    if (!fleet->Empty())
        return;

    GetUniverse().Delete(FleetID());
}


////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder() : 
    Order(),
    m_planet(UniverseObject::INVALID_OBJECT_ID),
    m_focus()
{}

ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, const std::string& focus) : 
    Order(empire),
    m_planet(planet),
    m_focus(focus)
{}

void ChangeFocusOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Planet* planet = GetObject<Planet>(PlanetID());

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

void ResearchQueueOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Empire* empire = Empires().Lookup(EmpireID());
    if (m_remove)
        empire->RemoveTechFromQueue(GetTech(m_tech_name));
    else
        empire->PlaceTechInQueue(GetTech(m_tech_name), m_position);
}

////////////////////////////////////////////////
// ProductionQueueOrder
////////////////////////////////////////////////
ProductionQueueOrder::ProductionQueueOrder() : 
    Order(),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_number(0),
    m_location(UniverseObject::INVALID_OBJECT_ID),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, BuildType build_type, const std::string& item, int number, int location) :
    Order(empire),
    m_build_type(build_type),
    m_item_name(item),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_number(number),
    m_location(location),
    m_index(INVALID_INDEX),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{
    if (build_type == BT_SHIP) {
        Logger().errorStream() << "Attempted to construct a ProductionQueueOrder for a BT_SHIP with a name, not a design id";
        build_type = INVALID_BUILD_TYPE;
        return;
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
    if (build_type == BT_BUILDING) {
        Logger().errorStream() << "Attempted to construct a ProductionQueueOrder for a BT_BUILDING with a design id, not a name";
        build_type = INVALID_BUILD_TYPE;
    }
}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_quantity, bool dummy) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_number(0),
    m_location(UniverseObject::INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(new_quantity),
    m_new_index(INVALID_INDEX)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index, int new_index) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_number(0),
    m_location(UniverseObject::INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(new_index)
{}

ProductionQueueOrder::ProductionQueueOrder(int empire, int index) :
    Order(empire),
    m_build_type(INVALID_BUILD_TYPE),
    m_item_name(""),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_number(0),
    m_location(UniverseObject::INVALID_OBJECT_ID),
    m_index(index),
    m_new_quantity(INVALID_QUANTITY),
    m_new_index(INVALID_INDEX)
{}

void ProductionQueueOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Empire* empire = Empires().Lookup(EmpireID());
    if (m_build_type == BT_BUILDING)
        empire->PlaceBuildInQueue(m_build_type, m_item_name, m_number, m_location);
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
    m_ship_design(),
    m_design_id(UniverseObject::INVALID_OBJECT_ID),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_ship_design(),
    m_design_id(existing_design_id_to_remember),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_ship_design(),
    m_design_id(design_id_to_erase),
    m_delete_design_from_empire(true),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design) :
    Order(empire),
    m_ship_design(ship_design),
    m_design_id(new_design_id),
    m_delete_design_from_empire(false),
    m_create_new_design(true)
{}

void ShipDesignOrder::ExecuteImpl() const
{
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
        if (m_ship_design.DesignedByEmpire() != EmpireID()) {
            Logger().errorStream() << "Tried to create a new ShipDesign designed by another empire";
            return;
        }

        // check if a design with this ID already exists
        if (universe.GetShipDesign(m_design_id)) {
            Logger().errorStream() << "Tried to create a new ShipDesign with an id of an already-existing ShipDesign";
            return;
        }
        ShipDesign* new_ship_design = new ShipDesign(m_ship_design);

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
    m_object_id(UniverseObject::INVALID_OBJECT_ID)
{}

ScrapOrder::ScrapOrder(int empire, int object_id) :
    Order(empire),
    m_object_id(object_id)
{}

void ScrapOrder::ExecuteImpl() const
{
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (Ship* ship = GetObject<Ship>(m_object_id)) {
        if (ship->SystemID() != UniverseObject::INVALID_OBJECT_ID && ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(true);
    } else if (Building* building = GetObject<Building>(m_object_id)) {
        int planet_id = building->PlanetID();
        if (const Planet* planet = GetObject<Planet>(planet_id)) {
            if (building->OwnedBy(empire_id) && planet->OwnedBy(empire_id))
                building->SetOrderedScrapped(true);
        }
    }
}

bool ScrapOrder::UndoImpl() const
{
    ValidateEmpireID();
    int empire_id = EmpireID();

    if (Ship* ship = GetObject<Ship>(m_object_id)) {
        if (ship->OwnedBy(empire_id))
            ship->SetOrderedScrapped(false);
    } else if (Building* building = GetObject<Building>(m_object_id)) {
        if (building->OwnedBy(empire_id))
            building->SetOrderedScrapped(false);
    }
    return true;
}
