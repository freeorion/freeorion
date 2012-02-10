// -*- C++ -*-
#ifndef _AppInterface_h_
#define _AppInterface_h_

#include <log4cpp/Category.hh>

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

/** Accessor for the App's empire manager */
EmpireManager& Empires();

/** Accessor for the App's universe object */
Universe& GetUniverse();

/** Accessor for all (on server) or all known (on client) objects ObjectMap */
ObjectMap& Objects();

/** Accessor for individual objects.  These are all implemented as separate
  * functions to avoid needing template code in header, as the template code
  * also needs to have implementation-dependent #ifdef code in it which would
  * make the header code not buildable in a common library used by different
  * implementations (ie. server vs. clients). */
UniverseObject* GetUniverseObject(int object_id);
UniverseObject* GetEmpireKnownObject(int object_id, int empire_id);
Planet* GetPlanet(int object_id);
Planet* GetEmpireKnownPlanet(int object_id, int empire_id);
System* GetSystem(int object_id);
System* GetEmpireKnownSystem(int object_id, int empire_id);
Ship* GetShip(int object_id);
Ship* GetEmpireKnownShip(int object_id, int empire_id);
Fleet* GetFleet(int object_id);
Fleet* GetEmpireKnownFleet(int object_id, int empire_id);
Building* GetBuilding(int object_id);
Building* GetEmpireKnownBuilding(int object_id, int empire_id);

/** Accessor for the App's logger */
log4cpp::Category& Logger();

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

#endif // _AppInterface_h_
