#include "Sound.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"

#ifdef FREEORION_MACOSX
# include <OpenAL/alc.h>
#else
# include <AL/alc.h>
#endif


namespace {
    const int BUFFER_SIZE = 409600; // The size of the buffer we read music data into.

    void InitOpenAL(int num_sources, ALuint *sources, ALuint *music_buffers) {
        ALCcontext *m_context;
        ALCdevice *m_device;
        ALenum error_code;

        m_device = alcOpenDevice(0);    /* currently only select the default output device - usually a NULL-terminated
                                         * string desctribing a device can be passed here (of type ALchar*) */
        if (m_device == 0) {
            Logger().errorStream() << "Unable to initialise OpenAL device: " << alGetString(alGetError()) << "\n";
        } else {
            m_context = alcCreateContext(m_device, 0);   // instead of 0 we can pass a ALCint* pointing to a set of
                                                         // attributes (ALC_FREQUENCY, ALC_REFRESH and ALC_SYNC)

            if ((m_context != 0) && (alcMakeContextCurrent(m_context) == AL_TRUE)) {
                alListenerf(AL_GAIN,1.0);
                alGetError(); // clear possible previous errors (just to be certain)
                alGenSources(num_sources, sources);
                error_code = alGetError();
                if(error_code != AL_NO_ERROR) {
                    Logger().errorStream() << "Unable to create OpenAL sources: " << alGetString(error_code) << "\n" << "Disabling OpenAL sound system!\n";
                    alcMakeContextCurrent(0);
                    alcDestroyContext(m_context);
                } else {
                    alGetError();
                    alGenBuffers(2, music_buffers);
                    error_code = alGetError();
                    if (error_code != AL_NO_ERROR) {
                        Logger().errorStream() << "Unable to create OpenAL buffers: " << alGetString(error_code) << "\n" << "Disabling OpenAL sound system!\n";
                        alDeleteBuffers(2, music_buffers);
                        alcMakeContextCurrent(0);
                        alcDestroyContext(m_context);
                    } else {
                        for (int i = 0; i < num_sources; ++i) {
                            alSourcei(sources[i], AL_SOURCE_RELATIVE, AL_TRUE);
                        }
                        Logger().debugStream() << "OpenAL initialized. Version "
                                               << alGetString(AL_VERSION)
                                               << "Renderer "
                                               << alGetString(AL_RENDERER)
                                               << "Vendor "
                                               << alGetString(AL_VENDOR) << "\n"
                                               << "Extensions: "
                                               << alGetString(AL_EXTENSIONS) << "\n";
                    }
                }
            } else {
                Logger().errorStream() << "Unable to create OpenAL context : " << alGetString(alGetError()) << "\n";
            }
        }
    }

#ifdef FREEORION_WIN32
    int _fseek64_wrap(FILE *f, ogg_int64_t off, int whence) {
        if (!f)
            return -1;
        return fseek(f,off,whence);
    }
#endif

    int RefillBuffer(OggVorbis_File* ogg_file, ALenum ogg_format, ALsizei ogg_freq, ALuint bufferName, ogg_int64_t buffer_size, int& loops)
    {
        ALenum m_openal_error;
        int endian = 0; /// 0 for little-endian (x86), 1 for big-endian (ppc)
        int bitStream,bytes,bytes_new;
        char* array = new char[buffer_size];
        bytes = 0;

        if (alcGetCurrentContext() != 0) {
            /* First, let's fill up the buffer. We need the loop, as ov_read treats (buffer_size - bytes) to read as a suggestion only */
            do {
                bytes_new = ov_read(ogg_file, &array[bytes],(buffer_size - bytes), endian, 2, 1, &bitStream);
                bytes += bytes_new;
                if (bytes_new == 0) {
                    if (loops != 0) {   // enter here if we need to play the same file again
                        if (loops > 0)
                            loops--;
                        ov_time_seek(ogg_file,0.0); // rewind to beginning
                    }
                    else
                        break;
                }
            } while ((buffer_size - bytes) > 4096);
            if (bytes > 0) {
                alBufferData(bufferName, ogg_format, array, static_cast < ALsizei > (bytes), ogg_freq);
                m_openal_error = alGetError();
                if (m_openal_error != AL_NONE)
                    Logger().errorStream() << "RefillBuffer: OpenAL ERROR: " << alGetString(m_openal_error);
            } else {
                ov_clear(ogg_file); // the app might think we still have something to play.
                delete array;
                return 1;
            }
            delete array;
            return 0;
        }
        delete array;
        return 1;
    }
}

