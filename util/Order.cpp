#include "Order.h"

#ifdef FREEORION_BUILD_SERVER

#include <vector>
using std::vector;

#ifndef _ServerApp_h_
#include "../server/ServerApp.h"
#endif

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

#ifndef _UniverseObject_h_
#include "../universe/UniverseObject.h"
#endif

// VERY VERY TEMPORARY!  This should go into some sort of external
// XML file that the server uses for game rules like this
// I will be coding such a class in the near future -- jbarcz1
const int INITIAL_COLONY_POP = 1;

#endif


void Order::ValidateEmpireID() const
{
#ifdef FREEORION_BUILD_SERVER
    // check to make sure specified empire exists.  
    ServerApp* app = ServerApp::GetApp();
    ServerEmpireManager* empire = &app->Empires();
    
    if(empire->Lookup(EmpireID()) == NULL)
    {
        throw std::runtime_error("Invalid empire ID specified for order.");
    }
#endif
}

////////////////////////////////////////////////
// PlanetBuildOrder
////////////////////////////////////////////////
PlanetBuildOrder::PlanetBuildOrder() : 
   Order(),
   m_planet(-1),
   m_build_type(INVALID),
   m_ship_type(-1)
{
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
#ifdef FREEORION_BUILD_SERVER

    // TODO:  unit test this code once universe building methods
    // are in place.  the semantics of the building methods may change
    ValidateEmpireID();
    
    ServerApp* app = ServerApp::GetApp();
    ServerUniverse* universe = &app->Universe();
    ServerEmpireManager* empire = &app->Empires();
    
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
           
      
#endif
}


////////////////////////////////////////////////
// FleetMoveOrder
////////////////////////////////////////////////
FleetMoveOrder::FleetMoveOrder() : 
   Order(),
   m_fleet(-1),
   m_dest_system(-1)
{
}

FleetMoveOrder::FleetMoveOrder(int empire, int fleet, int dest_system) : 
   Order(empire),
   m_fleet(fleet),
   m_dest_system(dest_system)
{
}

void FleetMoveOrder::Execute() const
{
#ifdef FREEORION_BUILD_SERVER
    ValidateEmpireID();

    ServerApp* app = ServerApp::GetApp();
    ServerUniverse* universe = &app->Universe();
    ServerEmpireManager* empire = &app->Empires();
    
    // look up the fleet in question
    UniverseObject* the_object = universe->Object(this->FleetID());
    Fleet* the_fleet = dynamic_cast<Fleet*> ( the_object );
    
    // perform sanity check
    if(the_fleet == NULL)
    {
        throw std::runtime_error("Non-fleet object ID specified in fleet move order.");
    }
    
    // verify that empire specified in order owns specified fleet
    if( ! empire->Lookup(EmpireID())->HasFleet(this->FleetID()) )
    {
        throw std::runtime_error("Empire specified in fleet order does not own specified fleet.");
    }
    
    // look up destination
    UniverseObject* another_object = universe->Object(this->DestinationSystemID());
    System* the_system = dynamic_cast<System*> (another_object);
    
    // perform another sanity check
    if(the_system == NULL)
    {
        throw std::runtime_error("Non-system destination ID specified in fleet move order.");
    }
    
    // set the destination
    the_fleet->SetMoveOrders(this->DestinationSystemID());
    
    // TODO:  check destination validity (future versions)
    
#endif
}


////////////////////////////////////////////////
// FleetMergeOrder
////////////////////////////////////////////////
FleetMergeOrder::FleetMergeOrder() : 
   Order(),
   m_fleet(-1)
{
}

FleetMergeOrder::FleetMergeOrder(int empire, int fleet, const std::vector<int>& ships) : 
   Order(empire),
   m_fleet(fleet),
   m_add_ships(ships)
{
}

void FleetMergeOrder::Execute() const
{
#ifdef FREEORION_BUILD_SERVER
    ValidateEmpireID();
    ServerApp* app = ServerApp::GetApp();
    ServerUniverse* universe = &app->Universe();
    ServerEmpireManager* empire = &app->Empires();
    
    // look up the fleet in question
    UniverseObject* the_object = universe->Object(this->FleetID());
    Fleet* target_fleet = dynamic_cast<Fleet*> ( the_object );
    
    // sanity check
    if(!target_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet merge order.");
    }
    
    // verify that empire cannot merge ships into somebody else's fleet.
    // this is just an additional security measure.  IT could be removed to
    // allow 'donations' of ships to other players, provided that GameCore
    // verifies IDs of the Empires issuing the orders.
    if( !empire->Lookup( EmpireID() )->HasFleet( this->FleetID() ) )
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
        UniverseObject* curr_ship = universe->Object(curr);
        Ship* a_ship = dynamic_cast<Ship*> ( curr_ship);
        if(!a_ship)
        {
            throw std::runtime_error("Illegal ship id specified in fleet merge order.");
        }
        
        // figure out what fleet this ship is coming from
        int original_fleet_id = a_ship->FleetID();
        Fleet* original_fleet = dynamic_cast<Fleet*> ( universe->Object(original_fleet_id ));
        if( original_fleet )
        {
            // if ship is coming from a fleet, verify that empire giving the order
            // is the owner of that fleet.  
            if( ! empire->Lookup(EmpireID())->HasFleet(original_fleet_id) )
            {
                throw std::runtime_error("Empire attempted to issue merge order to ships in another's fleet.");
            }
            
            // remove ship from the fleet it came from
            original_fleet->RemoveShip(curr);
        }
        else
        {
            // specified ship has no fleet.
            // we cannot verify its ownership, so assume this is legal
        }
        
        // send the ship to its new fleet
        a_ship->SetFleetID(this->FleetID());
        target_fleet->AddShip(curr);
        
        itr++;
    }
    
    
   
