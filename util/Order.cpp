#include "Order.h"
#include "../universe/UniverseObject.h"

#include "./AppInterface.h"

#ifndef _Planet_h_
#include "../universe/Planet.h"
#endif

#ifndef _Fleet_h_
#include "../universe/Fleet.h"
#endif

#ifndef _Ship_h_
#include "../universe/Ship.h"
#endif

#ifndef _System_h_
#include "../universe/System.h"
#endif

#include "../universe/Predicates.h"

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
    Order* GenNewFleetOrder(const XMLElement& elem)      {return new NewFleetOrder(elem);}
    Order* GenRenameOrder(const XMLElement& elem)   {return new RenameOrder(elem);}
    Order* GenFleetMoveOrder(const XMLElement& elem)     {return new FleetMoveOrder(elem);}
    Order* GenFleetTransferOrder(const XMLElement& elem) {return new FleetTransferOrder(elem);}
    Order* GenFleetColonizeOrder(const XMLElement& elem) {return new FleetColonizeOrder(elem);}
    Order* GenDeleteFleetOrder(const XMLElement& elem)   {return new DeleteFleetOrder(elem);}
}


/////////////////////////////////////////////////////
// ORDER
/////////////////////////////////////////////////////

Order::Order(const GG::XMLElement& elem)
{
    m_empire = lexical_cast<int>(elem.Child("m_empire").Attribute("value"));
}

GG::XMLElement Order::XMLEncode() const
{
    XMLElement elem("Order");
    
    XMLElement empire("m_empire");
    empire.SetAttribute("value", lexical_cast<std::string>(m_empire));
    elem.AppendChild(empire);
    
    return elem;
}


void Order::ValidateEmpireID() const
{
    EmpireManager* empire = &Empires(); 
    if(empire->Lookup(EmpireID()) == NULL)
    {
        throw std::runtime_error("Invalid empire ID specified for order.");
    }

}

void Order::InitOrderFactory(GG::XMLObjectFactory<Order>& fact)
{
    fact.AddGenerator("PlanetBuildOrder",   &GenPlanetBuildOrder);
    fact.AddGenerator("FleetSplitOrder",    &GenNewFleetOrder);
    fact.AddGenerator("FleetMoveOrder",     &GenFleetMoveOrder);
    fact.AddGenerator("FleetTransferOrder", &GenFleetTransferOrder);
    fact.AddGenerator("FleetColonizeOrder", &GenFleetColonizeOrder);
    fact.AddGenerator("DeleteFleetOrder",   &GenDeleteFleetOrder);
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
}

void RenameOrder::Execute() const
{
    ValidateEmpireID();

    UniverseObject* obj = GetUniverse().Object(m_object);\

    // verify that empire specified in order owns specified object
    if (!obj->WhollyOwnedBy(EmpireID()))
        throw std::runtime_error("Empire specified in rename order does not own specified object.");

    obj->Rename(m_name);
}

GG::XMLElement RenameOrder::XMLEncode() const
{
    XMLElement retval("RenameOrder");
    retval.AppendChild(Order::XMLEncode());
    retval.AppendChild(XMLElement("m_object", lexical_cast<std::string>(m_object)));
    retval.AppendChild(XMLElement("m_name", m_name));
    return retval;
}



////////////////////////////////////////////////
// PlanetBuildOrder
////////////////////////////////////////////////
PlanetBuildOrder::PlanetBuildOrder() : 
   Order(),
   m_planet(UniverseObject::INVALID_OBJECT_ID),
   m_build_type(ProdCenter::NOT_BUILDING)
{
}

PlanetBuildOrder::PlanetBuildOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{   
    if(elem.Tag() != ("PlanetBuildOrder"))
    {
        throw std::invalid_argument("Tried to construct PlanetBuildOrder from malformed XMLElement");
    }

    m_planet = lexical_cast<int>(elem.Child("m_planet").Attribute("value"));
    m_build_type = ProdCenter::BuildType(lexical_cast<int>(elem.Child("m_build_type").Attribute("value")));
}

PlanetBuildOrder::PlanetBuildOrder(int empire, int planet, ProdCenter::BuildType build) : 
   Order(empire),
   m_planet(planet),
   m_build_type(build)
{
}

