#include "AppInterface.h"

#ifdef FREEORION_BUILD_SERVER
# include "../server/ServerApp.h"
#else // a client build
# ifdef FREEORION_BUILD_HUMAN
#  include "../client/human/HumanClientApp.h"
# else
#  undef int64_t
#  include "../client/AI/AIClientApp.h"
# endif
#endif

#include "../universe/Planet.h"
#include "../universe/System.h"
#include "../universe/Ship.h"
#include "../universe/Fleet.h"
#include "../universe/Building.h"

const int INVALID_GAME_TURN = -(2 << 15) + 1;
const int BEFORE_FIRST_TURN = -(2 << 14);
const int IMPOSSIBLY_LARGE_TURN = 2 << 15;

EmpireManager& Empires() {
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->Empires();
#else
    return ClientApp::GetApp()->Empires();
#endif
}

Universe& GetUniverse() {
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->GetUniverse();
#else
    return ClientApp::GetApp()->GetUniverse();
#endif
}

ObjectMap& Objects() {
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->GetUniverse().Objects();
#else
    return ClientApp::GetApp()->GetUniverse().EmpireKnownObjects(ClientApp::GetApp()->EmpireID());
#endif
}

UniverseObject* GetUniverseObject(int object_id) {
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().Objects().Object(object_id);
#else
    // attempt to get live / up to date / mutable object
    UniverseObject* obj = GetUniverse().Objects().Object(object_id);
    // if not up to date info, use latest known out of date info about object
    if (!obj)
        obj = GetUniverse().EmpireKnownObjects(ClientApp::GetApp()->EmpireID()).Object(object_id);
    return obj;
#endif
}

UniverseObject* GetEmpireKnownObject(int object_id, int empire_id) {
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().EmpireKnownObjects(empire_id).Object(object_id);
#else
    return GetUniverseObject(object_id);// as of this writing, players don't have info about what other players know about objects
#endif
}

template <class T>
T* GetUniverseObject(int object_id)
{
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().Objects().Object<T>(object_id);
#else
    // attempt to get live / up to date / mutable object
    T* obj = GetUniverse().Objects().Object<T>(object_id);
    // if not up to date info, use latest known out of date info about object
    if (!obj)
        obj = GetUniverse().EmpireKnownObjects(ClientApp::GetApp()->EmpireID()).Object<T>(object_id);
    return obj;
#endif
}

template <class T>
T* GetEmpireKnownObject(int object_id, int empire_id)
{
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().EmpireKnownObjects(empire_id).Object<T>(object_id);
#else
    // as of this writing, players don't have info about what other players know about objects
    return GetUniverseObject<T>(object_id);
#endif
}

Planet* GetPlanet(int object_id)
{ return GetUniverseObject<Planet>(object_id); }

Planet* GetEmpireKnownPlanet(int object_id, int empire_id)
{ return GetEmpireKnownObject<Planet>(object_id, empire_id); }

System* GetSystem(int object_id)
{ return GetUniverseObject<System>(object_id); }

System* GetEmpireKnownSystem(int object_id, int empire_id)
{ return GetEmpireKnownObject<System>(object_id, empire_id); }

Ship* GetShip(int object_id)
{ return GetUniverseObject<Ship>(object_id); }

Ship* GetEmpireKnownShip(int object_id, int empire_id)
{ return GetEmpireKnownObject<Ship>(object_id, empire_id); }

Fleet* GetFleet(int object_id)
{ return GetUniverseObject<Fleet>(object_id); }

Fleet* GetEmpireKnownFleet(int object_id, int empire_id)
{ return GetEmpireKnownObject<Fleet>(object_id, empire_id); }

Building* GetBuilding(int object_id)
{ return GetUniverseObject<Building>(object_id); }

Building* GetEmpireKnownBuilding(int object_id, int empire_id)
{ return GetEmpireKnownObject<Building>(object_id, empire_id); }

log4cpp::Category& Logger()
{ return log4cpp::Category::getRoot(); }

int GetNewObjectID() {
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().GenerateObjectID();
#else
    return ClientApp::GetApp()->GetNewObjectID();
#endif
}

int GetNewDesignID() {
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().GenerateDesignID();
#else
    return ClientApp::GetApp()->GetNewDesignID();
#endif
}

int CurrentTurn() {
#ifdef FREEORION_BUILD_SERVER
    return ServerApp::GetApp()->CurrentTurn();
#else
    return const_cast<const ClientApp*>(ClientApp::GetApp())->CurrentTurn();
#endif
}
