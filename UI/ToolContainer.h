#ifndef _ToolContainer_h_
#define _ToolContainer_h_

#include "GGWnd.h"
#include "ToolWnd.h"

#ifndef _SDL_timer_h
#include "SDL_timer.h"
#endif

//!ToolContainer acts as a container for all of the Tooltip windows in an
//!entire application.  Once initialized, the container operates without any 
//!intervention until it is destroyed.  Only one object should be active at a 
//!time.  Windows are added to the ToolContainer using the AttachToolWnd() function.
class ToolContainer
{

private:
//! \name Structors
//!@{

    //! Default construction
    ToolContainer();
    
//!@}

public:
//! \name Structors
//!@{
    
    //! Main construction
    
    //! @param delay number of milliseconds the mouse must hover before a window appears.  Default value = 1000.
    ToolContainer(int delay=1000);
    
    //! Default Destruction
    ~ToolContainer();    //window destruction

//!@}    

//! \name Initialization
//!@{

//! Attaches a ToolWnd to its parent window

//! @param parent a pointer to a GG::Wnd representing the parent of the tooltip
//! @param tool the pointer to the ToolWnd
//!
//! A ToolWnd attached in this way will be destroyed by GG when the application exits.
//! DO NOT MANUALLY delete() A ToolWnd THAT HAS BEEN ATTACHED TO A PARENT!!!!

    bool AttachToolWnd(GG::Wnd* parent, ToolWnd* tool);
//!@}
protected:
//! \name Data Members
//!@{

    std::map<GG::Wnd*, ToolWnd*> toolList;   //!< a map of wnds to toolwnds
 
    SDL_TimerID timerID;     //!< SDL_Timer usage
    Uint32 startTime;        //!< SDL_Timer usage
    Uint32 currentTime;      //!< SDL_Timer usage
    Uint32 delayTime;        //!< time wait before tooltips appear in ms
    
//!@}

//! \name Internal Use
//!@{

    static Uint32 ShowCallback(Uint32, void*);    //!< callback function for SDL_Timer
    
//!@}
};    //ToolContainer

#endif
