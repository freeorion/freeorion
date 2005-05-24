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

#include <boost/lexical_cast.hpp>
using boost::lexical_cast;


#include <vector>
using std::vector;

using GG::XMLElement;

// TEMPORARY!  This should go into some sort of external
// XML file that the server uses for game rules like this
// I will be coding such a class in the near future -- jbarcz1
const int INITIAL_COLONY_POP = 1;

namespace
{
    Order* GenPlanetBuildOrder(const XMLElement& elem)   {return new PlanetBuildOrder(elem);}
    Order* GenRenameOrder(const XMLElement& elem)        {return new RenameOrder(elem);}
    Order* GenNewFleetOrder(const XMLElement& elem)      {return new NewFleetOrder(elem);}
    Order* GenFleetMoveOrder(const XMLElement& elem)     {return new FleetMoveOrder(elem);}
    Order* GenFleetTransferOrder(const XMLElement& elem) {return new FleetTransferOrder(elem);}
    Order* GenFleetColonizeOrder(const XMLElement& elem) {return new FleetColonizeOrder(elem);}
    Order* GenDeleteFleetOrder(const XMLElement& elem)   {return new DeleteFleetOrder(elem);}
    Order* GenChangeFocusOrder(const XMLElement& elem)   {return new ChangeFocusOrder(elem);}
    Order* GenResearchQueueOrder(const XMLElement& elem) {return new ResearchQueueOrder(elem);}

    bool temp_header_bool = RecordHeaderFile(OrderRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}


/////////////////////////////////////////////////////
// Order
/////////////////////////////////////////////////////
Order::Order() :
    m_empire(-1),
    m_executed(false)
{
}

Order::Order(const GG::XMLElement& elem)
{
    m_empire = lexical_cast<int>(elem.Child("m_empire").Text());
    m_executed = lexical_cast<bool>(elem.Child("m_executed").Text());
}

GG::XMLElement Order::XMLEncode() const
{
    XMLElement retval("Order");
    retval.AppendChild(XMLElement("m_empire", lexical_cast<std::string>(m_empire)));
    retval.AppendChild(XMLElement("m_executed", lexical_cast<std::string>(m_executed)));
    return retval;
}


void Order::ValidateEmpireID() const
{
    EmpireManager* empire = &Empires(); 
    if(empire->Lookup(EmpireID()) == NULL)
    {
        throw std::runtime_error("Invalid empire ID specified for order.");
    }

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

void Order::InitOrderFactory(GG::XMLObjectFactory<Order>& fact)
{
    fact.AddGenerator("PlanetBuildOrder",   &GenPlanetBuildOrder);
    fact.AddGenerator("RenameOrder",        &GenRenameOrder);
    fact.AddGenerator("FleetSplitOrder",    &GenNewFleetOrder);
    fact.AddGenerator("FleetMoveOrder",     &GenFleetMoveOrder);
    fact.AddGenerator("FleetTransferOrder", &GenFleetTransferOrder);
    fact.AddGenerator("FleetColonizeOrder", &GenFleetColonizeOrder);
    fact.AddGenerator("DeleteFleetOrder",   &GenDeleteFleetOrder);
    fact.AddGenerator("ChangeFocusOrder",   &GenChangeFocusOrder);
    fact.AddGenerator("ResearchQueueOrder", &GenResearchQueueOrder);
}


////////////////////////////////////////////////
// RenameOrder
////////////////////////////////////////////////
RenameOrder::RenameOrder() : 
    Order(),
    m_object(UniverseObject::INVALID_OBJECT_ID)
{
}
   
RenameOrder::RenameOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if (elem.Tag()!=("RenameOrder"))
        throw std::invalid_argument("Attempted to construct RenameOrder from malformed XMLElement");
    
    m_object = lexical_cast<int>(elem.Child("m_object").Text());
    m_name = elem.Child("m_name").Text();
}

RenameOrder::RenameOrder(int empire, int fleet, const std::string& name) : 
    Order(empire),
    m_object(fleet),
    m_name(name)
{
    if (name == "")
        throw std::invalid_argument("RenameOrder::RenameOrder() : Attempted to name an object \"\".");
}

GG::XMLElement RenameOrder::XMLEncode() const
{
    XMLElement retval("RenameOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_object", lexical_cast<std::string>(m_object)));
    retval.AppendChild(XMLElement("m_name", m_name));
    return retval;
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
// PlanetBuildOrder
////////////////////////////////////////////////
PlanetBuildOrder::PlanetBuildOrder() : 
    Order(),
    m_planet(UniverseObject::INVALID_OBJECT_ID),
    m_build_type(BT_NOT_BUILDING),
    m_name("")
{
}

PlanetBuildOrder::PlanetBuildOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{   
    if (elem.Tag() != ("PlanetBuildOrder"))
        throw std::invalid_argument("Tried to construct PlanetBuildOrder from malformed XMLElement");

    m_planet = lexical_cast<int>(elem.Child("m_planet").Text());
    m_build_type = lexical_cast<BuildType>(elem.Child("m_build_type").Text());
    m_name = elem.Child("m_name").Text();
}

PlanetBuildOrder::PlanetBuildOrder(int empire, int planet, BuildType build, const std::string& name) : 
    Order(empire),
    m_planet(planet),
    m_build_type(build),
    m_name(name)
{
}

GG::XMLElement PlanetBuildOrder::XMLEncode() const
{
    XMLElement retval("PlanetBuildOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_planet", lexical_cast<std::string>(m_planet)));
    retval.AppendChild(XMLElement("m_build_type", lexical_cast<std::string>(m_build_type)));
    retval.AppendChild(XMLElement("m_name", m_name));
    return retval;
}

