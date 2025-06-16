#ifndef _AppInterface_h_
#define _AppInterface_h_

#include "boost_fix.h"
#include "Export.h"
#include "../universe/Universe.h"
#include "../network/Networking.h"

class EmpireManager;
class Empire;
class SpeciesManager;
class Species;
class SupplyManager;
class Universe;
class UniverseObject;
class ObjectMap;
class Planet;
class System;
class Ship;
class Fleet;
class Building;
class Field;
class PythonParser;
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
    static IApp* GetApp() noexcept { return s_app; };

    //! Returns the ::Universe known to this application
    //!
    //! @return
    //! A constant reference to the single ::Universe instance representing the
    //! known universe of this application.
    virtual Universe& GetUniverse() noexcept = 0;

    /** Launches asynchronous parsing of game content, then starts
      * additional content parsing in the calling thread. \a barrier is
      * unblocked when the asynchronous parsing of named value refs is
      * completed, but the synchronous parsing of in the calling thread
      * or the other asynchronous parsing may still be ongoing
      * at that time.
      * Requires \a python to be initialized. */
    virtual void StartBackgroundParsing(const PythonParser& python, std::promise<void>&& barrier);

    [[nodiscard]] virtual ScriptingContext& GetContext() noexcept = 0;
    [[nodiscard]] virtual const ScriptingContext& GetContext() const noexcept = 0;

    /** Returns the set of known Empires for this application. */
    [[nodiscard]] virtual EmpireManager& Empires() noexcept = 0;

    [[nodiscard]] virtual Empire* GetEmpire(int id) = 0;

    [[nodiscard]] virtual SpeciesManager& GetSpeciesManager() noexcept = 0;

    [[nodiscard]] virtual SupplyManager& GetSupplyManager() noexcept = 0;

    [[nodiscard]] virtual std::string GetVisibleObjectName(const UniverseObject& object) = 0;

    /** On server or clients with no player: ALL_EMPIRES
      * On clients with a player: that player's ID */
    [[nodiscard]] virtual int EmpireID() const noexcept = 0;

    //! Returns the current game turn
    //!
    //! @return The number representing the current game turn.
    [[nodiscard]] virtual int CurrentTurn() const noexcept = 0;

    /** On server or AI clients, returns INVALID_OBJECT_ID
      * On UI clients, returns the ID of the object currently selected. */
    [[nodiscard]] virtual int SelectedSystemID() const { return INVALID_OBJECT_ID; }
    [[nodiscard]] virtual int SelectedPlanetID() const { return INVALID_OBJECT_ID; }
    [[nodiscard]] virtual int SelectedFleetID() const { return INVALID_OBJECT_ID; }
    [[nodiscard]] virtual int SelectedShipID() const { return INVALID_OBJECT_ID; }

    [[nodiscard]] static int MAX_AI_PLAYERS() noexcept;

    /** Returns the galaxy setup data used for the current game */
    [[nodiscard]] virtual const GalaxySetupData& GetGalaxySetupData() const noexcept = 0;

    /** Returns the networking client type for the given empire_id. */
    [[nodiscard]] virtual Networking::ClientType GetEmpireClientType(int empire_id) const = 0;

    /** Returns the networking client type for the given player_id. */
    [[nodiscard]] virtual Networking::ClientType GetPlayerClientType(int player_id) const = 0;

    [[nodiscard]] virtual int EffectsProcessingThreads() const = 0;

protected:
    static IApp* s_app; ///< a IApp pointer to the singleton instance of the app

    // NormalExitException is used to break out of the run loop, without calling
    // terminate and failing to unroll the stack.
    class NormalExitException {};
};

/** Accessor for the App's empire manager */
[[nodiscard]] inline EmpireManager& Empires() noexcept
{ return IApp::GetApp()->Empires(); }

/** Accessor for Empires */
[[nodiscard]] inline Empire* GetEmpire(int id)
{ return IApp::GetApp()->GetEmpire(id); }

/** Accessor for the App's species manager */
[[nodiscard]] inline SpeciesManager& GetSpeciesManager() noexcept
{ return IApp::GetApp()->GetSpeciesManager(); }

/** Accessor for the App's empire supply manager */
[[nodiscard]] inline SupplyManager& GetSupplyManager() noexcept
{ return IApp::GetApp()->GetSupplyManager(); }

/** Accessor for the App's universe object */
[[nodiscard]] inline Universe& GetUniverse() noexcept
{ return IApp::GetApp()->GetUniverse(); }

/** Accessor for all (on server) or all known (on client) objects ObjectMap */
[[nodiscard]] inline ObjectMap& Objects() {
    static ObjectMap empty_objects;
    auto app = IApp::GetApp();
    return app ? app->GetUniverse().Objects() : empty_objects;
}

/** Returns the object name of the universe object. This can be apperant object
 * name, if the application isn't supposed to see the real object name. */
[[nodiscard]] inline std::string GetVisibleObjectName(const UniverseObject& object)
{ return IApp::GetApp()->GetVisibleObjectName(object); }

/** Returns app's empire ID. This may be an actual empire ID or may be
  * ALL_EMPIRES. */
[[nodiscard]] inline int AppEmpireID() noexcept
{ return IApp::GetApp()->EmpireID(); }

/** Returns current game turn.  This is >= 1 during a game, BEFORE_FIRST_TURN
  * during galaxy setup, or is INVALID_GAME_TURN at other times */
[[nodiscard]] inline int CurrentTurn() noexcept
{ return IApp::GetApp()->CurrentTurn(); }

/** Returns the galaxy setup settings used in the current game. */
[[nodiscard]] inline const GalaxySetupData& GetGalaxySetupData() noexcept
{ return IApp::GetApp()->GetGalaxySetupData(); }

[[nodiscard]] inline Networking::ClientType GetEmpireClientType(int empire_id)
{ return IApp::GetApp()->GetEmpireClientType(empire_id); }

[[nodiscard]] inline Networking::ClientType GetPlayerClientType(int player_id)
{ return IApp::GetApp()->GetPlayerClientType(player_id); }

[[nodiscard]] inline int EffectsProcessingThreads()
{ return IApp::GetApp()->EffectsProcessingThreads(); }

#endif
