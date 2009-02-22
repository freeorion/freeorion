#include "CombatOrder.h"

#include <cassert>

////////////////////////////////////////////////////////////////////////////////
// ShipMission
////////////////////////////////////////////////////////////////////////////////
ShipMission::ShipMission() :
    m_type(NONE),
    m_target()
{}

ShipMission::ShipMission(Type type) :
    m_type(type)
{
    assert(m_type == NONE ||
           m_type == ATTACK_SHIPS_WEAKEST_FIRST_STANDOFF ||
           m_type == ATTACK_SHIPS_NEAREST_FIRST_STANDOFF ||
           m_type == ATTACK_SHIPS_WEAKEST_FIRST ||
           m_type == ATTACK_SHIPS_NEAREST_FIRST ||
           m_type == ENTER_STARLANE);
}

ShipMission::ShipMission(Type type, const Vec3& destination) :
    m_type(type),
    m_destination(destination),
    m_target()
{
    assert(m_type == MOVE_TO ||
           m_type == PATROL_TO);
}

ShipMission::ShipMission(Type type, int target) :
    m_type(type),
    m_destination(),
    m_target(target)
{
    assert(m_type == ATTACK_THIS_STANDOFF ||
           m_type == ATTACK_THIS ||
           m_type == DEFEND_THIS);
}


////////////////////////////////////////////////////////////////////////////////
// FighterMission
////////////////////////////////////////////////////////////////////////////////
FighterMission::FighterMission() :
    m_type(NONE),
    m_target()
{}

FighterMission::FighterMission(Type type) :
    m_type(type)
{
    assert(m_type == NONE ||
           m_type == ATTACK_FIGHTERS_BOMBERS_FIRST ||
           m_type == ATTACK_FIGHTERS_INTERCEPTORS_FIRST ||
           m_type == ATTACK_SHIPS_WEAKEST_FIRST ||
           m_type == ATTACK_SHIPS_NEAREST_FIRST ||
           m_type == RETURN_TO_BASE);
}

FighterMission::FighterMission(Type type, const Vec3& destination) :
    m_type(type),
    m_destination(destination),
    m_target()
{
    assert(m_type == MOVE_TO ||
           m_type == PATROL_TO);
}

FighterMission::FighterMission(Type type, int target) :
    m_type(type),
    m_destination(),
    m_target(target)
{
    assert(m_type == ATTACK_THIS ||
           m_type == DEFEND_THIS);
}


/////////////////////////////////////////////////////
// CombatOrder
/////////////////////////////////////////////////////
CombatOrder::CombatOrder() :
    m_order_type(),
    m_id()
{}

CombatOrder::CombatOrder(int id, const ShipMission& ship_mission) :
    m_order_type(SHIP_ORDER),
    m_id(id),
    m_ship_mission(ship_mission)
{}

CombatOrder::CombatOrder(int id, const FighterMission& fighter_mission) :
    m_order_type(FIGHTER_ORDER),
    m_id(id),
    m_fighter_mission(fighter_mission)
{}

CombatOrder::OrderType CombatOrder::Type() const
{ return m_order_type; }

int CombatOrder::ID() const
{ return m_id; }

const ShipMission& CombatOrder::GetShipMission() const
{
    assert(m_order_type == SHIP_ORDER);
    return m_ship_mission;
}

const FighterMission& CombatOrder::GetFighterMission() const
{
    assert(m_order_type == FIGHTER_ORDER);
    return m_fighter_mission;
}
