/* THIS IS UNTESTED CODE! Please give feedback if it works. */
#include "HumanClientAppSoundOpenAL.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

namespace {
    void InitOpenAL(int num_sources, ALuint *sources)
    {
        log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
        
        if (alutInit(NULL, NULL) == AL_FALSE) {
            logger.errorStream() << "Unable to initialise OpenAL: " << alutGetErrorString(alutGetError());
        } else {
            alGenSources(num_sources, sources);
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
                                 << alGetString(AL_EXTENSIONS);
        }
    }
}

HumanClientAppSoundOpenAL::HumanClientAppSoundOpenAL()
    : HumanClientApp()
{
    InitOpenAL(16, m_sources);
}

HumanClientAppSoundOpenAL::~HumanClientAppSoundOpenAL()
{
    alutExit();
}


void HumanClientAppSoundOpenAL::PlayMusic(const std::string& filename, int loops /* = 0*/)
{}

void HumanClientAppSoundOpenAL::StopMusic()
{}

void HumanClientAppSoundOpenAL::PlaySound(const std::string& filename)
{
    log4cpp::Category& logger = HumanClientApp::GetApp()->Logger();
    if (alcGetCurrentContext() != NULL)
    {
        std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);
        if (it != m_buffers.end())
        {
            alSourcei(m_sources[1],AL_BUFFER,it->second);
            alSourcePlay(m_sources[1]);
        }
        else
        {
            ALuint first_available_sample_slot = 0;
            ALuint sample;
            std::set<ALuint>::iterator used_index_it = m_used_sample_indices.begin();
            while (used_index_it != m_used_sample_indices.end() && *used_index_it == first_available_sample_slot)
            {
                ++first_available_sample_slot;
                ++used_index_it;
            }
            if ((sample = alutCreateBufferFromFile(filename.c_str())) != AL_NONE)
            {
                m_buffers[filename] = sample;
                m_used_sample_indices.insert(sample);
                alSourcei(m_sources[1],AL_BUFFER,sample);
                alSourcePlay(m_sources[1]);
            }
            else
                logger.errorStream() << "PlaySound: Cannot Create buffer for: " << filename.c_str() << " Reason:" << alutGetErrorString(alutGetError());
        }
    }
}

void HumanClientAppSoundOpenAL::FreeSound(const std::string& filename)
{
    std::map<std::string, ALuint>::iterator it = m_buffers.find(filename);
    if (it != m_buffers.end()) {
        alDeleteBuffers(1, &(it->second));
        m_used_sample_indices.erase(it->second);
        m_sounds.erase(it);
        m_buffers.erase(it);
    }
}

void HumanClientAppSoundOpenAL::FreeAllSounds()
{
    for (std::map<std::string, ALuint>::iterator it = m_buffers.begin(); 
         it != m_buffers.end(); ++it) {
        alDeleteBuffers(1, &(it->second));
    }
    m_used_sample_indices.clear();
    m_buffers.clear();
}

void HumanClientAppSoundOpenAL::SetMusicVolume(int vol)
{}

void HumanClientAppSoundOpenAL::SetUISoundsVolume(int vol)
{}


