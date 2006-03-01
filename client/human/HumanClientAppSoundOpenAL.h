#include "HumanClientApp.h"

#include <AL/al.h>

class HumanClientAppSoundOpenAL : public HumanClientApp
{
public:
    HumanClientAppSoundOpenAL();
    virtual ~HumanClientAppSoundOpenAL();

    virtual void PlayMusic(const std::string& filename, int loops = 0);
    virtual void StopMusic();
    virtual void PlaySound(const std::string& filename);
    virtual void FreeSound(const std::string& filename);
    virtual void FreeAllSounds();
    virtual void SetMusicVolume(int vol);
    virtual void SetUISoundsVolume(int vol);

private:
    ALuint                           m_sources[16];         ///< OpenAL sound sources. The first one is used for music
    int                               m_music_loops;        ///< the number of loops of the current music to play (< 0 for loop forever)
    int                               m_next_music_time;    ///< the time in ms that the next loop of the current music should play (0 if no repeats are scheduled)
    std::string                       m_music_name;         ///< the name of the currently-playing music file
    std::map<std::string, ALuint>     m_buffers;             ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    std::map<std::string, int>        m_sounds;             ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    std::set<int>                     m_used_sample_indices;///< the used indices in m_sounds

};