void PlanetBuildOrder::ExecuteImpl() const
{
    // TODO:  unit test this code once universe building methods
    // are in place.  the semantics of the building methods may change
    
    ValidateEmpireID();
 
    Universe& universe = GetUniverse();
    
    // look up object
    Planet* planet = universe.Object<Planet>(m_planet);
    
    // sanity check
    if(!planet)
        throw std::runtime_error("Non-Planet object ID specified in build order.");
    
    //  verify that empire specified in order owns specified planet
    if(!planet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire specified in build order does not own specified Planet.");
    
    planet->SetProduction(m_build_type, m_name);
}


////////////////////////////////////////////////
// CreateFleetOrder
////////////////////////////////////////////////
NewFleetOrder::NewFleetOrder() :
    Order()
{
}

NewFleetOrder::NewFleetOrder(const XMLElement& elem) : 
    Order(elem.Child("Order"))
{
    if (elem.Tag() != "FleetSplitOrder")
        throw std::invalid_argument("Attempted to construct CreateFleetOrder from malformed XMLElement");
    
    m_fleet_name = elem.Child("m_fleet_name").Text();
    m_system_id = lexical_cast<int>(elem.Child("m_system_id").Text());
    m_position = std::make_pair(lexical_cast<double>(elem.Child("m_position").Child("x").Text()), 
                                lexical_cast<double>(elem.Child("m_position").Child("y").Text()));
    m_new_id = lexical_cast<int>(elem.Child("m_new_id").Text());
    m_ship_id = lexical_cast<int>(elem.Child("m_ship_id").Text());
}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id, int ship_id/* = UniverseObject::INVALID_OBJECT_ID*/) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(system_id),
    m_new_id( new_id ),
    m_position(std::make_pair(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION)),
    m_ship_id(ship_id)
{
}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name,  const int new_id, double x, double y, int ship_id/* = UniverseObject::INVALID_OBJECT_ID*/) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(-1),
    m_new_id( new_id ),
    m_position(std::make_pair(x, y)),
    m_ship_id(ship_id)
{
}

