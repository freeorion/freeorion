// -*- c++ -*-
#include "HumanClientApp.h"

struct FSOUND_STREAM;

class HumanClientAppSoundFMOD : public HumanClientApp
{
public:
    HumanClientAppSoundFMOD();
    virtual ~HumanClientAppSoundFMOD();
    virtual void PlayMusic(const boost::filesystem::path& path, int loops = 0);
    virtual void StopMusic();
    virtual void PlaySound(const boost::filesystem::path& path);
    virtual void FreeSound(const boost::filesystem::path& path);
    virtual void FreeAllSounds();
    virtual void SetMusicVolume(int vol);
    virtual void SetUISoundsVolume(int vol);
    

private:
    FSOUND_STREAM*                    m_current_music;      ///< the currently-playing music, if any
    int                               m_music_channel;      ///< the channel on which the currently-playing music is playing (or -1 if no music is playing)
    int                               m_music_loops;        ///< the number of loops of the current music to play (< 0 for loop forever)
    int                               m_next_music_time;    ///< the time in ms that the next loop of the current music should play (0 if no repeats are scheduled)
    std::string                       m_music_name;         ///< the name of the currently-playing music file
    std::map<std::string, int>        m_sounds;             ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    std::set<int>                     m_used_sample_indices;///< the used indices in m_sounds

    virtual void RenderBegin();
};


