// -*- C++ -*-
#ifndef _AppInterface_h_
#define _AppInterface_h_

#ifndef _EmpireManager_h_
#include "../Empire/EmpireManager.h"
#endif

#ifndef _Universe_h_
#include "../universe/Universe.h"
#endif

#include <log4cpp/Category.hh>

/** Accessor for the App's empire manager */
EmpireManager& Empires();

/** Accessor for the App's universe object */
Universe& GetUniverse();
    
/** Accessor for the App's logger */
log4cpp::Category& Logger();

/** Returns a new object ID from the server */
int GetNewObjectID();

/** Returns current game turn.  This is >= 1 during a game, BEFORE_FIRST_TURN during galaxy setup, or is
    INVALID_GAME_TURN at other times */
int CurrentTurn();

// sentinel values returned by CurrentTurn().  Can't be an enum since CurrentGameTurn() needs to return an integer
// game turn number
extern const int INVALID_GAME_TURN;     ///< returned by CurrentGameTurn if a game is not currently in progress or being set up.
extern const int BEFORE_FIRST_TURN;     ///< returned by CurrentGameTurn if the galaxy is currently being set up
extern const int IMPOSSIBLY_LARGE_TURN; ///< a number that's almost assuredly larger than any real turn number that might come up

enum DifficultyLevel {
    INVALID_DIFFICULTY_LEVEL = -1,
    NOVICE,
    EASY,
    MEDIUM,
    HARD,
    IMPOSSIBLE,
    NUM_DIFFICULTY_LEVELS
};

DifficultyLevel CurrentDifficultyLevel();

/* add additional accessors here for app specific things
   that are needed for both the server and the client, but for which
   access will vary and requires an #ifdef */

inline std::string AppInterfaceRevision()
{return "$Id$";}

#endif // _AppInterface_h_
