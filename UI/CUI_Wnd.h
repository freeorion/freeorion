//CUI_Wnd.h
#ifndef _CUI_Wnd_h_
#define _CUI_Wnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#include "SDL.h"

//! This class is a superclass of all modal interface windows in GG.  It takes care of
/** the drawing and handling of common window interfaces like the close button, minimize
    button, and resize handle, if applicable.<br>
    
    CUI_Wnd's and ModalWnd's contain built in close buttons.  They also have optional
    minimize buttons.  Resizable windows will have a different look from non-resizable windows.
    
    All windows contain a close button.  Pressing this button will un-register the window
    from the Zlist and de-allocate the memory associated with it.  Thus any pointers 
    referencing the window become invalid if the window is closed.
    
    The minimize button functionality (not currently implemented) is available as
    a window creation flag: CUI_Wnd::MINIMIZE.  You can bitwise-OR them together
    with regular GG::Wnd creation flags.
    
    When  the GG::Wnd::RESIZABLE flag is specified, the window will receive the 
    resizable graphic style.    
    
    There are several things to keep in mind
    when utilizing these classes.<br>
    
    - Pass the title of the window as the first argument of the constructor.<br>
    - CUI_Wnd's and ModalWnd's do their own rendering.  Do not override the render function
      unless you have specific needs.  If you do, make sure you call CUI_Wnd::Render() before
      adding your own drawing.  The function exits with GL_TEXTURE_2D enabled.<br>
    - Three functions are user-overridable so that new events may be responded to.  OnClose()
      is called before the window's memory is deallocated after the user clicks the close button.
      OnMinimize() is called before the window becomes minimized after clicking the Minimize button.
      OnResize() is called as part of a response to the resize signal emitted via GG::Wnd.  Users
      are advised to use this function instead of making a slot connection with GG::Connect to respond
      to the resize signal.
    
*/

class CUI_ModalWnd : public GG::ModalWnd
{
public:
//! \name Structors
//!@{
    CUI_ModalWnd(const std::string& t, int x, int y, int h, int w, Uint32 flags = GG::Wnd::CLICKABLE); //!< Initializes the window to be a CUI window
    CUI_ModalWnd(const GG::XMLElement& elem);
    ~CUI_ModalWnd();    //!< Destructor

//!@}
protected:
//! \name Mutators
//!@{
    virtual void OnClose();    //!< called when window is closed via the close button
    virtual void OnMinimize(); //!< called when window is minimized
    virtual void OnResize(int x, int y); //!< called when window is resized
    
    
public:
    virtual int Render();    //!< renders the window as a FreeOrion window
    virtual int LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys); //!< Respond to drag events
//!@}

private:
//! \name Mutators
//!@{
    void OnCloseClick();    //!< mapped to the pressing of the square
    void OnMinimizeClick(); //!< mapped to pressing of the dash button
    void OnResizeClick(int x, int y); //!< mapped to resizing the window

    
//!@}

protected:
//! \name Data Members
//!@{
    std::string m_title;        //!< title of the window
    bool        m_minimize;     //!< whether the window minimizes or not

//!@}
};//CUI_ModalWnd

//! This class is a superclass of all interface windows in GG.  It takes care of
/** the drawing and handling of common window interfaces like the close button, minimize
    button, and resize handle, if applicable.<br>
    
    CUI_Wnd's and ModalWnd's contain built in close buttons.  They also have optional
    minimize buttons.  Resizable windows will have a different look from non-resizable windows.
    
    All windows contain a close button.  Pressing this button will un-register the window
    from the Zlist and de-allocate the memory associated with it.  Thus any pointers 
    referencing the window become invalid if the window is closed.
    
    The minimize button functionality (not currently implemented) is available as
    a window creation flag: CUI_Wnd::MINIMIZE.  You can bitwise-OR them together
    with regular GG::Wnd creation flags.
    
    When  the GG::Wnd::RESIZABLE flag is specified, the window will receive the 
    resizable graphic style.    
    
    There are several things to keep in mind
    when utilizing these classes.<br>
    
    - Pass the title of the window as the first argument of the constructor.<br>
    - CUI_Wnd's and ModalWnd's do their own rendering.  Do not override the render function
      unless you have specific needs.  If you do, make sure you call CUI_Wnd::Render() before
      adding your own drawing.  The function exits with GL_TEXTURE_2D enabled.<br>
    - Three functions are user-overridable so that new events may be responded to.  OnClose()
      is called before the window's memory is deallocated after the user clicks the close button.
      OnMinimize() is called before the window becomes minimized after clicking the Minimize button.
      OnResize() is called as part of a response to the resize signal emitted via GG::Wnd.  Users
      are advised to use this function instead of making a slot connection with GG::Connect to respond
      to the resize signal.
    
*/
class CUI_Wnd : public GG::Wnd
{
public:
    enum    //!< additional flags
    {
        MINIMIZABLE    =   256,
    };
//! \name Structors
//!@{
    CUI_Wnd(const std::string& t, int x, int y, int h, int w, Uint32 flags = GG::Wnd::CLICKABLE); //!< Initializes the window to be a CUI window
    CUI_Wnd(const GG::XMLElement& elem); //!< init through XML
    ~CUI_Wnd();    //!< Destructor

//!@}

protected:
//! \name Mutators
//!@{
    virtual void OnClose();    //!< called when window is closed via the close button
    virtual void OnMinimize(); //!< called when window is minimized
    virtual void OnResize(int x, int y); //!< called when window is resized
    
public:
    virtual int Render();    //!< renders the window as a FreeOrion window
    virtual int LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys); //!< Respond to drag events
//!@}

private:
//! \name Mutators
//!@{
    void OnCloseClick();    //!< mapped to the pressing of the square
    void OnMinimizeClick(); //!< mapped to pressing of the dash button
    void OnResizeClick(int x, int y); //!< mapped to resizing the window
    
    
//!@}
protected:
//! \name Data Members
//!@{
    std::string m_title;        //!< title of the window
    bool        m_minimize;     //!< whether the window minimizes or not
    bool        m_is_resizing;  //!< keeps resizing smooth
    
//!@}
};//CUI_Wnd

#endif
