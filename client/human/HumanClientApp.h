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



#ifndef _MIXER_H_

#include "SDL_mixer.h"

#endif


#ifndef _ClientUI_h_

#include "../../UI/ClientUI.h"

#endif



#include <string>

#include <map>

#include <set>

#include <vector>



/** the application framework class for the human player FreeOrion client. */

class HumanClientApp : public ClientApp, public SDLGGApp

{

public:

   /** \name Structors */ //@{

   HumanClientApp(const GG::XMLElement& elem);

   virtual ~HumanClientApp();

   //@}



   /** \name Mutators */ //@{

   /** plays a music file.  The music will be played \a repeats + 1 times, so passing a 0 plays the song once (1 repeat).

      To loop the music indefinitely, pass -1 for \a repeats.  The \a ms parameter controls how long it takes for the 

      music to fade in (0 indicates no fade).  \a position indicates the position in the song at which playback should 

      begin for most (but not all) music formats, this is in seconds.  See the SDL_mixer docs for details.  This 

      function also ends any other music that might currently be playing.  Also note that the music data are freed at 

      the end of playback.*/

   void PlayMusic(const std::string& filename, int repeats, int ms = 0, double position = 0.0);

   

   /** plays a sound file.  The sound will be played \a repeats + 1 times, so passing a 0 plays the sound once (1 repeat).

      To loop the sound indefinitely, pass -1 for \a repeats.  \a timeout indicates a timeout for the playback, in ms.  

      So timeout == 1000 ensures that the sound plays for no more than 1000ms (1 sec); timeout == -1 means there is 

      no timeout.  The data for any sound file played with this function will be cached indefinitely.*/

   void PlaySound(const std::string& filename, int repeats, int timeout = -1);

   

   /** frees the cached sound data associated with the filename.  The data will only be freed immediately if the sound 

      is not playing. Otherwise, the data are freed when the last currently-running playback of the sound ends.  Note 

      that this means an infinitely-looping sound will never be freed without being explicitly interrupted.*/

   void FreeSound(const std::string& filename);

   

   /** frees all cached sound data.  The data for each sound will only be freed immediately if that sound 

      is not playing. Otherwise, the data are freed when the last currently-running playback of each sound ends.  Note 

      that this means an infinitely-looping sound will never be freed without being explicitly interrupted.*/

   void FreeAllSounds();

   

   virtual void Enter2DMode();

   virtual void Exit2DMode();

   //@}



   static HumanClientApp* GetApp(); ///< returns HumanClientApp pointer to the single instance of the app

   

private:

   virtual void SDLInit();

   virtual void GLInit();

   virtual void Initialize();



   virtual void HandleSDLEvent(const SDL_Event& event);



   virtual void Update();

   virtual void Render();



   virtual void FinalCleanup();

   virtual void SDLQuit();

   

   virtual void HandleMessageImpl(const Message& msg);

   

   Process                             m_server_process; ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player

   Mix_Music*                          m_current_music;  ///< the currently-playing music, if any

   std::map<std::string, Mix_Chunk*>   m_sounds;         ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename

   std::vector<std::string>            m_channels;       ///< the filenames playing on the various sound channels

   std::set<std::string>               m_sounds_to_free; ///< the filenames of sounds that should be freed, once they have finished playing
   
   boost::shared_ptr<ClientUI>         m_ui;             ///< the one and only ClientUI object!

   

   static void EndOfMusicCallback();

   static void EndOfSoundCallback(int channel);

};



#endif // _HumanClientApp_h_



