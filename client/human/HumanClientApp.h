// -*- C++ -*-
#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_

#include "../ClientApp.h"
#include "../../util/Process.h"
#include "../../UI/ClientUI.h"

#include <GG/Ogre/OgreGUI.h>

#include <string>
#include <map>
#include <set>
#include <vector>


struct HumanClientFSM;
class MultiplayerLobbyWnd;
namespace Ogre {
    class Root;
    class RenderWindow;
    class SceneManager;
    class Camera;
    class Viewport;
}

/** the application framework class for the human player FreeOrion client. */
class HumanClientApp :
    public ClientApp,
    public GG::OgreGUI
{
public:
    class CleanQuit : public std::exception {};

    /** \name Structors */ //@{
    HumanClientApp(Ogre::Root* root,
                   Ogre::RenderWindow* window,
                   Ogre::SceneManager* scene_manager,
                   Ogre::Camera* camera,
                   Ogre::Viewport* viewport);
    virtual ~HumanClientApp();
    //@}

    /** \name Accessors */ //@{
    const std::string&  SaveFileName() const;       ///< returns the current game's filename (may be "")
    bool                SinglePlayerGame() const;   ///< returns true iff this game is a single-player game
    //@}

    /** \name Mutators */ //@{
    void                StartServer();                  ///< starts a server process on localhost
    void                FreeServer();                   ///< frees (relinquishes ownership and control of) any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void                KillServer();                   ///< kills any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void                NewSinglePlayerGame(bool quickstart = false);
    void                MulitplayerGame();
    void                SaveGame(const std::string& filename);          ///< saves the current game; blocks until all save-related network traffic is resolved.
    void                EndGame();                                      ///< kills the server (if appropriate) and ends the current game, leaving the application in its start state
    void                LoadSinglePlayerGame(std::string filename = ""); ///< loads a single player game chosen by the user; returns true if a game was loaded, and false if the operation was cancelled
    void                SetSaveFileName(const std::string& filename);   ///< records the current game's filename

    Ogre::SceneManager* SceneManager();
    Ogre::Camera*       Camera();
    Ogre::Viewport*     Viewport();

    float               GLVersion() const;

    virtual void        Enter2DMode();
    virtual void        Exit2DMode();
    virtual void        StartTurn();

    virtual void        Exit(int code);
    //@}

    static HumanClientApp*  GetApp();                   ///< returns HumanClientApp pointer to the single instance of the app

private:
    virtual void    HandleSystemEvents();
    virtual void    RenderBegin();

    void            HandleMessage(Message& msg);
    void            HandleSaveGameDataRequest();

    void            HandleWindowResize(GG::X w, GG::Y h);

    void            StartGame();
    void            Autosave(bool new_game);            ///< autosaves the current game, iff autosaves are enabled, and m_turns_since_autosave % autosaves.turns == 0
    void            EndGame(bool suppress_FSM_reset);
    void            UpdateFPSLimit();                   ///< polls options database to find if FPS should be limited, and if so, to what rate

    HumanClientFSM*             m_fsm;
    Process                     m_server_process;       ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
    boost::shared_ptr<ClientUI> m_ui;                   ///< the one and only ClientUI object!
    std::string                 m_save_filename;        ///< the name under which the current game has been saved
    bool                        m_single_player_game;   ///< true when this game is a single-player game
    bool                        m_game_started;         ///< true when a game is currently in progress
    int                         m_turns_since_autosave; ///< the number of turns that have elapsed since the last autosave
    bool                        m_in_save_game_cycle;   ///< true during SaveGame()'s send-request, receive-save-game-data-request, send-save-game-data cycle
    bool                        m_connected;            ///< true if we are in a state in which we are supposed to be connected to the server
    Ogre::Root*                 m_root;
    Ogre::SceneManager*         m_scene_manager;
    Ogre::Camera*               m_camera;
    Ogre::Viewport*             m_viewport;

    friend struct HumanClientFSM;
    friend struct IntroMenu;
    friend struct MPLobby;
    friend struct PlayingGame;
    friend struct WaitingForSPHostAck;
    friend struct WaitingForMPHostAck;
    friend struct WaitingForMPJoinAck;
    friend struct MPLobbyIdle;
    friend struct HostMPLobby;
    friend struct NonHostMPLobby;
    friend struct WaitingForTurnData;
    friend struct PlayingTurn;
    friend struct WaitingForTurnDataIdle;
    friend struct ResolvingCombat;
};

#endif // _HumanClientApp_h_
