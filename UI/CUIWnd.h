// -*- C++ -*-
//CUIWnd.h
#ifndef _CUIWnd_h_
#define _CUIWnd_h_

#include <GG/Button.h>
#include <GG/Wnd.h>
#include <GG/WndEvent.h>

class CUIEdit;
class CUIButton;


/** a simple minimize/restore button that toggles its appearance between the styles for minimize and restore*/
class CUI_MinRestoreButton : public GG::Button {
public:
   /** the two modes of operation of this class of button: as a minimize button or as a restore button */
   enum Mode {
       MIN_BUTTON, 
       RESTORE_BUTTON
   };
              
   CUI_MinRestoreButton(GG::X x, GG::Y y); ///< basic ctor

   Mode GetMode() const {return m_mode;} ///< returns the current mode of this button (is it a minimize button or a restore button?)

   void Render();

   void Toggle(); ///< toggles modes between MIN_BUTTON and RESTORE_BUTTON

private:
   Mode m_mode;
};

/** a basic X-shaped close button. */
class CUI_CloseButton : public GG::Button {
public:
   CUI_CloseButton(GG::X x, GG::Y y);

   void Render();
};


// Aditional window creation flags
extern GG::WndFlag MINIMIZABLE;    ///< allows the window to be minimized
extern GG::WndFlag CLOSABLE;       ///< allows the window to be closed


//! This class is a superclass of all interface windows in GG.  It takes care of
/** the drawing and handling of common window interfaces like the close button, minimize
    button, and resize handle, if applicable.<br>
    
    CUIWnd's contain built in close buttons.  They also have optional
    minimize buttons.  Resizable windows will have a different look from non-resizable windows.
    
    All windows contain a close button.  Pressing this button will un-register the window
    from the Zlist and de-allocate the memory associated with it.  Thus any pointers 
    referencing the window become invalid if the window is closed.
    
    The minimize button functionality is available as
    a window creation flag: CUIWnd::MINIMIZE.  You can bitwise-OR it together
    with regular GG::Wnd creation flags.
    
    When  the GG::Wnd::RESIZABLE flag is specified, the window will receive the 
    resizable graphic style.    
    
    There are several things to keep in mind
    when utilizing these classes.<br>
    
    - Pass the title of the window as the first argument of the constructor.<br>
    - CUIWnd's do their own rendering.  Do not override the render function
      unless you have specific needs.  If you do, make sure you call CUIWnd::Render() before
      adding your own drawing. <br>
    - Three functions are user-overridable so that new events may be responded to.  OnClose()
      is called before the window's memory is deallocated after the user clicks the close button.
      OnMinimize() is called before the window becomes minimized after clicking the Minimize button.
      OnResize() is called as part of a response to the resize signal emitted via GG::Wnd.  Users
      are advised to use this function instead of making a slot connection with GG::Connect to respond
      to the resize signal.
    
*/
class CUIWnd : public GG::Wnd {
public:
    //! \name Structors //@{
    CUIWnd(const std::string& t, GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE); //!< Constructs the window to be a CUI window
    virtual ~CUIWnd();  //!< Destructor
    //@}

    //! \name Accessors //@{
    bool            Minimized() const {return m_minimized;} //!< returns true if window is minimized
    virtual GG::Pt  ClientUpperLeft() const;
    virtual GG::Pt  ClientLowerRight() const;
    virtual bool    InWindow(const GG::Pt& pt) const;
    //@}

    //! \name Mutators //@{
    virtual void    SizeMove(const GG::Pt& ul, const GG::Pt& lr);
    virtual void    Render();
    virtual void    LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys);
    virtual void    LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {return LButtonUp(pt, mod_keys);}

    void            ToggleMinimized() {MinimizeClicked();}
    void            Close()           {CloseClicked();}
    //@}

    //! \name Mutators //@{
    virtual void    CloseClicked();                 //!< called when window is closed via the close button
    //@}

protected:
    //! \name Accessors //@{
    virtual GG::X   MinimizedWidth() const;         //!< the width of a minimized CUIWnd
    GG::X           LeftBorder() const;             //!< the distance on the left side between the outer edge of the window and the inner border
    GG::Y           TopBorder() const;              //!< the distance at the top between the outer edge of the window and the inner border
    GG::X           RightBorder() const;            //!< the distance on the right side between the outer edge of the window and the inner border
    GG::Y           BottomBorder() const;           //!< the distance at the bottom between the outer edge of the window and the inner border
    int             InnerBorderAngleOffset() const; //!< the distance from where the lower right corner of the inner border should be to where the angled portion of the inner border meets the right and bottom lines of the border
    //@}

    //! \name Mutators //@{
    virtual void    MinimizeClicked();              //!< called when window is minimized or restored via the minimize/restore button
    void            InitButtons();
    //@}

    bool                    m_resizable;      //!< true if the window is able to be resized
    bool                    m_closable;       //!< true if the window is able to be closed with a button press
    bool                    m_minimizable;    //!< true if the window is able to be minimized
    bool                    m_minimized;      //!< true if the window is currently minimized
    GG::Pt                  m_drag_offset;    //!< offset from the lower-right corner of the point being used to drag-resize
    GG::Pt                  m_original_size;  //!< keeps track of the size of the window before resizing

    CUI_CloseButton*        m_close_button;     //!< the close button
    CUI_MinRestoreButton*   m_minimize_button;  //!< the minimize/restore button

    static const GG::Y      BUTTON_TOP_OFFSET;
    static const GG::X      BUTTON_RIGHT_OFFSET;
    static const GG::X      MINIMIZED_WND_WIDTH;
    static const GG::X      BORDER_LEFT;
    static const GG::Y      BORDER_TOP;
    static const GG::X      BORDER_RIGHT;
    static const GG::Y      BORDER_BOTTOM;
    static const int        OUTER_EDGE_ANGLE_OFFSET;
    static const int        INNER_BORDER_ANGLE_OFFSET;
    static const int        RESIZE_HASHMARK1_OFFSET;
    static const int        RESIZE_HASHMARK2_OFFSET;
};


// This didn't seem big enough to warrant its own file, so this seemed like a good enough place for it....
/** provides a convenient modal wnd for getting text user input. */
class CUIEditWnd : public CUIWnd {
public:
    CUIEditWnd(GG::X w, const std::string& prompt_text, const std::string& edit_text, GG::Flags<GG::WndFlag> flags = GG::MODAL);

    virtual void ModalInit();
    virtual void KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys);

    const std::string& Result() const;

private:
    void OkClicked();

    std::string m_result;

    CUIEdit*    m_edit;
    CUIButton*  m_ok_bn;
    CUIButton*  m_cancel_bn;

    static const GG::X BUTTON_WIDTH;
    static const int CONTROL_MARGIN;
};

#endif // _CUIWnd_h_
