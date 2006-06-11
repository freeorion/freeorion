#include "HumanClientAppSoundFMOD.h"
#include "../../util/OptionsDB.h"

#include <fmod.h>
#include <fmod_errors.h>

namespace {
    enum SoundDriver {
        SOUND_DRIVER_NOSOUND = FSOUND_OUTPUT_NOSOUND,
        SOUND_DRIVER_DIRECT_SOUND = FSOUND_OUTPUT_DSOUND,
        SOUND_DRIVER_WINDOWS_MULTIMEDIA_WAVEOUT = FSOUND_OUTPUT_WINMM,
        SOUND_DRIVER_ASIO = FSOUND_OUTPUT_ASIO,
        SOUND_DRIVER_OSS = FSOUND_OUTPUT_OSS,
        SOUND_DRIVER_ESD = FSOUND_OUTPUT_ESD,
        SOUND_DRIVER_ALSA = FSOUND_OUTPUT_ALSA
    };

    /** Assumes that sound has not been initialized! */
    std::vector<SoundDriver> GetSoundDrivers()
    {
        std::vector<SoundDriver> retval;
        
#if defined(FREEORION_WIN32)
        if (FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_DIRECT_SOUND);
        if (FSOUND_SetOutput(FSOUND_OUTPUT_WINMM) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_WINDOWS_MULTIMEDIA_WAVEOUT);
        if (FSOUND_SetOutput(FSOUND_OUTPUT_ASIO) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_ASIO);
#elif defined(FREEORION_LINUX)
        if (FSOUND_SetOutput(FSOUND_OUTPUT_ALSA) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_ALSA);
        if (FSOUND_SetOutput(FSOUND_OUTPUT_OSS) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_OSS);
        if (FSOUND_SetOutput(FSOUND_OUTPUT_ESD) && FSOUND_GetNumDrivers())
            retval.push_back(SOUND_DRIVER_ESD);
#endif

        FSOUND_SetOutput(FSOUND_OUTPUT_NOSOUND);

        return retval;
    }

    void InitFMOD(unsigned int channels = 64, unsigned int memory_size = 4*1024*1024)
    {
        const int SAMPLE_FREQ = 44100;

        assert(0 < channels);
        assert((memory_size % 512) == 0);

        log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();

        if (FSOUND_GetVersion() < FMOD_VERSION) {
            logger.errorStream() << "InitFMOD() : You are using the wrong version of the FMOD DLL!  This program was built with FMOD " << FMOD_VERSION;
        }

        if (!FSOUND_SetMemorySystem(malloc(memory_size), memory_size, 0, 0, 0)) {
            logger.errorStream() << "InitFMOD() : Error initializing FMOD memory; FMOD error was \"" << FMOD_ErrorString(FSOUND_GetError()) << "\"";
        }

        FSOUND_SetOutput(-1);
        FSOUND_SetDriver(0);
        SoundDriver default_sound_driver = static_cast<SoundDriver>(FSOUND_GetOutput());
        bool success = true;
        if (!FSOUND_Init(SAMPLE_FREQ, channels, FSOUND_INIT_USEDEFAULTMIDISYNTH)) {
            success = false;
            // first chance: try all other devices for this output driver
            int num_devices = FSOUND_GetNumDrivers();
            for (int i = 1; i < num_devices; ++i) { // start at 1, since 0 obviously didn't work
                FSOUND_SetDriver(i);
                if (FSOUND_Init(SAMPLE_FREQ, channels, FSOUND_INIT_USEDEFAULTMIDISYNTH)) {
                    success = true;
                    break;
                }
            }
            // second chance: try other output drivers's devices
            if (!success) {
                std::vector<SoundDriver> sound_drivers = GetSoundDrivers();
                std::vector<SoundDriver>::iterator it = std::find(sound_drivers.begin(), sound_drivers.end(), default_sound_driver);
                if (it != sound_drivers.end()) {
                    sound_drivers.erase(it);
                }
                for (unsigned int i = 0; i < sound_drivers.size(); ++i) {
                    FSOUND_SetOutput(sound_drivers[i]);
                    num_devices = FSOUND_GetNumDrivers();
                    for (int j = 0; j < num_devices; ++j) { 
                        FSOUND_SetDriver(j);
                        if (FSOUND_Init(SAMPLE_FREQ, channels, FSOUND_INIT_USEDEFAULTMIDISYNTH)) {
                            success = true;
                            break;
                        }
                    }
                }
            }
        }

        if (!success) {
            FSOUND_SetOutput(FSOUND_OUTPUT_NOSOUND);
            FSOUND_SetDriver(0);
            if (FSOUND_Init(SAMPLE_FREQ, channels, FSOUND_INIT_USEDEFAULTMIDISYNTH)) {
                logger.errorStream() << "InitFMOD() : No suitable sound configuration could be found ... sound disabled";
            } else {
                logger.errorStream() << "InitFMOD() : No suitable sound configuration could be found, and FSOUND_Init() failed even with "
                    "FSOUND_OUTPUT_NOSOUND as the output (sound disabled); FMOD error was \"" << FMOD_ErrorString(FSOUND_GetError()) << "\"";
            }
        }
    }
}

