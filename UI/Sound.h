#ifndef _Sound_h_
#define _Sound_h_

#include <boost/filesystem/path.hpp>

#include <memory>


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

    class InitializationFailureException : public std::runtime_error {
    public:
        explicit InitializationFailureException(const std::string& s) : std::runtime_error(s) {}
    };

    /** Returns the singleton instance of Sound.*/
    static Sound& GetSound();

    /** Plays a music file.  The file will be played in an infinitve loop if \a loop is < 0, and it will be played \a
        loops + 1 times otherwise. */
    void PlayMusic(const boost::filesystem::path& path, int loops = 0);

    /** Pauses music play, to be continued from the same position */
    void PauseMusic();

    /** Resumes music play */
    void ResumeMusic();

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

    /** Enables the sound system.  Throws runtime_error on failure. */
    void Enable();

    /** Disable the sound system. */
    void Disable();

private:
    class Impl;

    std::unique_ptr<Impl> const m_impl;

    Sound();

    ~Sound();
};

#endif // _Sound_h_
