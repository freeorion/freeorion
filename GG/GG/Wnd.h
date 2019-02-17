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

/** \file Wnd.h \brief Contains the Wnd class, upon which all GG GUI elements
    are based. */

#ifndef _GG_Wnd_h_
#define _GG_Wnd_h_

#include <GG/Base.h>
#include <GG/Exception.h>
#include <GG/Flags.h>

#include <boost/signals2/trackable.hpp>

#include <list>
#include <set>
#include <vector>
#include <memory>


namespace GG {

class BrowseInfoWnd;
class Layout;
class StyleFactory;
class Timer;
class WndEvent;


/** Wnd creation flags type. */
GG_FLAG_TYPE(WndFlag);

/** Clicks hit this window, rather than passing through it, and mouse-overs
    detect that they are over this window. */
extern GG_API const WndFlag INTERACTIVE;

/** When a mouse button is held down over this window, it expects to receive
    multiple *ButtonDown messages. */
extern GG_API const WndFlag REPEAT_BUTTON_DOWN;

/** This window can be dragged around independently. */
extern GG_API const WndFlag DRAGABLE;

/** This window can be resized by the user, with the mouse. */
extern GG_API const WndFlag RESIZABLE;

/** This windows is an "on-top" window, and will always appear above all
    non-on-top and non-modal windows.  Note that this only applies to
    top-level (Parent()-less) Wnds. */
extern GG_API const WndFlag ONTOP;

/** This window is modal; while it is active, no other windows are
    interactive.  Modal windows are considered above "on-top" windows, and
    should not be flagged as OnTop.  Note that this only applies to top-level
    (Parent()-less) Wnds. */
extern GG_API const WndFlag MODAL;

/** When a keyboard key is held down while this window has input focus, it
    expects to receive KeyPress messages. */
extern GG_API const WndFlag REPEAT_KEY_PRESS;

/** None of the above flags */
extern GG_API const WndFlag NO_WND_FLAGS;


/** \brief This is the basic GG window class.

    <h3>Window Geometry</h3>

    <br>The coordinates of Wnd boundaries are STL-style (as are most
    range-values throughout GG), meaning that LowerRight() denotes the "last +
    1" pixel of a Wnd.  The on-screen representation of a rectangular Wnd
    covers the pixels from UpperLeft() to LowerRight() - Pt(X1, Y1), \a not
    UpperLeft() to LowerRight().  Each Wnd has a client area from
    ClientUpperLeft() to ClientLowerRight().  These two methods are virtual,
    and may return anything the user likes; the default implementation is to
    return UpperLeft() and LowerRight(), respectively, meaning that the client
    area is the entire window.

    <h3>Child Windows</h3>

    <br>It is assumed that child windows exists within the boundaries of their
    parents, although this is not required.  By default, Wnds do not clip
    their children; child clipping can be turned on or off using
    SetChildClippingMode(), which clips all children to the client and/or
    non-client areas of the Wnd.  Subclasses can override BeginClippingImpl(),
    EndClippingImpl(), BeginNonclientClippingImpl(), and
    EndNonclientClippingImpl() if the clipping should be done using techniques
    other than scissor clipping and stencil testing, or if the Wnd is
    nonrectangular.  Regardless of clipping, all clicks that land on a child
    but outside of the parent will not reach the child, since clicks are
    detected by seaching the top-level Wnds and then searching the children
    within the ones that are hit.  Ideally, "sibling" child windows should not
    overlap (unless they can without interfering).  If this is impossible or
    undesirable, and control is needed over the order in which children are
    layered, MoveChildUp() and MoveChildDown() provide such control.

    <br>Windows are owned by the GUI, as top level or modal windows, owned by
    their parent windows as children, or during a drag drop operation jointly
    owned by the GUI and the originating/accepting window.  Changes of ownership
    are indicated by passing a shared_ptr.  Other objects should use weak_ptr to
    refer to windows that they do not wish to preserve beyond its natural demise.
    This avoids "leaking" a window by storing a shared_ptr to a window that is
    no longer part of the hierarchy.  This would prevent a window from being
    removed from memory, event processing, etc.

    <h3>Effects of Window-Creation Flags</h3>

    <br>Resizable() windows are able to be stretched by the user by dragging
    the areas of the window outside the client area.  So the RESIZABLE flag
    will have no effect on a window that does not have non-default
    ClientUpperLeft() and/or ClientLowerRight().  The WindowRegion() method
    can also be overidden in derived classes, and can return regions that are
    appropriate to nonrectangular windows, or those whose client area must
    cover the entire window.

    <br>OnTop() windows are drawn after all others (except Modal() ones), to
    ensure that they remain on top.  This means that other non-OnTop() windows
    that are moved to the top of the z-order stop at some z-value below the
    lowest OnTop() window in the z-order.  On-topness is useful for modeless
    dialog boxes, among other things.

    <br>Modal() windows are available (by setting the MODAL window creation
    flag), and are also always-on-top, but are handled differently and do not
    have ONTOP specified in their creation flags.  Modal windows are executed
    by calling Run(), which registers them as modal windows and starts the
    local execution of the GUI's event pump.  Execution of the code that calls
    Run() is effectively halted until Run() returns.  Derived classes that
    wish to use modal execution should set m_done = true to escape from the
    modal execution loop.  EventPump has more information about processing
    during modal dialog execution.

    <br>Note that OnTop() and Modal() flags only apply to top-level
    (Parent()-less) Wnds.

    <h3>Resource Management</h3>
    Wnd uses std::shared_ptr and std::weak_ptr to manage its resources.

    Parent windows point to their children with shared_ptr and own the children.
    All other pointers should be non-owning weak_ptr to prevent leaks and/or
    referencing windows already removed from the hierarchy.

    Each window uses shared_from_this() to refer to itself.  The internal
    weak_ptr from shared_from_this is not constructed until after the Wnd is
    assigned to at least one shared_ptr.  Consequently, neither AttachChild()
    nor SetLayout() can be called from within a constructor.

    A default factory function Create<T> is provided that creates the shared_ptr
    and then calls CompleteConstruction() in order to assemble the children.  A
    dervied class can override CompleteConstruction() to correctly assemble its
    children.

    <h3>Signal Considerations</h3>

    <br>Wnd inherits from boost::signals2::trackable.  This means that any
    slots contained in a Wnd object or Wnd-derived object will automatically
    be disconnected from any connected signals when the Wnd is destroyed.
    Every Wnd responds to input as driven by the singleton GUI object.

    <h3>Event Filters</h3>

    <br>Every Wnd can also have its incoming WndEvents filtered by an
    arbitrary number of other Wnds.  Each such Wnd in a Wnd's "filter chain"
    gets an opportunity, one at a time, to process an incoming WndEvent, or
    pass it on to the next filter in the chain.  If all EventFilter() calls in
    the chain return false, the filtered Wnd then gets the chance to process
    the WndEvent as normal.  Filter Wnds are traversed in reverse order that
    they are installed, and no filter Wnd can be in a filter chain more than
    once.  Installing the same filter Wnd more than once removes the Wnd from
    the filter chain and re-adds it to the beginning of the chain.  Note that
    the default implementation of EventFilter() is to return false and do
    nothing else, so installing a Wnd-derived type with no overridden
    EventFilter() in a filter Wnd will have no effect.  Also note that just as
    it is legal for keyboard accelerator slots to do nontrivial work and still
    return false (causing a keystroke event to be generated), EventFilter()
    may return false even when it does nontrivial work, and the next filter in
    the chain will also get a chance to process the WndEvent.  It is even
    possible to have an arbitrary number of filters that all do processing on
    an WndEvent, and finally let the filtered Wnd do its normal WndEvent
    processing.

    <h3>Layouts</h3>

    <br>Layouts arrange children in the client area of a window, and can be
    assigned to a window in 4 ways.  HorizontalLayout(), VerticalLayout(), and
    GridLayout() all arrange the window's client-area children automatically,
    and take ownership of them as their own children, becoming the window's
    only client-area child.  Any existing layout is removed first.
    SetLayout() allows you to attach a pre-configured Layout object directly,
    without automatically arranging the window's client-area children.
    Because SetLayout() does no auto-arrangement, it does not know how to
    place any client-area children the window may have at the time it is
    called; for this reason, it not only removes any previous layout, but
    deletes all current client-area children as well.  Therefore, SetLayout()
    should usually only be called before any client-area children are attached
    to a window; all client-area children should be attached directly to the
    layout.  <br>When a window has an attached layout and is resized, it
    resizes its layout automatically.  Further, if a window is part of a
    layout, it notifies its containing layout whenever it is moved, resized,
    or has its MinSize() changed.  This ensures that layouts are always
    current.  Note the use of the phrase "client-area children".  This refers
    to children entirely within the client area of the window.

    <h3>Frame Timing</h3>

    <br>Each frame is a cycle culminating in rendering the screen.  The
    following things happen, in order:
      + events from the GUI and the data sources are handled
      + PreRender() is called to update layout if required.
      + Render() is called.

    Wnd does not distinguish between events from the GUI and events from the
    data sources.

    PreRender() and Render() are only called if the window is visible.

    PreRender() defers all layout changes until after all events have been
    handled.  This improves performance by preventing event handling causing
    multiple layout actions between calls to Render().  PreRender() allows Wnd
    to perform any required expensive updates only once per frame.

    PreRender() prevents bugs caused by early events in a frame changing the
    layout so that later events trigger on a layout different from the layout
    visible to the player.  This is a change from the original GG behavior.
    Core parts of GG will continue to immediately update layout until
    peripheral parts are updated to expect deferred updates.

    The default implementation of PreRender() only clears the flag set by RequirePreRender().
    For a class to defer its layout to the prerender phase it needs to override PreRender() and
    implement its layout changes in PreRender().

    Legacy GG did not have a prerender phase and all layout changes were immediate.  Any Wnd class
    that does not override PreRender() will update layout immediately when executing mutating functions.

    <h3>Browse Info</h3>

    <br>Browse info is a non-interactive informational window that pops up
    after the user keeps the mouse over the Wnd for a certain period of time.
    This can reproduce "tooltip"-like functionality, but is not limited to
    displaying only text.  An arbitrary BrowseInfoWnd-derived window can be
    displayed.  There can be multiple browse info modes, numbered 0 through
    N - 1.  Each mode has a time associated with it, and after the associated
    time has elapsed, that mode is entered.  This is intended to allow
    different levels of detail to be shown for different lengths of mouse
    presence.  For instance, hovering over a Wnd for 1 second might produce a
    box that says "FooWnd", but leaving it there a total of 2 seconds might
    produce a box that says "FooWnd: currently doing nothing".  When the mouse
    leaves the Wnd, a click occurs, etc., the Wnd reverts to browse mode -1,
    indicating that no browse info should be displayed.  By default, every Wnd
    has a single browse info mode at time DefaultBrowseTime(), using the
    DefaultBrowseInfoWnd(), with no associated text.  Note that
    DefaultBrowseInfoWnd() returns a null window unless it is set by the user.
    As this implies, it is legal to have no BrowseInfoWnd associated with a
    browse mode, in which case nothing is shown.  Also note that it is legal
    to have no text associated with a browse mode. \see BrowseInfoWnd

    <h3>Style Factory</h3>

    <br>A StyleFactory is responsible for creating controls and dialogs that
    other Wnds may need (e.g. when Slider needs to create a Button for its
    sliding tab).  There is an GUI-wide StyleFactory available, but for
    complete customization, each Wnd may have one installed as well.  The
    GetStyleFactory() method returns the one installed in the Wnd, if one
    exists, or the GUI-wide one otherwise. */
class GG_API Wnd : public boost::signals2::trackable,
                   public std::enable_shared_from_this<Wnd>
{
public:
    /** \brief The data necessary to represent a browse info mode.

        Though \a browse_text will not apply to all browse info schemes, it is
        nevertheless part of BrowseInfoMode, since it will surely be the most
        common data displayed in a BrowseInfoWnd. */
    struct GG_API BrowseInfoMode
    {
        /** The time the cursor must linger over the Wnd before this mode
            becomes active, in ms. */
        unsigned int time;

        /** The BrowseInfoWnd used to display the browse info for this
            mode. */
        std::shared_ptr<BrowseInfoWnd> wnd;

        /** The text to display in the BrowseInfoWnd shown for this mode. */
        std::string text;
    };

    /** The type of the iterator parameters passed to DropsAcceptable(). */
    typedef std::map<const Wnd*, bool>::iterator DropsAcceptableIter;

    /** The modes of child clipping. */
    enum ChildClippingMode {
        /** No child clipping is performed. */
        DontClip,

        /** Children or parts of children that fall outside the client area
            are not visible. */
        ClipToClient,

        /** Children or parts of children that fall outside the window's area
            are not visible. */
        ClipToWindow,

        /** Acts as ClipToClient on children whose NonClientChild() member
            returns false.  For a child C whose NonClientChild() returns true,
            any part of C that is inside the client area or outside the
            window's area is not visible.  This mode is useful for Wnds that
            have client contents that should be clipped, but that also have
            nonclient children (e.g. minimize/maximize/close buttons). */
        ClipToClientAndWindowSeparately
    };

    /** \name Structors */ ///@{
    virtual ~Wnd();
    //@}

    /** Create is the default factory which allocates and configures a T type
        derived from Wnd.  It requires that the T constructor followed by
        T->CompleteConstruction() produce a correct T. */
    template <typename T, typename... Args>
    static std::shared_ptr<T> Create(Args&&... args)
    {
        // This intentionally doesn't use std::make_shared in order to make lazy cleanup of
        // weak_ptrs a low priority.

        // std::make_shared<T> might depending on
        // the stdlib implementation allocate a single block of memory for the
        // shared_ptr control block and T.  This is efficient in terms of
        // number of memory allocations.  However, after the shared_ptr count
        // decreases to zero any existing weak_ptrs will still prevent the
        // block of memory from being released.

        // std::shared_ptr<T>(new T()) allocates the memory for T and the
        // shared_ptr control block in two separate allocations.  When the
        // shared_ptr count decreases to zero the memory allocated for T is
        // immediately released.

        // Allocating shared_ptr in this manner means any floating weak_ptrs
        // will not prevent more that a smart pointer control block worth of
        // memory from being released.
        std::shared_ptr<T> wnd(new T(std::forward<Args>(args)...));
        wnd->CompleteConstruction();
        return wnd;
    }

    /** CompleteConstruction() should be overriden to complete construction of derived classes that
        need a fully formed weak_from_this() (e.g. to call AttachChild()) to be correctly constructed. */
    virtual void CompleteConstruction() {};

    /** \name Accessors */ ///@{
    /** Returns true iff a click over this window does not pass through.  Note
        that this also determines whether a mouse-over will detect this window
        or the ones under it. */
    bool Interactive() const;

    /** Returns true iff holding a keyboard key while this Wnd has the input
        focus generates multiple key-press messages. */
    bool RepeatKeyPress() const;

    /** Returns true iff holding a mouse button down over this Wnd generates
        multiple button-down messages. */
    bool RepeatButtonDown() const;

    /** Returns true iff this Wnd be dragged by the user. */
    bool Dragable() const;

    /** Returns true iff this Wnd can be resized by the user. */
    bool Resizable() const;

    /** Returns true iff this Wnd is an on-top Wnd. */
    bool OnTop() const;

    /** Returns true iff this Wnd is a modal Wnd. */
    bool Modal() const;

    /** Returns the mode to use for child clipping. */
    ChildClippingMode GetChildClippingMode() const;

    /** Returns true iff this Wnd should be considered a non-client-area child
        of its parent, for clipping purposes.  \see ChildClippingMode. */
    bool NonClientChild() const;

    /** Returns true iff this Wnd will be rendered if it is registered. */
    bool Visible() const;

    /** Returns true if this Wnd will be pre-rendered. */
    bool PreRenderRequired() const;

    /** Returns the name of this Wnd.  This name is not used by GG in any way;
        it only exists for user convenience. */
    const std::string& Name() const;

    /** Returns the string key that defines the type of data that this Wnd
        represents in drag-and-drop drags.  Returns an empty string when this
        Wnd cannot be drag-and-dropped. */
    const std::string& DragDropDataType() const;

    /** Returns the upper-left corner of window in \a screen \a coordinates
        (taking into account parent's screen position, if any) */
    Pt UpperLeft() const;
    X Left() const;
    Y Top() const;

    /** Returns (one pixel past) the lower-right corner of window in \a screen
        \a coordinates (taking into account parent's screen position, if
        any) */
    Pt LowerRight() const;
    X Right() const;
    Y Bottom() const;

    /** Returns the upper-left corner of window, relative to its parent's
        client area, or in screen coordinates if no parent exists. */
    Pt RelativeUpperLeft() const;

    /** Returns (one pixel past) the lower-right corner of window, relative to
        its parent's client area, or in screen coordinates if no parent
        exists. */
    Pt RelativeLowerRight() const;

    X Width() const;  ///< Returns width of window.
    Y Height() const; ///< Returns height of window.

    /** Returns a \a Pt packed with width in \a x and height in \a y. */
    Pt Size() const;

    Pt MinSize() const; ///< Returns the minimum allowable size of window.
    Pt MaxSize() const; ///< Returns the maximum allowable size of window.

    /** Returns the size of the minimum bounding box that can enclose the Wnd
        and still show all of its elements, plus enough room for interaction
        with those elements (if applicable).  For example, a TextControl's
        MinUsableSize() is just the area of its text, and a Scroll's
        MinUsableSize() is the combined sizes of its up-button, down-button,
        and tab (plus a bit of room in which to drag the tab). */
    virtual Pt MinUsableSize() const;

    /** Returns upper-left corner of window's client area in screen
        coordinates (or of the entire area, if no client area is specified).
        Virtual because different windows have different shapes (and so ways
        of calculating client area). */
    virtual Pt ClientUpperLeft() const;

    /** Returns (one pixel past) lower-right corner of window's client area in
        screen coordinates (or of the entire area, if no client area is
        specified).  Virtual because different windows have different shapes
        (and so ways of calculating client area). */
    virtual Pt ClientLowerRight() const;

    /** Returns the size of the client area \see Size(). */
    Pt ClientSize() const;

    X ClientWidth() const;  ///< Returns the width of the client area.
    Y ClientHeight() const; ///< Returns the height of the client area.

    /** Returns \a pt translated from screen- to window-coordinates. */
    Pt ScreenToWindow(const Pt& pt) const;

    /** Returns \a pt translated from screen- to client-coordinates. */
    Pt ScreenToClient(const Pt& pt) const;

    /** Returns true if screen-coordinate point \a pt falls within the
        window. */
    virtual bool InWindow(const Pt& pt) const;

    /** Returns true if screen-coordinate point \a pt falls within the
        window's client area. */
    virtual bool InClient(const Pt& pt) const;

    /** Returns child list; the list is const, but the children may be
        manipulated. */
    const std::list<std::shared_ptr<Wnd>>& Children() const;

    /** Returns the window's parent (may be null). */
    std::shared_ptr<Wnd> Parent() const;

    /** Returns the earliest ancestor window (may be null). */
    std::shared_ptr<Wnd> RootParent() const;

    /** Returns the layout for the window, if any. */
    std::shared_ptr<Layout> GetLayout() const;

    /** Returns the layout containing the window, if any. */
    Layout* ContainingLayout() const;

    /** Returns the browse modes for the Wnd, including time cutoffs (in
        milliseconds), the BrowseInfoWnds to be displayed for each browse info
        mode, and the text (if any) to be displayed in each mode.  As the time
        that the cursor is over this Wnd exceeds each mode's time, the
        corresponding Wnd is shown superimposed over this Wnd and its
        children.  Set the first time cutoff to 0 for immediate browse info
        display. */
    const std::vector<BrowseInfoMode>& BrowseModes() const;

    /** Returns the text to display for browse info mode \a mode.  \throw
        std::out_of_range May throw std::out_of_range if \a mode is not a
        valid browse mode. */
    const std::string& BrowseInfoText(std::size_t mode) const;

    /** Returns the currently-installed style factory if none exists, or the
        GUI-wide one otherwise. */
    const std::shared_ptr<StyleFactory>& GetStyleFactory() const;

    /** Returns the region under point \a pt. */
    virtual WndRegion WindowRegion(const Pt& pt) const;

    /** Adjusts \p ul and \p lr to correct for minsize and maxsize.*/
    void ClampRectWithMinAndMaxSize(Pt& ul, Pt& lr) const;
    //@}

    /** \name Mutators */ ///@{
    /** Sets the string key that defines the type of data that this Wnd
        represents in a drag-and-drop drag.  This should be set to the empty
        string when this Wnd cannot be used in drag-and-drop. */
    void SetDragDropDataType(const std::string& data_type);

    /** Indicates to the Wnd that a child Wnd \a wnd is being dragged in a
        drag-and-drop operation, which gives it the opportunity to add other
        associated drag-and-drop Wnds (see GUI::RegisterDragDropWnd()).  \a
        offset indicates the position of the mouse relative to \a wnd's
        UpperLeft(). */
    virtual void StartingChildDragDrop(const Wnd* wnd, const Pt& offset);

    /** When the user drops Wnds onto this Wnd, a DragDropHere event is
        generated, which determines which of the dropped Wnds are acceptable
        by the dropped-on Wnd by calling DropsAcceptable. The acceptable Wnds
        are then passed to AcceptDrops(), which handles the receipt of one or
        more drag-and-drop wnds into this Wnd.

        The shared_ptrs are passed by value to allow the compiler to move rvalues.
    */
    virtual void AcceptDrops(const Pt& pt, std::vector<std::shared_ptr<Wnd>> wnds, Flags<ModKey> mod_keys);

    /** Handles the cancellation of the dragging of one or more child windows,
        whose dragging was established by the most recent call to
        StartingChildDragDrop().  Note that even if an accepting Wnd accepts
        some but not all Wnds via DropsAcceptable(), this function will be
        called on those Wnds not accepted.  \note CancellingChildDragDrop()
        and ChildrenDraggedAway() are always called in that order, and are
        always called at the end of any drag-and-drop sequence performed on a
        child of this Wnd, whether the drag-and-drop is successful or not. */
    virtual void CancellingChildDragDrop(const std::vector<const Wnd*>& wnds);

    /** Handles the removal of one or more child windows that have been
        dropped onto another window which has accepted them as drops via
        DropsAcceptable().  The accepting window retains ownership.  \note
        CancellingChildDragDrop() and ChildrenDraggedAway() are always called
        in that order, and are always called at the end of any drag-and-drop
        sequence performed on a child of this Wnd, whether the drag-and-drop
        is successful or not. */
    virtual void ChildrenDraggedAway(const std::vector<Wnd*>& wnds, const Wnd* destination);

    /** Sets a name for this Wnd.  This name is not used by GG in any way; it
        only exists for user convenience. */
    void SetName(const std::string& name);

    /** Suppresses rendering of this window (and possibly its children) during
        render loop. */
    virtual void Hide();

    /** Enables rendering of this window (and possibly its children) during
        render loop. */
    virtual void Show();

    /** Called during Run(), after a modal window is registered, this is the
        place that subclasses should put specialized modal window
        initialization, such as setting focus to child controls. */
    virtual void ModalInit();

    /** Sets the mode to use for child clipping. */
    void SetChildClippingMode(ChildClippingMode mode);

    /** Sets whether this Wnd should be considered a non-client-area child of
        its parent, for clipping purposes.  \see ChildClippingMode. */
    void NonClientChild(bool b);

    void MoveTo(const Pt& pt);     ///< Moves upper-left corner of window to \a pt.
    void OffsetMove(const Pt& pt); ///< Moves window by \a pt pixels.

    /** Resizes and/or moves window to new upper-left and lower right
        boundaries. */
    virtual void SizeMove(const Pt& ul, const Pt& lr);

    /** Resizes window without moving upper-left corner. */
    void Resize(const Pt& sz);

    /** Sets the minimum allowable size of window \a pt. */
    virtual void SetMinSize(const Pt& sz);

    /** Sets the maximum allowable size of window \a pt. */
    virtual void SetMaxSize(const Pt& sz);

    /** Places \a wnd in child ptr list, sets's child's \a m_parent member to
        \a this. This takes ownership of \p wnd. */
    void AttachChild(std::shared_ptr<Wnd> wnd);

    /** Places \a wnd at the end of the child ptr list, so it is rendered last
        (on top of the other children). */
    void MoveChildUp(Wnd* wnd);
    void MoveChildUp(const std::shared_ptr<Wnd>& wnd);

    /** Places \a wnd at the beginning of the child ptr list, so it is
        rendered first (below the other children). */
    void MoveChildDown(Wnd* wnd);
    void MoveChildDown(const std::shared_ptr<Wnd>& wnd);

    /** Removes \a wnd from the child ptr list and resets child's m_parent. */
    void DetachChild(Wnd* wnd);
    void DetachChild(const std::shared_ptr<Wnd>& wnd);

    /** Remove \p wnd from the child ptr list and reset \p wnd. */
    template <typename T>
    void DetachChildAndReset(T& wnd)
    {
        DetachChild(wnd);
        wnd.reset();
    }

    /** Removes all Wnds from child ptr list and resets all childrens' m_parents. */
    void DetachChildren();

    /** Adds \a wnd to the front of the event filtering chain. */
    void InstallEventFilter(const std::shared_ptr<Wnd>& wnd);

    /** Removes \a wnd from the filter chain. */
    void RemoveEventFilter(const std::shared_ptr<Wnd>& wnd);

    /** Places the window's client-area children in a horizontal layout,
        handing ownership of the window's client-area children over to the
        layout.  Removes any current layout which may exist. */
    void HorizontalLayout();

    /** Places the window's client-area children in a vertical layout, handing
        ownership of the window's client-area children over to the layout.
        Removes any current layout which may exist. */
    void VerticalLayout();

    /** Places the window's client-area children in a grid layout, handing
        ownership of the window's client-area children over to the layout.
        Removes any current layout which may exist. */
    void GridLayout();

    /** Sets \a layout as the layout for the window.  Removes any current
        layout which may exist, and deletes all client-area child windows. */
    void SetLayout(const std::shared_ptr<Layout>& layout);

    /** Removes the window's layout, handing ownership of all its children
        back to the window, with the sizes and positions they had before the
        layout resized them.  If no layout exists for the window, no action is
        taken. */
    void RemoveLayout();

    /** Removes the window's layout, including all attached children, and
        returns it.  If no layout exists for the window, no action is
        taken. */
    std::shared_ptr<Layout> DetachLayout();

    /** Sets the margin that should exist between the outer edges of the
        windows in the layout and the edge of the client area.  If no layout
        exists for the window, this has no effect. */
    void SetLayoutBorderMargin(unsigned int margin);

    /** Sets the margin that should exist between the windows in the layout.
        If no layout exists for the window, this has no effect. */
    void SetLayoutCellMargin(unsigned int margin);

    /** Update Wnd prior to Render().

        PreRender() is called before Render() if RequirePreRender() was called.
        The default PreRender() resets the flag from RequirePreRender().
        Wnd::PreRender() should be called in any overrides to reset
        RequirePreRender().

        In the GUI processing loop the PreRender() of child windows whose
        RequirePreRender() flag is set will have been called before their
        parent PreRender(). */
    virtual void PreRender();

    /** Require that PreRender() be called to update layout before the next Render(). */
    virtual void RequirePreRender();

    /** Draws this Wnd.  Note that Wnds being dragged for a drag-and-drop
        operation are rendered twice -- once in-place as normal, once in the
        location of the drag operation, attached to the cursor.  Such Wnds may
        wish to render themselves differently in those two cases.  To
        determine which render is being performed, they can call
        GUI::GetGUI()->RenderingDragDropWnds(). */
    virtual void Render();

    /** This executes a modal window and gives it its modality.  For non-modal
        windows, this function is a no-op.  It returns false if the window is
        non-modal, or true after successful modal execution.*/
    virtual bool Run();

    /** Ends the current execution of Run(), if any. */
    virtual void EndRun();

    /** Sets the time cutoff (in milliseconds) for a browse info mode.  If \a
        mode is not less than the current number of modes, extra modes will be
        created as needed.  The extra nodes will be set to the value of the
        last time at the time the method is called, or \a time if there were
        initially no modes. */
    void SetBrowseModeTime(unsigned int time, std::size_t mode = 0);

    /** Sets the Wnd that is used to show browse info about this Wnd in the
        browse info mode \a mode.  \throw std::out_of_range May throw
        std::out_of_range if \a mode is not a valid browse mode. */
    void SetBrowseInfoWnd(const std::shared_ptr<BrowseInfoWnd>& wnd, std::size_t mode = 0);

    /** Removes the Wnd that is used to show browse info about this Wnd in the
        browse info mode \a mode (but does nothing to the mode itself).
        \throw std::out_of_range May throw std::out_of_range if \a mode is not
        a valid browse mode. */
    void ClearBrowseInfoWnd(std::size_t mode = 0);

    /** Sets the browse info window for mode \a mode to a Wnd with the
        specified color and border color which contains the specified text.
        \throw std::out_of_range May throw std::out_of_range if \a mode is not
        a valid browse mode. */
    void SetBrowseText(const std::string& text, std::size_t mode = 0);

    /** Sets the browse modes for the Wnd, including time cutoffs (in
        milliseconds), the BrowseInfoWnds to be displayed for each browse info
        mode, and the text (if any) to be displayed in each mode.  As the time
        that the cursor is over this Wnd exceeds each mode's time, the
        corresponding Wnd is shown superimposed over this Wnd and its
        children.  Set the first time cutoff to 0 for immediate browse info
        display. */
    void SetBrowseModes(const std::vector<BrowseInfoMode>& modes);

    /** Sets the currently-installed style factory. */
    void SetStyleFactory(const std::shared_ptr<StyleFactory>& factory);
    //@}


    /** Returns the single time to place in the browse modes during Wnd
        construction. */
    static unsigned int DefaultBrowseTime();

    /** Sets the single time to place in the browse modes during Wnd
        construction. */
    static void SetDefaultBrowseTime(unsigned int time);

    /** Returns the single BrowseInfoWnd to place in the browse modes during
        Wnd construction.  This returns a TextBoxBrowseInfoWnd with a default
        parameterization. */
    static const std::shared_ptr<BrowseInfoWnd>& DefaultBrowseInfoWnd();

    /** Sets the single BrowseInfoWnd to place in the browse modes during Wnd
        construction. */
    static void SetDefaultBrowseInfoWnd(const std::shared_ptr<BrowseInfoWnd>& browse_info_wnd);

    /** \name Exceptions */ ///@{
    /** The base class for Wnd exceptions. */
    GG_ABSTRACT_EXCEPTION(Exception);

    /** Thrown when a request to perform a layout fails due to child Wnds in
        illegal starting positions, or when a SetLayout() call would result in
        an illegal state. */
    GG_CONCRETE_EXCEPTION(BadLayout, GG::Wnd, Exception);
    //@}

protected:
    /** Sets the \a second member of each iterator to true or false,
        indicating whether the Wnd in the \a first member would be accepted if
        dropped on this Wnd at \a pt. */
    virtual void DropsAcceptable(DropsAcceptableIter first, DropsAcceptableIter last,
                                 const Pt& pt, Flags<ModKey> mod_keys) const;

    /** The states a Wnd may be in, with respect to drag-and-drop operations.
        Wnds may wish to consider the current state when rendering to provide
        visual feedback to the user. */
    enum DragDropRenderingState {
        /** No drag-and-drop is taking place at all with this Wnd. */
        NOT_DRAGGED,

        /** This Wnd is being dragged-and-dropped, but we're currently rendering
            the unmoving copy.  The dragged copy is rendered at another time. */
        IN_PLACE_COPY,

        /** This Wnd is being dragged-and-dropped, and it is currently over a
            drop target that \b will \b not accept it. */
        DRAGGED_OVER_UNACCEPTING_DROP_TARGET,

        /** This Wnd is being dragged-and-dropped, and it is currently over a
            drop target that \b will accept it. */
        DRAGGED_OVER_ACCEPTING_DROP_TARGET
    };

    /** \name Structors */ ///@{
    Wnd();

    /** Ctor that allows a size and position to be specified, as well as
        creation flags. */
    Wnd(X x, Y y, X w, Y h, Flags<WndFlag> flags = INTERACTIVE | DRAGABLE);
    //@}

    /** \name Accessors */ ///@{
    /** Returns the states the Wnd is in, with respect to drag-and-drop
        operations.  Wnds may wish to consider the current state when
        rendering to provide visual feedback to the user. */
    DragDropRenderingState GetDragDropRenderingState() const;
    //@}

    /** \name Mutators */ ///@{
    /** Respond to left button down msg.  A window receives this whenever any
        input device button changes from up to down while over the window.
        \note If this Wnd was created with the REPEAT_BUTTON_DOWN flag, this
        method may be called multiple times during a single button
        press-release cycle.  \see GG::GUI */
    virtual void LButtonDown(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to left button drag msg (even if this Wnd is not dragable).
        Drag messages are only sent to the window over which the button was
        pressed at the beginning of the drag. A window receives this whenever
        any input device button is down and the cursor is moving while over
        the window.  The window will also receive drag messages when the mouse
        is being dragged outside the window's area. */
    virtual void LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);

    /** Respond to release of left mouse button outside this Wnd, if it was
        originally depressed over this Wnd.  A Wnd will receive an LButtonUp()
        message whenever a drag that started over its area ends, even if the
        cursor is not currently over the window when this happens. */
    virtual void LButtonUp(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to release of left mouse button over this Wnd, if it was also
        originally depressed over this Wnd.  A Wnd will receive an LButtonUp()
        message whenever a drag that started over its area ends over its area
        as well. */
    virtual void LClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to second left click in window within the time limit.  A
        window will receive an LDoubleClick() message instead of an
        LButtonDown() or LClick() message if the left input device button is
        pressed over a window that was l-clicked within a double-click time
        interval.  Note that this means a double click is always preceded by a
        click.  For a double click to occur, no other window may have received
        a *Click() or *ButtonDown() message in during the interval. */
    virtual void LDoubleClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to middle button down msg.  \see LButtonDown() */
    virtual void MButtonDown(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to middle button drag msg (even if this Wnd is not dragable).
        \see LDrag() */
    virtual void MDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);

    /** Respond to release of middle mouse button outside this Wnd, if it was
        originally depressed over this Wnd.  \see LButtonUp()  */
    virtual void MButtonUp(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to release of middle mouse button over this Wnd, if it was
        also originally depressed over this Wnd.  \see LClick()  */
    virtual void MClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to second middle click in window within the time limit.  \see
        LDoubleClick() */
    virtual void MDoubleClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to right button down msg.  \see LButtonDown() */
    virtual void RButtonDown(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to right button drag msg (even if this Wnd is not dragable).
        \see LDrag() */
    virtual void RDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);

    /** Respond to release of right mouse button outside this Wnd, if it was
        originally depressed over this Wnd.  \see LButtonUp()  */
    virtual void RButtonUp(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to release of right mouse button over this Wnd, if it was also
        originally depressed over this Wnd.  \see LClick()  */
    virtual void RClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to second right click in window within the time limit.  \see
        LDoubleClick() */
    virtual void RDoubleClick(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to cursor entering window's coords. */
    virtual void MouseEnter(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to cursor moving about within the Wnd, or to cursor lingering
        within the Wnd for a long period of time.  A MouseHere() message will
        not be generated the first time the cursor enters the window's area.
        In that case, a MouseEnter() message is generated. */
    virtual void MouseHere(const Pt& pt, Flags<ModKey> mod_keys);

    /** Respond to cursor leaving window's coords. */
    virtual void MouseLeave();

    /** Respond to movement of the mouse wheel (move > 0 indicates the wheel
        is rolled up, < 0 indicates down) */
    virtual void MouseWheel(const Pt& pt, int move, Flags<ModKey> mod_keys);

    /** Respond to the cursor entering the Wnd's coords while dragging
        drag-and-drop Wnds.  \a drop_wnds_acceptable will have the bools
        set to true or valse to indicate whether this Wnd can accept the
        dragged wnds as a drop. */
    virtual void DragDropEnter(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                               Flags<ModKey> mod_keys);

    /** Respond to cursor moving about within the Wnd, or to cursor lingering
        within the Wnd for a long period of time, while dragging drag-and-drop
        Wnds.  A DragDropHere() message will not be generated the first time
        the cursor enters the window's area.  In that case, a DragDropEnter()
        message is generated.  \a drop_wnds_acceptable will have the bools
        set to true or valse to indicate whether this Wnd can accept the
        dragged wnds as a drop. */
    virtual void DragDropHere(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                              Flags<ModKey> mod_keys);

    /** Polls this Wnd about whether the Wnds in \a drop_wnds_acceptable will
        be accpeted by this Wnd by calling DropsAcceptable(...) */
    virtual void CheckDrops(const Pt& pt, std::map<const Wnd*, bool>& drop_wnds_acceptable,
                            Flags<ModKey> mod_keys);

    /** Respond to cursor leaving the Wnd's bounds while dragging
        drag-and-drop Wnds. */
    virtual void DragDropLeave();

    /** Respond to down-keystrokes (focus window only).  A window may receive
        KeyPress() messages passed up to it from its children.  For instance,
        Control-derived classes pass KeyPress() messages to their Parent()
        windows by default.  \note Though mouse clicks consist of a press and
        a release, all Control classes by default respond immediately to
        KeyPress(), not KeyRelease(); in fact, by default no Wnd class does
        anything at all on a KeyRelease event.  \note \a key_code_point will
        be zero if Unicode support is unavailable. */
    virtual void KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys);

    /** Respond to up-keystrokes (focus window only).  A window may receive
        KeyRelease() messages passed up to it from its children.  For
        instance, Control-derived classes pass KeyRelease() messages to their
        Parent() windows by default.  \note \a key_code_point will be zero if
        Unicode support is unavailable. */
    virtual void KeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys);

    /** Respond to text input regardless of the method. Focus window only.
        A window may receive TextInput() messages passed up to it from its
        children. */
    virtual void TextInput(const std::string* text);

    /** Respond to this window gaining the input focus. */
    virtual void GainingFocus();

    /** Respond to this window losing the input focus. */
    virtual void LosingFocus();

    /** Respond to Timer \a timer firing at time \a ticks. */
    virtual void TimerFiring(unsigned int ticks, Timer* timer);

    /** Handles an WndEvent destined for Wnd \a w, but which this Wnd is
        allowed to handle first.  Returns true if this filter processed the
        message. */
    virtual bool EventFilter(Wnd* w, const WndEvent& event);

    /** Handles all messages, and calls appropriate function (LButtonDown(),
        LDrag(), etc.). */
    void HandleEvent(const WndEvent& event);

    /** Sends the current event to Parent() for processing, if Parent() is
        non-null.  This must only be called from within a WndEvent handler
        (e.g. LClick()). */
    void ForwardEventToParent();

    /** Sets up child clipping for this window. */
    void BeginClipping();

    /** Restores state to what it was before BeginClipping() was called. */
    void EndClipping();

    /** Sets up non-client-area-only child clipping for this window.  \note
        This function is only useful when GetChildClippingMode() is
        ClipToClientAndWindowSeparately. */
    void BeginNonclientClipping();

    /** Restores state to what it was before BeginNonclientClipping() was
        called.  \note This function is only useful when
        GetChildClippingMode() is ClipToClientAndWindowSeparately. */
    void EndNonclientClipping();

    virtual void SetParent(const std::shared_ptr<Wnd>& wnd);
    //@}

    /** Modal Wnd's set this to true to stop modal loop. */
    bool m_done = false;

private:
    void ValidateFlags();              ///< Sanity-checks the window creation flags
    virtual void BeginClippingImpl(ChildClippingMode mode);
    virtual void EndClippingImpl(ChildClippingMode mode);
    virtual void BeginNonclientClippingImpl();
    virtual void EndNonclientClippingImpl();

    /** Code common to DetachChild and DetachChildren. */
    void DetachChildCore(Wnd* wnd);

    /// m_parent may be expired or null if there is no parent.  m_parent will reset itself if expired.
    mutable std::weak_ptr<Wnd>      m_parent;
    std::string                     m_name = "";                ///< A user-significant name for this Wnd
    std::list<std::shared_ptr<Wnd>> m_children;                 ///< List of ptrs to child windows kept in order of decreasing area
    bool                            m_visible = true;
    bool                            m_needs_prerender = false;  ///< Indicates if Wnd needs a PreRender();
    std::string                     m_drag_drop_data_type = ""; ///< The type of drag-and-drop data this Wnd represents, if any. If empty/blank, indicates that this Wnd cannot be drag-dropped.
    ChildClippingMode               m_child_clipping_mode;
    bool                            m_non_client_child = false;
    Pt                              m_upperleft;                ///< Upper left point of window
    Pt                              m_lowerright;               ///< Lower right point of window
    Pt                              m_min_size;                 ///< Minimum window size Pt(0, 0) (= none) by default
    Pt                              m_max_size;                 ///< Maximum window size Pt(1 << 30, 1 << 30) (= none) by default

    /** The Wnds that are filtering this Wnd's events. These are in reverse
        order: top of the stack is back(). */
    std::vector<std::weak_ptr<Wnd>> m_filters;

    std::set<std::weak_ptr<Wnd>, std::owner_less<std::weak_ptr<Wnd>>>
                                    m_filtering;                ///< The Wnds in whose filter chains this Wnd lies
    mutable std::weak_ptr<Layout>   m_layout;                   ///< The layout for this Wnd, if any
    mutable std::weak_ptr<Layout>   m_containing_layout;        ///< The layout that contains this Wnd, if any
    std::vector<BrowseInfoMode>     m_browse_modes;             ///< The browse info modes for this window

    /** The style factory to use when creating dialogs or child controls. */
    std::shared_ptr<StyleFactory>   m_style_factory;

    /** Flags supplied at window creation for clickability, dragability,
        resizability, etc. */
    Flags<WndFlag>                  m_flags;

    /** The default time to set for the first (and only) value in
        m_browse_mode_times during Wnd contruction */
    static unsigned int s_default_browse_time;

    /** The default BrowseInfoWmd to set for the first (and only) value in
        m_browse_mode_times during Wnd contruction */
    static std::shared_ptr<BrowseInfoWnd> s_default_browse_info_wnd;

    friend class GUI;   ///< GUI needs access to \a m_children, etc.
    friend struct GUIImpl;
    friend class Timer; ///< Timer needs to be able to call HandleEvent
};

} // namespace GG

#endif
