#include "Order.h"

#ifdef FREEORION_BUILD_SERVER
#include "../server/ServerApp.h"
#endif

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
   ServerApp* app = ServerApp::GetApp();
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
   ServerApp* app = ServerApp::GetApp();
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
   ServerApp* app = ServerApp::GetApp();
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
   ServerApp* app = ServerApp::GetApp();
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
   ServerApp* app = ServerApp::GetApp();
#endif
}