////////////////////////////////////////////////////////////
// TempUISoundDisabler
////////////////////////////////////////////////////////////
Sound::TempUISoundDisabler::TempUISoundDisabler()
{ Sound::GetSound().m_UI_sounds_temporarily_disabled.push(true); }

Sound::TempUISoundDisabler::~TempUISoundDisabler()
{ Sound::GetSound().m_UI_sounds_temporarily_disabled.pop(); }

////////////////////////////////////////////////////////////
// Sound
////////////////////////////////////////////////////////////
Sound& Sound::GetSound()
{
    static Sound sound;
    return sound;
}

Sound::Sound() :
    m_sources(),
    m_music_loops(),
    m_music_name(),
    m_buffers(),
    m_music_buffers(),
    m_ogg_file(),
    m_ogg_format(),
    m_ogg_freq(),
    m_UI_sounds_temporarily_disabled()
{
    InitOpenAL(NUM_SOURCES, m_sources, m_music_buffers);
}

Sound::~Sound()
{}

void Sound::PlayMusic(const boost::filesystem::path& path, int loops /* = 0*/)
{
    ALenum m_openal_error;
    std::string filename = path.string();
    FILE *m_f = 0;
    vorbis_info *vorbis_info;
    m_music_loops = 0;

#ifdef FREEORION_WIN32
    ov_callbacks callbacks = {

    (size_t (*)(void *, size_t, size_t, void *))  fread,

    (int (*)(void *, ogg_int64_t, int))           _fseek64_wrap,

    (int (*)(void *))                             fclose,

    (long (*)(void *))                            ftell

    };
#endif

    if (alcGetCurrentContext() != 0)
    {
        if (m_music_name.size() > 0)
            StopMusic();
       
        if ((m_f = fopen(filename.c_str(), "rb")) != 0) // make sure we CAN open it
        {
#ifdef FREEORION_WIN32
            if (!(ov_test_callbacks(m_f, &m_ogg_file, 0, 0, callbacks))) // check if it's a proper ogg
#else
            if (!(ov_test(m_f, &m_ogg_file, 0, 0))) // check if it's a proper ogg
#endif
            {
                ov_test_open(&m_ogg_file); // it is, now fully open the file
                /* now we need to take some info we will need later */
                vorbis_info = ov_info(&m_ogg_file, -1);
                if (vorbis_info->channels == 1)
                    m_ogg_format = AL_FORMAT_MONO16;
                else
                    m_ogg_format = AL_FORMAT_STEREO16;
                m_ogg_freq = vorbis_info->rate;
                m_music_loops = loops;
                /* fill up the buffers and queue them up for the first time */
                if (!RefillBuffer(&m_ogg_file, m_ogg_format, m_ogg_freq, m_music_buffers[0], BUFFER_SIZE, m_music_loops))
                {
                    alSourceQueueBuffers(m_sources[0],1,&m_music_buffers[0]); // queue up the buffer if we manage to fill it
                    if (!RefillBuffer(&m_ogg_file, m_ogg_format, m_ogg_freq, m_music_buffers[1], BUFFER_SIZE, m_music_loops))
                    {
                        alSourceQueueBuffers(m_sources[0],1,&m_music_buffers[1]);
                        m_music_name = filename; // yup, we're playing something that takes up more than 2 buffers
                    }
                    else
                    {
                        m_music_name.clear();  // m_music_name.clear() must always be called before ov_clear. Otherwise
                    }
                    alSourcePlay(m_sources[0]); // play if at least one buffer is queued
                }
                else
                {
                    m_music_name.clear();  // m_music_name.clear() must always be called before ov_clear. Otherwise
                }
            }
            else
            {
                Logger().errorStream() << "PlayMusic: unable to open file " << filename.c_str() << " possibly not a .ogg vorbis file. Aborting\n";
                m_music_name.clear(); //just in case
                ov_clear(&m_ogg_file);
            }
        }
        else
            Logger().errorStream() << "PlayMusic: unable to open file " << filename.c_str() << " I/O Error. Aborting\n";
    }
    m_openal_error = alGetError();
    if (m_openal_error != AL_NONE)
        Logger().errorStream() << "PlayMusic: OpenAL ERROR: " << alGetString(m_openal_error);
}

