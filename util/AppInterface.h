#ifndef _AppInterface_h_
#define _AppInterface_h_

#include "Export.h"
#include "../universe/Universe.h"
#include "../network/Networking.h"

class EmpireManager;
class Empire;
class SupplyManager;
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
struct GalaxySetupData;

class FO_COMMON_API IApp {
protected:
    IApp();

public:
    IApp(const IApp&) = delete;

    IApp(IApp&&) = delete;

    virtual ~IApp();

    const IApp& operator=(const IApp&) = delete;

    IApp& operator=(IApp&&) = delete;

    /** Returns a IApp pointer to the singleton instance of the app. */
    static IApp* GetApp();

    //! Returns the ::Universe known to this application
    //!
    //! @return
    //! A constant reference to the single ::Universe instance representing the
    //! known universe of this application.
    virtual Universe& GetUniverse() = 0;

    /** Start parsing universe object types on a separate thread. */
    virtual void StartBackgroundParsing();

    /** Returns the set of known Empires for this application. */
    virtual EmpireManager& Empires() = 0;

    virtual Empire* GetEmpire(int id) = 0;

    virtual SupplyManager& GetSupplyManager() = 0;

    virtual std::shared_ptr<UniverseObject> GetUniverseObject(int object_id) = 0;

    /** Accessor for known objects of specified empire. */
    virtual ObjectMap& EmpireKnownObjects(int empire_id) = 0;

    virtual std::shared_ptr<UniverseObject> EmpireKnownObject(int object_id, int empire_id) = 0;

    virtual std::string GetVisibleObjectName(std::shared_ptr<const UniverseObject> object) = 0;

    //! Returns the current game turn
    //!
    //! @return The number representing the current game turn.
    virtual int CurrentTurn() const = 0;

    static int MAX_AI_PLAYERS(); ///<Maximum number of AIs

    /** Returns the galaxy setup data used for the current game */
    virtual const GalaxySetupData& GetGalaxySetupData() const = 0;

    /** Returns the networking client type for the given empire_id. */
    virtual Networking::ClientType GetEmpireClientType(int empire_id) const = 0;

    /** Returns the networking client type for the given player_id. */
    virtual Networking::ClientType GetPlayerClientType(int player_id) const = 0;

    virtual int EffectsProcessingThreads() const = 0;

protected:
    static IApp* s_app; ///< a IApp pointer to the singleton instance of the app

    // NormalExitException is used to break out of the run loop, without calling
    // terminate and failing to unroll the stack.
    class NormalExitException {};
};

/** Accessor for the App's empire manager */
inline EmpireManager& Empires()
{ return IApp::GetApp()->Empires(); }

/** Accessor for Empires */
inline Empire* GetEmpire(int id)
{ return IApp::GetApp()->GetEmpire(id); }

/** Accessor for the App's empire supply manager */
inline SupplyManager& GetSupplyManager()
{ return IApp::GetApp()->GetSupplyManager(); }

/** Accessor for the App's universe object */
inline Universe& GetUniverse()
{ return IApp::GetApp()->GetUniverse(); }

/** Accessor for the App's universe object */
inline std::shared_ptr<const Pathfinder> GetPathfinder()
{ return IApp::GetApp()->GetUniverse().GetPathfinder(); }

/** Accessor for all (on server) or all known (on client) objects ObjectMap */
inline ObjectMap& Objects() {
    static ObjectMap empty_objects;
    auto app = IApp::GetApp();
    return app ? app->GetUniverse().Objects() : empty_objects;
}

/** Accessor for known objects of specified empire. */
inline ObjectMap& EmpireKnownObjects(int empire_id)
{ return IApp::GetApp()->EmpireKnownObjects(empire_id); }

/** Returns the object name of the universe object. This can be apperant object
 * name, if the application isn't supposed to see the real object name. */
inline std::string GetVisibleObjectName(std::shared_ptr<const UniverseObject> object)
{ return IApp::GetApp()->GetVisibleObjectName(object); }

/** Returns current game turn.  This is >= 1 during a game, BEFORE_FIRST_TURN
  * during galaxy setup, or is INVALID_GAME_TURN at other times */
inline int CurrentTurn()
{ return IApp::GetApp()->CurrentTurn(); }

/** Returns the galaxy setup settings used in the current game. */
inline const GalaxySetupData& GetGalaxySetupData()
{ return IApp::GetApp()->GetGalaxySetupData(); }

inline Networking::ClientType GetEmpireClientType(int empire_id)
{ return IApp::GetApp()->GetEmpireClientType(empire_id); }

inline Networking::ClientType GetPlayerClientType(int player_id)
{ return IApp::GetApp()->GetPlayerClientType(player_id); }

inline int EffectsProcessingThreads()
{ return IApp::GetApp()->EffectsProcessingThreads(); }


// sentinel values returned by CurrentTurn().  Can't be an enum since
// CurrentGameTurn() needs to return an integer game turn number
FO_COMMON_API extern const int INVALID_GAME_TURN;     ///< returned by CurrentGameTurn if a game is not currently in progress or being set up.
FO_COMMON_API extern const int BEFORE_FIRST_TURN;     ///< returned by CurrentGameTurn if the galaxy is currently being set up
FO_COMMON_API extern const int IMPOSSIBLY_LARGE_TURN; ///< a number that's almost assuredly larger than any real turn number that might come up

#endif // _AppInterface_h_
