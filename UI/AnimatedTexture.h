//AnimatedTexture.h

#ifndef _AnimatedTexture_h_
#define _AnimatedTexture_h_

#include "GGApp.h"

#include "GGStaticGraphic.h"

#include "GGTexture.h"

#include "XMLDoc.h"

#include "boost/shared_ptr.hpp" //include for smart pointers

//! This class operates similarly to GG::Texture, only it represents an animated image.
//! An animated graphic consists of a set of GG::SubTextures, which cycle at a specified
//! time interval.  Each frame can be displayed for an independent amount of time.  For
//! instance: frame 1 can be displayed for 5 seconds, and frame 2 can be displayed for 3.
//!  Animations can either be looping, whereas they start over from the beginning once they reach the last frame
//! and continue to play infinitely.  Or they can be one shot animations that play once and then pause on their final frame.
//! Animations can be controlled similarly to a VCR using the functions: Play(), Pause(), and Stop().
//! \see GG::Texture, GG::SubTexture

class AnimatedTexture
{
public:
    //! This enum defines the different types of animations
    enum
    {
        ANIM_LOOPING = 0,    //!< The animation will infinitely loop
        ANIM_ONCE    = 1     //!< The animation will run once and freeze on its last frame
    };//enum
    
    //! This enum defines the different states of an animation
    enum
    {
        ANIM_PAUSED  = 0,    //!< The animation is paused.
        ANIM_STOPPED = 1,    //!< The animation is stopped.
        ANIM_PLAYING = 2    //!< The animation is currently playing.
    };//enum
public:
    //! \name Structors
    //!@{
    
    //! Creates an empty texture with the given type.
    
    //! @param type either ANIM_LOOPING or ANIM_ONCE
    AnimatedTexture(int type);
    
    AnimatedTexture(AnimatedTexture &rhs); //!< Copy Constructor
    
    //! Creates an animated texture from an XML element
    
    //! @param elem The XML element used to create this AnimatedTexture
    AnimatedTexture(const GG::XMLElement& elem);
    
    //! Destruction
    ~AnimatedTexture();
    //!@}
    
    GG::XMLElement XMLEncode() const;    //!< Builds an XMLElement from this object
    
    //!@}
    
public:
    //! \name Operations
    //!@{
    
    //! Adds a subtexture to this AnimatedTexture as the next frame
    
    //! @param s a shared pointer to a GG::SubTexture representing the next frame of animation
    //! @param time the amount of time (in milliseconds) this frame is to be displayed
//    inline void AddNextFrame(GG::SubTexture* s, int time)
    inline void AddNextFrame(boost::shared_ptr<GG::SubTexture> s, int time)
    {
        m_frames.push_back(s);    //insert the frame
        m_times.push_back(time);  //insert the amt of time  
    }
    
    inline int Width() {return m_frames.empty() ? 0 : m_frames[m_current_frame]->Width();}  //!< Returns the width of the current frame
    inline int Height() {return m_frames.empty() ? 0 : m_frames[m_current_frame]->Height();}//!< Returns the height of the current frame

    //!@}

    //! \name Control
    //!@{
    
    void Play();    //!< Restart the animation if paused, or start it from the beginning if stopped
    
    void Pause();      //!< Pause the animation on the current frame.
    
    void Stop();       //!< Stop the animation and reset it to frame 0
    
    //! Sets the animation type to ANIM_LOOPING or ANIM_ONCE
    inline void SetType(int type) {m_type=type;}
    
    //! Retrives the animation type.
    inline int Type() {return m_type;}
    //!@}
    
    //! \name Operations
    //!@{
    
    //! \see GG::Texture::OrthoBlit()
    void OrthoBlit(const GG::Pt& pt, bool enter_2d_mode=true);
    
    //! \see GG::Texture::OrthoBlit()
    void OrthoBlit(const GG::Pt& pt1, const GG::Pt& pt2, const GLfloat* tex_coords=0, bool enter_2d_mode=true);
    
    //!@}
protected:

    AnimatedTexture();    //!< Default ctor
    
    //! \name Internal Functions
    //!@{
    
    //! Animate is called by the rendering code.
    //! It takes care of updating the frames.
    
    void Animate();
    
    //!@}
protected:

    //! \name Data Members
    //!@{
    
    int m_type;      //!< The type of animated, ANIM_LOOPING or ANIM_ONCE
    
    int m_current_frame;    //<! Index of the frame currently being displayed.
    int m_current_time;    //<! Keeps track of the time
    int m_state;            //!< State of the animation, ANIM_PAUSED, ANIM_STOPPED, ANIM_PLAYING
    
    //! A vector of shared pointer to AnimationFrame's.  These represent the frames of animation.
//    std::vector<GG::SubTexture*> m_frames;
    std::vector< boost::shared_ptr<GG::SubTexture> > m_frames;
    std::vector<int>             m_times; //!< Time for each frame (in milliseconds)  
    
    //!@}
    
};//AnimatedTexture



#endif
