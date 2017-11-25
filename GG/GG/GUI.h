// -*- C++ -*-
/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */
   
/** \file GUI.h \brief Contains GUI class, which encapsulates the state and
    behavior of the entire GG GUI. */

#ifndef _GG_GUI_h_
#define _GG_GUI_h_

#include <GG/Font.h>
#include <GG/WndEvent.h>

#include <boost/signals2/signal.hpp>

#include <chrono>


namespace boost { namespace archive {
    class xml_oarchive;
    class xml_iarchive;
} }

namespace GG {

class Cursor;
class Wnd;
class EventPumpBase;
class ModalEventPump;
class StyleFactory;
class Texture;
class Timer;
struct GUIImpl;

template <typename T>
std::shared_ptr<T> LockAndResetIfExpired(std::weak_ptr<T>& ptr) {
    auto locked = ptr.lock();
    if (!locked)
        ptr.reset();
    return locked;
}

/** \brief An abstract base for an GUI framework class to drive the GG GUI.

    This class has all the essential services that GG requires: 
    - GUI initialization and emergency exit
    - GG event handling
    - rendering of GUI windows
    - entry into and cleanup after a "2D" mode, in case the app also uses non-orthographic 3D projections
    - registration into, removal from, and movement within a z-ordering of windows, including handling of "always-on-top" windows
    - handling of modal windows
    - drag-and-drop support
    - inter-frame time updates and FPS calculations
    - limits on FPS speed
    - access to the dimensions of the application window or screen
    - mouse state information
    - keyboard accelerators
    - management of fonts and textures

    <p>The user is required to provide several functions.  The most vital
    functions the user is required to provide are: Enter2DMode(),
    Exit2DMode(), DeltaT(), PollAndRender() [virtual private], and Run()
    [virtual private].  Without these, GUI is pretty useless.  In addition,
    HandleEvent() must be driven from PollAndRender().  The code driving
    HandleEvent() must interact with the hardware and/or operating system, and
    supply the appropriate EventType's, key presses, and mouse position info
    to HandleEvent().  It is the author's recommendation that the user use the
    provided SDL driver to do this.

    <p>Keyboard accelerators may be defined, as mentioned above.  Each defined
    accelerator has its own signal which is emitted each time the accelerator
    is detected.  Client code should listen to the appropriate signal to act
    on an accelerator invocation.  Each slot that is signalled with a keyboard
    accelerator should return true if it processed the accelerator, or false
    otherwise.  This lets GUI know whether or not it should create a keystroke
    event and process it normally, sending it to the Wnd that currently has
    focus.  Note that since signals can be connected to multiple slots, if
    even one slot returns true, no kestroke event is created.  It is perfectly
    legal to return false even if an accelerator is processed, as long as you
    also then want the focus Wnd to receive a keystroke event.  Also, note
    that all accelerators are processed before, and possbily instead of, any
    key events.  So setting a plain "h" as a keyboard accelerator can (if it
    is processed normally by a slot) prevent any Wnd anywhere in your
    application from receiving "h" keystrokes.  To avoid this:
    - Define accelerators with modifier keys like CTRL and ALT, or
    - Have slots that process these accelerators return false, or
    - Do not connect anything to such an accelerator, in which case it will return false.

    <p>A GUI-wide StyleFactory can be set; this controls the actual types of
    controls and dialogs that are created when a control or dialog creates one
    (e.g. when FileDlg encounters an error and creates a ThreeButtonDlg).
    This is overridden by any StyleFactory that may be installed in an
    individual Wnd.

    <p>A note about "button-down-repeat".  When you click on the down-button
    on a scroll-bar, you probably expect the the button's action (scrolling
    down one increment) to repeat when you hold down the button, much like the
    way kestrokes are repeated when you hold down a keyboard key.  This is in
    fact what happens, and it is accomplished by having the scroll and its
    buttons respond to extra LButtonDown events generated by the GUI.  These
    extra messages, occur ButtonDownRepeatDelay() milliseconds after the
    button is first depressed, repeating every ButtonDownRepeatInterval()
    milliseconds thereafter.  Only Wnds created with the REPEAT_BUTTON_DOWN
    flag receive such extra messages.
*/
class GG_API GUI
{
private:
    struct OrCombiner
    {
        typedef bool result_type; 
        template<class InIt> bool operator()(InIt first, InIt last) const;
    };

public:
    /** \name Signal Types */ ///@{
    /** Emitted when a keyboard accelerator is invoked. A return value of true
        indicates that the accelerator was processed by some slot; otherwise,
        a keystroke event is processed instead. */
    typedef boost::signals2::signal<bool (), OrCombiner> AcceleratorSignalType;
    //@}