XMLElement NewFleetOrder::XMLEncode() const
{
    XMLElement retval("FleetSplitOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_fleet_name", m_fleet_name));
    retval.AppendChild(XMLElement("m_system_id", lexical_cast<std::string>(m_system_id)));
    retval.AppendChild(XMLElement("m_new_id", lexical_cast<std::string>(m_new_id)));
    retval.AppendChild(XMLElement("m_position"));
    retval.LastChild().AppendChild(XMLElement("x", lexical_cast<std::string>(m_position.first)));
    retval.LastChild().AppendChild(XMLElement("y", lexical_cast<std::string>(m_position.second)));
    retval.AppendChild(XMLElement("m_ship_id", lexical_cast<std::string>(m_ship_id)));
    return retval;
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
        if (m_ship_id != UniverseObject::INVALID_OBJECT_ID)
            fleet->AddShip(m_ship_id);
        system->Insert(fleet);
    } else {
        fleet = new Fleet(m_fleet_name, m_position.first, m_position.second, EmpireID());
        // an ID is provided to ensure consistancy between server and client universes
        universe.InsertID(fleet, m_new_id);
        if (m_ship_id != UniverseObject::INVALID_OBJECT_ID)
            fleet->AddShip(m_ship_id);
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
{
}

FleetMoveOrder::FleetMoveOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag()!=("FleetMoveOrder"))
        throw std::invalid_argument("Attempted to construct FleetMoveOrder from malformed XMLElement");
    
    m_fleet = lexical_cast<int>(elem.Child("m_fleet").Text());
    m_start_system = lexical_cast<int>(elem.Child("m_start_system").Text());
    m_dest_system = lexical_cast<int>(elem.Child("m_dest_system").Text());
    m_route = GG::ContainerFromString<std::vector<int> >(elem.Child("m_route").Text());
    m_route_length = lexical_cast<double>(elem.Child("m_route_length").Text());
}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet, int start_system, int dest_system) : 
    Order(empire),
    m_fleet(fleet),
    m_start_system(start_system),
    m_dest_system(dest_system)
{
    std::pair<std::list<System*>, double> route = GetUniverse().ShortestPath(start_system, dest_system);
    for (std::list<System*>::iterator it = route.first.begin(); it != route.first.end(); ++it) {
        m_route.push_back((*it)->ID());
    }
    m_route_length = route.second;
}

XMLElement FleetMoveOrder::XMLEncode() const
{
    XMLElement retval("FleetMoveOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_fleet", lexical_cast<std::string>(m_fleet)));
    retval.AppendChild(XMLElement("m_start_system", lexical_cast<std::string>(m_start_system)));
    retval.AppendChild(XMLElement("m_dest_system", lexical_cast<std::string>(m_dest_system)));
    retval.AppendChild(XMLElement("m_route", GG::StringFromContainer<std::vector<int> >(m_route)));
    retval.AppendChild(XMLElement("m_route_length", lexical_cast<std::string>(m_route_length)));
    return retval;
}

void FleetMoveOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    
    // look up the fleet in question
    UniverseObject* the_object = universe.Object(FleetID());
    Fleet* the_fleet = universe_object_cast<Fleet*> ( the_object );
    
    // perform sanity check
    if(the_fleet == NULL)
    {
        throw std::runtime_error("Non-fleet object ID specified in fleet move order.");
    }
    
    // verify that empire specified in order owns specified fleet
    if( !the_fleet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire " + boost::lexical_cast<std::string>(EmpireID()) + 
                                 " specified in fleet order does not own specified fleet " + boost::lexical_cast<std::string>(FleetID()) + ".");
    }
    
    // look up destination
    UniverseObject* another_object = universe.Object(DestinationSystemID());
    System* the_system = universe_object_cast<System*> (another_object);
    
    // perform another sanity check
    if(the_system == NULL)
    {
        throw std::runtime_error("Non-system destination ID specified in fleet move order.");
    }

    // TODO:  check destination validity (once ship range is decided on)

    // set the movement route
    std::list<System*> route;
    for (unsigned int i = 0; i < m_route.size(); ++i) {
        route.push_back(GetUniverse().Object<System>(m_route[i]));
    }
    the_fleet->SetRoute(route, m_route_length);
}


////////////////////////////////////////////////
// FleetTransferOrder
////////////////////////////////////////////////
FleetTransferOrder::FleetTransferOrder() : 
    Order(),
    m_fleet_from(UniverseObject::INVALID_OBJECT_ID),
    m_fleet_to(UniverseObject::INVALID_OBJECT_ID)
{
}

FleetTransferOrder::FleetTransferOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag() !=("FleetTransferOrder"))
        throw std::invalid_argument("Attempted to construct FleetTransferOrder from malformed XMLElement");
    
    m_fleet_from = lexical_cast<int> (elem.Child("m_fleet_from").Text());
    m_fleet_to = lexical_cast<int> (elem.Child("m_fleet_to").Text());
    m_add_ships = GG::ContainerFromString<std::vector<int> >(elem.Child("m_add_ships").Text());
}

FleetTransferOrder::FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships) : 
    Order(empire),
    m_fleet_from(fleet_from),
    m_fleet_to(fleet_to),
    m_add_ships(ships)
{
}

