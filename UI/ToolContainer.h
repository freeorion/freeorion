// -*- C++ -*-
#ifndef _ToolContainer_h_
#define _ToolContainer_h_

#ifndef _SDL_timer_h
#include "SDL_timer.h"
#endif

#include <map>
#include <string>

namespace GG {class Wnd;}

//!ToolContainer acts as a container for all of the tool tip windows in an
//!entire application.  Once initialized, the container operates without any 
//!intervention until it is destroyed.  Only one object should be active at a 
//!time.  Windows are added to the ToolContainer using the AttachToolWnd() function.
class ToolContainer
{
public:
    //! \name Structors
    //!@{
    //! Basic ctor
    //! @param delay number of milliseconds the mouse must hover before a window appears.
    ToolContainer(int delay = 1000);
    
    ~ToolContainer();    ///< dtor
    //!@}    

    //! \name Initialization
    //!@{
    //! Attaches a ToolWnd to its parent window
    //! @param parent a pointer to a GG::Wnd representing the parent of the tool tip
    //! @param tool the pointer to the ToolWnd
    //! A ToolWnd attached in this way will be destroyed by GG when the application exits.
    //! DO NOT MANUALLY delete() A ToolWnd THAT HAS BEEN ATTACHED TO A PARENT!!!!
    bool AttachToolWnd(GG::Wnd* parent, GG::Wnd* tool_tip);
    //!@}

private:
    std::map<GG::Wnd*, GG::Wnd*> m_tool_tip_wnds; //!< a map of wnds to toolwnds
    SDL_TimerID                  m_timer_id;      //!< SDL_Timer usage

    static Uint32 Callback(Uint32, void*);    //!< callback function for SDL_Timer    
};

inline std::pair<std::string, std::string> ToolContainerRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _ToolContainer_h_