    /// These are the only events absolutely necessary for GG to function
    /// properly
    enum EventType {
        IDLE,        ///< nothing has changed since the last message, but the GUI might want to update some things anyway
        KEYPRESS,    ///< a down key press or key repeat, with or without modifiers like Alt, Ctrl, Meta, etc.
        KEYRELEASE,  ///< a key release, with or without modifiers like Alt, Ctrl, Meta, etc.
        TEXTINPUT,   ///< user inputted unicode text using some method
        LPRESS,      ///< a left mouse button press
        MPRESS,      ///< a middle mouse button press
        RPRESS,      ///< a right mouse button press
        LRELEASE,    ///< a left mouse button release
        MRELEASE,    ///< a middle mouse button release
        RRELEASE,    ///< a right mouse button release
        MOUSEMOVE,   ///< movement of the mouse; may include relative motion in addition to absolute position
        MOUSEWHEEL   ///< rolling of the mouse wheel; this event is accompanied by the amount of roll in the y-component of the mouse's relative position (+ is up, - is down)
    };

    /** The type of iterator returned by non-const accel_begin() and
        accel_end(). */
    typedef std::set<std::pair<Key, Flags<ModKey>>>::iterator accel_iterator;

    /** The type of iterator returned by const accel_begin() and
        accel_end(). */
    typedef std::set<std::pair<Key, Flags<ModKey>>>::const_iterator const_accel_iterator;

    /** \name Structors */ ///@{
    virtual ~GUI();
    //@}

