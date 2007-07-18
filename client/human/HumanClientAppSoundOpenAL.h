// -*- C++ -*-
#include "HumanClientApp.h"

#include <AL/al.h>

#include <vorbis/vorbisfile.h>

class HumanClientAppSoundOpenAL : public HumanClientApp
{
public:
    HumanClientAppSoundOpenAL();
    virtual ~HumanClientAppSoundOpenAL();

    virtual void PlayMusic(const boost::filesystem::path& path, int loops = 0);
    virtual void StopMusic();
    virtual void PlaySound(const boost::filesystem::path& path);
    virtual void FreeSound(const boost::filesystem::path& path);
    virtual void FreeAllSounds();
    virtual void SetMusicVolume(int vol);
    virtual void SetUISoundsVolume(int vol);

private:
    static const int NUM_SOURCES = 16; // The number of sources for OpenAL to create. Should be 2 or more.

    ALuint                            m_sources[NUM_SOURCES]; ///< OpenAL sound sources. The first one is used for music
    int                               m_music_loops;          ///< the number of loops of the current music to play (< 0 for loop forever)
    std::string                       m_music_name;           ///< the name of the currently-playing music file
    std::map<std::string, ALuint>     m_buffers;              ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    ALuint                            m_music_buffers[2];     ///< two additional buffers for music. statically defined as they'll be changed many times.
    OggVorbis_File                    m_ogg_file;             ///< the currently open ogg file
    ALenum                            m_ogg_format;           ///< mono or stereo
    ALsizei                           m_ogg_freq;             ///< sampling frequency
   
    int RefillBuffer(ALuint *bufferName); ///< delegated here for simplicity - it's used by both PlayMusic and RenderBegin. Returns 1 if encounteres end of playback (no more music loops).
    virtual void RenderBegin();
};