void Sound::StopMusic()
{
    if (alcGetCurrentContext() != 0)
    {
        alSourceStop(m_sources[0]);
        if (m_music_name.size() > 0)
        {
            m_music_name.clear();  // do this to avoid music being re-started by other functions
            ov_clear(&m_ogg_file); // and unload the file for good measure. the file itself is closed now, don't re-close it again
        }
        alSourcei(m_sources[0], AL_BUFFER, 0);
    }
}

void Sound::PlaySound(const boost::filesystem::path& path, bool is_ui_sound/* = false*/)
{
    if (!GetOptionsDB().Get<bool>("UI.sound.enabled") || (is_ui_sound && UISoundsTemporarilyDisabled()))
        return;

    std::string filename = path.string();
    ALuint current_buffer;
    ALenum source_state;
    ALsizei ogg_freq;
    FILE *file = 0;
    int m_i;
    bool found_buffer = true;
    bool found_source = false;

#ifdef FREEORION_WIN32
    ov_callbacks callbacks = {

    (size_t (*)(void *, size_t, size_t, void *))  fread,

    (int (*)(void *, ogg_int64_t, int))           _fseek64_wrap,

    (int (*)(void *))                             fclose,

    (long (*)(void *))                            ftell

    };
#endif

    if (alcGetCurrentContext() != 0)
    {
        /* First check if the sound data of the file we want to play is already buffered somewhere */
        std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);
        if (it != m_buffers.end())
            current_buffer = it->second;
        else {
            if ((file = fopen(filename.c_str(), "rb")) != 0) // make sure we CAN open it
            {
                OggVorbis_File ogg_file;
                vorbis_info *vorbis_info;
                ALenum ogg_format;
#ifdef FREEORION_WIN32
                if (!(ov_test_callbacks(file, &ogg_file, 0, 0, callbacks))) // check if it's a proper ogg
#else
                if (!(ov_test(file, &ogg_file, 0, 0))) // check if it's a proper ogg
#endif
                {
                    ov_test_open(&ogg_file); // it is, now fully open the file
                    /* now we need to take some info we will need later */
                    vorbis_info = ov_info(&ogg_file, -1);
                    if (vorbis_info->channels == 1)
                        ogg_format = AL_FORMAT_MONO16;
                    else
                        ogg_format = AL_FORMAT_STEREO16;
                    ogg_freq = vorbis_info->rate;
                    ogg_int64_t byte_size = ov_pcm_total(&ogg_file, -1) * vorbis_info->channels * 2;
                    if (byte_size <= 1024 * 1024 * 1024) {
                        /* fill up the buffers and queue them up for the first time */
                        ALuint sound_handle;
                        alGenBuffers(1, &sound_handle);

                        int loop = 0;

                        RefillBuffer(&ogg_file, ogg_format, ogg_freq, sound_handle, byte_size, loop);

                        current_buffer = sound_handle;
                        m_buffers.insert(std::make_pair(filename, sound_handle));
                    }
                    else
                    {
                        Logger().errorStream() << "PlaySound: unable to open file " << filename.c_str() << " too big to buffer. Aborting\n";
                    }
                    ov_clear(&ogg_file);
                }
                else
                {
                    Logger().errorStream() << "PlaySound: unable to open file " << filename.c_str() << " possibly not a .ogg vorbis file. Aborting\n";
                }
            }
        }
        if (found_buffer) {
            /* Now that we have the buffer, we need to find a source to send it to */
            for (m_i = 1; m_i < NUM_SOURCES; ++m_i) {   // as we're playing sounds we start at 1. 0 is reserved for music
                alGetSourcei(m_sources[m_i],AL_SOURCE_STATE,&source_state);
                if ((source_state != AL_PLAYING) && (source_state != AL_PAUSED)) {
                    found_source = true;
                    alSourcei(m_sources[m_i], AL_BUFFER, current_buffer);
                    alSourcePlay(m_sources[m_i]);
                    break; // so that the sound won't block all the sources
                }
            }
            if (!found_source)
                Logger().errorStream() << "PlaySound: Could not find aviable source - playback aborted\n";
        }
        source_state = alGetError();
        if (source_state != AL_NONE)
            Logger().errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(source_state);
            /* it's important to check for errors, as some functions won't work properly if
             * they're called when there is a unchecked previous error. */
    }
}

