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
    Order* GenPlanetBuildOrder(const XMLElement& elem) { return new PlanetBuildOrder(elem);};
    Order* GenFleetMoveOrder(const XMLElement& elem) { return new FleetMoveOrder(elem);};
    Order* GenFleetMergeOrder(const XMLElement& elem) { return new FleetMergeOrder(elem);};
    Order* GenFleetSplitOrder(const XMLElement& elem) { return new FleetSplitOrder(elem);};
    Order* GenFleetColonizeOrder(const XMLElement& elem) { return new FleetColonizeOrder(elem);};
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
    fact.AddGenerator("PlanetBuildOrder", &GenPlanetBuildOrder);
    fact.AddGenerator("FleetMoveOrder", &GenFleetMoveOrder);
    fact.AddGenerator("FleetMergeOrder", &GenFleetMergeOrder);
    fact.AddGenerator("FleetSplitOrder", &GenFleetSplitOrder);
    fact.AddGenerator("FleetColonizeOrder", &GenFleetColonizeOrder);
}

////////////////////////////////////////////////
// PlanetBuildOrder
////////////////////////////////////////////////
PlanetBuildOrder::PlanetBuildOrder() : 
   Order(),
   m_planet(UniverseObject::INVALID_OBJECT_ID),
   m_build_type(INVALID),
   m_ship_type(-1)
{
}

PlanetBuildOrder::PlanetBuildOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{   
    if(elem.Tag() != ("PlanetBuildOrder"))
    {
        throw std::invalid_argument("Tried to construct PlanetBuildOrder from malformed XMLElement");
    }

    m_planet = lexical_cast<int> (elem.Child("m_planet").Attribute("value"));
    m_build_type = (BuildType)lexical_cast<int> (elem.Child("m_build_type").Attribute("value"));
    m_ship_type = lexical_cast<int> (elem.Child("m_ship_type").Attribute("value"));
}

PlanetBuildOrder::PlanetBuildOrder(int empire, int planet, BuildType build, int ship_type/* = -1*/) : 
   Order(empire),
   m_planet(planet),
   m_build_type(build),
   m_ship_type(ship_type)
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
    UniverseObject* the_object = universe->Object(this->PlanetID());
    Planet* the_planet = dynamic_cast<Planet*> ( the_object );
    
    // sanity check
    if(the_planet == NULL)
    {
        throw std::runtime_error("Non-planet object ID specified in planet build order.");
    }
    
    //  verify that empire specified in order owns specified planet
    if( ! empire->Lookup(EmpireID())->HasPlanet(this->PlanetID()) )
    {
        throw std::runtime_error("Empire specified in planet build order does not own specified planet.");
    }
    
    // change the build type
    switch(this->BuildOrder())
    {
        case INDUSTRY_BUILD:
            the_planet->BuildIndustry();
            break;
            
        case RESEARCH_BUILD:
            the_planet->DoResearch();
            break;
            
        case SHIP_BUILD:
            the_planet->BuildShip( ShipType() );
            break;
        
        case DEF_BASE:  
            the_planet->BuildDefBase();
            break;
        
    }
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
    ship_type.SetAttribute("value", lexical_cast<std::string>(m_ship_type));
    
    elem.AppendChild(planet);
    elem.AppendChild(build_type);
    elem.AppendChild(ship_type);
    
   
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
    m_dest_system = lexical_cast<int>(elem.Child("m_dest_system").Attribute("value"));
}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet, int dest_system) : 
   Order(empire),
   m_fleet(fleet),
   m_dest_system(dest_system)
{
}

void FleetMoveOrder::Execute() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    EmpireManager& empire = Empires();
    
    // look up the fleet in question
    UniverseObject* the_object = universe.Object(this->FleetID());
    Fleet* the_fleet = dynamic_cast<Fleet*> ( the_object );
    
    // perform sanity check
    if(the_fleet == NULL)
    {
        throw std::runtime_error("Non-fleet object ID specified in fleet move order.");
    }
    
    // verify that empire specified in order owns specified fleet
    if( ! empire.Lookup(EmpireID())->HasFleet(this->FleetID()) )
    {
        throw std::runtime_error("Empire specified in fleet order does not own specified fleet.");
    }
    
    // look up destination
    UniverseObject* another_object = universe.Object(this->DestinationSystemID());
    System* the_system = dynamic_cast<System*> (another_object);
    
    // perform another sanity check
    if(the_system == NULL)
    {
        throw std::runtime_error("Non-system destination ID specified in fleet move order.");
    }
    
    // set the destination
    the_fleet->SetMoveOrders(this->DestinationSystemID());
    
    // TODO:  check destination validity (once ship range is decided on)
}