    /** \name Accessors */ ///@{
    const std::string&          AppName() const;                    ///< returns the user-defined name of the application
    std::shared_ptr<Wnd>                        FocusWnd() const;                   ///< returns the GG::Wnd that currently has the input focus
    bool                        FocusWndAcceptsTypingInput() const; ///< returns true iff the current focus GG::Wnd accepts typing input
    std::shared_ptr<Wnd>                        PrevFocusInteractiveWnd() const;    ///< returns the previous Wnd to the current FocusWnd. Cycles through INTERACTIVE Wnds, in order determined by parent-child relationships
    std::shared_ptr<Wnd>                        NextFocusInteractiveWnd() const;    ///< returns the next Wnd to the current FocusWnd.
    std::shared_ptr<Wnd>                        GetWindowUnder(const Pt& pt) const; ///< returns the GG::Wnd under the point pt
    unsigned int                DeltaT() const;                     ///< returns ms since last frame was rendered
    virtual unsigned int        Ticks() const = 0;                  ///< returns milliseconds since the app started running
    bool                        RenderingDragDropWnds() const;      ///< returns true iff drag-and-drop Wnds are currently being rendered
    bool                        FPSEnabled() const;                 ///< returns true iff FPS calulations are turned on
    double                      FPS() const;                        ///< returns the frames per second at which the GUI is rendering
    std::string                 FPSString() const;                  ///< returns a string of the form "[m_FPS] frames per second"
    double                      MaxFPS() const;                     ///< returns the maximum allowed frames per second of rendering speed.  0 indicates no limit.
    virtual X                   AppWidth() const = 0;               ///< returns the width of the application window/screen
    virtual Y                   AppHeight() const = 0;              ///< returns the height of the application window/screen
    unsigned int                KeyPressRepeatDelay() const;        ///< returns the \a delay value set by EnableKeyPressRepeat()
    unsigned int                KeyPressRepeatInterval() const;     ///< returns the \a interval value set by EnableKeyPressRepeat()
    unsigned int                ButtonDownRepeatDelay() const;      ///< returns the \a delay value set by EnableMouseButtonDownRepeat()
    unsigned int                ButtonDownRepeatInterval() const;   ///< returns the \a interval value set by EnableMouseButtonDownRepeat()
    unsigned int                DoubleClickInterval() const;        ///< returns the maximum interval allowed between clicks that is still considered a double-click, in ms
    unsigned int                MinDragTime() const;                ///< returns the minimum time (in ms) an item must be dragged before it is a valid drag
    unsigned int                MinDragDistance() const;            ///< returns the minimum distance an item must be dragged before it is a valid drag
    bool                        DragWnd(const Wnd* wnd, unsigned int mouse_button) const;   ///< returns true if \a wnd is currently being dragged with button \a mouse_button
    bool                        DragDropWnd(const Wnd* wnd) const;  ///< returns true if \a wnd is currently begin dragged as part of a drag-and-drop operation
    bool                        AcceptedDragDropWnd(const Wnd* wnd) const;  ///< returns true if \a wnd is currently begin dragged as part of a drag-and-drop operation, and it is over a drop target that will accept it
    bool                        MouseButtonDown(unsigned int bn) const;     ///< returns the up/down states of the mouse buttons
    Pt                          MousePosition() const;              ///< returns the absolute position of mouse, based on the last mouse motion event
    Pt                          MouseMovement() const;              ///< returns the relative position of mouse, based on the last mouse motion event
    Flags<ModKey>               ModKeys() const;                    ///< returns the set of modifier keys that are currently depressed, based on the last event
    bool                        MouseLRSwapped() const;             ///< returns true if the left and right mouse button press events are set to be swapped before event handling. This is to facilitate left-handed mouse users semi-automatically.
    const std::map<Key, Key>&   KeyMap() const;                     ///< returns the the key remappings set, which causes the GUI to respond to one Key press as though a different Key were pressed.
    virtual std::string         ClipboardText() const;              ///< returns text stored in a clipboard

    /** Returns the (begin, end) indices of the code points of all the
        word-tokens in the given string.  This is perhaps an odd place for
        this function to exist, but the notion of what a "word" is is so
        application-specific that it was placed here so that users can
        customize this behavior. */
    virtual std::set<std::pair<CPSize, CPSize>>     FindWords(const std::string& str) const;
    virtual std::set<std::pair<StrSize, StrSize>>   FindWordsStringIndices(const std::string& str) const;
    /** Returns true if \a word is a word that appears in \a str */
    virtual bool                                    ContainsWord(const std::string& str, const std::string& word) const;

    /** Returns the currently-installed style factory. */
    const std::shared_ptr<StyleFactory>&    GetStyleFactory() const;

    bool                                    RenderCursor() const; ///< returns true iff the GUI is responsible for rendering the cursor

    /* Returns the currently-installed cursor. */
    const std::shared_ptr<Cursor>&          GetCursor() const;

    /** Returns an iterator to one past the first defined keyboard accelerator. */
    const_accel_iterator                    accel_begin() const;

    /** Returns an iterator to one past the last defined keyboard accelerator. */
    const_accel_iterator                    accel_end() const;

    /** Returns the signal that is emitted when the requested keyboard accelerator is invoked. */
    AcceleratorSignalType&                  AcceleratorSignal(Key key, Flags<ModKey> mod_keys = MOD_KEY_NONE) const;

    /** Returns true iff keyboard accelerator signals fire while modal windows are open. */
    bool ModalAcceleratorSignalsEnabled() const;

