//AnimatedGraphic.h

#ifndef _AnimatedGraphic_h_
#define _AnimatedGraphic_h_

#ifndef _GGControl_h_
#include "GGControl.h"
#endif

#ifndef _AnimatedTexture_h_
#include "AnimatedTexture.h"
#endif

//! AnimatedGraphic is similar to GG::StaticGraphic, except this graphic is animated.
//! It includes many of the same functions as GG::StaticGraphic, but the two are not
//! interchangable.
//! \see GG::Control, GG::StaticGraphic
class AnimatedGraphic : public GG::Control
{
public:
//! \name Structors
//!@{

//    AnimatedGraphic(); //!< default ctor
//    AnimatedGraphic(int x, int y, int w, int h, AnimatedTexture* texture, Uint32 style=0, Uint32 flags=0);  //!<build a graphic from an animated texture
    AnimatedGraphic(int x, int y, int w, int h, boost::shared_ptr<AnimatedTexture> texture, Uint32 style=0, Uint32 flags=0);  //!<build a graphic from an animated texture
    ~AnimatedGraphic();    //!<default dtor

//!@}

//! \name Operations
//!@{
    int Render();   //!< Draws this control to the screen.
    
    inline Uint32 Style() {return m_style;}    //!< Retrieves the style of this control. \see GG::StaticGraphic
    
    inline void SetStyle(Uint32 style) {m_style=style; ValidateStyle();} //!< Sets the style. \see GG::StaticGraphic
    
//!@}
protected:
//! \name Data Members
//!@{
//    AnimatedTexture* m_anim_graphic;
    boost::shared_ptr<AnimatedTexture> m_anim_graphic; //!< shared pointer to the AnimatedTexture to display on this control
    Uint32 m_style;    //!< The style of the control.  \see GG::StaticGraphic
    
//!@}

    void ValidateStyle();   ///< ensures that the style flags are consistent

};//AnimatedGraphic

#endif
