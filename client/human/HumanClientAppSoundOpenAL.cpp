/* THIS IS UNTESTED CODE! Please give feedback if it works. */
#include "HumanClientAppSoundOpenAL.h"
#include "../../util/OptionsDB.h"

#include <al.h>
#include <alc.h>
#include <AL/alut.h>
#include <vorbis/vorbisfile.h>

namespace {
    void InitOpenAL(int num_sources, ALuint *sources, ALuint *music_buffers)
    {
        log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
        
        if (alutInit(NULL, NULL) == AL_FALSE) {
            logger.errorStream() << "Unable to initialise OpenAL: " << alutGetErrorString(alutGetError()) << "\n";
        } else {
            alGenSources(num_sources, sources);
            alGenBuffers(2, music_buffers);
            for (int i=0; i<num_sources; i++) {
                alSourcei(sources[i], AL_SOURCE_RELATIVE, AL_TRUE);
            }
            logger.debugStream() << "OpenAL initialized. Version " 
                                 << alGetString(AL_VERSION)
                                 << "Renderer "
                                 << alGetString(AL_RENDERER)
                                 << "Vendor "
                                 << alGetString(AL_VENDOR)
                                 << "\nExtensions: " 
                                 << alGetString(AL_EXTENSIONS)
                                 << "\n";
        }
    }
}

HumanClientAppSoundOpenAL::HumanClientAppSoundOpenAL()
    : HumanClientApp()
{
    InitOpenAL(M_SOURCES_NUM, m_sources, m_music_buffers);
}

HumanClientAppSoundOpenAL::~HumanClientAppSoundOpenAL()
{
    alutExit();
}

int HumanClientAppSoundOpenAL::RefillBuffer(ALuint *bufferName)
{
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
    ALenum m_openal_error;
    int endian = 0; /// 0 for little-endian (x86), 1 for big-endian (ppc)
    int bitStream,bytes,bytes_new;
    char array[BUFFER_SIZE];
    bytes=0;
   
    if (alcGetCurrentContext() != NULL)
    {
        /* First, let's fill up the buffer. We need the loop, as ov_read treats (BUFFER_SIZE - bytes) to read as a suggestion only */
        do
        {
            bytes_new = ov_read(&m_ogg_file, &array[bytes],(BUFFER_SIZE - bytes), endian, 2, 1, &bitStream);
            bytes += bytes_new;
            if (bytes_new == 0)
            {
                if (m_music_loops != 0) // enter here if we need to play the same file again
                {
                    if (m_music_loops > 0)
                        m_music_loops--;
                    ov_time_seek(&m_ogg_file,0.0); // rewind to beginning
                }
                else
                    break;
            }
        } while ((BUFFER_SIZE - bytes) > 4096);
        if (bytes > 0)
        {
            alBufferData(bufferName[0], m_ogg_format, array, static_cast < ALsizei > (bytes),m_ogg_freq);
            m_openal_error = alGetError();
            if (m_openal_error != AL_NONE)
                logger.errorStream() << "RefillBuffer: OpenAL ERROR: " << alGetString(m_openal_error);
        }
        else
        {
            if (m_music_name.size() > 0)
            {
                m_music_name.clear();  // do this to avoid music being re-started by other functions
                ov_clear(&m_ogg_file); // and unload the file for good measure. the file itself is closed now, don't re-close it again
            }
            return 1;
        }
        return 0;
    }
    return 0;
}

void HumanClientAppSoundOpenAL::PlayMusic(const std::string& filename, int loops /* = 0*/)
{
    ALenum m_openal_error;
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
    FILE *m_f = NULL;
    vorbis_info *m_ogg_info;
    m_music_loops = 0;
   
    if (alcGetCurrentContext() != NULL)
    {
        if (m_music_name.size() > 0)
            StopMusic();
       
        if ((m_f = fopen(filename.c_str(), "rb")) != NULL) // make sure we CAN open it
        {
            if (!(ov_test(m_f, &m_ogg_file, NULL, 0))) // check if it's a proper ogg
            {
                ov_test_open(&m_ogg_file); // it is, now fully open the file
                /* now we need to take some info we will need later */
                m_ogg_info = ov_info(&m_ogg_file, -1);
                if (m_ogg_info->channels == 1)
                    m_ogg_format = AL_FORMAT_MONO16;
                else
                    m_ogg_format = AL_FORMAT_STEREO16;
                m_ogg_freq = m_ogg_info->rate;
                m_music_loops = loops;
                /* fill up the buffers and queue them up for the first time */
                RefillBuffer(&m_music_buffers[0]);
                RefillBuffer(&m_music_buffers[1]);
                alSourceQueueBuffers(m_sources[0],2,m_music_buffers);
                alSourcePlay(m_sources[0]);
                m_music_name = filename; // yup, we're playing something
            }
            else
            {
                logger.errorStream() << "PlayMusic: unable to open file " << filename.c_str() << " possibly not a .ogg vorbis file. Aborting\n";
                m_music_name.clear(); //just in case
                ov_clear(&m_ogg_file);
            }
        }
        else
            logger.errorStream() << "PlayMusic: unable to open file " << filename.c_str() << " I/O Error. Aborting\n";
    }
    m_openal_error = alGetError();
    if (m_openal_error != AL_NONE)
        logger.errorStream() << "PlayMusic: OpenAL ERROR: " << alGetString(m_openal_error);
}