HumanClientAppSoundFMOD::HumanClientAppSoundFMOD()
    : HumanClientApp(),
      m_current_music(0),
      m_music_channel(-1),
      m_music_loops(0),
      m_next_music_time(0)
{
    InitFMOD();
}

void HumanClientAppSoundFMOD::PlayMusic(const std::string& filename, int loops)
{
    StopMusic();
    m_current_music = FSOUND_Stream_Open(filename.c_str(), FSOUND_2D, 0, 0);
    m_music_channel = FSOUND_Stream_Play(FSOUND_FREE, m_current_music);
    FSOUND_SetVolumeAbsolute(m_music_channel, GetOptionsDB().Get<int>("music-volume"));
    m_music_loops = loops;
    m_next_music_time = m_music_loops ? GG::GUI::GetGUI()->Ticks() + FSOUND_Stream_GetLengthMs(m_current_music) : 0;
    m_music_name = filename;
}

void HumanClientAppSoundFMOD::StopMusic()
{
    if (m_current_music) {
        FSOUND_Stream_Close(m_current_music);
        m_current_music = 0;
        m_music_channel = -1;
        m_next_music_time = 0;
        m_music_loops = 0;
    }
}

void HumanClientAppSoundFMOD::PlaySound(const std::string& filename)
{
    std::map<std::string, int>::iterator it = m_sounds.find(filename);
    if (it != m_sounds.end()) {
        if (FSOUND_SAMPLE* sample = FSOUND_Sample_Get(it->second))
            FSOUND_PlaySound(FSOUND_FREE, sample);
    } else {
        int first_available_sample_slot = 0;
        std::set<int>::iterator used_index_it = m_used_sample_indices.begin();
        while (used_index_it != m_used_sample_indices.end() && *used_index_it == first_available_sample_slot) {
            ++first_available_sample_slot;
            ++used_index_it;
        }
        if (FSOUND_SAMPLE* sample = FSOUND_Sample_Load(first_available_sample_slot, filename.c_str(), 0, 0, 0)) {
            m_sounds[filename] = first_available_sample_slot;
            m_used_sample_indices.insert(first_available_sample_slot);
            FSOUND_PlaySound(FSOUND_FREE, sample);
        }
    }
}

void HumanClientAppSoundFMOD::FreeSound(const std::string& filename)
{
    std::map<std::string, int>::iterator it = m_sounds.find(filename);
    if (it != m_sounds.end()) {
        FSOUND_Sample_Free(FSOUND_Sample_Get(it->second));
        m_used_sample_indices.erase(it->second);
        m_sounds.erase(it);
    }
}
   
void HumanClientAppSoundFMOD::FreeAllSounds()
{
    for (std::map<std::string, int>::iterator it = m_sounds.begin(); it != m_sounds.end(); ++it) {
        FSOUND_Sample_Free(FSOUND_Sample_Get(it->second));
    }
    m_used_sample_indices.clear();
    m_sounds.clear();
}

void HumanClientAppSoundFMOD::SetMusicVolume(int vol)
{
    vol = std::max(0, std::min(vol*vol/255, 255));
    if (m_music_channel != -1)
        FSOUND_SetVolumeAbsolute(m_music_channel, vol);
    GetOptionsDB().Set<int>("music-volume", vol);
}

void HumanClientAppSoundFMOD::SetUISoundsVolume(int vol)
{
    vol = std::max(0, std::min(vol*vol/255, 255));
    FSOUND_SetSFXMasterVolume(vol);
    GetOptionsDB().Set<int>("UI.sound.volume", vol);
}

void HumanClientAppSoundFMOD::RenderBegin()
{
    if (m_next_music_time && m_next_music_time <= GG::GUI::GetGUI()->Ticks() && (m_music_loops == -1 || (--m_music_loops + 1)))
        PlayMusic(m_music_name, m_music_loops);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // this is the only line in SDLGUI::RenderBegin()
}

HumanClientAppSoundFMOD::~HumanClientAppSoundFMOD()
{
    FSOUND_Close();
}