    /** Returns true iff any modal Wnds are open. */
    bool ModalWndsOpen() const;

    /** Saves \a wnd to file \a filename during the next render cycle.  If \a
        wnd is not rendered during the render cycle, or PNG support is not
        enabled, this is a no-op. */
    void SaveWndAsPNG(const Wnd* wnd, const std::string& filename) const;
    //@}

    /** \name Mutators */ ///@{
    void            operator()();                 ///< external interface to Run()
    virtual void    ExitApp(int code = 0) = 0;           ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetGUI()

    /** Handles all waiting system events (from SDL, DirectInput, etc.).  This
        function should only be called from custom EventPump event
        handlers. */
    virtual void    HandleSystemEvents() = 0;

    /** Event handler for GG events. */
    void HandleGGEvent(EventType event, Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys, const Pt& pos, const Pt& rel, const std::string* text = nullptr);

    void            ClearEventState();

    void            SetFocusWnd(const std::shared_ptr<Wnd>& wnd);          ///< sets the input focus window to \a wnd
    /** Suspend the GUI thread for \p us microseconds.  Singlethreaded GUI subclasses may do
        nothing here, or may pause for \p us microseconds. */
    virtual void    Wait(std::chrono::microseconds us);
    virtual void    Wait(unsigned int ms);          ///< suspends the GUI thread for \a ms milliseconds.  Singlethreaded GUI subclasses may do nothing here, or may pause for \a ms milliseconds.

    /** Adds \p wnd into the z-list.  A registered window is owned by the GUI as a top level
        window. Registering a null pointer or registering the same window multiple times is a no-op. */
    void            Register(std::shared_ptr<Wnd> wnd);
    /** Adds \p wnd onto the modal windows "stack".  Modal windows are owned by the GUI as a
        top-level window. */
    void            RegisterModal(std::shared_ptr<Wnd> wnd);
    void            Remove(const std::shared_ptr<Wnd>& wnd);               ///< removes \a wnd from the z-list.  Removing a null pointer or removing the same window multiple times is a no-op.
    void            MoveUp(const std::shared_ptr<Wnd>& wnd);               ///< moves \a wnd to the top of the z-list
    void            MoveDown(const std::shared_ptr<Wnd>& wnd);             ///< moves \a wnd to the bottom of the z-list

    /** Creates a new ModalEventPump that will terminate when \a done is set to
        true. */
    virtual std::shared_ptr<ModalEventPump> CreateModalEventPump(bool& done);

    /** Adds \a wnd to the set of current drag-and-drop Wnds, to be rendered
        \a offset pixels from the cursor position. \a originating_wnd
        indicates the original owner of \a wnd before the drag-and-drop.
        \throw std::runtime_error May throw std::runtime_error if there are
        already other Wnds registered that belong to a window other than \a
        originating_wnd. */
    void           RegisterDragDropWnd(std::shared_ptr<Wnd> wnd, const Pt& offset, std::shared_ptr<Wnd> originating_wnd);
    void           CancelDragDrop();             ///< clears the set of current drag-and-drop Wnds

    void           RegisterTimer(Timer& timer);  ///< adds \a timer to the list of active timers
    void           RemoveTimer(Timer& timer);    ///< removes \a timer from the list of active timers