void HumanClientAppSoundOpenAL::StopMusic()
{
    int num_buffers_processed;
    ALuint buffer_name_yay;
   
    if (alcGetCurrentContext() != NULL)
    {
        alSourceStop(m_sources[0]);
        m_music_name.clear();  // do this to avoid being re-started by other functions
        ov_clear(&m_ogg_file); // and unload the file for good measure. the file itself is still open, but PlayMusic is safeguarded against it.
        alGetSourcei(m_sources[0],AL_BUFFERS_PROCESSED,&num_buffers_processed);         // we need to unqueue any unplayed buffers
        alSourceUnqueueBuffers (m_sources[0], num_buffers_processed, &buffer_name_yay); // otherwise they'll cause problems if we open another file
    }
}

 void HumanClientAppSoundOpenAL::PlaySound(const std::string& filename)
 {
     log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
    ALuint m_current_buffer;
    ALenum m_source_state;
    int m_i;
    int m_found_buffer = 1;
    int m_found_source = 0;
   
     if (alcGetCurrentContext() != NULL)
     {
        /* First check if the sound data of the file we want to play is already buffered somewhere */
         std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);
         if (it != m_buffers.end())
            m_current_buffer=it->second;
        else
         {
            /* We buffer the file if it wasn't previously */
            if ((m_current_buffer = alutCreateBufferFromFile(filename.c_str())) != AL_NONE)
                m_buffers[filename] = m_current_buffer;
            else
             {
                logger.errorStream() << "PlaySound: Cannot create buffer for: " << filename.c_str() << " Reason:" << alutGetErrorString(alutGetError());
                m_found_buffer = 0;
             }
        }
        if (m_found_buffer)
        {
            /* Now that we have the buffer, we need to find a source to send it to */
            for (m_i=1;m_i<M_SOURCES_NUM;m_i++) // as we're playing sounds we start at 1. 0 is reserved for music
             {
                alGetSourcei(m_sources[m_i],AL_SOURCE_STATE,&m_source_state);
                if ((m_source_state != AL_PLAYING) && (m_source_state != AL_PAUSED))
                {
                    m_found_source = 1;
                    alSourcei(m_sources[m_i], AL_BUFFER, m_current_buffer);
                    alSourcePlay(m_sources[m_i]);
                    break; // so that the sound won't block all the sources
                }
             }
            if (!m_found_source)
                logger.errorStream() << "PlaySound: Could not find aviable source - playback aborted\n";
         }
        m_source_state = alGetError();
        if (m_source_state != AL_NONE)
            logger.errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(m_source_state);
            /* it's important to check for errors, as some functions (mainly alut) won't work properly if
             * they're called when there is a unchecked previous error. */
     }
 }
 
 void HumanClientAppSoundOpenAL::FreeSound(const std::string& filename)
 {
    ALenum m_openal_error;
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
     std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);
   
     if (it != m_buffers.end()) {
         alDeleteBuffers(1, &(it->second));
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            logger.errorStream() << "FreeSound: OpenAL ERROR: " << alGetString(m_openal_error);
        else
            m_buffers.erase(it); /* we don't erase if there was an error, as the buffer may not have been
                                    removed - potential memory leak */
     }
 }
 
 void HumanClientAppSoundOpenAL::FreeAllSounds()
 {
    ALenum m_openal_error;
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
   
     for (std::map<std::string, ALuint>::iterator it = m_buffers.begin();
          it != m_buffers.end(); ++it) {
         alDeleteBuffers(1, &(it->second));
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            logger.errorStream() << "FreeAllSounds: OpenAL ERROR: " << alGetString(m_openal_error);
        else
            m_buffers.erase(it); /* same as in FreeSound */
     }
 }
 
 void HumanClientAppSoundOpenAL::SetMusicVolume(int vol)
{
    ALenum m_openal_error;
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
   
    /* normalize value, then apply to all sound sources */
    vol = std::max(0, std::min(vol, 255));
    GetOptionsDB().Set<int>("music-volume", vol);
    if (alcGetCurrentContext() != NULL)
    {
        alSourcef(m_sources[0],AL_GAIN, ((ALfloat) vol)/255.0);
        /* it is highly unlikely that we'll get an error here but better safe than sorry */
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            logger.errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(m_openal_error);
    }
}
 
 void HumanClientAppSoundOpenAL::SetUISoundsVolume(int vol)
{
    ALenum m_openal_error;
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
   
    /* normalize value, then apply to all sound sources */
    vol = std::max(0, std::min(vol, 255));
    GetOptionsDB().Set<int>("UI.sound.volume", vol);
    if (alcGetCurrentContext() != NULL)
    {
        for (int it=1; it < M_SOURCES_NUM; it++)
            alSourcef(m_sources[it],AL_GAIN, ((ALfloat) vol)/255.0);
        /* it is highly unlikely that we'll get an error here but better safe than sorry */
        m_openal_error = alGetError();
        if (m_openal_error != AL_NONE)
            logger.errorStream() << "PlaySound: OpenAL ERROR: " << alGetString(m_openal_error);
    }
}
 
void HumanClientAppSoundOpenAL::RenderBegin()
{
    ALint    state;
    int      num_buffers_processed;
   
    if ((alcGetCurrentContext() != NULL) && (m_music_name.size() > 0))
    {
        alGetSourcei(m_sources[0],AL_BUFFERS_PROCESSED,&num_buffers_processed);
        while (num_buffers_processed > 0)
        {
            ALuint buffer_name_yay;
            alSourceUnqueueBuffers (m_sources[0], 1, &buffer_name_yay);
            if (RefillBuffer(&buffer_name_yay))
                break; /// this happens if RefillBuffer returns 1, meaning it encountered EOF and the file shouldn't be repeated
            alSourceQueueBuffers(m_sources[0],1,&buffer_name_yay);
            num_buffers_processed--;
        }
        alGetSourcei(m_sources[0], AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED)  /// this may happen if the source plays all its buffers before we manage to refill them
            alSourcePlay(m_sources[0]);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // this is the only line in SDLGUI::RenderBegin()
}