#include "Predicates.h"

#include "Building.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "Field.h"
#include "UniverseObject.h"

////////////////////////////////////////////////
// UniverseObjectVisitor
////////////////////////////////////////////////
UniverseObjectVisitor::~UniverseObjectVisitor()
{}

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<UniverseObject> obj) const
{ return TemporaryPtr<UniverseObject>(); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Building> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Fleet> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Planet> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Ship> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<System> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Field> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

////////////////////////////////////////////////
// StationaryFleetVisitor
////////////////////////////////////////////////
StationaryFleetVisitor::~StationaryFleetVisitor()
{}

StationaryFleetVisitor::StationaryFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<UniverseObject> StationaryFleetVisitor::Visit(TemporaryPtr<Fleet> obj) const {
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->FinalDestinationID() == obj->SystemID()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<UniverseObject>();
}

////////////////////////////////////////////////
// OrderedMovingFleetVisitor
////////////////////////////////////////////////
OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor()
{}

OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<UniverseObject> OrderedMovingFleetVisitor::Visit(TemporaryPtr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->FinalDestinationID() != obj->SystemID() &&
        obj->SystemID() != INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<UniverseObject>();
}

////////////////////////////////////////////////
// MovingFleetVisitor
////////////////////////////////////////////////
MovingFleetVisitor::~MovingFleetVisitor()
{}

MovingFleetVisitor::MovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<UniverseObject> MovingFleetVisitor::Visit(TemporaryPtr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<UniverseObject>();
}