XMLElement FleetMoveOrder::XMLEncode() const
{
    XMLElement elem("FleetMoveOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement fleet("m_fleet");
    XMLElement dest_system("m_dest_system");
    
    fleet.SetAttribute("value", lexical_cast<std::string>(m_fleet));
    dest_system.SetAttribute("value", lexical_cast<std::string>(m_dest_system));
    
    elem.AppendChild(fleet);
    elem.AppendChild(dest_system);

    return elem;
}

////////////////////////////////////////////////
// FleetMergeOrder
////////////////////////////////////////////////
FleetMergeOrder::FleetMergeOrder() : 
   Order(),
   m_fleet_from(UniverseObject::INVALID_OBJECT_ID),
   m_fleet_to(UniverseObject::INVALID_OBJECT_ID)
{
}

FleetMergeOrder::FleetMergeOrder(const GG::XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag() !=("FleetMergeOrder"))
        throw std::invalid_argument("Attempted to construct FleetMergeOrder from malformed XMLElement");
    
    m_fleet_from = lexical_cast<int> (elem.Child("m_fleet_from").Attribute("value"));
    m_fleet_to = lexical_cast<int> (elem.Child("m_fleet_to").Attribute("value"));
    
    XMLElement container_elem = elem.Child("m_add_ships");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_add_ships.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
}

FleetMergeOrder::FleetMergeOrder(int empire, int fleet_from, int fleet_to, const std::vector<int>& ships) : 
   Order(empire),
   m_fleet_from(fleet_from),
   m_fleet_to(fleet_to),
   m_add_ships(ships)
{
}

void FleetMergeOrder::Execute() const
{

    ValidateEmpireID();

    Universe& universe = GetUniverse();
    EmpireManager& empire = Empires();

    // look up the source fleet and destination fleet
    Fleet* source_fleet = dynamic_cast<Fleet*> ( universe.Object(this->SourceFleet()) );
    Fleet* target_fleet = dynamic_cast<Fleet*> ( universe.Object(this->DestinationFleet()) );
    
    // sanity check
    if(!source_fleet || !target_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet merge order.");
    }
    
    // verify that empire is not trying to take ships from somebody else's fleet
    if( !empire.Lookup( EmpireID() )->HasFleet( this->SourceFleet() ) )
    {
        throw std::runtime_error("Empire attempted to merge ships from another's fleet.");
    }
    
    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided that GameCore
    // verifies IDs of the Empires issuing the orders.
    if( !empire.Lookup( EmpireID() )->HasFleet( this->DestinationFleet() ) )
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
        int original_fleet_id = a_ship->FleetID();
        if(a_ship->FleetID() != SourceFleet() )
        {
            throw std::runtime_error("Ship in merge order is not in specified source fleet.");
        }
        
        // send the ship to its new fleet
        a_ship->SetFleetID(this->DestinationFleet());
        source_fleet->RemoveShip(curr);
        target_fleet->AddShip(curr);
        
        itr++;
    }
    
    if(source_fleet->ShipCount() == 0)
    {
        // if fleet is out of ships, then get rid of it
        universe.Delete(source_fleet->ID());
        empire.Lookup(EmpireID())->RemoveFleet(source_fleet->ID());
    }
}

