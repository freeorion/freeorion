// -*- C++ -*-
#ifndef _Sound_h_
#define _Sound_h_

#ifdef FREEORION_MACOSX
# include <OpenAL/al.h>
#else
# include <AL/al.h>
#endif

#include <vorbis/vorbisfile.h>

#include <boost/filesystem/path.hpp>

#include <map>
#include <stack>


class Sound
{
public:
    /** Temporarily disables UI sound effects, saving the old state (on or off), for later restoration upon object
        destruction.  TempUISoundDisablers should be created at the beginning of any function in which Controls that
        emit sounds are to be programmatically altered, e.g. the ctor of a window class that contains a ListBox with an
        initially-selected item.  If this were not done, the list-select sound would be played when the window was
        constructed, which would make the sound seem to be malfunctioning. */
    struct TempUISoundDisabler
    {
        TempUISoundDisabler();
        ~TempUISoundDisabler();
    };

    /** Returns the singleton instance of Sound. */
    static Sound& GetSound();

    /** Plays a music file.  The file will be played in an infinitve loop if \a loop is < 0, and it will be played \a
        loops + 1 times otherwise. */
    void PlayMusic(const boost::filesystem::path& path, int loops = 0);

    /** Stops playing music. */
    void StopMusic();

    /** Plays a sound file. */
    void PlaySound(const boost::filesystem::path& path, bool is_ui_sound = false);

    /** Frees the cached sound data associated with the filename. */
    void FreeSound(const boost::filesystem::path& path);

    /** Frees all cached sound data. */
    void FreeAllSounds();

    /** Sets the music volume from 0 (muted) to 255 (full volume); \a vol is range-adjusted. */
    void SetMusicVolume(int vol);

    /** Sets the UI sounds volume from 0 (muted) to 255 (full volume); \a vol is range-adjusted. */
    void SetUISoundsVolume(int vol);

    /** Does the work that must be done by the sound system once per frame. */
    void DoFrame();

private:
    Sound();  ///< ctor.
    ~Sound(); ///< dotr.

    bool UISoundsTemporarilyDisabled() const;

    int RefillBuffer(ALuint *bufferName); ///< delegated here for simplicity - it's used by both PlayMusic and RenderBegin. Returns 1 if encounteres end of playback (no more music loops).

    static const int NUM_SOURCES = 16; // The number of sources for OpenAL to create. Should be 2 or more.

    ALuint                            m_sources[NUM_SOURCES]; ///< OpenAL sound sources. The first one is used for music
    int                               m_music_loops;          ///< the number of loops of the current music to play (< 0 for loop forever)
    std::string                       m_music_name;           ///< the name of the currently-playing music file
    std::map<std::string, ALuint>     m_buffers;              ///< the currently-cached (and possibly playing) sounds, if any; keyed on filename
    ALuint                            m_music_buffers[2];     ///< two additional buffers for music. statically defined as they'll be changed many times.
    OggVorbis_File                    m_ogg_file;             ///< the currently open ogg file
    ALenum                            m_ogg_format;           ///< mono or stereo
    ALsizei                           m_ogg_freq;             ///< sampling frequency
    std::stack<bool>                  m_UI_sounds_temporarily_disabled;

    friend struct TempUISoundDisabler;
};

#endif // _Sound_h_
