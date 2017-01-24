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

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<UniverseObject> obj) const
{ return boost::shared_ptr<UniverseObject>(); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Building> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Fleet> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Planet> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Ship> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<System> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Field> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }

boost::shared_ptr<UniverseObject> UniverseObjectVisitor::Visit(boost::shared_ptr<Fighter> obj) const
{ return Visit(boost::static_pointer_cast<UniverseObject>(obj)); }


////////////////////////////////////////////////
// StationaryFleetVisitor
////////////////////////////////////////////////
StationaryFleetVisitor::~StationaryFleetVisitor()
{}

StationaryFleetVisitor::StationaryFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

boost::shared_ptr<UniverseObject> StationaryFleetVisitor::Visit(boost::shared_ptr<Fleet> obj) const {
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->TravelRoute().empty()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return boost::shared_ptr<UniverseObject>();
}

////////////////////////////////////////////////
// OrderedMovingFleetVisitor
////////////////////////////////////////////////
OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor()
{}

OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

boost::shared_ptr<UniverseObject> OrderedMovingFleetVisitor::Visit(boost::shared_ptr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        !obj->TravelRoute().empty() &&
        obj->SystemID() != INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return boost::shared_ptr<UniverseObject>();
}

////////////////////////////////////////////////
// MovingFleetVisitor
////////////////////////////////////////////////
MovingFleetVisitor::~MovingFleetVisitor()
{}

MovingFleetVisitor::MovingFleetVisitor(int empire/* = ALL_EMPIRES*/) :
    empire_id(empire)
{}

boost::shared_ptr<UniverseObject> MovingFleetVisitor::Visit(boost::shared_ptr<Fleet> obj) const {
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID && 
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
        return obj;
    return boost::shared_ptr<UniverseObject>();
}
