#ifndef _GGHumanClientApp_h_
#define _GGHumanClientApp_h_

#include "../ClientApp.h"
#include "../../util/Process.h"
#include "../../UI/SDLGUI.h"
#include "../../UI/ClientUI.h"
#include "../../util/OptionsDB.h"
#include "HumanClientFSM.h"
#include <boost/statechart/event_base.hpp>
#include <memory>
#include <mutex>
#include <queue>
#include <string>


class MultiPlayerLobbyWnd;
struct PreviewInformation;

/** the application framework class for the human player FreeOrion client. */
class GGHumanClientApp final :
    public ClientApp,
    public SDLGUI
{
public:
    using FullscreenSwitchSignalType = boost::signals2::signal<void (bool)>;
    using RepositionWindowsSignalType = boost::signals2::signal<void ()>;

    GGHumanClientApp() = delete;

    explicit GGHumanClientApp(std::string name);

    GGHumanClientApp(GG::X width, GG::Y height, std::string name, GG::X x, GG::Y y,
                     bool fullscreen, bool fake_mode_change);

    GGHumanClientApp(const GGHumanClientApp&) = delete;
    GGHumanClientApp(GGHumanClientApp&&) = delete;
    ~GGHumanClientApp() override;

    const GGHumanClientApp& operator=(const GGHumanClientApp&) = delete;
    GGHumanClientApp& operator=(const GGHumanClientApp&&) = delete;

    static void InitLogging();

    [[nodiscard]] int SelectedSystemID() const override;
    [[nodiscard]] int SelectedPlanetID() const override;
    [[nodiscard]] int SelectedFleetID() const override;
    [[nodiscard]] int SelectedShipID() const override;

    int EffectsProcessingThreads() const override;
    bool SinglePlayerGame() const;  ///< returns true iff this game is a single-player game
    bool CanSaveNow() const;        ///< returns true / false to indicate whether this client can currently safely initiate a game save
    int  AutoTurnsLeft() const;     ///< returns number of turns left to execute automatically
    bool HaveWindowFocus() const;   ///< as far as the HCA knows, does the game window have focus?

    void StartTurn(const SaveGameUIData& ui_data) override;
    void UnreadyTurn();             ///< Revoke ready state of turn orders.

    /** \brief Handle UI and state updates with changes in turn phase. */
    void HandleTurnPhaseUpdate(Message::TurnProgressPhase phase_id) override;
    void SetSinglePlayerGame(bool sp = true);
    void NewSinglePlayerGame(bool quickstart = false);
    void MultiPlayerGame();                     ///< shows multiplayer connection window, and then transitions to multiplayer lobby if connected
    void StartMultiPlayerGameFromLobby();       ///< begins
    void CancelMultiplayerGameFromLobby();      ///< cancels out of multiplayer game
    void SaveGame(const std::string& filename); ///< saves the current game; blocks until all save-related network traffic is resolved

    void SaveGameCompleted();                   ///< Accepts acknowledgement that server has completed the save

    /** \p is_new_game should be true for a new game and false for a loaded game. */
    void StartGame(bool is_new_game);

    /** Check if the CombatLogManager has incomplete logs that need fetching and
      * start fetching them from the server. */
    void UpdateCombatLogManager();

    /** Update the logger in OptionsDB and the other processes if hosting. */
    void ChangeLoggerThreshold(const std::string& option_name, LogLevel option_value);

    void ResetToIntro(bool skip_savegame);

    /** Exit the App with \p exit_code. */
    void ExitApp(int exit_code = 0) override;

    void ResetClientData(bool save_connection = false);
    void LoadSinglePlayerGame(std::string filename = "");

    /** Requests the savegame previews from the server in \p relative_directory
      * to the server save directory. */
    void RequestSavePreviews(const std::string& relative_directory);
    void Autosave();                    ///< Autosaves the current game, iff autosaves are enabled and any turn number requirements are met
    void ContinueSinglePlayerGame();    ///< Load the newest single player autosave and continue playing game
    [[nodiscard]] bool IsLoadGameAvailable() const;
    [[nodiscard]] std::string SelectLoadFile(); ///< Lets the user select a multiplayer save to load
    void InitAutoTurns(int auto_turns); ///< Initialize auto turn counter
    void DecAutoTurns(int n = 1);       ///< Decrease auto turn counter
    void EliminateSelf();               ///< Resign from the game

    [[nodiscard]] ClientUI& GetUI() noexcept { return m_ui; }
    [[nodiscard]] const ClientUI& GetUI() const noexcept { return m_ui; }

    void Reinitialize();
    [[nodiscard]] float GLVersion() const;

    void UpdateCombatLogs(const Message& msg);
    /** Update any open SaveGameDialog with previews from the server. */
    void HandleSaveGamePreviews(const Message& msg);

    /** Update this client's authorization roles */
    void HandleSetAuthRoles(const Message& msg);

    void OpenURL(const std::string& url);
    /** Opens the users preferred application for file manager at the specified path @p browse_path */
    void BrowsePath(const std::filesystem::path& browse_path);

    mutable FullscreenSwitchSignalType  FullscreenSwitchSignal;
    mutable RepositionWindowsSignalType RepositionWindowsSignal;

    /** Adds window dimension options to OptionsDB after the start of main, but before GGHumanClientApp constructor.
        OSX will not tolerate static initialization of SDL, to check screen size. */
    static void AddWindowSizeOptionsAfterMainStart(OptionsDB& db);

    /** Converts server address to correct option name */
    [[nodiscard]] static std::string EncodeServerAddressOption(const std::string& server);

    /** If hosting then send the logger state to the server. */
    void SendLoggingConfigToServer();

    [[nodiscard]] boost::intrusive_ptr<const boost::statechart::event_base> GetDeferredPostedEvent();
    void PostDeferredEvent(boost::intrusive_ptr<const boost::statechart::event_base> event);

    void Initialize() override;

private:
    struct AppParams {
        GG::X width;
        GG::Y height;
        GG::X left;
        GG::Y top;
        bool fullscreen;
        bool fake_mode_change;
    };
    static AppParams DefaultAppParams();
    GGHumanClientApp(std::string name, AppParams params);

    /** Starts a server process on localhost.

        Throws a runtime_error if the server process can't be started.

        Throws LocalServerAlreadyRunningException (derived from runtime_error
        in GGHumanClientApp.cpp) if another server is already running. */
    void StartServer();

    /** Frees (relinquishes ownership and control of) any running server
      * process already started by this client; performs no cleanup of other
      * processes, such as AIs. */
    void FreeServer();


    std::mutex m_event_queue_guard;
    using event_ptr_t = boost::intrusive_ptr<const boost::statechart::event_base>;
    std::queue<event_ptr_t> m_posted_event_queue;

    void HandleSystemEvents() override;
    void RenderBegin() override;
    void HandleMessage(Message&& msg);
    void HandleWindowMove(GG::X w, GG::Y h);
    void HandleWindowResize(GG::X w, GG::Y h);
    void HandleAppQuitting();
    void HandleFocusChange(bool gained_focus);
    void ConnectKeyboardAcceleratorSignals();   ///< installs the following 3 global hotkeys: quit, exit, togglefullscreen
    bool HandleHotkeyResetGame();               ///< quit current game to IntroScreen
    bool HandleHotkeyExitApp();                 ///< quit current game & freeorion to Desktop
    bool ToggleFullscreen();                    ///< toggle to/from fullscreen display
    void UpdateFPSLimit();                      ///< polls options database to find if FPS should be limited, and if so, to what rate

    /** If a game is not running re-parse the universe otherwise inform the
        player changes will effect new games. */
    void HandleResoureDirChange();

    void DisconnectedFromServer();  ///< called by ClientNetworking when the TCP connection to the server is lost

    /** ExitSDL is bound by ResetOrExitApp() to exit the application. */
    void ExitSDL(int exit_code);

    /** Either reset to IntroMenu (\p reset is true), or exit the
        application.  If \p skip_savegame is true abort in progress save
        games. \p exit_code is the exit code. */
    void ResetOrExitApp(bool reset, bool skip_savegame, int exit_code = 0);

    HumanClientFSM m_fsm;
    Process        m_server_process;   ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
    ClientUI       m_ui;

    bool m_single_player_game = true;   ///< true when this game is a single-player game
    bool m_game_started = false;        ///< true when a game is currently in progress
    bool m_exit_handled = false;        ///< true when the exit logic is already being handled
    bool m_connected = false;           ///< true if we are in a state in which we are supposed to be connected to the server
    bool m_have_window_focus = true;
    int  m_auto_turns = 0;              ///< auto turn counter

    /** Filenames of all in progress saves.  There maybe multiple saves in
        progress if a player and an autosave are initiated at the same time. */
    std::queue<std::string>             m_game_saves_in_progress;
    boost::signals2::signal<void ()>    SaveGamesCompletedSignal;
};

[[nodiscard]] GGHumanClientApp& GetApp();

#endif
