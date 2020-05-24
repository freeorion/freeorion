#include "UniverseObjectVisitors.h"

#include "Building.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"


////////////////////////////////////////////////
// StationaryFleetVisitor
////////////////////////////////////////////////
StationaryFleetVisitor::~StationaryFleetVisitor()
{}

StationaryFleetVisitor::StationaryFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

std::shared_ptr<UniverseObject> StationaryFleetVisitor::Visit(std::shared_ptr<Fleet> obj) const {
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->TravelRoute().empty()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return nullptr;
}

////////////////////////////////////////////////
// OrderedMovingFleetVisitor
////////////////////////////////////////////////
OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor()
{}

OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

std::shared_ptr<UniverseObject> OrderedMovingFleetVisitor::Visit(std::shared_ptr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        !obj->TravelRoute().empty() &&
        obj->SystemID() != INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return nullptr;
}

////////////////////////////////////////////////
// MovingFleetVisitor
////////////////////////////////////////////////
MovingFleetVisitor::~MovingFleetVisitor()
{}

MovingFleetVisitor::MovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

std::shared_ptr<UniverseObject> MovingFleetVisitor::Visit(std::shared_ptr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return nullptr;
}

////////////////////////////////////////////////
// OwnedVisitor
////////////////////////////////////////////////
OwnedVisitor::OwnedVisitor(int empire) :
    empire_id(empire)
{}

OwnedVisitor::~OwnedVisitor() = default;

std::shared_ptr<UniverseObject> OwnedVisitor::Visit(std::shared_ptr<UniverseObject> obj) const
{
    if (obj->OwnedBy(empire_id))
        return obj;
    return nullptr;
}

////////////////////////////////////////////////
// HostileVisitor
////////////////////////////////////////////////
HostileVisitor::HostileVisitor(int viewing_empire, int owning_empire) :
    viewing_empire_id(viewing_empire),
    owning_empire_id(owning_empire)
{}

HostileVisitor::~HostileVisitor() = default;

std::shared_ptr<UniverseObject> HostileVisitor::Visit(std::shared_ptr<UniverseObject> obj) const
{
    if (obj->HostileToEmpire(viewing_empire_id))
        return obj;
    return nullptr;
}
