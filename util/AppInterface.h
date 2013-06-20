// -*- C++ -*-
#ifndef _AppInterface_h_
#define _AppInterface_h_

#include <string>

#if defined(_MSC_VER) && defined(int64_t)
#undef int64_t
#endif

class EmpireManager;
class Universe;
class UniverseObject;
class ObjectMap;
class Planet;
class System;
class Ship;
class Fleet;
class Building;
class Field;

/** Accessor for the App's empire manager */
EmpireManager& Empires();

/** Accessor for the App's universe object */
Universe& GetUniverse();

/** Accessor for all (on server) or all known (on client) objects ObjectMap */
ObjectMap& Objects();

/** Accessor for known objects of specified empire. */
ObjectMap& EmpireKnownObjects(int empire_id);

/** Accessor for individual objects. */
UniverseObject* GetUniverseObject(int object_id);
UniverseObject* GetEmpireKnownObject(int object_id, int empire_id);
Planet* GetPlanet(int object_id);
Planet* GetEmpireKnownPlanet(int object_id, int empire_id);
System* GetSystem(int object_id);
System* GetEmpireKnownSystem(int object_id, int empire_id);
Field* GetField(int object_id);
Field* GetEmpireKnownField(int object_id, int empire_id);
Ship* GetShip(int object_id);
Ship* GetEmpireKnownShip(int object_id, int empire_id);
Fleet* GetFleet(int object_id);
Fleet* GetEmpireKnownFleet(int object_id, int empire_id);
Building* GetBuilding(int object_id);
Building* GetEmpireKnownBuilding(int object_id, int empire_id);

/** Returns the object name of the universe object. This can be apperant object
 * name, if the application isn't supposed to see the real object name. */
std::string GetVisibleObjectName(const UniverseObject* object);

/** Returns a new object ID from the server */
int GetNewObjectID();

/** Returns a new object ID from the server */
int GetNewDesignID();

/** Returns current game turn.  This is >= 1 during a game, BEFORE_FIRST_TURN during galaxy setup, or is
    INVALID_GAME_TURN at other times */
int CurrentTurn();

// sentinel values returned by CurrentTurn().  Can't be an enum since CurrentGameTurn() needs to return an integer
// game turn number
extern const int INVALID_GAME_TURN;     ///< returned by CurrentGameTurn if a game is not currently in progress or being set up.
extern const int BEFORE_FIRST_TURN;     ///< returned by CurrentGameTurn if the galaxy is currently being set up
extern const int IMPOSSIBLY_LARGE_TURN; ///< a number that's almost assuredly larger than any real turn number that might come up

/* add additional accessors here for app specific things
   that are needed for both the server and the client, but for which
   access will vary and requires an #ifdef */

class IApp {
public:
    virtual ~IApp();

    static IApp*      GetApp(); ///< returns a IApp pointer to the singleton instance of the app

    virtual Universe& GetUniverse() = 0;  ///< returns applications copy of Universe

    virtual EmpireManager& Empires() = 0; ///< returns the set of known Empires for this application

    virtual UniverseObject* GetUniverseObject(int object_id) = 0;

    /** Accessor for known objects of specified empire. */
    virtual ObjectMap& EmpireKnownObjects(int empire_id) = 0;

    virtual UniverseObject* EmpireKnownObject(int object_id, int empire_id) = 0;

    virtual std::string GetVisibleObjectName(const UniverseObject* object) = 0;

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

#endif // _AppInterface_h_
