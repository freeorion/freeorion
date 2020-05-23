#ifndef _CUIWnd_h_
#define _CUIWnd_h_

#include <GG/GGFwd.h>
#include <GG/Button.h>
#include <GG/Wnd.h>
#include <GG/WndEvent.h>
#include <GG/GLClientAndServerBuffer.h>
#include <unordered_set>


/** a simple minimize/restore button that toggles its appearance between the styles for minimize and restore*/
class CUI_MinRestoreButton : public GG::Button {
public:
   /** the two modes of operation of this class of button: as a minimize button or as a restore button */
   enum class Mode {
       MINIMIZE,
       RESTORE
   };

   CUI_MinRestoreButton();

   Mode GetMode() const {return m_mode;} ///< returns the current mode of this button (is it a minimize button or a restore button?)

   void Render() override;

   void Toggle(); ///< toggles modes between MIN_BUTTON and RESTORE_BUTTON

private:
   Mode m_mode;
};

/** a basic Pin-shaped pin button. */
class CUI_PinButton : public GG::Button {
public:
    CUI_PinButton();

    void Toggle(bool pinned); // Switches icon from Pin to Pinned and back
};

// Aditional window creation flags
extern GG::WndFlag MINIMIZABLE;    ///< allows the window to be minimized
extern GG::WndFlag CLOSABLE;       ///< allows the window to be closed
extern GG::WndFlag PINABLE;        ///< allows the window to be pinned


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
      are advised to use this function instead of making a slot connection with boost::signals to respond
      to the resize signal.
*/
class CUIWnd : public GG::Wnd {
public:
    //! \name Structors //@{
    /** Constructs the window to be a CUI window. Specifying \a config_name
      * causes the window to save its position and other properties to the
      * OptionsDB under that name, if no other windows are currently using that
      * name. */
    CUIWnd(const std::string& wnd_name, GG::X x, GG::Y y, GG::X w, GG::Y h,
           GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE,
           const std::string& config_name = "", bool visible = true);

    /** Constructs a CUI window without specifying the initial (default)
      * position, either call InitSizeMove() if the window is positioned by
      * something else or override CalculatePosition() then call
      * ResetDefaultPosition() for windows that position themselves. */
    CUIWnd(const std::string& wnd_name, GG::Flags<GG::WndFlag> flags = GG::INTERACTIVE,
           const std::string& config_name = "", bool visible = true);

    void CompleteConstruction() override;
    /** Virtual destructor. */
    virtual ~CUIWnd();
    //@}

    //! \name Accessors //@{
    bool    Minimized() const {return m_minimized;} //!< returns true if window is minimized
    GG::Pt  ClientUpperLeft() const override;
    GG::Pt  ClientLowerRight() const override;
    bool    InWindow(const GG::Pt& pt) const override;
    GG::X   LeftBorder() const;                 //!< the distance on the left side between the outer edge of the window and the inner border
    GG::Y   TopBorder() const;                  //!< the distance at the top between the outer edge of the window and the inner border
    GG::X   RightBorder() const;                //!< the distance on the right side between the outer edge of the window and the inner border
    GG::Y   BottomBorder() const;               //!< the distance at the bottom between the outer edge of the window and the inner border
    //@}

    //! \name Mutators //@{
    void SizeMove(const GG::Pt& ul, const GG::Pt& lr) override;
    void Render() override;
    void LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) override;
    void LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;

    void LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override
    { return LButtonUp(pt, mod_keys); }

    void MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) override;
    void MouseLeave() override;
    void Hide() override;
    void Show() override;

    void Flash() { m_flashing = true; };
    void StopFlash() { m_flashing = false; };
    void SetFlashDuration(int ms) { m_flash_duration = ms; }

    void ToggleMinimized() { MinimizeClicked(); }
    void Close()           { CloseClicked(); }
    void ValidatePosition();                                 //!< calls SizeMove() to trigger position-checking and position the window entirely within the parent window/app window
    void InitSizeMove(const GG::Pt& ul, const GG::Pt& lr);   //!< sets default positions and if default positions were set beforehand, calls SizeMove()
    //@}

    //! \name Mutators //@{
    virtual void    CloseClicked();                     //!< called when window is closed via the close button
    virtual void    PinClicked();                       //!< called when window is pinned or unpinned via the pin button
    //@}

    //! \name Statics //@{
    static void     InvalidateUnusedOptions();          //!< removes unregistered and registered-but-unused window options from the OptionsDB so that new windows fall back to their default properties.

    //@}

