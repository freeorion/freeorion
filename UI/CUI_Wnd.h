// -*- C++ -*-
//CUI_Wnd.h
#ifndef _CUI_Wnd_h_
#define _CUI_Wnd_h_

#ifndef _GGWnd_h_
#include "GGWnd.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

class CUIEdit;
class CUIButton;
class CUIButton;


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

   bool Render();

   void Toggle(); ///< toggles modes between MIN_BUTTON and RESTORE_BUTTON

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

   bool Render();
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
    using Wnd::SizeMove;

    //! additional window creation flags
    enum {MINIMIZABLE = 1 << 10,    //!< allows the window to be minimized
          CLOSABLE =    1 << 11     //!< allows the window to be closed
         };

    //! \name Structors //@{
    CUI_Wnd(const std::string& t, int x, int y, int h, int w, Uint32 flags = GG::Wnd::CLICKABLE); //!< Constructs the window to be a CUI window
    CUI_Wnd(const GG::XMLElement& elem); //!< Construction through XML
    ~CUI_Wnd();    //!< Destructor
    //@}

    //! \name Accessors //@{
    bool Minimized() const {return m_minimized;} //!< returns true if window is minimized
    virtual bool InWindow(const GG::Pt& pt) const;

    virtual GG::XMLElement XMLEncode() const; ///< constructs an XMLElement from a CUI_Wnd object
    //@}

    //! \name Mutators //@{
    virtual void SizeMove(int x1, int y1, int x2, int y2);
    virtual bool Render();
    virtual void LButtonDown(const GG::Pt& pt, Uint32 keys);
    virtual void LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys);
    virtual void LButtonUp(const GG::Pt& pt, Uint32 keys);
    virtual void LClick(const GG::Pt& pt, Uint32 keys) {return LButtonUp(pt, keys);}

    void ToggleMinimized() {MinimizeClicked();}
    void Close()           {CloseClicked();}
    //@}

protected:
    //! \name Accessors //@{
    virtual int MinimizedLength() const;//!< the width of a minimized CUI_Wnd
    int LeftBorder() const;             //!< the distance on the left side between the outer edge of the window and the inner border
    int TopBorder() const;              //!< the distance at the top between the outer edge of the window and the inner border
    int RightBorder() const;            //!< the distance on the right side between the outer edge of the window and the inner border
    int BottomBorder() const;           //!< the distance at the bottom between the outer edge of the window and the inner border
    int InnerBorderAngleOffset() const; //!< the distance from where the lower right corner of the inner border should be to where the angled portion of the inner border meets the right and bottom lines of the border
    //@}

    //! \name Mutators //@{
public:
    virtual void CloseClicked();    //!< called when window is closed via the close button
protected:
    virtual void MinimizeClicked(); //!< called when window is minimized or restored via the minimize/restore button
    //@}

private:
    void InitButtons();

    bool       m_resizable;      //!< true if the window is able to be resized
    bool       m_closable;       //!< true if the window is able to be closed with a button press
    bool       m_minimizable;    //!< true if the window is able to be minimized
    bool       m_minimized;      //!< true if the window is currently minimized
    GG::Pt     m_resize_offset;  //!< offset from the lower-right corner of the point being used to drag-resize
    GG::Pt     m_original_size;  //!< keeps track of the size of the window before resizing
    
    CUI_CloseButton*       m_close_button;     //!< the close button
    CUI_MinRestoreButton*  m_minimize_button;  //!< the minimize/restore button
};


// This didn't seem big enough to warrant its own file, so this seemed like a good enough place for it....
/** provides a convenient modal wnd for getting text user input. */
class CUIEditWnd : public CUI_Wnd
{
public:
    CUIEditWnd(int w, const std::string& prompt_text, const std::string& edit_text, Uint32 flags = Wnd::MODAL);

    void Keypress(GG::Key key, Uint32 key_mods);

    const std::string& Result() const;

private:
    void OkClicked();

    std::string m_result;

    CUIEdit*    m_edit;
    CUIButton*  m_ok_bn;
    CUIButton*  m_cancel_bn;

    static const int BUTTON_WIDTH = 75;
    static const int CONTROL_MARGIN = 5;
};

inline std::pair<std::string, std::string> CUI_WndRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _CUI_Wnd_h_