    virtual void   Enter2DMode() = 0;            ///< saves any current GL state, sets up GG-friendly 2D drawing mode.  GG expects an orthographic projection, with the origin in the upper left corner, and with one unit of GL space equal to one pixel on the screen.
    virtual void   Exit2DMode() = 0;             ///< restores GL to its condition prior to Enter2DMode() call
    void           EnableFPS(bool b = true);     ///< turns FPS calulations on or off
    void           SetMaxFPS(double max);        ///< sets the maximum allowed FPS, so the render loop does not act as a spinlock when it runs very quickly.  0 indicates no limit.
    void           EnableKeyPressRepeat(unsigned int delay, unsigned int interval);         ///< delay and interval are in ms; Setting delay to 0 disables key press repeating completely.
    void           EnableMouseButtonDownRepeat(unsigned int delay, unsigned int interval);  ///< delay and interval are in ms; Setting delay to 0 disables mouse button-down repeating completely.
    void           SetDoubleClickInterval(unsigned int interval); ///< sets the maximum interval allowed between clicks that is still considered a double-click, in ms
    void           SetMinDragTime(unsigned int time);     ///< sets the minimum time (in ms) an item must be dragged before it is a valid drag
    void           SetMinDragDistance(unsigned int distance); ///< sets the minimum distance an item must be dragged before it is a valid drag

    /** Returns an iterator to the first defined keyboard accelerator. */
    accel_iterator accel_begin();

    /** Returns an iterator to one past the last defined keyboard accelerator. */
    accel_iterator accel_end();

    /** Establishes a keyboard accelerator.  Any key modifiers may be
        specified, or none at all. */
    void           SetAccelerator(Key key, Flags<ModKey> mod_keys = MOD_KEY_NONE);

    /** Removes a keyboard accelerator.  Any key modifiers may be specified,
        or none at all. */
    void           RemoveAccelerator(Key key, Flags<ModKey> mod_keys = MOD_KEY_NONE);

    /** Removes a keyboard accelerator. */
    void           RemoveAccelerator(accel_iterator it);

    /** Sets whether to emit keyboard accelerator signals while modal windows are open. */
    void           EnableModalAcceleratorSignals(bool allow = true);

    /** Sets whether to swap left and right mouse button events. */
    void           SetMouseLRSwapped(bool swapped = true);

    /** Sets which Key presses are substitued for actual key presses before event handling. */
    void           SetKeyMap(const std::map<Key, Key>& key_map);

    /** Returns a shared_ptr to the desired font, supporting all printable
        ASCII characters. */
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts);

