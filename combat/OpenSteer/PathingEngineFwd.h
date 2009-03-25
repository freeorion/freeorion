// -*- C++ -*-
#ifndef PATHING_ENGINE_FWD_H
#define PATHING_ENGINE_FWD_H

#include "ProximityDatabase.h"
#include "../universe/Enums.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>


namespace OpenSteer {
    class AbstractVehicle;
    class AbstractObstacle;
}

class CombatFighterFormation;
class CombatFighter;
class CombatShip;
class PathingEngine;

class CombatObject;

typedef boost::shared_ptr<CombatObject> CombatObjectPtr;
typedef boost::weak_ptr<CombatObject> CombatObjectWeakPtr;
typedef boost::shared_ptr<CombatFighterFormation> CombatFighterFormationPtr;
typedef boost::shared_ptr<CombatFighter> CombatFighterPtr;
typedef boost::shared_ptr<CombatShip> CombatShipPtr;

typedef ProximityDatabase<OpenSteer::AbstractVehicle*> ProximityDB;
typedef ProximityDB::TokenType ProximityDBToken;

#endif
