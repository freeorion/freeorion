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

AnimatedTexture::AnimatedTexture(AnimatedTexture& rhs)
{
    *this=rhs;
}//AnimatedTexture(AnimatedTexture&)


AnimatedTexture::AnimatedTexture(const GG::XMLElement& elem):
    m_current_frame(0),
    m_current_time(0),
    m_state(ANIM_STOPPED)
{
    using namespace GG;

    if(elem.Tag() != "AnimatedTexture")
    {
        throw std::invalid_argument("Attempted to construct an AnimatedTexture from an XMLElement that had a tag other than \"AnimatedTexture\"");
    }
    
    //clear vectors, just in case
    m_frames.clear();
    m_times.clear();
    
    //read animation type
    const XMLElement* curr_elem = &elem.Child("m_type");
    m_type = lexical_cast<int>(curr_elem->Attribute("value"));
    
    curr_elem = &elem.Child("m_frames");
    const int num_frames = lexical_cast<int>(curr_elem->Attribute("value"));  //use this for num of frames
    
    //create frames
    for(int i=0; i<num_frames; ++i)
    {
        const XMLElement* frame_elem = &curr_elem->Child("frame" + lexical_cast<string>(i));
        //first get frame time
        const XMLElement* temp_elem = &frame_elem->Child("time");
        m_times.push_back(lexical_cast<int>(temp_elem->Attribute("value")));
        
        m_frames.push_back(boost::shared_ptr<GG::SubTexture>(new SubTexture(frame_elem->Child("GG::SubTexture"))));
    }
    
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
    if(SDL_GetTicks() >= (m_current_time + m_times[m_current_frame]))
    {
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
}//Animate()

GG::XMLElement AnimatedTexture::XMLEncode() const 
{
    using namespace GG;
    
    XMLElement retval("AnimatedTexture"), temp, temp2;
    
    //m_current_frame and m_current_time always set to 0 on load
    //m_state always set to ANIM_STOPPED on load
    
    temp = XMLElement("m_type");
    temp.SetAttribute("value", lexical_cast<string>(m_type));
    retval.AppendChild(temp);
    
    //do frames and times
    temp = XMLElement("m_frames");
    temp.SetAttribute("value", lexical_cast<string>(m_frames.size())); //value is number of frames
    
    
    for(int i=0; i<m_frames.size(); ++i)
    {
        temp2 = XMLElement("frame"+lexical_cast<string>(i));    //write frame and number
                
        XMLElement time_temp("time");
        time_temp.SetAttribute("value",lexical_cast<string>(m_times[i]));
        
        temp2.AppendChild(time_temp);                        //append time for this frame
        temp2.AppendChild(m_frames[i]->XMLEncode());        //append the subtexture
        
        temp.AppendChild(temp2);
    }
    retval.AppendChild(temp);
    
    return retval;
    
}//XMLEncode()