    /** Returns a shared_ptr to the desired font, supporting all printable
        ASCII characters, from the in-memory contents \a file_contents. */
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  const std::vector<unsigned char>& file_contents);

    /** Returns a shared_ptr to the desired font, supporting all the
        characters in the UnicodeCharsets in the range [first, last). */
    template <class CharSetIter>
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  CharSetIter first, CharSetIter last);

    /** Returns a shared_ptr to the desired font, supporting all the
        characters in the UnicodeCharsets in the range [first, last), from the
        in-memory contents \a file_contents. */
    template <class CharSetIter>
    std::shared_ptr<Font> GetFont(const std::string& font_filename, unsigned int pts,
                                  const std::vector<unsigned char>& file_contents,
                                  CharSetIter first, CharSetIter last);

    /** Returns a shared_ptr to existing font \a font in a new size, \a pts. */
    std::shared_ptr<Font> GetFont(const std::shared_ptr<Font>& font, unsigned int pts);

    /** Removes the desired font from the managed pool; since shared_ptr's are
        used, the font may be deleted much later. */
    void                       FreeFont(const std::string& font_filename, unsigned int pts);

    /** Adds an already-constructed texture to the managed pool \warning
        calling code <b>must not</b> delete \a texture; the texture pool will
        do that. */
    std::shared_ptr<Texture> StoreTexture(Texture* texture, const std::string& texture_name);

    /** Adds an already-constructed texture to the managed pool. */
    std::shared_ptr<Texture> StoreTexture(const std::shared_ptr<Texture> &texture, const std::string& texture_name);

    /** Loads the requested texture from file \a name; mipmap textures are
      * generated if \a mipmap is true. */
    std::shared_ptr<Texture> GetTexture(const std::string& name, bool mipmap = false);

    /** Loads the requested texture from file \a name; mipmap textures are
      * generated if \a mipmap is true. */
    std::shared_ptr<Texture> GetTexture(const boost::filesystem::path& path, bool mipmap = false);

    /** Removes the desired texture from the managed pool; since shared_ptr's
      * are used, the texture may be deleted much later. */
    void                       FreeTexture(const std::string& name);

    /** Removes the desired texture from the managed pool; since shared_ptr's
      * are used, the texture may be deleted much later. */
    void                       FreeTexture(const boost::filesystem::path& path);

    /** Sets the currently-installed style factory. */
    void SetStyleFactory(const std::shared_ptr<StyleFactory>& factory);

    void RenderCursor(bool render); ///< set this to true iff the GUI should render the cursor

    /** Sets the currently-installed cursor. */
    void SetCursor(const std::shared_ptr<Cursor>& cursor);

    virtual bool SetClipboardText(const std::string& text); ///< sets text stored in clipboard
    bool CopyFocusWndText();                                ///< copies current focus Wnd as text to clipboard
    bool CopyWndText(const Wnd* wnd);                       ///< copies \a wnd as text to clipboard
    bool PasteFocusWndText(const std::string& text);        ///< attempts to paste \a text into the current focus Wnd
    bool PasteWndText(Wnd* wnd, const std::string& text);   ///< attempts to paste \a text into the Wnd \a wnd
    bool PasteFocusWndClipboardText();                      ///< attempts to paste clipboard contents into the current focus Wnd
    bool CutFocusWndText();                                 ///< copies current focus Wnd as text to clipboard, then pastes an empty string into that Wnd
    bool CutWndText(Wnd* wnd);                              ///< copies \a wnd as text to clipboard, then pastes an empty string to that Wnd
    bool FocusWndSelectAll();                               ///< selects all of anything selectable in the current focus Wnd
    bool WndSelectAll(Wnd* wnd);                            ///< selects all of anything selectable in the Wnd \a wnd
    bool FocusWndDeselect();                                ///< deselects anything selected in the current focus Wnd
    bool WndDeselect(Wnd* wnd);                             ///< deselects all of anything selectable in the Wnd \a wnd

    bool SetPrevFocusWndInCycle();                          ///< sets the focus Wnd to the next INTERACTIVE Wnd in a cycle determined by Wnd parent-child relationships
    bool SetNextFocusWndInCycle();                          ///< sets the focus Wnd to the next in the cycle.
    //@}

    static GUI*  GetGUI();                  ///< allows any GG code access to GUI framework by calling GUI::GetGUI()

    /** If \p wnd is visible recursively call PreRenderWindow() on all \p wnd's children and then
        call \p wnd->PreRender().  The order guarantees that when wnd->PreRender() is called all
        of \p wnd's children have already been prerendered.*/
    static void  PreRenderWindow(const std::shared_ptr<Wnd>& wnd);
    static void  PreRenderWindow(Wnd* wnd);
    static void  RenderWindow(const std::shared_ptr<Wnd>& wnd);    ///< renders a window (if it is visible) and all its visible descendents recursively
    static void  RenderWindow(Wnd* wnd);    ///< renders a window (if it is visible) and all its visible descendents recursively
    virtual void RenderDragDropWnds();      ///< renders Wnds currently being drag-dropped


    /** Emitted whenever the GUI's AppWidth() and/or AppHeight() change. */
    boost::signals2::signal<void (X, Y)>    WindowResizedSignal;

    /** Emitted whenever the GUI's window's left and top positions change. */
    boost::signals2::signal<void (X, Y)>    WindowMovedSignal;

    /** Emitted when the Window in which the GUI is operating gains or loses
      * focus. bool parameter is true when gaining focus, and false otherwise.*/
     boost::signals2::signal<void (bool)>   FocusChangedSignal;

    /** Emitted whenever the window manager requests the window close. */
    boost::signals2::signal<void ()>    WindowClosingSignal;

    /** Emitted whenever the app is requested to close. */
    boost::signals2::signal<void ()>    AppQuittingSignal;

    /** \name Exceptions */ ///@{
    /** The base class for GUI exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when an attempt is made to invoke either of the save- or
        load-window functions before they have been set. */
    GG_CONCRETE_EXCEPTION(BadFunctionPointer, GG::GUI, Exception);
    //@}

    /** Returns a list of resolutions that are supported for full-screen.
     * The format is [width]x[height] @ [bits per pixel].
     * This is not a conceptually ideal place for this function, but it needs to be
     * implemented using the underlying graphics system,
     * and deriving GUI is the way to connect GiGi to a graphics system, so here it is.
     */
    virtual std::vector<std::string> GetSupportedResolutions() const = 0;

    /** Returns the default resolution of the display */
    virtual Pt GetDefaultResolution(int display_id) const = 0;

