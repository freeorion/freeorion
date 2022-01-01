#include "UniverseObjectVisitors.h"

#include "Building.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"


auto StationaryFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->TravelRoute().empty()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}

auto OrderedMovingFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        !obj->TravelRoute().empty() &&
        obj->SystemID() != INVALID_OBJECT_ID &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}

auto MovingFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}

auto OwnedVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{ return obj->OwnedBy(empire_id) ? obj : nullptr; }

auto UnownedVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{ return obj->Unowned() ? obj : nullptr; }

auto UnownedVisitor::Visit(const std::shared_ptr<System>& obj) const -> std::shared_ptr<UniverseObject>
{ return obj; }

auto HostileVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{ return obj->HostileToEmpire(hostile_to_empire_id, empires) ? obj : nullptr; }

auto NotInSetsVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{
    int obj_id = obj->ID();
    if (set1.find(obj_id) != set1.end())
        return nullptr;
    return set2.find(obj_id) != set2.end() ? nullptr : obj;
}
