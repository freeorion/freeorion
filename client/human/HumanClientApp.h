// -*- C++ -*-
#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_


#ifndef _SDLGGApp_h_
#include "SDLGGApp.h"
#endif

#ifndef _ClientApp_h_
#include "../ClientApp.h"
#endif

#ifndef _Process_h_
#include "../../util/Process.h"
#endif

#ifndef _ClientUI_h_
#include "../../UI/ClientUI.h"
#endif

#include <string>
#include <map>
#include <set>
#include <vector>

class MultiplayerLobbyWnd;
struct FSOUND_STREAM;

/** the application framework class for the human player FreeOrion client. */
class HumanClientApp : public ClientApp, public SDLGGApp
{
public:
    /** \name Structors */ //@{
    HumanClientApp();
    virtual ~HumanClientApp();
    //@}

    /** \name Accessors */ //@{
    const std::string&  SaveFileName() const     {return m_save_filename;} ///< returns the current game's filename (may be "")
    bool                SinglePlayerGame() const {return m_single_player_game;} ///< returns true iff this game is a single-player game

    virtual Message     TurnOrdersMessage(bool save_game_data = false) const;

    /** Returns a map from Planet IDs to pending (issued earlier this turn and undo-able) colonization order IDs. */
    std::map<int, int> PendingColonizationOrders() const;
    //@}

    /** \name Mutators */ //@{
    void StartServer(); ///< starts a server process on localhost
    void FreeServer();  ///< frees (relinquishes ownership and control of) any running server process already started by this client; performs no cleanup of other processes, such as AIs
    void KillServer();  ///< kills any running server process already started by this client; performs no cleanup of other processes, such as AIs

    void EndGame();     ///< kills the server (if appropriate) and ends the current game, leaving the application in its start state

    void SetLobby(MultiplayerLobbyWnd* lobby); ///< registers a lobby dialog so that Messages can reach it; passing 0 unsets the lobby dialog

    /** plays a music file.  The file will be played in an infinitve loop if \a loop is < 0, and it will be played \a loops + 1 times otherwise. */
    void PlayMusic(const std::string& filename, int loops = 0);

	/** stops playing music */
	void StopMusic();

    /** plays a sound file.*/
    void PlaySound(const std::string& filename);

    /** frees the cached sound data associated with the filename.*/
    void FreeSound(const std::string& filename);

    /** frees all cached sound data.*/
    void FreeAllSounds();

    /** sets the music volume from 0 (muted) to 255 (full volume); \a vol is range-adjusted */
    void SetMusicVolume(int vol);

    /** sets the UI sounds volume from 0 (muted) to 255 (full volume); \a vol is range-adjusted */
    void SetUISoundsVolume(int vol);

    bool LoadSinglePlayerGame(); ///< loads a single player game chosen by the user; returns true if a game was loaded, and false if the operation was cancelled
    void SetSaveFileName(const std::string& filename) {m_save_filename = filename;} ///< records the current game's filename

    virtual void Enter2DMode();
    virtual void Exit2DMode();
    //@}

    static HumanClientApp* GetApp(); ///< returns HumanClientApp pointer to the single instance of the app

    /// override default so that UI can be updated
    virtual void         StartTurn();   ///< encodes order sets and sends turn orders message

    ///< loads the requested texture from file \a name; mipmap textures are generated if \a mipmap is true
    ///< load default missing.png if name isn't found
    boost::shared_ptr<GG::Texture> GetTextureOrDefault(const std::string& name, bool mipmap = false); 

private:
    virtual void SDLInit();
    virtual void GLInit();
    virtual void Initialize();

    virtual void HandleNonGGEvent(const SDL_Event& event);

    virtual void RenderBegin();

    virtual void FinalCleanup();
    virtual void SDLQuit();

    virtual void HandleMessageImpl(const Message& msg);
    virtual void HandleServerDisconnectImpl();

    void Autosave(int turn_number, bool new_game); ///< autosaves the current game, iff autosaves are enabled, and m_turns_since_autosave % autosaves.turns == 0

    Process                           m_server_process;     ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
    FSOUND_STREAM*                    m_current_music;      ///< the currently-playing music, if any
    int                               m_music_channel;      ///< the channel on which the currently-playing music is playing (or -1 if no music is playing)
    int                               m_music_loops;        ///< the number of loops of the current music to play (< 0 for loop forever)
    int                               m_next_music_time;    ///< the time in ms that the next loop of the current music should play (0 if no repeats are scheduled)
    std::string                       m_music_name;         ///< the name of the currently-playing music file
    std::map<std::string, int>        m_sounds;             ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    std::set<int>                     m_used_sample_indices;///< the used indices in m_sounds
    boost::shared_ptr<ClientUI>       m_ui;                 ///< the one and only ClientUI object!
    std::string                       m_save_filename;      ///< the name under which the current game has been saved
    bool                              m_single_player_game; ///< true when this game is a single-player game
    bool                              m_game_started;       ///< true when a game is currently in progress
    int                               m_turns_since_autosave; ///< the number of turns that have elapsed since the last autosave
};

inline std::pair<std::string, std::string> HumanClientAppRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _HumanClientApp_h_