protected:
    /** \name Structors */ ///@{
    GUI(const std::string& app_name); ///< protected ctor, called by derived classes
    //@}

    /** \name Mutators */ ///@{
    void           ProcessBrowseInfo();    ///< determines the current browse info mode, if any
    /** Allow all windows in the z-list to update data before rendering. */
    virtual void   PreRender();
    virtual void   RenderBegin() = 0;      ///< clears the backbuffer, etc.
    virtual void   Render();               ///< renders the windows in the z-list
    virtual void   RenderEnd() = 0;        ///< swaps buffers, etc.

    // EventPumpBase interface
    void SetFPS(double FPS);               ///< sets the FPS value based on the most recent calculation
    void SetDeltaT(unsigned int delta_t);  ///< sets the time between the most recent frame and the one before it, in ms

    //@}

    virtual void   Run() = 0;              ///< initializes GUI state, then executes main event handler/render loop (PollAndRender())

    /** Determine if the app has the mouse focus. */
    virtual bool AppHasMouseFocus() const { return true; };

private:
    bool           ProcessBrowseInfoImpl(Wnd* wnd);
    std::shared_ptr<Wnd>           ModalWindow() const;    // returns the current modal window, if any

    // Returns the window under \a pt, sending Mouse{Enter|Leave} or
    // DragDrop{Enter|Leave} as appropriate
    std::shared_ptr<Wnd>           CheckedGetWindowUnder(const Pt& pt, Flags<ModKey> mod_keys);

    static GUI*                       s_gui;
    std::unique_ptr<GUIImpl>          m_impl;

    friend class EventPumpBase; ///< allows EventPumpBase types to drive GUI
    friend struct GUIImpl;
};

/** Returns true if lwnd == rwnd or if lwnd contains rwnd */
GG_API bool MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd);
GG_API bool MatchesOrContains(const std::shared_ptr<Wnd>& lwnd, const Wnd* rwnd);
GG_API bool MatchesOrContains(const Wnd* lwnd, const std::shared_ptr<Wnd>& rwnd);
GG_API bool MatchesOrContains(const std::shared_ptr<Wnd>& lwnd, const std::shared_ptr<Wnd>& rwnd);

/* returns the storage value of mod_keys that should be used with keyboard
    accelerators the accelerators don't care which side of the keyboard you
    use for CTRL, SHIFT, etc., and whether or not the numlock or capslock are
    engaged.*/
GG_API Flags<ModKey> MassagedAccelModKeys(Flags<ModKey> mod_keys);


// template implementations
template<class InIt> 
bool GUI::OrCombiner::operator()(InIt first, InIt last) const
{
    bool retval = false;
    while (first != last)
        retval |= static_cast<bool>(*first++);
    return retval;
}

template <class CharSetIter>
std::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts,
                                   CharSetIter first, CharSetIter last)
{ return GetFontManager().GetFont(font_filename, pts, first, last); }

template <class CharSetIter>
std::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts,
                                   const std::vector<unsigned char>& file_contents,
                                   CharSetIter first, CharSetIter last)
{ return GetFontManager().GetFont(font_filename, pts, file_contents, first, last); }

} // namespace GG

#endif
