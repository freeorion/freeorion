//CUI_Wnd.h
#ifndef _CUI_Wnd_h_
#define _CUI_Wnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#include "SDL.h"

/** a simple minimize/restore button that toggles its appearance between the styles for minimize and restore*/
class CUI_MinRestoreButton : public GG::Button
{
public:
   /** the two modes of operation of this class of button: as a minimize button or as a restore button */
   enum Mode {MIN_BUTTON, 
              RESTORE_BUTTON
             };
              
   CUI_MinRestoreButton(int x, int y); ///< basic ctor
   CUI_MinRestoreButton(const GG::XMLElement& elem); ///< ctor that constructs a CUI_MinRestoreButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUI_MinRestoreButton object

   Mode GetMode() const {return m_mode;} ///< returns the current mode of this button (is it a minimize button or a restore button?)

   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a CUI_MinRestoreButton object

   int Render();

   void Toggle() {m_mode = (m_mode == MIN_BUTTON ? RESTORE_BUTTON : MIN_BUTTON);} ///< toggles modes between MIN_BUTTON and RESTORE_BUTTON

private:
   Mode m_mode;
};

/** a basic X-shaped close button. */
class CUI_CloseButton : public GG::Button
{
public:
   CUI_CloseButton(int x, int y);
   CUI_CloseButton(const GG::XMLElement& elem); ///< ctor that constructs a CUI_CloseButton object from an XMLElement. \throw std::invalid_argument May throw std::invalid_argument if \a elem does not encode a CUI_CloseButton object

   virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a CUI_CloseButton object

   int Render();
};

//! This class is a superclass of all interface windows in GG.  It takes care of
/** the drawing and handling of common window interfaces like the close button, minimize
    button, and resize handle, if applicable.<br>
    
    CUI_Wnd's contain built in close buttons.  They also have optional
    minimize buttons.  Resizable windows will have a different look from non-resizable windows.
    
    All windows contain a close button.  Pressing this button will un-register the window
    from the Zlist and de-allocate the memory associated with it.  Thus any pointers 
    referencing the window become invalid if the window is closed.
    
    The minimize button functionality is available as
    a window creation flag: CUI_Wnd::MINIMIZE.  You can bitwise-OR it together
    with regular GG::Wnd creation flags.
    
    When  the GG::Wnd::RESIZABLE flag is specified, the window will receive the 
    resizable graphic style.    
    
    There are several things to keep in mind
    when utilizing these classes.<br>
    
    - Pass the title of the window as the first argument of the constructor.<br>
    - CUI_Wnd's do their own rendering.  Do not override the render function
      unless you have specific needs.  If you do, make sure you call CUI_Wnd::Render() before
      adding your own drawing. <br>
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
    //! additional window creation flags
    enum {MINIMIZABLE = 256   //!< allows the window to be minimized
         };

    //! \name Structors //@{
    CUI_Wnd(const std::string& t, int x, int y, int h, int w, Uint32 flags = GG::Wnd::CLICKABLE); //!< Constructs the window to be a CUI window
    CUI_Wnd(const GG::XMLElement& elem); //!< Construction through XML
    ~CUI_Wnd();    //!< Destructor
    //@}

    //! \name Accessors //@{
    bool Minimized() const {return m_minimized;} //!< returns true if window is minimized

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a CUI_Wnd object
    //@}

    //! \name Mutators //@{
    virtual int Render();
    virtual int LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual int LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);
    virtual int LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual int LClick(const GG::Pt& pt, Uint32 keys) {return LButtonUp(pt, keys);}
    //@}

    static int S_MINIMIZED_WND_LENGTH;

protected:
    //! \name Mutators //@{
    virtual void OnClose();    //!< called when window is closed via the close button
    virtual void OnMinimize(); //!< called when window is minimized
    virtual void OnResize(int x, int y); //!< called when window is resized
    //@}

private:
    void InitButtons();
    void CloseClicked();    //!< mapped to the pressing of the square
    void MinimizeClicked(); //!< mapped to pressing of the dash button
    void ResizeClicked(int x, int y); //!< mapped to resizing the window

    bool       m_minimizable;    //!< true if the window is able to be minimized
    bool       m_minimized;      //!< true if the window is currently minimized
    GG::Pt     m_resize_offset;  //!< offset from the lower-right corner of the point being used to drag-resize
    GG::Pt     m_original_size;  //!< keeps track of the size of the window before resizing
    
    CUI_CloseButton*       m_close_button;     //!< the close button
    CUI_MinRestoreButton*  m_minimize_button;  //!< the minimize/restore button
};

#endif
