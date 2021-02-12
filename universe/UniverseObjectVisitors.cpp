#include "UniverseObjectVisitors.h"

#include "Building.h"
#include "Field.h"
#include "Fighter.h"
#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"


StationaryFleetVisitor::StationaryFleetVisitor(int empire) :
    empire_id(empire)
{}

StationaryFleetVisitor::~StationaryFleetVisitor() = default;

auto StationaryFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if ((obj->FinalDestinationID() == INVALID_OBJECT_ID ||
         obj->TravelRoute().empty()) &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}


OrderedMovingFleetVisitor::OrderedMovingFleetVisitor(int empire) :
    empire_id(empire)
{}

OrderedMovingFleetVisitor::~OrderedMovingFleetVisitor() = default;

auto OrderedMovingFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        !obj->TravelRoute().empty() &&
        obj->SystemID() != INVALID_OBJECT_ID &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}


MovingFleetVisitor::MovingFleetVisitor(int empire) :
    empire_id(empire)
{}

MovingFleetVisitor::~MovingFleetVisitor() = default;

auto MovingFleetVisitor::Visit(const std::shared_ptr<Fleet>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->FinalDestinationID() != INVALID_OBJECT_ID &&
        obj->SystemID() == INVALID_OBJECT_ID &&
        (empire_id == ALL_EMPIRES || (!obj->Unowned() && obj->Owner() == empire_id)))
    { return obj; }
    return nullptr;
}


OwnedVisitor::OwnedVisitor(int empire) :
    empire_id(empire)
{}

OwnedVisitor::~OwnedVisitor() = default;

auto OwnedVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->OwnedBy(empire_id))
        return obj;
    return nullptr;
}


HostileVisitor::HostileVisitor(int viewing_empire, int owning_empire) :
    viewing_empire_id(viewing_empire),
    owning_empire_id(owning_empire)
{}

HostileVisitor::~HostileVisitor() = default;

auto HostileVisitor::Visit(const std::shared_ptr<UniverseObject>& obj) const -> std::shared_ptr<UniverseObject>
{
    if (obj->HostileToEmpire(viewing_empire_id, Empires())) // TODO: get from parameter or member...
        return obj;
    return nullptr;
}
