#ifndef _GodotClientApp_h_
#define _GodotClientApp_h_

#include "../ClientApp.h"

#include <queue>
#include <string>

#ifndef FREEORION_ANDROID
#  include "../../util/Process.h"
#endif

class GodotClientApp : public ClientApp {
public:
    GodotClientApp();

    GodotClientApp(const GodotClientApp&) = delete;
    GodotClientApp(GodotClientApp&&) = delete;
    ~GodotClientApp() override;

    const GodotClientApp& operator=(const GodotClientApp&) = delete;
    GodotClientApp& operator=(const GodotClientApp&&) = delete;

    void StartParsingContent();
    int EffectsProcessingThreads() const override;
    bool SinglePlayerGame() const;  ///< returns true iff this game is a single-player game

    void SetSinglePlayerGame(bool sp = true);

    void NewSinglePlayerGame();

    int  AutoTurnsLeft() const;     ///< returns number of turns left to execute automatically
    void InitAutoTurns(int auto_turns); ///< Initialize auto turn counter
    void DecAutoTurns(int n = 1);       ///< Decrease auto turn counter

    /** \p is_new_game should be true for a new game and false for a loaded game. */
    void StartGame(bool is_new_game);

    static GodotClientApp* GetApp();
private:
    /** Starts a server process on localhost.

        Throws a runtime_error if the server process can't be started.

        Throws LocalServerAlreadyRunningException (derived from runtime_error
        in HumanClientApp.cpp) if another server is already running. */
    void StartServer();

    /** Frees (relinquishes ownership and control of) any running server
      * process already started by this client; performs no cleanup of other
      * processes, such as AIs. */
    void FreeServer();
#ifndef FREEORION_ANDROID
    Process m_server_process;   ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
#endif
    bool m_single_player_game = true;   ///< true when this game is a single-player game
    bool m_game_started = false;        ///< true when a game is currently in progress
    int  m_auto_turns = 0;              ///< auto turn counter

    /** Filenames of all in progress saves.  There maybe multiple saves in
        progress if a player and an autosave are initiated at the same time. */
    std::queue<std::string>             m_game_saves_in_progress;
};

#endif

