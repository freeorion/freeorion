#include "Predicates.h"

#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "Field.h"
#include "Fighter.h"
#include "UniverseObject.h"

////////////////////////////////////////////////
// UniverseObjectVisitor
////////////////////////////////////////////////
UniverseObjectVisitor::~UniverseObjectVisitor()
{}

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<UniverseObject> obj) const
{ return nullptr; }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Building> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Fleet> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Planet> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Ship> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<System> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Field> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }

std::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(std::shared_ptr<Fighter> obj) const
{ return Visit(std::static_pointer_cast<UniverseObject>(obj)); }


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