XMLElement FleetTransferOrder::XMLEncode() const
{
    XMLElement retval("FleetTransferOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_fleet_from", lexical_cast<std::string>(m_fleet_from)));
    retval.AppendChild(XMLElement("m_fleet_to", lexical_cast<std::string>(m_fleet_to)));
    retval.AppendChild(XMLElement("m_add_ships", GG::StringFromContainer<std::vector<int> >(m_add_ships)));
    return retval;
}

void FleetTransferOrder::ExecuteImpl() const
{

    ValidateEmpireID();

    Universe& universe = GetUniverse();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = universe.Object<Fleet>(SourceFleet());
    Fleet* target_fleet = universe.Object<Fleet>(DestinationFleet());
    
    // sanity check
    if(!source_fleet || !target_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet merge order.");
    }
    
    // verify that empire is not trying to take ships from somebody else's fleet
    if( !source_fleet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire attempted to merge ships from another's fleet.");
    }
    
    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided the server
    // verifies IDs of the Empires issuing the orders.
    if( !target_fleet->OwnedBy(EmpireID()) )
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
        if(!a_ship)
        {
            throw std::runtime_error("Illegal ship id specified in fleet merge order.");
        }
        
        // figure out what fleet this ship is coming from -- verify its the one we
        // said it comes from
        if(a_ship->FleetID() != SourceFleet() )
        {
            throw std::runtime_error("Ship in merge order is not in specified source fleet.");
        }
        
        // send the ship to its new fleet
        a_ship->SetFleetID(DestinationFleet());
        source_fleet->RemoveShip(curr);
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
{
}

FleetColonizeOrder::FleetColonizeOrder(const GG::XMLElement& elem) :
    Order(elem.Child("Order"))
{
    if(elem.Tag() != ("FleetColonizeOrder"))
        throw std::invalid_argument("Attempted to construct FleetColonizeOrder from malformed XMLElement");
    
    m_ship   = lexical_cast<int>(elem.Child("m_ship").Text());
    m_planet = lexical_cast<int>(elem.Child("m_planet").Text());
    m_colony_fleet_id = lexical_cast<int>(elem.Child("m_colony_fleet_id").Text());
    m_colony_fleet_name = elem.Child("m_colony_fleet_name").Text();
}

FleetColonizeOrder::FleetColonizeOrder(int empire, int ship, int planet) :
    Order(empire),
    m_ship(ship),
    m_planet(planet)
{
}

void FleetColonizeOrder::ServerExecute() const
{
    Universe& universe = GetUniverse();
    universe.Delete(m_ship);
    Planet* planet = universe.Object<Planet>(m_planet);
    planet->ResetMaxMeters();
    planet->AdjustMaxMeters();
    planet->AdjustPop(INITIAL_COLONY_POP);
    planet->GetMeter(METER_FARMING)->SetCurrent(INITIAL_COLONY_POP);
    planet->GetMeter(METER_HEALTH)->SetCurrent(planet->GetMeter(METER_HEALTH)->Max());
    planet->AddOwner(EmpireID());
}

XMLElement FleetColonizeOrder::XMLEncode() const
{
    XMLElement retval("FleetColonizeOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_ship", lexical_cast<std::string>(m_ship)));
    retval.AppendChild(XMLElement("m_planet", lexical_cast<std::string>(m_planet)));
    retval.AppendChild(XMLElement("m_colony_fleet_id", lexical_cast<std::string>(m_colony_fleet_id)));
    retval.AppendChild(XMLElement("m_colony_fleet_name", m_colony_fleet_name));
    return retval;
}

void FleetColonizeOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();

    GG::XMLDoc doc;
    doc.root_node = universe.XMLEncode(EmpireID());
    std::ofstream ofs("before.xml");
    doc.WriteDoc(ofs);
    ofs.close();

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
    
    Fleet* fleet = universe.Object<Fleet>(m_colony_fleet_id);
    Planet* planet = universe.Object<Planet>(m_planet);

    planet->SetIsAboutToBeColonized(false);
    
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

    GG::XMLDoc doc;
    doc.root_node = universe.XMLEncode(EmpireID());
    std::ofstream ofs("after.xml");
    doc.WriteDoc(ofs);
    ofs.close();

    return true;
}


