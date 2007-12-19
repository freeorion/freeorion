#include "Order.h"

#include "AppInterface.h"
#include "../universe/Fleet.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OrderSet.h"
#include "../universe/Planet.h"
#include "../universe/Predicates.h"
#include "../universe/Ship.h"
#include "../universe/System.h"
#include "../universe/UniverseObject.h"
#include "../Empire/EmpireManager.h"
#include "../Empire/Empire.h"

#include <boost/lexical_cast.hpp>

#include <fstream>
#include <vector>

const Tech* GetTech(const std::string& name);

using boost::lexical_cast;
using std::vector;

#define DEBUG_CREATE_FLEET_ORDER 0
#define DEBUG_FLEET_MOVE_ORDER   0
#if DEBUG_CREATE_FLEET_ORDER || DEBUG_FLEET_MOVE_ORDER
#  include <iostream>
#endif

// TEMPORARY!  This should go into some sort of external config file that the server uses for game rules like this I
// will be coding such a class in the near future
const int INITIAL_COLONY_POP = 1;

/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
Order::Order() :
    m_empire(-1),
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
    if (name == "")
        throw std::invalid_argument("RenameOrder::RenameOrder() : Attempted to name an object \"\".");
}

void RenameOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    UniverseObject* obj = GetUniverse().Object(m_object);

    // verify that empire specified in order owns specified object
    if (!obj->WhollyOwnedBy(EmpireID()))
        throw std::runtime_error("Empire specified in rename order does not own specified object.");

    // disallow the name "", since that denotes an unknown object
    if (m_name == "")
        throw std::runtime_error("Name \"\" specified in rename order is invalid.");

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
    m_position(std::make_pair(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION)),
    m_ship_ids(ship_ids)
{
#if DEBUG_CREATE_FLEET_ORDER
    std::cerr << "NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id, int ship_id) : \n"
              << "    m_empire=" << EmpireID() << "\n"
              << "    m_fleet_name=" << m_fleet_name << "\n"
              << "    m_system_id=" << m_system_id << "\n"
              << "    m_position=(" << m_position.first << " " << m_position.second << ")\n"
              << "    m_new_id=" << m_new_id << "\n"
              << "    m_ship_ids.size()=" << m_ship_ids.size() << "\n"
              << std::endl;
#endif
}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name,  const int new_id, double x, double y, const std::vector<int>& ship_ids) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(-1),
    m_new_id(new_id),
    m_position(std::make_pair(x, y)),
    m_ship_ids(ship_ids)
{
#if DEBUG_CREATE_FLEET_ORDER
    std::cerr << "NewFleetOrder(int empire, const std::string& fleet_name,  const int new_id, double x, double y, int ship_id : \n"
              << "    m_empire=" << EmpireID() << "\n"
              << "    m_fleet_name=" << m_fleet_name << "\n"
              << "    m_system_id=" << m_system_id << "\n"
              << "    m_position=(" << m_position.first << " " << m_position.second << ")\n"
              << "    m_new_id=" << m_new_id << "\n"
              << "    m_ship_ids.size()=" << m_ship_ids.size() << "\n"
              << std::endl;
#endif
}

void NewFleetOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    Fleet* fleet = 0;
    if (m_system_id != UniverseObject::INVALID_OBJECT_ID) {
        System* system = universe.Object<System>(m_system_id);
        fleet = new Fleet(m_fleet_name, system->X(), system->Y(), EmpireID());
        // an ID is provided to ensure consistancy between server and client universes
        universe.InsertID(fleet, m_new_id);
        system->Insert(fleet);
    } else {
        fleet = new Fleet(m_fleet_name, m_position.first, m_position.second, EmpireID());
        // an ID is provided to ensure consistency between server and client universes
        universe.InsertID(fleet, m_new_id);
    }
    for (unsigned int i = 0; i < m_ship_ids.size(); ++i) {
        // verify that empire is not trying to take ships from somebody else's fleet
        if (!universe.Object(m_ship_ids[i])->OwnedBy(EmpireID()))
            throw std::runtime_error("Empire attempted to create a new fleet with ships from another's fleet.");
        fleet->AddShip(m_ship_ids[i]);
    }
}


////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() : 
    Order(),
    m_fleet(UniverseObject::INVALID_OBJECT_ID),
    m_dest_system(UniverseObject::INVALID_OBJECT_ID),
    m_route_length(0.0)
{}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet, int start_system, int dest_system) : 
    Order(empire),
    m_fleet(fleet),
    m_start_system(start_system),
    m_dest_system(dest_system)
{
    std::pair<std::list<System*>, double> route = GetUniverse().ShortestPath(start_system, dest_system, empire);
    for (std::list<System*>::iterator it = route.first.begin(); it != route.first.end(); ++it) {
        m_route.push_back((*it)->ID());
    }
    m_route_length = route.second;

#if DEBUG_FLEET_MOVE_ORDER
    std::cerr << "FleetMoveOrder(int empire, int fleet, int start_system, int dest_system) : \n"
              << "    m_empire=" << EmpireID() << "\n"
              << "    m_fleet=" << m_fleet << "\n"
              << "    m_start_system=" << m_start_system << "\n"
              << "    m_dest_system=" << m_dest_system << "\n"
              << "    m_route.size()=" << m_route.size() << "\n"
              << "    m_route_length=" << m_route_length << "\n"
              << std::endl;
#endif
}

void FleetMoveOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    
    Fleet* fleet = universe.Object<Fleet>(FleetID());
    System* system = universe.Object<System>(DestinationSystemID());
    
    // perform sanity checks
    if (!fleet) throw std::runtime_error("Non-fleet object ID specified in fleet move order.");
    if (!system) throw std::runtime_error("Non-system destination ID specified in fleet move order.");

    // verify that empire specified in order owns specified fleet
    if ( !fleet->OwnedBy(EmpireID()) )
        throw std::runtime_error("Empire " + boost::lexical_cast<std::string>(EmpireID()) + 
                                 " specified in fleet order does not own specified fleet " + boost::lexical_cast<std::string>(FleetID()) + ".");

    // TODO:  check destination validity (once ship range is decided on)

    // set the movement route
    std::list<System*> route;
    for (unsigned int i = 0; i < m_route.size(); ++i) {
        route.push_back(universe.Object<System>(m_route[i]));
    }
    fleet->SetRoute(route, m_route_length);
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

    Universe& universe = GetUniverse();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = universe.Object<Fleet>(SourceFleet());
    Fleet* target_fleet = universe.Object<Fleet>(DestinationFleet());

    // sanity check
    if (!source_fleet || !target_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet merge order.");
    }

    // verify that empire is not trying to take ships from somebody else's fleet
    if ( !source_fleet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire attempted to merge ships from another's fleet.");
    }

    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided the server
    // verifies IDs of the Empires issuing the orders.
    if ( !target_fleet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire attempted to merge ships into another's fleet.");
    }

    // iterate down the ship vector and add each one to the fleet
    // after first verifying that it is a valid ship id
    vector<int>::const_iterator itr = m_add_ships.begin();
    while(itr != m_add_ships.end())
    {
        // find the ship, verify that ID is valid
        int curr = (*itr);
        Ship* a_ship = universe.Object<Ship>(curr);
        if (!a_ship)
        {
            throw std::runtime_error("Illegal ship id specified in fleet merge order.");
        }

        // figure out what fleet this ship is coming from -- verify its the one we
        // said it comes from
        if (a_ship->FleetID() != SourceFleet() )
        {
            throw std::runtime_error("Ship in merge order is not in specified source fleet.");
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
    universe.Delete(m_ship);
    Planet* planet = universe.Object<Planet>(m_planet);
    planet->SetPrimaryFocus(FOCUS_BALANCED);
    planet->SetSecondaryFocus(FOCUS_BALANCED);
    planet->ResetMaxMeters();
    planet->ApplyUniverseTableMaxMeterAdjustments();
    planet->AdjustPop(INITIAL_COLONY_POP);
    planet->GetMeter(METER_FARMING)->SetCurrent(INITIAL_COLONY_POP);
    planet->GetMeter(METER_HEALTH)->SetCurrent(planet->GetMeter(METER_HEALTH)->Max());
    planet->AddOwner(EmpireID());
}

void FleetColonizeOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    // look up the ship and fleet in question
    Ship* ship = universe.Object<Ship>(m_ship);
    Fleet* fleet = universe.Object<Fleet>(ship->FleetID());

    // verify that empire issuing order owns specified fleet
    if (!fleet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire attempted to issue colonize order to another's fleet.");

    // verify that planet exists and is un-occupied.
    Planet* planet = universe.Object<Planet>(m_planet);
    if (!planet)
        throw std::runtime_error("Colonization order issued with invalid planet id.");

    if (!planet->Unowned())
        throw std::runtime_error("Colonization order issued for owned planet.");    

    // verify that planet is in same system as the fleet
    if (planet->SystemID() != fleet->SystemID() ||
        planet->SystemID() == UniverseObject::INVALID_OBJECT_ID) {
        throw std::runtime_error("Fleet specified in colonization order is not in "
                                 "specified system.");
    }

    planet->SetIsAboutToBeColonized(true);

    m_colony_fleet_id = fleet->ID(); // record the fleet in which the colony ship started
    m_colony_fleet_name = fleet->Name();

    // Remove colony ship from fleet; if colony ship is only ship in fleet, delete the fleet.
    // This leaves the ship in existence, and in its starting system, but not in any fleet;
    // this situation will be resolved by either ServerExecute() or UndoImpl().
    if (fleet->NumShips() == 1) {
        universe.Delete(fleet->ID());
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
    
    Planet* planet = universe.Object<Planet>(m_planet);
    Fleet* fleet = universe.Object<Fleet>(m_colony_fleet_id);
    Ship* ship = universe.Object<Ship>(m_ship);

    // if the fleet from which the colony ship came no longer exists or has moved, recreate it
    if (!fleet || fleet->SystemID() != ship->SystemID()) {
        System* system = planet->GetSystem();
        int new_fleet_id = !fleet ? m_colony_fleet_id : GetNewObjectID();
        fleet = new Fleet(!fleet ? m_colony_fleet_name : "Colony Fleet", system->X(), system->Y(), EmpireID());
        if (new_fleet_id == UniverseObject::INVALID_OBJECT_ID)
            throw std::runtime_error("FleetColonizeOrder::UndoImpl(): Unable to obtain a new fleet ID");
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

    Fleet* fleet = GetUniverse().Object<Fleet>(FleetID());

    if (!fleet)
        throw std::runtime_error("Illegal fleet id specified in fleet delete order.");

    if (!fleet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire attempted to issue deletion order to another's fleet.");

    // this needs to be a no-op, instead of an exception case, because of its interaction with cancelled colonize orders
    // that cause a fleet to be deleted, then silently reconsistuted
    if (fleet->NumShips())
        return;

    GetUniverse().Delete(FleetID());
}


////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder() : 
    Order(),
    m_planet(UniverseObject::INVALID_OBJECT_ID),
    m_focus(FOCUS_UNKNOWN)
{}

ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, FocusType focus, bool primary) : 
    Order(empire),
    m_planet(planet),
    m_focus(focus),
    m_primary(primary)
{}

void ChangeFocusOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Planet* planet = GetUniverse().Object<Planet>(PlanetID());

    if (!planet)
        throw std::runtime_error("Illegal planet id specified in change planet focus order.");

    if (!planet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire attempted to issue change planet focus to another's planet.");

    m_primary ? planet->SetPrimaryFocus(m_focus) : planet->SetSecondaryFocus(m_focus);
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
{
}

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
    if (build_type == BT_SHIP)
        throw std::invalid_argument("Attempted to construct a ProductionQueueOrder for a BT_SHIP with a name, not a design id");
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
    if (build_type == BT_BUILDING || build_type == BT_ORBITAL)
        throw std::invalid_argument("Attempted to construct a ProductionQueueOrder for a BT_BUILDING or BT_ORBITAL with a design id, not a name");
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
    if (m_build_type == BT_BUILDING || m_build_type == BT_ORBITAL)
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
        throw std::runtime_error("Malformed ProductionQueueOrder.");
}



////////////////////////////////////////////////
// ShipDesignOrder
////////////////////////////////////////////////
//ShipDesign                  m_ship_design;
//int                         m_design_id;
//bool                        m_delete_design_from_empire;
//bool                        m_create_new_design;
ShipDesignOrder::ShipDesignOrder() :
    Order(),
    m_ship_design(),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int existing_design_id_to_remember) :
    Order(empire),
    m_ship_design(),
    m_delete_design_from_empire(false),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int design_id_to_erase, bool dummy) :
    Order(empire),
    m_ship_design(),
    m_delete_design_from_empire(true),
    m_create_new_design(false)
{}

ShipDesignOrder::ShipDesignOrder(int empire, int new_design_id, const ShipDesign& ship_design) :
    Order(empire),
    m_ship_design(ship_design),
    m_delete_design_from_empire(false),
    m_create_new_design(true)
{}

void ShipDesignOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Empire* empire = Empires().Lookup(EmpireID());
    if (m_delete_design_from_empire) {
        if (!empire->ShipDesignKept(m_design_id))
            throw std::runtime_error("Tried to remove a ShipDesign that the empire wasn't remembering");
        empire->RemoveShipDesign(m_design_id);

    } else if (m_create_new_design) {
        if (m_ship_design.DesignedByEmpire() != EmpireID())
            throw std::runtime_error("Tried to create a new ShipDesign designed by another empire");

        Universe& universe = GetUniverse();

        // check if a design with this ID alreadyh exists
        if (universe.GetShipDesign(m_design_id))
            throw std::runtime_error("Tried to create a new ShipDesign with an id of an already-existing ShipDesign");
        ShipDesign* new_ship_design = new ShipDesign(m_ship_design);

        universe.InsertShipDesignID(new_ship_design, m_design_id);
        empire->AddShipDesign(m_design_id);

    } else if (!m_create_new_design && !m_delete_design_from_empire) {
        // check if empire is already remembering the design
        if (empire->ShipDesignKept(m_design_id))
            throw std::runtime_error("Tried to remember a ShipDesign that was already being remembered");

        // check if the empire can see any objects that have this design (thus enabling it to be copied)
        std::vector<Ship*> ship_vec = GetUniverse().FindObjects<Ship>();
        bool known = false;
        for (std::vector<Ship*>::const_iterator it = ship_vec.begin(); it != ship_vec.end(); ++it) {
            if (Universe::ALL_OBJECTS_VISIBLE || (*it)->GetVisibility(EmpireID()) != UniverseObject::NO_VISIBILITY) {
                if ((*it)->ShipDesignID() == m_design_id) {
                    known = true;
                    break;
                }
            }
        }

        if (known)
            empire->AddShipDesign(m_design_id);
        else
            throw std::runtime_error("Tried to remember a ShipDesign that this empire can't see");
 
    } else {
        throw std::runtime_error("Malformed ShipDesignOrder.");
    }
}
