#ifndef _GodotClientApp_h_
#define _GodotClientApp_h_

#include "../ClientApp.h"

#include <queue>
#include <string>

#include "../../util/Process.h"

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

#ifndef FREEORION_ANDROID
    void NewSinglePlayerGame();
#endif

    static GodotClientApp* GetApp();
private:
#ifndef FREEORION_ANDROID
    /** Starts a server process on localhost.

        Throws a runtime_error if the server process can't be started.

        Throws LocalServerAlreadyRunningException (derived from runtime_error
        in HumanClientApp.cpp) if another server is already running. */
    void StartServer();

    /** Frees (relinquishes ownership and control of) any running server
      * process already started by this client; performs no cleanup of other
      * processes, such as AIs. */
    void FreeServer();

    Process m_server_process;   ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
#endif
    bool m_single_player_game = true;   ///< true when this game is a single-player game

    /** Filenames of all in progress saves.  There maybe multiple saves in
        progress if a player and an autosave are initiated at the same time. */
    std::queue<std::string>             m_game_saves_in_progress;
};

#endif