////////////////////////////////////////////////
// DeleteFleetOrder
////////////////////////////////////////////////
DeleteFleetOrder::DeleteFleetOrder() : 
    Order(),
    m_fleet(-1)
{
}

DeleteFleetOrder::DeleteFleetOrder(const GG::XMLElement& elem):
    Order(elem.Child("Order"))
{
    if(elem.Tag() != ("DeleteFleetOrder"))
        throw std::invalid_argument("Attempted to construct DeleteFleetOrder from malformed XMLElement");

    m_fleet = lexical_cast<int>(elem.Child("m_fleet").Text());
}

DeleteFleetOrder::DeleteFleetOrder(int empire, int fleet) : 
    Order(empire),
    m_fleet(fleet)
{
}

GG::XMLElement DeleteFleetOrder::XMLEncode() const
{
    XMLElement retval("DeleteFleetOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_fleet", lexical_cast<std::string>(m_fleet)));
    return retval;
}

void DeleteFleetOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Fleet* fleet = GetUniverse().Object<Fleet>(FleetID());

    if (!fleet)
        throw std::runtime_error("Illegal fleet id specified in fleet colonize order.");

    if (!fleet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire attempted to issue deletion order to another's fleet.");

    if (fleet->NumShips())
        throw std::runtime_error("Attempted to delete an unempty fleet.");

    GetUniverse().Delete(FleetID());
}


////////////////////////////////////////////////
// ChangeFocusOrder
////////////////////////////////////////////////
ChangeFocusOrder::ChangeFocusOrder() : 
    Order(),
    m_planet(UniverseObject::INVALID_OBJECT_ID),
    m_focus(FOCUS_UNKNOWN)
{
}

ChangeFocusOrder::ChangeFocusOrder(const GG::XMLElement& elem):
    Order(elem.Child("Order"))
{
    if(elem.Tag() != ("ChangeFocusOrder"))
        throw std::invalid_argument("Attempted to construct ChangeFocusOrder from malformed XMLElement");

    m_planet = lexical_cast<int>(elem.Child("m_planet").Text());
    m_focus = lexical_cast<FocusType>(elem.Child("m_focus").Text());
    m_primary = lexical_cast<bool>(elem.Child("m_primary").Text());
}

ChangeFocusOrder::ChangeFocusOrder(int empire, int planet, FocusType focus, bool primary) : 
    Order(empire),
    m_planet(planet),
    m_focus(focus),
    m_primary(primary)
{
}

GG::XMLElement ChangeFocusOrder::XMLEncode() const
{
    XMLElement retval("ChangeFocusOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_planet", lexical_cast<std::string>(m_planet)));
    retval.AppendChild(XMLElement("m_focus", lexical_cast<std::string>(m_focus)));
    retval.AppendChild(XMLElement("m_primary", lexical_cast<std::string>(m_primary)));
    return retval;
}

void ChangeFocusOrder::ExecuteImpl() const
{
    ValidateEmpireID();

    Planet* planet = GetUniverse().Object<Planet>(PlanetID());

    if(!planet)
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
{
}

ResearchQueueOrder::ResearchQueueOrder(const GG::XMLElement& elem):
    Order(elem.Child("Order"))
{
    if (elem.Tag() != ("ResearchQueueOrder"))
        throw std::invalid_argument("Attempted to construct ResearchQueueOrder from malformed XMLElement");

    m_tech_name = elem.Child("m_tech_name").Text();
    m_position = lexical_cast<int>(elem.Child("m_position").Text());
    m_remove = lexical_cast<bool>(elem.Child("m_remove").Text());
}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name) : 
    Order(empire),
    m_tech_name(tech_name),
    m_position(-1),
    m_remove(true)
{
}

ResearchQueueOrder::ResearchQueueOrder(int empire, const std::string& tech_name, int position) : 
    Order(empire),
    m_tech_name(tech_name),
    m_position(position),
    m_remove(false)
{
}

GG::XMLElement ResearchQueueOrder::XMLEncode() const
{
    XMLElement retval("ResearchQueueOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_tech_name", m_tech_name));
    retval.AppendChild(XMLElement("m_position", lexical_cast<std::string>(m_position)));
    retval.AppendChild(XMLElement("m_remove", lexical_cast<std::string>(m_remove)));
    return retval;
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