XMLElement FleetMergeOrder::XMLEncode() const
{
    XMLElement elem("FleetMergeOrder");
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
// FleetSplitOrder
////////////////////////////////////////////////
FleetSplitOrder::FleetSplitOrder() : 
   Order()
{
}

FleetSplitOrder::FleetSplitOrder(const XMLElement& elem) : Order(elem.Child("Order"))
{
    if(elem.Tag() != ("FleetSplitOrder"))
        throw std::invalid_argument("Attempted to construct FleetSplitOrder from malformed XMLElement");
    
    XMLElement container_elem = elem.Child("m_remove_ships");
    for(int i=0; i<container_elem.NumChildren(); i++)
    {
        m_remove_ships.push_back(  lexical_cast<int> (container_elem.Child(i).Attribute("value") ) );
    }
}

FleetSplitOrder::FleetSplitOrder(int empire, const std::vector<int>& ships) : 
   Order(empire),
   m_remove_ships(ships)
{
}

void FleetSplitOrder::Execute() const
{
    ValidateEmpireID();

    Universe& universe = GetUniverse();
    EmpireManager& empire = Empires();

    // iterate down the ship vector and remove each ship from its fleet
    // we check as we go to make sure that:
    //     - ship and fleet IDs are valid
    //     - empire issuing order owns all fleets
    //     - all fleets are located within same system
    //
    double fleet_x = -1;
    double fleet_y = -1;
    vector<int>::const_iterator itr = m_remove_ships.begin();
    while(itr != m_remove_ships.end())
    {
        // find the ship, verify that ID is valid
        int curr = (*itr);
        Ship* a_ship = dynamic_cast<Ship*> ( universe.Object(curr) );
        if(!a_ship)
        {
            throw std::runtime_error("Illegal ship id specified in fleet split order.");
        }
        
        // find the fleet that the ship is in
        Fleet* original_fleet = dynamic_cast<Fleet*> ( universe.Object(a_ship->FleetID() ));
        if(!original_fleet)
        {
            // specified ship has no fleet.  ERROR!
            throw std::runtime_error("Ship in split order has no assigned fleet, or assigned fleet ID is not valid.");
        }
        
        // verify that issuing empire owns the ship's fleet
        if( !empire.Lookup( EmpireID() )->HasFleet( a_ship->FleetID() ) )
        {
            throw std::runtime_error("Empire attempted to split ships from another's fleet.");
        }
        
        // *****************************************************************
        // verify that fleet is in same location as the others, and that
        // all fleets are in a system (not moving)
        // ******************************************************************
        
        // make sure that fleet has no move orders
        if( original_fleet->MoveOrders() != UniverseObject::INVALID_OBJECT_ID)
        {
            throw std::runtime_error("Split order involves moving fleet.");
        }
        
        if(fleet_x == -1 && fleet_y == -1)
        {
            // means this is first iteration of the loop and we dont know yet
            // where all this is happening
            fleet_x = original_fleet->X();
            fleet_y = original_fleet->Y();
        }
        else
        {
            if(original_fleet->X() != fleet_x || original_fleet->Y() != fleet_y)
            {
                throw std::runtime_error("Split order involves fleets in different systems.");
            }
        }
        
        original_fleet->RemoveShip(curr);
        if(original_fleet->ShipCount() == 0)
        {
            universe.Delete(original_fleet->ID());
            empire.Lookup(EmpireID())->RemoveFleet(original_fleet->ID());
        }
        itr++;
    }
    
    //  Create new fleet with the specified ships in it
    Fleet* fleet = new Fleet("New Fleet", fleet_x, fleet_y, EmpireID());
    fleet->AddShips(m_remove_ships);
    universe.Insert(fleet);
    empire.Lookup(EmpireID())->AddFleet(fleet->ID());
    
}

XMLElement FleetSplitOrder::XMLEncode() const
{
    XMLElement elem("FleetSplitOrder");
    elem.AppendChild(Order::XMLEncode());
    
    XMLElement remove_ships("m_remove_ships");
    int i=0;
    for( vector<int>::const_iterator itr = m_remove_ships.begin(); itr != m_remove_ships.end(); itr++)
    {
        GG::XMLElement item("index" + lexical_cast<std::string>(i) );
        i++;
        item.SetAttribute("value", lexical_cast<std::string>( (*itr) ) );
        remove_ships.AppendChild(item);
    }
    
    elem.AppendChild(remove_ships);
    
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
    UniverseObject* the_object = universe->Object(this->FleetID());
    Fleet* colony_fleet = dynamic_cast<Fleet*> ( the_object );
   
    // sanity check -- ensure fleet exists
    if(!colony_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet colonize order.");
    }
     
    // verify that empire issuing order owns specified fleet
    if( !empire->Lookup( EmpireID() )->HasFleet( this->FleetID() ) )
    {
        throw std::runtime_error("Empire attempted to issue colonize order to another's fleet.");
    }
    
    
    
    // verify that planet exists and is un-occupied.
    the_object = universe->Object(this->PlanetID());
    Planet* target_planet = dynamic_cast<Planet*> ( the_object );
    if(target_planet == NULL)
    {
        throw std::runtime_error("Colonization order issued with invalid planet id.");
    }
    if(target_planet->Owners().size() != 0)
    {
        throw std::runtime_error("Colonization order issued for owned planet.");    
    }
    
    // verify that planet is in same position as the fleet
    if(  (target_planet->X() != colony_fleet->X())  || target_planet->Y() != colony_fleet->Y())
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
            
            // make this order's empire the owner of the planet
            target_planet->AddOwner( EmpireID() );
            
            // add empire to system owner list, if planet is in a system
            // add planet to the empire's list of owned planets
            if(target_planet->SystemID() != UniverseObject::INVALID_OBJECT_ID )
            {
                universe->Object(target_planet->SystemID())->AddOwner( EmpireID() );
                empire->Lookup(EmpireID())->AddPlanet(PlanetID());
            }
            
            // remove colony ship from fleet and deallocate it
            colony_fleet->RemoveShip(curr_ship_id);
            universe->Delete(curr_ship_id);
            
            // If colony ship was only ship in fleet, remove fleet
            if(colony_fleet->ShipCount() == 0)
            {
                universe->Delete(colony_fleet->ID());
                empire->Lookup(EmpireID())->RemoveFleet(colony_fleet->ID());
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

