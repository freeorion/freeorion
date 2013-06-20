#include "AppInterface.h"

#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"
#include "../universe/Building.h"
#include "../universe/Field.h"
#include "../universe/Universe.h"

#include "Logger.h"
#include "OptionsDB.h"

#include <boost/timer.hpp>
#include <string>

const int INVALID_GAME_TURN = -(2 << 15) + 1;
const int BEFORE_FIRST_TURN = -(2 << 14);
const int IMPOSSIBLY_LARGE_TURN = 2 << 15;

EmpireManager& Empires() {
    return IApp::GetApp()->Empires();
}

Universe& GetUniverse() {
    return IApp::GetApp()->GetUniverse();
}

ObjectMap& Objects()
{ return GetUniverse().Objects(); }

ObjectMap& EmpireKnownObjects(int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id); }

UniverseObject* GetUniverseObject(int object_id)
{ return IApp::GetApp()->GetUniverseObject(object_id); }

std::string GetVisibleObjectName(const UniverseObject* object)
{ return IApp::GetApp()->GetVisibleObjectName(object); }

UniverseObject* GetEmpireKnownObject(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObject(object_id, empire_id); }

Planet* GetPlanet(int object_id)
{ return Objects().Object<Planet>(object_id); }

Planet* GetEmpireKnownPlanet(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<Planet>(object_id); }

System* GetSystem(int object_id)
{ return Objects().Object<System>(object_id); }

System* GetEmpireKnownSystem(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<System>(object_id); }

Field* GetField(int object_id)
{ return Objects().Object<Field>(object_id); }

Field* GetEmpireKnownField(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<Field>(object_id); }

Ship* GetShip(int object_id)
{ return Objects().Object<Ship>(object_id); }

Ship* GetEmpireKnownShip(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<Ship>(object_id); }

Fleet* GetFleet(int object_id)
{ return Objects().Object<Fleet>(object_id); }

Fleet* GetEmpireKnownFleet(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<Fleet>(object_id); }

Building* GetBuilding(int object_id)
{ return Objects().Object<Building>(object_id); }

Building* GetEmpireKnownBuilding(int object_id, int empire_id)
{ return EmpireKnownObjects(empire_id).Object<Building>(object_id); }

int GetNewObjectID()
{ return IApp::GetApp()->GetNewObjectID(); }

int GetNewDesignID()
{ return IApp::GetApp()->GetNewDesignID(); }

int CurrentTurn()
{ return IApp::GetApp()->CurrentTurn(); }

////////////////////////////////////////////////
// IApp
////////////////////////////////////////////////
// static member(s)
IApp*  IApp::s_app = 0;

IApp::IApp() {
    if (s_app)
        throw std::runtime_error("Attempted to construct a second instance of Application");

    s_app = this;
}

IApp::~IApp()
{ s_app = 0; }

IApp* IApp::GetApp()
{ return s_app; }

