#include "Predicates.h"

#include "Fleet.h"
#include "Planet.h"
#include "Ship.h"
#include "System.h"
#include "UniverseObject.h"

bool IsStationaryFleetFunctor::operator()(const UniverseObject* obj) const
{
    if (const Fleet* fleet = dynamic_cast<const Fleet*>(obj)) {
        if ((fleet->FinalDestinationID() == UniverseObject::INVALID_OBJECT_ID ||
             fleet->FinalDestinationID() == fleet->SystemID()) && 
            (empire_id == -1 || (!fleet->Owners().empty() && *fleet->Owners().begin() == empire_id)))
            return true;
    }
    return false;
}

bool IsOrderedMovingFleetFunctor::operator()(const UniverseObject* obj) const
{
    if (const Fleet* fleet = dynamic_cast<const Fleet*>(obj)) {
        if (fleet->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID &&
            fleet->FinalDestinationID() != fleet->SystemID() &&
            fleet->SystemID() != UniverseObject::INVALID_OBJECT_ID && 
            (empire_id == -1 || (!fleet->Owners().empty() && *fleet->Owners().begin() == empire_id)))
            return true;
    }
    return false;
}

bool IsMovingFleetFunctor::operator()(const UniverseObject* obj) const
{
    if (const Fleet* fleet = dynamic_cast<const Fleet*>(obj)) {
        if (fleet->FinalDestinationID() != UniverseObject::INVALID_OBJECT_ID &&
            fleet->SystemID() == UniverseObject::INVALID_OBJECT_ID && 
            (empire_id == -1 || (!fleet->Owners().empty() && *fleet->Owners().begin() == empire_id)))
            return true;
    }
    return false;
}
