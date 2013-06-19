
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
#include "../universe/Field.h"

#include "Logger.h"
#include "OptionsDB.h"

#include <boost/timer.hpp>
#include <string>

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

ObjectMap& Objects()
{ return GetUniverse().Objects(); }

ObjectMap& EmpireKnownObjects(int empire_id) {
#ifdef FREEORION_BUILD_SERVER
    return GetUniverse().EmpireKnownObjects(empire_id);
#else
    int client_empire_id = ClientApp::GetApp()->EmpireID();
    if (empire_id == ALL_EMPIRES || empire_id == client_empire_id)
        return Objects();
    return GetUniverse().EmpireKnownObjects(empire_id); // should be empty as of this writing, as other empires' known objects aren't sent to clients
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
        obj = EmpireKnownObjects(ClientApp::GetApp()->EmpireID()).Object(object_id);
    return obj;
#endif
}

UniverseObject* GetEmpireKnownObject(int object_id, int empire_id) {
#ifdef FREEORION_BUILD_SERVER
    return EmpireKnownObjects(empire_id).Object(object_id);
#else
    return GetUniverseObject(object_id);// as of this writing, players don't have info about what other players know about objects
#endif
}

std::string GetVisibleObjectName(const UniverseObject* object)
{
    if(NULL == object)
    {
        Logger().errorStream() << "GetVisibleObjectName: expected non null object pointer.";
        return std::string();
    }

#ifdef FREEORION_BUILD_SERVER
    std::string name_text = object->Name();
#else
    int client_empire_id = ClientApp::GetApp()->EmpireID();
    std::string name_text = object->PublicName(client_empire_id);
    if (const System* system = universe_object_cast<const System*>(object))
        name_text = system->ApparentName(client_empire_id);
#endif
    return name_text;
}

template <class T>
T* GetUniverseObject(int object_id)
{
    return Objects().Object<T>(object_id);
}

template <class T>
T* GetEmpireKnownObject(int object_id, int empire_id)
{
    return EmpireKnownObjects(empire_id).Object<T>(object_id);
}

Planet* GetPlanet(int object_id)
{ return GetUniverseObject<Planet>(object_id); }

Planet* GetEmpireKnownPlanet(int object_id, int empire_id)
{ return GetEmpireKnownObject<Planet>(object_id, empire_id); }

System* GetSystem(int object_id)
{ return GetUniverseObject<System>(object_id); }

System* GetEmpireKnownSystem(int object_id, int empire_id)
{ return GetEmpireKnownObject<System>(object_id, empire_id); }

Field* GetField(int object_id)
{ return GetUniverseObject<Field>(object_id); }

Field* GetEmpireKnownField(int object_id, int empire_id)
{ return GetEmpireKnownObject<Field>(object_id, empire_id); }

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
