//AnimatedTexture.cpp

#include "AnimatedTexture.h"
#include "ClientUI.h"

AnimatedTexture::AnimatedTexture():
    m_current_frame(0),
    m_current_time(0),
    m_state(ANIM_STOPPED),
    m_type(ANIM_LOOPING)
{
    m_frames.clear();
    m_times.clear();
}//AnimatedTexture()

AnimatedTexture::AnimatedTexture(int type):
    m_current_frame(0),
    m_current_time(0),
    m_state(ANIM_STOPPED),
    m_type(type)
{
    m_frames.clear();
    m_times.clear();
}//AnimatedTexture(int)

AnimatedTexture::AnimatedTexture::AnimatedTexture(AnimatedTexture& rhs)
{
    *this=rhs;
}//AnimatedTexture(AnimatedTexture&)


AnimatedTexture::AnimatedTexture(const GG::XMLElement& elem)
{
    //TODO: Implementation.
    
}//AnimatedTexture(XMLElement)

AnimatedTexture::~AnimatedTexture()
{
 
}//~AnimatedTexture()

void AnimatedTexture::OrthoBlit(const GG::Pt& pt, bool enter_2d_mode)
{
    if(m_frames.empty())
    {
        return;            //no frames, don't render
    }
  
    //call Animate to update frames
    Animate();
    //render the given subtexture
    m_frames[m_current_frame]->OrthoBlit(pt, enter_2d_mode);

}//OrthoBlit(Pt, bool)

void AnimatedTexture::OrthoBlit(const GG::Pt& pt1, const GG::Pt& pt2, const GLfloat* tex_coords, bool enter_2d_mode)
{
    //The texture shall not draw itself, it will draw a subtexture pertaining to
    //  the current frame
    if(m_frames.empty())
    {
        return;            //no frames, don't render
    }

    //call Animate to update frames
    Animate();
    //render the given subtexture

    m_frames[m_current_frame]->OrthoBlit(pt1, pt2, enter_2d_mode);

}//OrthoBlit(Pt, Pt, GLfloat*, etc.)

void AnimatedTexture::Pause()
{
    if(m_state!=ANIM_STOPPED)
        m_state=ANIM_PAUSED;    
}//Pause()

void AnimatedTexture::Play()
{  
    if(m_state==ANIM_STOPPED)
    {
        //if its stopped, reset the timers and frames
        m_current_frame=0;
        m_current_time=SDL_GetTicks();
    }
    
    m_state=ANIM_PLAYING;        //set state to playing
}//Play()

void AnimatedTexture::Stop()
{
    m_state=ANIM_STOPPED; //stop playing
}//Stop()

void AnimatedTexture::Animate()
{
    ClientUI::s_logger.debug("Animate() entered");
    if(SDL_GetTicks() >= (m_current_time + m_times[m_current_frame]))
    {
    ClientUI::s_logger.debug("Advancing frame");
        //time to increment animations
        if(m_current_frame >= m_frames.size()-1) 
        {
            if(m_type==ANIM_LOOPING)
            {
                m_current_frame=0;            //restart the animation
            }
            else  //ANIM_ONCE
            {
                m_current_frame=m_frames.size()-1; //leave it on the last frame
            }
            return;  //do not update frames, we did it already :)
        }
        //we got this far, so advance one frame
        ++m_current_frame;
        m_current_time=SDL_GetTicks(); //get a new time reference for next frame
    }
    ClientUI::s_logger.debug("Animate() left");
}//Animate()

GG::XMLElement AnimatedTexture::XMLEncode() const 
{
    //TODO: Implementation
    
}//XMLEncode()


