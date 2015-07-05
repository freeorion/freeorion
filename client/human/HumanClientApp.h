// -*- C++ -*-
#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_

#include "../ClientApp.h"
#include "../../util/Process.h"
#include "../../UI/ClientUI.h"

#include <GG/SDL/SDLGUI.h>

#include <string>
#include <map>
#include <set>
#include <vector>

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

    /** \name Structors */ //@{
    HumanClientApp(int width, int height, bool calculate_FPS, const std::string& name, int x, int y, bool fullscreen, bool fake_mode_change);
    virtual ~HumanClientApp();
    //@}

    /** \name Accessors */ //@{
    bool                SinglePlayerGame() const;   ///< returns true iff this game is a single-player game
    bool                CanSaveNow() const;         ///< returns true / false to indicate whether this client can currently safely initiate a game save
    int                 AutoTurnsLeft();            ///< returns number of turns left to execute automatically
    //@}

    /** \name Mutators */ //@{
    void                SetSinglePlayerGame(bool sp = true);

    void                StartServer();                  ///< starts a server process on localhost
    void                FreeServer();                   ///< frees (relinquishes ownership and control of) any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void                KillServer();                   ///< kills any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void                NewSinglePlayerGame(bool quickstart = false);
    void                MultiPlayerGame();                              ///< shows multiplayer connection window, and then transitions to multiplayer lobby if connected
    void                StartMultiPlayerGameFromLobby();                ///< begins
    void                CancelMultiplayerGameFromLobby();               ///< cancels out of multiplayer game
    void                SaveGame(const std::string& filename);          ///< saves the current game; blocks until all save-related network traffic is resolved.
    void                StartGame();
    void                EndGame(bool suppress_FSM_reset = false);       ///< kills the server (if appropriate) and ends the current game, leaving the application in its start state
    void                LoadSinglePlayerGame(std::string filename = "");///< loads a single player game chosen by the user; returns true if a game was loaded, and false if the operation was cancelled
    void                RequestSavePreviews(const std::string& directory, PreviewInformation& previews); ///< Requests the savegame previews for choosing one.
    void                Autosave();                                     ///< autosaves the current game, iff autosaves are enabled and any turn number requirements are met
    std::string         SelectLoadFile();                               //< Lets the user select a multiplayer save to load.
    std::string         SelectSaveFile();                               //< Lets the user select a multiplayer save to save to.
    void                InitAutoTurns(int auto_turns);                  ///< Initialize auto turn counter
    void                DecAutoTurns(int n = 1);                        ///< Decrease auto turn counter

    boost::shared_ptr<ClientUI>
                        GetClientUI() { return m_ui; }

    void                Reinitialize();

    float               GLVersion() const;

    virtual void        StartTurn();

    void                HandleSaveGameDataRequest();
    //@}

    static std::pair<int, int>  GetWindowWidthHeight();
    static std::pair<int, int>  GetWindowLeftTop();

    static HumanClientApp*      GetApp();               ///< returns HumanClientApp pointer to the single instance of the app

protected:
    virtual void Initialize();

private:
    virtual void    HandleSystemEvents();
    virtual void    RenderBegin();

    void            HandleMessage(Message& msg);

    void            HandleWindowMove(GG::X w, GG::Y h);
    void            HandleWindowResize(GG::X w, GG::Y h);
    void            HandleWindowClosing();
    void            HandleWindowClose();
    void            HandleFocusChange();

    void            ConnectKeyboardAcceleratorSignals();
    bool            QuitGame();
    bool            ExitGame();
    bool            ToggleFullscreen();

    void            UpdateFPSLimit();                   ///< polls options database to find if FPS should be limited, and if so, to what rate

    void            DisconnectedFromServer();           ///< called by ClientNetworking when the TCP connection to the server is lost


    HumanClientFSM*             m_fsm;
    Process                     m_server_process;       ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
    boost::shared_ptr<ClientUI> m_ui;                   ///< the one and only ClientUI object!
    bool                        m_single_player_game;   ///< true when this game is a single-player game
    bool                        m_game_started;         ///< true when a game is currently in progress
    bool                        m_connected;            ///< true if we are in a state in which we are supposed to be connected to the server
    int                         m_auto_turns;           ///< auto turn counter
};

#endif // _HumanClientApp_h_
