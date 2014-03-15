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

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const UniverseObject> obj)
{ return TemporaryPtr<const UniverseObject>(); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const Building> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const Fleet> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const Planet> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const Ship> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const System> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<const UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<const Field> obj)
{ return Visit(boost::static_pointer_cast<const UniverseObject>(obj)); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<UniverseObject> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const UniverseObject>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Building> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const Building>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Fleet> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const Fleet>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Planet> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const Planet>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Ship> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const Ship>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<System> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const System>(obj))); }

TemporaryPtr<UniverseObject> UniverseObjectVisitor::Visit(TemporaryPtr<Field> obj)
{ return boost::const_pointer_cast<UniverseObject>(Visit(boost::const_pointer_cast<const Field>(obj))); }

UniverseObjectVisitor::operator UniverseObjectVisitorRR& ()
{ return *static_cast<UniverseObjectVisitorRR*>(this); }


////////////////////////////////////////////////
// StationaryFleetVisitor
////////////////////////////////////////////////
StationaryFleetVisitor::~StationaryFleetVisitor()
{}

StationaryFleetVisitor::StationaryFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<const UniverseObject> StationaryFleetVisitor::Visit(TemporaryPtr<const Fleet> obj) {
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->FinalDestinationID() == obj->SystemID()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<const UniverseObject>();
}

////////////////////////////////////////////////
// OrderedMovingFleetVisitor
////////////////////////////////////////////////
OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor()
{}

OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<const UniverseObject> OrderedMovingFleetVisitor::Visit(TemporaryPtr<const Fleet> obj) {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->FinalDestinationID() != obj->SystemID() &&
        obj->SystemID() != INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<const UniverseObject>();
}

////////////////////////////////////////////////
// MovingFleetVisitor
////////////////////////////////////////////////
MovingFleetVisitor::~MovingFleetVisitor()
{}

MovingFleetVisitor::MovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

TemporaryPtr<const UniverseObject> MovingFleetVisitor::Visit(TemporaryPtr<const Fleet> obj) {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return TemporaryPtr<const UniverseObject>();
}
