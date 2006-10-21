#include "HumanClientApp.h"

#include <al.h>
#include <vorbis/vorbisfile.h>

#define M_SOURCES_NUM 16 // the number of sources for OpenAL to create. Should be 2 or more
#define BUFFER_SIZE 40960 // the size of the buffer we read music data into.

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
    ALuint                            m_sources[M_SOURCES_NUM];         ///< OpenAL sound sources. The first one is used for music
    int                               m_music_loops;        ///< the number of loops of the current music to play (< 0 for loop forever)
    /// int                           m_next_music_time;    ///< the time in ms that the next loop of the current music should play (0 if no repeats are scheduled)
    std::string                       m_music_name;         ///< the name of the currently-playing music file
    std::map<std::string, ALuint>     m_buffers;            ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    ALuint                            m_music_buffers[2];   ///< two additional buffers for music. statically defined as they'll be changed many time.
    OggVorbis_File                    m_ogg_file;           ///< the currently open ogg file
    ALenum                            m_ogg_format;         ///< mono or stereo
    ALsizei                           m_ogg_freq;           ///< sampling frequency
   
    int RefillBuffer(ALuint *bufferName); ///< delegated here for simplicity - it's used by both PlayMusic and RenderBegin. Returns 1 if encounteres end of playback (no more music loops).
    virtual void RenderBegin();
};