void PlanetBuildOrder::Execute() const
{
    // TODO:  unit test this code once universe building methods
    // are in place.  the semantics of the building methods may change
    
    ValidateEmpireID();
 
    EmpireManager* empire = &Empires();
    Universe* universe = &GetUniverse();
    
    // look up object
    UniverseObject* the_object = universe->Object(PlanetID());
    Planet* the_planet = dynamic_cast<Planet*> ( the_object );
    
    // sanity check
    if(the_planet == NULL)
    {
        throw std::runtime_error("Non-planet object ID specified in planet build order.");
    }
    
    //  verify that empire specified in order owns specified planet
    if( !the_planet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire specified in planet build order does not own specified planet.");
    }
    
    the_planet->SetProduction(m_build_type);
}

GG::XMLElement PlanetBuildOrder::XMLEncode() const
{
    XMLElement elem("PlanetBuildOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement planet("m_planet");
    XMLElement build_type("m_build_type");
    XMLElement ship_type("m_ship_type");
    
    planet.SetAttribute("value", lexical_cast<std::string>(m_planet));
    build_type.SetAttribute("value", lexical_cast<std::string>(m_build_type));
    
    elem.AppendChild(planet);
    elem.AppendChild(build_type);
    elem.AppendChild(ship_type);
   
    return elem;
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
    if(elem.Tag() != ("FleetSplitOrder"))
        throw std::invalid_argument("Attempted to construct CreateFleetOrder from malformed XMLElement");
    
    const XMLElement* curr_elem = &elem.Child("m_fleet_name");
    m_fleet_name = curr_elem->Text();

    curr_elem = &elem.Child("m_system_id");
    m_system_id = lexical_cast<int>(curr_elem->Attribute("value"));

    curr_elem = &elem.Child("m_position");
    m_position = std::make_pair(lexical_cast<double>(curr_elem->Attribute("x")), lexical_cast<double>(curr_elem->Attribute("y")));

    curr_elem = &elem.Child("m_new_id");
    m_new_id = lexical_cast<int>(curr_elem->Attribute("value"));

}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name, const int new_id, int system_id) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(system_id),
    m_new_id( new_id ),
    m_position(std::make_pair(UniverseObject::INVALID_POSITION, UniverseObject::INVALID_POSITION))
{
}

NewFleetOrder::NewFleetOrder(int empire, const std::string& fleet_name,  const int new_id, double x, double y) :
    Order(empire),
    m_fleet_name(fleet_name),
    m_system_id(-1),
    m_new_id( new_id ),
    m_position(std::make_pair(x, y))
{
}

void NewFleetOrder::Execute() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    Fleet* fleet = 0;
    if (m_system_id != UniverseObject::INVALID_OBJECT_ID) {
        System* system = dynamic_cast<System*>(universe.Object(m_system_id));
        fleet = new Fleet(m_fleet_name, system->X(), system->Y(), EmpireID());
        
        // an ID is provided to ensure consistancy between server and client uiverses
        universe.InsertID(fleet, m_new_id );
        system->Insert(fleet);
    } else {
        fleet = new Fleet(m_fleet_name, m_position.first, m_position.second, EmpireID());
        // an ID is provided to ensure consistancy between server and client uiverses
        universe.InsertID(fleet, m_new_id );
    }
}

XMLElement NewFleetOrder::XMLEncode() const
{
    XMLElement elem("FleetSplitOrder");
    elem.AppendChild(Order::XMLEncode());

    XMLElement temp("m_fleet_name", m_fleet_name);
    elem.AppendChild(temp);

    temp = XMLElement("m_system_id");
    temp.SetAttribute("value", lexical_cast<std::string>(m_system_id));
    elem.AppendChild(temp);

    temp = XMLElement("m_new_id");
    temp.SetAttribute("value", lexical_cast<std::string>(m_new_id));
    elem.AppendChild(temp);

    temp = XMLElement("m_position");
    temp.SetAttribute("x", lexical_cast<std::string>(m_position.first));
    temp.SetAttribute("y", lexical_cast<std::string>(m_position.second));
    elem.AppendChild(temp);
    
    return elem;
}


////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() : 
   Order(),
   m_fleet(UniverseObject::INVALID_OBJECT_ID),
   m_dest_system(UniverseObject::INVALID_OBJECT_ID)
{
}

FleetMoveOrder::FleetMoveOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag()!=("FleetMoveOrder"))
        throw std::invalid_argument("Attempted to construct FleetMoveOrder from malformed XMLElement");
    
    m_fleet = lexical_cast<int>(elem.Child("m_fleet").Attribute("value"));
    m_start_system = lexical_cast<int>(elem.Child("m_start_system").Attribute("value"));
    m_dest_system = lexical_cast<int>(elem.Child("m_dest_system").Attribute("value"));
}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet, int start_system, int dest_system) : 
   Order(empire),
   m_fleet(fleet),
   m_start_system(start_system),
   m_dest_system(dest_system)
{
}