void Sound::FreeSound(const boost::filesystem::path& path) {
    ALenum m_openal_error;
    std::string filename = path.string();
    std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);

    if (it != m_buffers.end()) {
        alDeleteBuffers(1, &(it->second));
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            Logger().errorStream() << "FreeSound: OpenAL ERROR: " << alGetString(m_openal_error);
        else
            m_buffers.erase(it); /* we don't erase if there was an error, as the buffer may not have been
                                    removed - potential memory leak */
    }
}

void Sound::FreeAllSounds() {
    ALenum m_openal_error;

    for (std::map<std::string, ALuint>::iterator it = m_buffers.begin();
         it != m_buffers.end(); ++it)
    {
        alDeleteBuffers(1, &(it->second));
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            Logger().errorStream() << "FreeAllSounds: OpenAL ERROR: " << alGetString(m_openal_error);
        else
            m_buffers.erase(it); /* same as in FreeSound */
    }
}

void Sound::SetMusicVolume(int vol) {
    ALenum m_openal_error;

    /* normalize value, then apply to all sound sources */
    vol = std::max(0, std::min(vol, 255));
    GetOptionsDB().Set<int>("UI.sound.music-volume", vol);
    if (alcGetCurrentContext() != 0)
    {
        alSourcef(m_sources[0],AL_GAIN, ((ALfloat) vol)/255.0);
        /* it is highly unlikely that we'll get an error here but better safe than sorry */
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            Logger().errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(m_openal_error);
    }
}

void Sound::SetUISoundsVolume(int vol) {
    ALenum m_openal_error;

    /* normalize value, then apply to all sound sources */
    vol = std::max(0, std::min(vol, 255));
    GetOptionsDB().Set<int>("UI.sound.volume", vol);
    if (alcGetCurrentContext() != 0) {
        for (int it = 1; it < NUM_SOURCES; ++it)
            alSourcef(m_sources[it],AL_GAIN, ((ALfloat) vol)/255.0);
        /* it is highly unlikely that we'll get an error here but better safe than sorry */
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            Logger().errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(m_openal_error);
    }
}

void Sound::DoFrame() {
    ALint    state;
    int      num_buffers_processed;

    if ((alcGetCurrentContext() != 0) && (m_music_name.size() > 0)) {
        alGetSourcei(m_sources[0],AL_BUFFERS_PROCESSED,&num_buffers_processed);
        while (num_buffers_processed > 0) {
            ALuint buffer_name_yay;
            alSourceUnqueueBuffers (m_sources[0], 1, &buffer_name_yay);
            if (RefillBuffer(&m_ogg_file, m_ogg_format, m_ogg_freq, buffer_name_yay, BUFFER_SIZE, m_music_loops))
            {
                m_music_name.clear();  // m_music_name.clear() must always be called before ov_clear. Otherwise
                break; /// this happens if RefillBuffer returns 1, meaning it encountered EOF and the file shouldn't be repeated
            }
            alSourceQueueBuffers(m_sources[0],1,&buffer_name_yay);
            num_buffers_processed--;
        }
        alGetSourcei(m_sources[0], AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED)  /// this may happen if the source plays all its buffers before we manage to refill them
            alSourcePlay(m_sources[0]);
    }
} 

bool Sound::UISoundsTemporarilyDisabled() const
{ return !m_UI_sounds_temporarily_disabled.empty(); }
