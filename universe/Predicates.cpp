#include "Predicates.h"

#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"

#include "../util/MultiplayerCommon.h"

////////////////////////////////////////////////
// UniverseObjectVisitor
////////////////////////////////////////////////
UniverseObjectVisitor::~UniverseObjectVisitor()
{}

UniverseObject* UniverseObjectVisitor::Visit(UniverseObject* obj) const
{
    return 0;
}

UniverseObject* UniverseObjectVisitor::Visit(Building* obj) const
{
    return Visit(static_cast<UniverseObject*>(obj));
}

UniverseObject* UniverseObjectVisitor::Visit(Fleet* obj) const
{
    return Visit(static_cast<UniverseObject*>(obj));
}

UniverseObject* UniverseObjectVisitor::Visit(Planet* obj) const
{
    return Visit(static_cast<UniverseObject*>(obj));
}

UniverseObject* UniverseObjectVisitor::Visit(Ship* obj) const
{
    return Visit(static_cast<UniverseObject*>(obj));
}

UniverseObject* UniverseObjectVisitor::Visit(System* obj) const
{
    return Visit(static_cast<UniverseObject*>(obj));
}


////////////////////////////////////////////////
// StationaryFleetVisitor
////////////////////////////////////////////////
StationaryFleetVisitor::~StationaryFleetVisitor()
{}

StationaryFleetVisitor::StationaryFleetVisitor(int empire/* = -1*/) :
    empire_id(empire)
{
}

UniverseObject* StationaryFleetVisitor::Visit(Fleet* obj) const
{
    if ((obj->FinalDestinationID() == UniverseObject::INVALID_OBJECT_ID ||
         obj->FinalDestinationID() == obj->SystemID()) && 
        (empire_id == -1 || (!obj->Owners().empty() && *obj->Owners().begin() == empire_id)))
        return obj;
    return 0;
}

////////////////////////////////////////////////
// OrderedMovingFleetVisitor
////////////////////////////////////////////////
OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor()
{}

OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire/* = -1*/) :
    empire_id(empire)
{
}

UniverseObject* OrderedMovingFleetVisitor::Visit(Fleet* obj) const
{
    if (obj->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID &&
        obj->FinalDestinationID() != obj->SystemID() &&
        obj->SystemID() != UniverseObject::INVALID_OBJECT_ID && 
        (empire_id == -1 || (!obj->Owners().empty() && *obj->Owners().begin() == empire_id)))
        return obj;
    return 0;
}

////////////////////////////////////////////////
// MovingFleetVisitor
////////////////////////////////////////////////
MovingFleetVisitor::~MovingFleetVisitor()
{}

MovingFleetVisitor::MovingFleetVisitor(int empire/* = -1*/) :
    empire_id(empire)
{
}

UniverseObject* MovingFleetVisitor::Visit(Fleet* obj) const
{
    if (obj->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID &&
        obj->SystemID() == UniverseObject::INVALID_OBJECT_ID && 
        (empire_id == -1 || (!obj->Owners().empty() && *obj->Owners().begin() == empire_id)))
        return obj;
    return 0;
}
