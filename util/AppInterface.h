// -*- C++ -*-
#ifndef _AppInterface_h_
#define _AppInterface_h_

#include "Export.h"
#include "../universe/Universe.h"

class EmpireManager;
class Universe;
class UniverseObject;
class ObjectMap;
class ResourceCenter;
class PopCenter;
class Planet;
class System;
class Ship;
class Fleet;
class Building;
class Field;

class FO_COMMON_API IApp {
public:
    virtual ~IApp();

    static IApp*      GetApp(); ///< returns a IApp pointer to the singleton instance of the app

    virtual Universe& GetUniverse() = 0;  ///< returns applications copy of Universe

    virtual EmpireManager& Empires() = 0; ///< returns the set of known Empires for this application

    virtual TemporaryPtr<UniverseObject> GetUniverseObject(int object_id) = 0;

    /** Accessor for known objects of specified empire. */
    virtual ObjectMap& EmpireKnownObjects(int empire_id) = 0;

    virtual TemporaryPtr<UniverseObject> EmpireKnownObject(int object_id, int empire_id) = 0;

    virtual std::string GetVisibleObjectName(TemporaryPtr<const UniverseObject> object) = 0;

    /** returns a universe object ID which can be used for new objects.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    virtual int GetNewObjectID() = 0;

    /** returns a design ID which can be used for a new design to uniquely identify it.
        Can return INVALID_OBJECT_ID if an ID cannot be created. */
    virtual int GetNewDesignID() = 0;

    virtual int CurrentTurn() const = 0;        ///< returns the current game turn

protected:
    IApp();

    static IApp* s_app; ///< a IApp pointer to the singleton instance of the app

private:
    const IApp& operator=(const IApp&); // disabled
    IApp(const IApp&); // disabled
};

/** Accessor for the App's empire manager */
inline EmpireManager& Empires()
{ return IApp::GetApp()->Empires(); }

/** Accessor for the App's universe object */
inline Universe& GetUniverse()
{ return IApp::GetApp()->GetUniverse(); }

/** Accessor for all (on server) or all known (on client) objects ObjectMap */
inline ObjectMap& Objects()
{ return IApp::GetApp()->GetUniverse().Objects(); }

/** Accessor for known objects of specified empire. */
inline ObjectMap& EmpireKnownObjects(int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id); }

/** Accessor for individual objects. */
inline TemporaryPtr<UniverseObject> GetUniverseObject(int object_id)
{ return IApp::GetApp()->GetUniverseObject(object_id); }

inline TemporaryPtr<UniverseObject> GetEmpireKnownObject(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObject(object_id, empire_id); }

inline TemporaryPtr<ResourceCenter> GetResourceCenter(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<ResourceCenter>(object_id); }

inline TemporaryPtr<ResourceCenter> GetEmpireKnownResourceCenter(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<ResourceCenter>(object_id); }

inline TemporaryPtr<PopCenter> GetPopCenter(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<PopCenter>(object_id); }

inline TemporaryPtr<PopCenter> GetEmpireKnownPopCenter(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<PopCenter>(object_id); }

inline TemporaryPtr<Planet> GetPlanet(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<Planet>(object_id); }

inline TemporaryPtr<Planet> GetEmpireKnownPlanet(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<Planet>(object_id); }

inline TemporaryPtr<System> GetSystem(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<System>(object_id); }

inline TemporaryPtr<System> GetEmpireKnownSystem(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<System>(object_id); }

inline TemporaryPtr<Field> GetField(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<Field>(object_id); }

inline TemporaryPtr<Field> GetEmpireKnownField(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<Field>(object_id); }

inline TemporaryPtr<Ship> GetShip(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<Ship>(object_id); }

inline TemporaryPtr<Ship> GetEmpireKnownShip(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<Ship>(object_id); }

inline TemporaryPtr<Fleet> GetFleet(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<Fleet>(object_id); }

inline TemporaryPtr<Fleet> GetEmpireKnownFleet(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<Fleet>(object_id); }

inline TemporaryPtr<Building> GetBuilding(int object_id)
{ return IApp::GetApp()->GetUniverse().Objects().Object<Building>(object_id); }

inline TemporaryPtr<Building> GetEmpireKnownBuilding(int object_id, int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id).Object<Building>(object_id); }

/** Returns the object name of the universe object. This can be apperant object
 * name, if the application isn't supposed to see the real object name. */
inline std::string GetVisibleObjectName(TemporaryPtr<const UniverseObject> object)
{ return IApp::GetApp()->GetVisibleObjectName(object); }

/** Returns a new object ID from the server */
inline int GetNewObjectID()
{ return IApp::GetApp()->GetNewObjectID(); }

/** Returns a new object ID from the server */
inline int GetNewDesignID()
{ return IApp::GetApp()->GetNewDesignID(); }

/** Returns current game turn.  This is >= 1 during a game, BEFORE_FIRST_TURN during galaxy setup, or is
    INVALID_GAME_TURN at other times */
inline int CurrentTurn()
{ return IApp::GetApp()->CurrentTurn(); }

// sentinel values returned by CurrentTurn().  Can't be an enum since CurrentGameTurn() needs to return an integer
// game turn number
FO_COMMON_API extern const int INVALID_GAME_TURN;     ///< returned by CurrentGameTurn if a game is not currently in progress or being set up.
FO_COMMON_API extern const int BEFORE_FIRST_TURN;     ///< returned by CurrentGameTurn if the galaxy is currently being set up
FO_COMMON_API extern const int IMPOSSIBLY_LARGE_TURN; ///< a number that's almost assuredly larger than any real turn number that might come up

#endif // _AppInterface_h_