protected:
    //! \name Accessors //@{
    virtual GG::Pt          MinimizedSize() const;              //!< the size of a minimized CUIWnd
    int                     InnerBorderAngleOffset() const;     //!< the distance from where the lower right corner of the inner border should be to where the angled portion of the inner border meets the right and bottom lines of the border
    bool                    InResizeTab(const GG::Pt& pt) const;//!< returns true iff the specified \a pt is in the region where dragging will resize this Wnd
    void                    SaveOptions() const;                //!< saves options for this window to the OptionsDB if config_name was specified in the constructor

    virtual GG::Rect        CalculatePosition() const;          //!< override this if a class determines its own position/size and return the calculated values, called by ResetDefaultPosition()
    //@}

    //! \name Statics //@{
    static const std::string AddWindowOptions(const std::string& config_name,
                                              int left, int top, int width, int height,
                                              bool visible, bool pinned, bool minimized);   //!< Adds OptionsDB entries for a window under a given name along with default values.

    static const std::string AddWindowOptions(const std::string& config_name,
                                              GG::X left, GG::Y top, GG::X width, GG::Y height,
                                              bool visible, bool pinned, bool minimized);   //!< overload that accepts GG::X and GG::Y instead of ints

    static void              InvalidateWindowOptions(const std::string& config_name);       //!< removes options containing \a config_name, logs an error instead if "ui."+config_name+".initialized" exists (i.e. if a window is currently using that name)
    //@}

    //! \name Mutators //@{
    virtual void    MinimizeClicked();              //!< called when window is minimized or restored via the minimize/restore button
    virtual void    InitButtons();                  //!< called to create the buttons, withtout positioning them
    virtual void    PositionButtons();              //!< called to position the buttons

    virtual void    InitBuffers();
    void            LoadOptions();                  //!< loads options for this window from the OptionsDB
    void            Init();                         //!< performs initialization common to all CUIWnd constructors
    void            ResetDefaultPosition();         //!< called via signal from the ClientUI, passes the value from CalculatePosition() to InitSizeMove()

    void SetParent(const std::shared_ptr<GG::Wnd>& wnd) override;
    /** Flags options currently at their default values for later use in SaveDefaultedOptions */
    void            SetDefaultedOptions();
    /** Sets the default value any options previously determined from calls to SetDefaultedOptions to their current value */
    void            SaveDefaultedOptions();
    //@}

    bool                    m_resizable = false;    //!< true if the window is able to be resized
    bool                    m_closable = false;     //!< true if the window is able to be closed with a button press
    bool                    m_minimizable = false;  //!< true if the window is able to be minimized
    bool                    m_minimized = false;    //!< true if the window is currently minimized
    bool                    m_pinable = false;      //!< true if the window is able to be pinned
    bool                    m_pinned = false;       //!< true if the window is currently pinned
    bool                    m_flashing = false;     //!< true if the window is currently flashing

    int                     m_flash_duration = 1000;//!< time in milliseconds to switch between bright and dark

    GG::Pt                  m_drag_offset;          //!< offset from the lower-right corner of the point being used to drag-resize
    GG::Pt                  m_original_size;        //!< keeps track of the size of the window before resizing

    bool                    m_mouse_in_resize_tab = false;

    bool                    m_config_save = true;   //!< true if SaveOptions() is currently allowed to write to the OptionsDB
    const std::string       m_config_name;          //!< the name that this window will use to save its properties to the OptionsDB, the default empty string means "do not save"

    std::shared_ptr<GG::Button>             m_close_button;     //!< the close button
    std::shared_ptr<CUI_MinRestoreButton>   m_minimize_button;  //!< the minimize/restore button
    std::shared_ptr<CUI_PinButton>          m_pin_button;       //!< the pin button

    std::unordered_set<std::string> m_defaulted_options;

    GG::GL2DVertexBuffer                                m_vertex_buffer;
    std::vector<std::pair<std::size_t, std::size_t>>    m_buffer_indices;

    static const GG::Y      BUTTON_TOP_OFFSET;
    static const GG::X      BUTTON_RIGHT_OFFSET;
    static const GG::X      MINIMIZED_WND_WIDTH;
    static const GG::X      BORDER_LEFT;
    static const GG::X      BORDER_RIGHT;
    static const GG::Y      BORDER_BOTTOM;
    static const int        OUTER_EDGE_ANGLE_OFFSET;
    static const int        INNER_BORDER_ANGLE_OFFSET;
    static const int        TITLE_OFFSET;
    static const int        RESIZE_HASHMARK1_OFFSET;
    static const int        RESIZE_HASHMARK2_OFFSET;
};


// This didn't seem big enough to warrant its own file, so this seemed like a good enough place for it....
/** provides a convenient modal wnd for getting text user input. */
class CUIEditWnd : public CUIWnd {
public:
    CUIEditWnd(GG::X w, const std::string& prompt_text, const std::string& edit_text, GG::Flags<GG::WndFlag> flags = GG::MODAL);
    void CompleteConstruction() override;
    void ModalInit() override;
    void KeyPress(GG::Key key, std::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) override;

    const std::string& Result() const;

private:
    void OkClicked();

    std::string m_result;

    std::shared_ptr<GG::Edit>   m_edit;
    std::shared_ptr<GG::Button> m_ok_bn;
    std::shared_ptr<GG::Button> m_cancel_bn;

    static const GG::X BUTTON_WIDTH;
    static const int CONTROL_MARGIN;
};

#endif // _CUIWnd_h_