void FleetMoveOrder::Execute() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    EmpireManager& empire = Empires();
    
    // look up the fleet in question
    UniverseObject* the_object = universe.Object(FleetID());
    Fleet* the_fleet = dynamic_cast<Fleet*> ( the_object );
    
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
    System* the_system = dynamic_cast<System*> (another_object);
    
    // perform another sanity check
    if(the_system == NULL)
    {
        throw std::runtime_error("Non-system destination ID specified in fleet move order.");
    }
    
    // set the movement route
    std::pair<std::list<System*>, double> route = GetUniverse().ShortestPath(m_start_system, m_dest_system);
    the_fleet->SetRoute(route.first, route.second);
    
    // TODO:  check destination validity (once ship range is decided on)
}

XMLElement FleetMoveOrder::XMLEncode() const
{
    XMLElement elem("FleetMoveOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement fleet("m_fleet");
    XMLElement start_system("m_start_system");
    XMLElement dest_system("m_dest_system");
    
    fleet.SetAttribute("value", lexical_cast<std::string>(m_fleet));
    start_system.SetAttribute("value", lexical_cast<std::string>(m_start_system));
    dest_system.SetAttribute("value", lexical_cast<std::string>(m_dest_system));
    
    elem.AppendChild(fleet);
    elem.AppendChild(start_system);
    elem.AppendChild(dest_system);

    return elem;
}

////////////////////////////////////////////////
// FleetMergeOrder
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
    
    m_fleet_from = lexical_cast<int> (elem.Child("m_fleet_from").Attribute("value"));
    m_fleet_to = lexical_cast<int> (elem.Child("m_fleet_to").Attribute("value"));
    
    XMLElement container_elem = elem.Child("m_add_ships");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_add_ships.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
}

FleetTransferOrder::FleetTransferOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships) : 
   Order(empire),
   m_fleet_from(fleet_from),
   m_fleet_to(fleet_to),
   m_add_ships(ships)
{
}

void FleetTransferOrder::Execute() const
{

    ValidateEmpireID();

    Universe& universe = GetUniverse();
    EmpireManager& empire = Empires();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = dynamic_cast<Fleet*> ( universe.Object(SourceFleet()) );
    Fleet* target_fleet = dynamic_cast<Fleet*> ( universe.Object(DestinationFleet()) );
    
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
        Ship* a_ship = dynamic_cast<Ship*> ( universe.Object(curr));
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

XMLElement FleetTransferOrder::XMLEncode() const
{
    XMLElement elem("FleetTransferOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement fleet_from("m_fleet_from");
    XMLElement fleet_to("m_fleet_to");
    XMLElement ships("m_add_ships");
    
    fleet_from.SetAttribute("value", lexical_cast<std::string>(m_fleet_from));
    fleet_to.SetAttribute("value", lexical_cast<std::string>(m_fleet_to));
    
    int i=0;
    for( vector<int>::const_iterator itr = m_add_ships.begin(); itr != m_add_ships.end(); itr++)
    {
        GG::XMLElement item("index" + lexical_cast<std::string>(i) );
        i++;
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        ships.AppendChild(item);
    }
    
    elem.AppendChild(fleet_from);
    elem.AppendChild(fleet_to);
    elem.AppendChild(ships);
   
    return elem;
}


////////////////////////////////////////////////
// FleetColonizeOrder
////////////////////////////////////////////////
FleetColonizeOrder::FleetColonizeOrder() : 
   Order(),
   m_fleet(-1),
   m_planet(-1)
{
}

FleetColonizeOrder::FleetColonizeOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag() != ("FleetColonizeOrder"))
        throw std::invalid_argument("Attempted to construct FleetColonizeOrder from malformed XMLElement");
    
    m_fleet = lexical_cast<int> (elem.Child("m_fleet").Attribute("value"));
    m_planet = lexical_cast<int> (elem.Child("m_planet").Attribute("value"));
}

FleetColonizeOrder::FleetColonizeOrder(int empire, int fleet, int planet) : 
   Order(empire),
   m_fleet(fleet),
   m_planet(planet)
{
}

