#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_

#include "../ClientApp.h"
#include "../../util/Process.h"
#include "../../UI/ClientUI.h"
#include "../../util/OptionsDB.h"

#include <GG/SDL/SDLGUI.h>

#include <memory>
#include <queue>
#include <string>

struct HumanClientFSM;
class MultiPlayerLobbyWnd;
struct PreviewInformation;

/** the application framework class for the human player FreeOrion client. */
class HumanClientApp :
    public ClientApp,
    public GG::SDLGUI
{
public:
    typedef boost::signals2::signal<void (bool)> FullscreenSwitchSignalType;
    typedef boost::signals2::signal<void ()>     RepositionWindowsSignalType;

    HumanClientApp() = delete;

    HumanClientApp(int width, int height, bool calculate_FPS,
                   const std::string& name, int x, int y,
                   bool fullscreen, bool fake_mode_change);

    HumanClientApp(const HumanClientApp&) = delete;
    HumanClientApp(HumanClientApp&&) = delete;
    ~HumanClientApp() override;

    const HumanClientApp& operator=(const HumanClientApp&) = delete;
    HumanClientApp& operator=(const HumanClientApp&&) = delete;

    /** \name Accessors */ //@{
    int EffectsProcessingThreads() const override;
    bool SinglePlayerGame() const;  ///< returns true iff this game is a single-player game
    bool CanSaveNow() const;        ///< returns true / false to indicate whether this client can currently safely initiate a game save
    int  AutoTurnsLeft() const;     ///< returns number of turns left to execute automatically
    bool HaveWindowFocus() const;   ///< as far as the HCA knows, does the game window have focus?
    //@}

    /** \name Mutators */ //@{
    void HandleTurnUpdate() override;           ///< Handle background events that need starting when the turn updates
    void StartTurn(const SaveGameUIData& ui_data) override;
    void UnreadyTurn();                         ///< Revoke ready state of turn orders.

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
    bool IsLoadGameAvailable() const;
    std::string SelectLoadFile();       ///< Lets the user select a multiplayer save to load
    void InitAutoTurns(int auto_turns); ///< Initialize auto turn counter
    void DecAutoTurns(int n = 1);       ///< Decrease auto turn counter
    void EliminateSelf();               ///< Resign from the game

    ClientUI& GetClientUI()
    { return *m_ui.get(); }

    void Reinitialize();
    float GLVersion() const;

    void UpdateCombatLogs(const Message& msg);
    /** Update any open SaveGameDialog with previews from the server. */
    void HandleSaveGamePreviews(const Message& msg);

    /** Update this client's authorization roles */
    void HandleSetAuthRoles(const Message& msg);

    void OpenURL(const std::string& url);
    /** Opens the users preferred application for file manager at the specified path @p browse_path */
    void BrowsePath(const boost::filesystem::path& browse_path);
    //@}

    mutable FullscreenSwitchSignalType  FullscreenSwitchSignal;
    mutable RepositionWindowsSignalType RepositionWindowsSignal;

    static std::pair<int, int>  GetWindowWidthHeight();
    static std::pair<int, int>  GetWindowLeftTop();

    static HumanClientApp*      GetApp();               ///< returns HumanClientApp pointer to the single instance of the app

    /** Adds window dimension options to OptionsDB after the start of main, but before HumanClientApp constructor.
        OSX will not tolerate static initialization of SDL, to check screen size. */
    static void AddWindowSizeOptionsAfterMainStart(OptionsDB& db);

    /** If hosting then send the logger state to the server. */
    void            SendLoggingConfigToServer();

protected:
    void Initialize() override;

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

    void HandleSystemEvents() override;
    void RenderBegin() override;
    void HandleMessage(Message& msg);
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

    std::unique_ptr<HumanClientFSM> m_fsm;

    Process m_server_process;   ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player

    /** The only instance of the ClientUI. */
    std::unique_ptr<ClientUI> m_ui;

    bool m_single_player_game = true;   ///< true when this game is a single-player game
    bool m_game_started = false;        ///< true when a game is currently in progress
    bool m_connected = false;           ///< true if we are in a state in which we are supposed to be connected to the server
    int  m_auto_turns = 0;              ///< auto turn counter
    bool m_have_window_focus = true;

    /** Filenames of all in progress saves.  There maybe multiple saves in
        progress if a player and an autosave are initiated at the same time. */
    std::queue<std::string>             m_game_saves_in_progress;
    boost::signals2::signal<void ()>    SaveGamesCompletedSignal;
};

#endif // _HumanClientApp_h_