#endif
}


////////////////////////////////////////////////
// FleetSplitOrder
////////////////////////////////////////////////
FleetSplitOrder::FleetSplitOrder() : 
   Order(),
   m_fleet(-1)
{
}

FleetSplitOrder::FleetSplitOrder(int empire, int fleet, const std::vector<int>& ships) : 
   Order(empire),
   m_fleet(fleet),
   m_remove_ships(ships)
{
}

void FleetSplitOrder::Execute() const
{
#ifdef FREEORION_BUILD_SERVER
    ValidateEmpireID();
    ServerApp* app = ServerApp::GetApp();
    ServerUniverse* universe = &app->Universe();
    ServerEmpireManager* empire = &app->Empires();
        
    
    // look up the fleet in question
    UniverseObject* the_object = universe->Object(this->FleetID());
    Fleet* target_fleet = dynamic_cast<Fleet*> ( the_object );
    
    // sanity check
    if(!target_fleet)
    {
        throw std::runtime_error("Illegal fleet id specified in fleet split order.");
    }
    
    // verify that empire giving the order is the owner of the fleet
    if( !empire->Lookup( EmpireID() )->HasFleet( this->FleetID() ) )
    {
        throw std::runtime_error("Empire attempted to split ships from another's fleet.");
    }
    
    // iterate down the ship vector and remove each ship from the fleet
    // after first verifying that it is a valid ship id
    vector<int>::const_iterator itr = m_remove_ships.begin();
    while(itr != m_remove_ships.end())
    {
        // find the ship, verify that ID is valid
        int curr = (*itr);
        UniverseObject* curr_ship = universe->Object(curr);
        Ship* a_ship = dynamic_cast<Ship*> ( curr_ship);
        if(!a_ship)
        {
            throw std::runtime_error("Illegal ship id specified in fleet split order.");
        }
        
        // make sure the ship is in the specified fleet
        int original_fleet_id = a_ship->FleetID();
        if(original_fleet_id != FleetID())
        {
            throw std::runtime_error("Fleet ID in split order does not match fleet ID in affected ship.");
        }
        
        // remove ship from its original fleet
        Fleet* original_fleet = dynamic_cast<Fleet*> ( universe->Object(original_fleet_id ));
        if(original_fleet)
        {
            original_fleet->RemoveShip(curr);
        }
        else
        {
            // specified ship has no fleet.  ERROR!
            throw std::runtime_error("Ship in split order has no assigned fleet, or assigned fleet ID is not valid.");
        }
        itr++;
    }
    
#endif
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

FleetColonizeOrder::FleetColonizeOrder(int empire, int fleet, int planet) : 
   Order(empire),
   m_fleet(fleet),
   m_planet(planet)
{
}
    




void FleetColonizeOrder::Execute() const
{
#ifdef FREEORION_BUILD_SERVER
    ValidateEmpireID();
    ServerApp* app = ServerApp::GetApp();
    ServerUniverse* universe = &app->Universe();
    ServerEmpireManager* empire = &app->Empires();
    
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
    
    // verify that planet is in same system as the fleet
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
        if(curr_ship->Design().id == ShipDesign::COLONY)
        {
            // adjust planet population
            target_planet->AdjustPop(INITIAL_COLONY_POP);
            
            // make this order's empire the owner of the planet
            target_planet->AddOwner( EmpireID() );
            
            // add empire to system owner list, if planet is in a system
            if(target_planet->SystemID() != UniverseObject::INVALID_ID )
            {
                universe->Object(target_planet->SystemID())->AddOwner( EmpireID() );
            }
            
            // remove colony ship from fleet and deallocate it
            colony_fleet->RemoveShip(curr_ship_id);
            universe->Delete(curr_ship_id);
            
            // order processing is now done
            return;
        }
    
    }
    
    throw std::runtime_error("Colonization order issued to fleet without colony ship.");
    
#endif
}

