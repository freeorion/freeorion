#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_

#include "../ClientApp.h"
#include "../../util/Process.h"
#include "../../UI/ClientUI.h"
#include "../../util/OptionsDB.h"

#include <GG/SDL/SDLGUI.h>

#include <memory>
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
    class CleanQuit : public std::exception {};

    typedef boost::signals2::signal<void (bool)> FullscreenSwitchSignalType;
    typedef boost::signals2::signal<void ()>     RepositionWindowsSignalType;

    /** \name Structors */ //@{
    HumanClientApp(int width, int height, bool calculate_FPS, const std::string& name, int x, int y, bool fullscreen, bool fake_mode_change);
    virtual ~HumanClientApp();
    //@}

    /** \name Accessors */ //@{
    int EffectsProcessingThreads() const override;

    bool                SinglePlayerGame() const;   ///< returns true iff this game is a single-player game
    bool                CanSaveNow() const;         ///< returns true / false to indicate whether this client can currently safely initiate a game save
    int                 AutoTurnsLeft() const;      ///< returns number of turns left to execute automatically
    bool                HaveWindowFocus() const;    ///< as far as the HCA knows, does the game window have focus?
    //@}

    /** \name Mutators */ //@{
    /** Handle background events that need starting when the turn updates.
     */
    void HandleTurnUpdate() override;

    void StartTurn() override;

    void                SetSinglePlayerGame(bool sp = true);

    void                StartServer();                  ///< starts a server process on localhost
    void                FreeServer();                   ///< frees (relinquishes ownership and control of) any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void                NewSinglePlayerGame(bool quickstart = false);
    void                MultiPlayerGame();                              ///< shows multiplayer connection window, and then transitions to multiplayer lobby if connected
    void                StartMultiPlayerGameFromLobby();                ///< begins
    void                CancelMultiplayerGameFromLobby();               ///< cancels out of multiplayer game
    void                SaveGame(const std::string& filename);          ///< saves the current game; blocks until all save-related network traffic is resolved.
    /** Accepts acknowledgement that server has completed the save.*/
    void                SaveGameCompleted();
    void                StartGame();

    /** Check if the CombatLogManager has incomplete logs that need fetching and start fetching
        them from the server.
     */
    void                UpdateCombatLogManager();

    /** Update the logger in OptionsDB and the other processes if hosting. */
    void                ChangeLoggerThreshold(const std::string& option_name, LogLevel option_value);

    void                ResetToIntro();
    void                ExitApp();
    void                ResetClientData();
    void                LoadSinglePlayerGame(std::string filename = "");///< loads a single player game chosen by the user; returns true if a game was loaded, and false if the operation was cancelled
    void                RequestSavePreviews(const std::string& directory, PreviewInformation& previews); ///< Requests the savegame previews for choosing one.
    void                Autosave();                                     ///< autosaves the current game, iff autosaves are enabled and any turn number requirements are met
    std::string         SelectLoadFile();                               //< Lets the user select a multiplayer save to load.
    void                InitAutoTurns(int auto_turns);                  ///< Initialize auto turn counter
    void                DecAutoTurns(int n = 1);                        ///< Decrease auto turn counter

    ClientUI& GetClientUI()
    { return *m_ui.get(); }

    void                Reinitialize();

    float               GLVersion() const;

    void                HandleSaveGameDataRequest();
    void                UpdateCombatLogs(const Message& msg);

    void                OpenURL(const std::string& url);
    //@}

    mutable FullscreenSwitchSignalType  FullscreenSwitchSignal;
    mutable RepositionWindowsSignalType RepositionWindowsSignal;

    static std::pair<int, int>  GetWindowWidthHeight();
    static std::pair<int, int>  GetWindowLeftTop();

    static HumanClientApp*      GetApp();               ///< returns HumanClientApp pointer to the single instance of the app

    /** Adds window dimension options to OptionsDB after the start of main, but before HumanClientApp constructor.
        OSX will not tolerate static initialization of SDL, to check screen size. */
    static void AddWindowSizeOptionsAfterMainStart(OptionsDB& db);

protected:
    void Initialize() override;

private:
    void HandleSystemEvents() override;

    void RenderBegin() override;

    void            HandleMessage(Message& msg);

    void            HandleWindowMove(GG::X w, GG::Y h);
    void            HandleWindowResize(GG::X w, GG::Y h);
    void            HandleAppQuitting();
    void            HandleFocusChange(bool gained_focus);

    void            ConnectKeyboardAcceleratorSignals();///< installs the following 3 global hotkeys: quit, exit, togglefullscreen
    bool            HandleHotkeyResetGame();            ///< quit current game to IntroScreen
    bool            HandleHotkeyExitApp();              ///< quit current game & freeorion to Desktop
    bool            ToggleFullscreen();                 ///< toggle to/from fullscreen display

    void            UpdateFPSLimit();                   ///< polls options database to find if FPS should be limited, and if so, to what rate

    void            DisconnectedFromServer();           ///< called by ClientNetworking when the TCP connection to the server is lost

    /** If hosting then send the logger state to the server. */
    void            SendLoggingConfigToServer();

    void            ResetOrExitApp(bool reset);

    std::unique_ptr<HumanClientFSM> m_fsm;

    Process                     m_server_process;       ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player

    /** The only instance of the ClientUI. */
    std::unique_ptr<ClientUI> m_ui;

    bool                        m_single_player_game;   ///< true when this game is a single-player game
    bool                        m_game_started;         ///< true when a game is currently in progress
    bool                        m_connected;            ///< true if we are in a state in which we are supposed to be connected to the server
    int                         m_auto_turns;           ///< auto turn counter
    bool                        m_have_window_focus;

    bool                        m_save_game_in_progress;
    boost::signals2::signal<void ()> SaveGameCompletedSignal;
};

#endif // _HumanClientApp_h_