void FleetColonizeOrder::Execute() const
{
    ValidateEmpireID();

    Universe* universe = &GetUniverse();
    EmpireManager* empire = &Empires();
    
    // look up the fleet in question
    UniverseObject* the_object = universe->Object(FleetID());
    Fleet* colony_fleet = dynamic_cast<Fleet*> ( the_object );
   
    // sanity check -- ensure fleet exists
    if(!colony_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet colonize order.");
    }
     
    // verify that empire issuing order owns specified fleet
    if( !colony_fleet->OwnedBy(EmpireID()) )
    {
        throw std::runtime_error("Empire attempted to issue colonize order to another's fleet.");
    }
    
    // verify that planet exists and is un-occupied.
    the_object = universe->Object(PlanetID());
    Planet* target_planet = dynamic_cast<Planet*> ( the_object );
    if(target_planet == NULL)
    {
        throw std::runtime_error("Colonization order issued with invalid planet id.");
    }
    if(!target_planet->Unowned())
    {
        throw std::runtime_error("Colonization order issued for owned planet.");    
    }
    
    // verify that planet is in same system as the fleet
    if (target_planet->SystemID() != colony_fleet->SystemID() && target_planet->SystemID() != UniverseObject::INVALID_OBJECT_ID)
    {
        throw std::runtime_error("Fleet specified in colonization order is not in specified system.");
    }
    
    // verify that fleet contains a colony ship.  We iterate until we find
    // the first colony ship, then get rid of it, and use it to populate the planet
    for(Fleet::iterator ships = colony_fleet->begin(); ships != colony_fleet->end(); ships++)
    {
        int curr_ship_id = (*ships);
        Ship* curr_ship = dynamic_cast<Ship*> (universe->Object(curr_ship_id));
        if(curr_ship->Design().colonize)
        {
            // adjust planet population
            target_planet->AdjustPop(INITIAL_COLONY_POP);
            target_planet->SetWorkforce(INITIAL_COLONY_POP);
            target_planet->SetMaxWorkforce( target_planet->MaxPop() );
            
            // make this order's empire the owner of the planet
            target_planet->AddOwner( EmpireID() );
            
            // add empire to system owner list, if planet is in a system
            // add planet to the empire's list of owned planets
            if(target_planet->SystemID() != UniverseObject::INVALID_OBJECT_ID )
            {
                target_planet->GetSystem()->AddOwner(EmpireID());
            }
            
            // remove colony ship from fleet and deallocate it
            colony_fleet->RemoveShip(curr_ship_id);
            universe->Delete(curr_ship_id);
            
            // If colony ship was only ship in fleet, remove fleet
            if(colony_fleet->NumShips() == 0)
            {
                universe->Delete(colony_fleet->ID());
            }
            
            // order processing is now done
            return;
        }
    }
    throw std::runtime_error("Colonization order issued to fleet without colony ship.");
}


XMLElement FleetColonizeOrder::XMLEncode() const
{
    XMLElement elem("FleetColonizeOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement fleet("m_fleet");
    XMLElement planet("m_planet");
    
    fleet.SetAttribute("value", lexical_cast<std::string>(m_fleet));
    planet.SetAttribute("value", lexical_cast<std::string>(m_planet));
    
    elem.AppendChild(fleet);
    elem.AppendChild(planet);
    return elem;
}

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

    m_fleet = lexical_cast<int>(elem.Child("m_fleet").Attribute("value"));
}

DeleteFleetOrder::DeleteFleetOrder(int empire, int fleet) : 
    Order(empire),
    m_fleet(fleet)
{
}

void DeleteFleetOrder::Execute() const
{
    ValidateEmpireID();

    Fleet* fleet = dynamic_cast<Fleet*>(GetUniverse().Object(FleetID()));
    Empire* empire = Empires().Lookup(EmpireID());

    if(!fleet)
        throw std::runtime_error("Illegal fleet id specified in fleet colonize order.");

    if (!fleet->OwnedBy(EmpireID()))
        throw std::runtime_error("Empire attempted to issue deletion order to another's fleet.");

    if (fleet->NumShips())
        throw std::runtime_error("Attempted to delete an unempty fleet.");

    GetUniverse().Delete(FleetID());
}

GG::XMLElement DeleteFleetOrder::XMLEncode() const
{
    XMLElement elem("DeleteFleetOrder");
    elem.AppendChild(Order::XMLEncode());
    XMLElement temp("m_fleet");
    temp.SetAttribute("value", lexical_cast<std::string>(m_fleet));
    elem.AppendChild(temp);
    return elem;
}

