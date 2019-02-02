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

#include <GG/GUI.h>

#include <GG/BrowseInfoWnd.h>
#include <GG/Config.h>
#include <GG/Cursor.h>
#include <GG/Edit.h>
#include <GG/EventPump.h>
#include <GG/Layout.h>
#include <GG/ListBox.h>
#include <GG/StyleFactory.h>
#include <GG/Timer.h>
#include <GG/utf8/checked.h>
#include <GG/ZList.h>

#if GG_HAVE_LIBPNG
# if GIGI_CONFIG_USE_OLD_IMPLEMENTATION_OF_GIL_PNG_IO
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#  endif
#  include "gilext/io/png_io.hpp"
#  include "gilext/io/png_io_v2_compat.hpp"
#  if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
#   pragma GCC diagnostic pop
#  endif
# else
#  include <boost/gil/extension/io/png.hpp>
# endif
#endif

#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>

#include <thread>
#include <cassert>
#include <iostream>
#include <fstream>
#include <list>


using namespace GG;

namespace {
    const bool INSTRUMENT_GET_WINDOW_UNDER = false;

    struct AcceleratorEcho
    {
        AcceleratorEcho(Key key, Flags<ModKey> mod_keys) :
            m_str("GG SIGNAL : GUI::AcceleratorSignal(key=" +
                  boost::lexical_cast<std::string>(key) +
                  " mod_keys=" +
                  boost::lexical_cast<std::string>(mod_keys) +
                  ")")
        {}
        bool operator()()
        {
            std::cerr << m_str << std::endl;
            return false;
        }
        std::string m_str;
    };

    Key           KeyMappedKey(Key key, const std::map<Key, Key>& key_map)
    {
        auto it = key_map.find(key);
        if (it != key_map.end())
            return it->second;
        return key;
    }

    // calculates WndEvent::EventType corresponding to a given mouse button
    // and a given left mouse button event type. For example, given the 
    // left mouse button drag and button 2 (the right mouse button),
    // this will return right button drag.
    WndEvent::EventType ButtonEvent(WndEvent::EventType left_type, unsigned int mouse_button)
    { return WndEvent::EventType(left_type + (WndEvent::MButtonDown - WndEvent::LButtonDown) * mouse_button); }

    typedef utf8::wchar_iterator<std::string::const_iterator> utf8_wchar_iterator;
    typedef boost::xpressive::basic_regex<utf8_wchar_iterator> word_regex;
    typedef boost::xpressive::regex_iterator<utf8_wchar_iterator> word_regex_iterator;
    const wchar_t WIDE_DASH = '-';
    const word_regex DEFAULT_WORD_REGEX =
        +boost::xpressive::set[boost::xpressive::_w | WIDE_DASH];

    void WriteWndToPNG(const Wnd* wnd, const std::string& filename)
    {
#if GG_HAVE_LIBPNG
        Pt ul = wnd->UpperLeft();
        Pt size = wnd->Size();

        std::vector<GLubyte> bytes(Value(size.x) * Value(size.y) * 4);

        glFinish();

        glPushClientAttrib(GL_CLIENT_PIXEL_STORE_BIT);

        glPixelStorei(GL_PACK_SWAP_BYTES, false);
        glPixelStorei(GL_PACK_LSB_FIRST, false);
        glPixelStorei(GL_PACK_ROW_LENGTH, 0);
        glPixelStorei(GL_PACK_SKIP_ROWS, 0);
        glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        glReadPixels(Value(ul.x),
                     Value(GUI::GetGUI()->AppHeight() - wnd->Bottom()),
                     Value(size.x),
                     Value(size.y),
                     GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     &bytes[0]);

        glPopClientAttrib();

        using namespace boost::gil;
        write_view(
            filename,
            flipped_up_down_view(
                interleaved_view(
                    Value(size.x),
                    Value(size.y),
                    static_cast<rgba8_pixel_t*>(static_cast<void*>(&bytes[0])),
                    Value(size.x) * sizeof(rgba8_pixel_t))),
            png_tag());
#endif
    }
}


// implementation data types
struct GG::GUIImpl
{
    GUIImpl();

    void HandleMouseButtonPress(  unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleMouseDrag(         unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleMouseButtonRelease(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleIdle(             Flags<ModKey> mod_keys, const GG::Pt& pos, int curr_ticks);

    void HandleKeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);

    void HandleKeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);

    void HandleTextInput(        const std::string* text);
    void HandleMouseMove(        Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks);
    void HandleMouseWheel(       Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks);
    void HandleMouseEnter(       Flags<ModKey> mod_keys, const GG::Pt& pos, const std::shared_ptr<Wnd>& w);

    void ClearState();

    std::shared_ptr<Wnd> FocusWnd() const;
    void SetFocusWnd(const std::shared_ptr<Wnd>& wnd);

    std::string  m_app_name;              // the user-defined name of the apllication

    ZList        m_zlist;                 // object that keeps the GUI windows in the correct depth ordering
    std::weak_ptr<Wnd>         m_focus_wnd;             // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)
    std::list<std::pair<std::shared_ptr<Wnd>, std::weak_ptr<Wnd>>>
                 m_modal_wnds;            // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)
    bool         m_allow_modal_accelerator_signals; // iff true: keyboard accelerator signals will be output while modal window(s) is open

    bool         m_mouse_button_state[3]; // the up/down states of the three buttons on the mouse are kept here
    Pt           m_mouse_pos;             // absolute position of mouse, based on last MOUSEMOVE event
    Pt           m_mouse_rel;             // relative position of mouse, based on last MOUSEMOVE event
    Flags<ModKey>m_mod_keys;              // currently-depressed modifier keys, based on last KEYPRESS event

    int          m_key_press_repeat_delay;          // see note above GUI class definition
    int          m_key_press_repeat_interval;
    int          m_last_key_press_repeat_time;      // last time of a simulated key press message

    std::pair<Key, std::uint32_t> m_last_pressed_key_code_point;

    int          m_prev_key_press_time;             // the time of the most recent key press

    int          m_mouse_button_down_repeat_delay;      // see note above GUI class definition
    int          m_mouse_button_down_repeat_interval;
    int          m_last_mouse_button_down_repeat_time;  // last time of a simulated button-down message

    int          m_double_click_interval; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          m_min_drag_time;         // the minimum amount of time that a drag must be in progress before it is considered a drag, in ms
    int          m_min_drag_distance;     // the minimum distance that a drag must cover before it is considered a drag

    int          m_prev_mouse_button_press_time; // the time of the most recent mouse button press
    Pt           m_prev_mouse_button_press_pos;  // the location of the most recent mouse button press
    std::weak_ptr<Wnd>         m_prev_wnd_under_cursor; // GUI window most recently under the input cursor; may be 0
    int          m_prev_wnd_under_cursor_time; // the time at which prev_wnd_under_cursor was initially set to its current value
    std::weak_ptr<Wnd>         m_curr_wnd_under_cursor; // GUI window currently under the input cursor; may be 0
    std::weak_ptr<Wnd>         m_drag_wnds[3];          // GUI window currently being clicked or dragged by each mouse button
    Pt           m_prev_wnd_drag_position;// the upper-left corner of the dragged window when the last *Drag message was generated
    Pt           m_wnd_drag_offset;       // the offset from the upper left corner of the dragged window to the cursor for the current drag
    bool         m_curr_drag_wnd_dragged; // true iff the currently-pressed window (m_drag_wnds[N]) has actually been dragged some distance (in which case releasing the mouse button is not a click). note that a dragged wnd is one being continuously repositioned by the dragging, and not a wnd being drag-dropped.
    std::shared_ptr<Wnd>         m_curr_drag_wnd;         // nonzero iff m_curr_drag_wnd_dragged is true (that is, we have actually started dragging the Wnd, not just pressed the mouse button); will always be one of m_drag_wnds.
    std::weak_ptr<Wnd>         m_curr_drag_drop_here_wnd;// the Wnd that most recently received a DragDropEnter or DragDropHere message (0 if DragDropLeave was sent as well, or if none)
    Pt           m_wnd_resize_offset;     // offset from the cursor of either the upper-left or lower-right corner of the GUI window currently being resized
    WndRegion    m_wnd_region;            // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    /** The current browse info window, if any. */
    std::shared_ptr<BrowseInfoWnd> m_browse_info_wnd;

    int          m_browse_info_mode;      // the current browse info mode (only valid if browse_info_wnd is non-null)
    Wnd*         m_browse_target;         // the current browse info target

    std::weak_ptr<Wnd>         m_drag_drop_originating_wnd; // the window that originally owned the Wnds in drag_drop_wnds

    /** The Wnds currently being dragged and dropped. They are owned by the GUI and rendered separately.*/
    std::map<std::shared_ptr<Wnd>, Pt> m_drag_drop_wnds;

    /** Tracks whether Wnd is acceptable for dropping on the current target Wnd.*/
    std::map<const Wnd*, bool> m_drag_drop_wnds_acceptable;

    std::set<std::pair<Key, Flags<ModKey>>>
                 m_accelerators;          // the keyboard accelerators

    /** The signals emitted by the keyboard accelerators. */
    std::map<std::pair<Key, Flags<ModKey>>, std::shared_ptr<GUI::AcceleratorSignalType>> m_accelerator_sigs;

    bool         m_mouse_lr_swap;         // treat left and right mouse events as each other
    std::map<Key, Key>
                 m_key_map;               // substitute Key press events with different Key press events

    int          m_delta_t;               // the number of ms since the last frame
    bool         m_rendering_drag_drop_wnds;
    double       m_FPS;                   // the most recent calculation of the frames per second rendering speed (-1.0 if calcs are disabled)
    bool         m_calc_FPS;              // true iff FPS calcs are to be done
    double       m_max_FPS;               // the maximum allowed frames per second rendering speed

    Wnd*         m_double_click_wnd;      // GUI window most recently clicked
    unsigned int m_double_click_button;   // the index of the mouse button used in the last click
    int          m_double_click_start_time;// the time from which we started measuring double_click_time, in ms
    int          m_double_click_time;     // time elapsed since last click, in ms

    std::shared_ptr<StyleFactory> m_style_factory;
    bool                            m_render_cursor;
    std::shared_ptr<Cursor> m_cursor;

    std::set<Timer*>  m_timers;

    const Wnd* m_save_as_png_wnd;
    std::string m_save_as_png_filename;

    std::string m_clipboard_text;
};

GUIImpl::GUIImpl() :
    m_focus_wnd(),
    m_allow_modal_accelerator_signals(false),
    m_mouse_pos(X(-1000), Y(-1000)),
    m_mouse_rel(X(0), Y(0)),
    m_mod_keys(),
    m_key_press_repeat_delay(250),
    m_key_press_repeat_interval(66),
    m_last_key_press_repeat_time(0),
    m_last_pressed_key_code_point{GGK_UNKNOWN, 0u},
    m_prev_key_press_time(-1),
    m_mouse_button_down_repeat_delay(250),
    m_mouse_button_down_repeat_interval(66),
    m_last_mouse_button_down_repeat_time(0),
    m_double_click_interval(500),
    m_min_drag_time(250),
    m_min_drag_distance(5),
    m_prev_mouse_button_press_time(-1),
    m_prev_wnd_under_cursor(),
    m_prev_wnd_under_cursor_time(-1),
    m_curr_wnd_under_cursor(),
    m_drag_wnds(),
    m_prev_wnd_drag_position(Pt()),
    m_wnd_drag_offset(Pt()),
    m_curr_drag_wnd_dragged(false),
    m_curr_drag_wnd(nullptr),
    m_curr_drag_drop_here_wnd(),
    m_wnd_region(WR_NONE),
    m_browse_info_mode(0),
    m_browse_target(nullptr),
    m_drag_drop_originating_wnd(),
    m_mouse_lr_swap(false),
    m_delta_t(0),
    m_rendering_drag_drop_wnds(false),
    m_FPS(-1.0),
    m_calc_FPS(false),
    m_max_FPS(0.0),
    m_double_click_wnd(nullptr),
    m_double_click_button(0),
    m_double_click_start_time(-1),
    m_double_click_time(-1),
    m_style_factory(new StyleFactory()),
    m_render_cursor(false),
    m_cursor(),
    m_save_as_png_wnd(nullptr),
    m_clipboard_text()
{
    m_drag_wnds[0].reset();
    m_drag_wnds[1].reset();
    m_drag_wnds[2].reset();
    m_mouse_button_state[0] = m_mouse_button_state[1] = m_mouse_button_state[2] = false;
}

void GUIImpl::HandleMouseButtonPress(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    m_last_mouse_button_down_repeat_time = 0;
    m_prev_mouse_button_press_time = 0;
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;
    m_prev_wnd_under_cursor_time = curr_ticks;
    m_prev_mouse_button_press_time = curr_ticks;
    m_prev_mouse_button_press_pos = pos;

    m_mouse_button_state[mouse_button] = true;

    // If not already dragging, start tracking the dragged window.
    if (m_drag_wnds[0].expired() && m_drag_wnds[1].expired() && m_drag_wnds[2].expired()) {
        m_drag_wnds[mouse_button] = curr_wnd_under_cursor;
        if (curr_wnd_under_cursor) {
            m_prev_wnd_drag_position = curr_wnd_under_cursor->UpperLeft();
            m_wnd_drag_offset = pos - m_prev_wnd_drag_position;
        }
    }

    // if this window is not a disabled Control window, it becomes the focus window
    auto control = (curr_wnd_under_cursor
                    ? dynamic_cast<Control*>(curr_wnd_under_cursor.get())
                    : nullptr);
    if (control && !control->Disabled())
        SetFocusWnd(curr_wnd_under_cursor);

    if (curr_wnd_under_cursor) {
        m_wnd_region = curr_wnd_under_cursor->WindowRegion(pos); // and determine whether a resize-region of it is being dragged
        if (m_wnd_region % 3 == 0) // left regions
            m_wnd_resize_offset.x = curr_wnd_under_cursor->Left() - pos.x;
        else
            m_wnd_resize_offset.x = curr_wnd_under_cursor->Right() - pos.x;
        if (m_wnd_region < 3) // top regions
            m_wnd_resize_offset.y = curr_wnd_under_cursor->Top() - pos.y;
        else
            m_wnd_resize_offset.y = curr_wnd_under_cursor->Bottom() - pos.y;
        auto&& drag_wnds_root_parent = curr_wnd_under_cursor->RootParent();
        GUI::s_gui->MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : curr_wnd_under_cursor);
        curr_wnd_under_cursor->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonDown, mouse_button), pos, m_mod_keys));
    }

    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseDrag(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    const auto&& dragged_wnd = LockAndResetIfExpired(m_drag_wnds[mouse_button]);
    if (!dragged_wnd)
        return;

    if (m_wnd_region == WR_MIDDLE || m_wnd_region == WR_NONE) {
        // send drag message to window or initiate drag-and-drop

        Pt diff = m_prev_mouse_button_press_pos - pos;
        int drag_distance = Value(diff.x * diff.x) + Value(diff.y * diff.y);

        // check that sufficient time has passed and the mouse has moved far
        // enough since the previous drag motion response to signal the drag to
        // the UI
        if (m_min_drag_time < (curr_ticks - m_prev_mouse_button_press_time) &&
            m_min_drag_distance * m_min_drag_distance < drag_distance &&
            !m_drag_drop_wnds.count(dragged_wnd))
        {
            // several conditions to allow drag-and-drop to occur:
            if (!dragged_wnd->Dragable() &&             // normal-dragable non-drop wnds can't be drag-dropped
                dragged_wnd->DragDropDataType() != "" &&// Wnd must have a defined drag-drop data type to be drag-dropped
                mouse_button == 0)                      // left mouse button drag-drop only
            {
                auto&& parent = dragged_wnd->Parent();
                Pt offset = m_prev_mouse_button_press_pos - dragged_wnd->UpperLeft();
                // start drag
                GUI::s_gui->RegisterDragDropWnd(dragged_wnd, offset, parent);
                // inform parent
                if (parent)
                    parent->StartingChildDragDrop(dragged_wnd.get(), offset);
            } else {
                // can't drag-and-drop...

                // instead signal to dragged-over Wnd that a drag has occurred.
                Pt start_pos = dragged_wnd->UpperLeft();
                Pt move = (pos - m_wnd_drag_offset) - m_prev_wnd_drag_position;
                dragged_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDrag, mouse_button), pos, move, m_mod_keys));

                // update "latest" drag position, so future drags will have
                // proper position from which to judge distance dragged since
                // the lastdrag position
                m_prev_wnd_drag_position = dragged_wnd->UpperLeft();

                // if Wnd is draggable and position has changed, initiate or
                // update normal dragging (non-drop, such as repositioning a
                // Wnd without "dropping" it at the end)
                if (dragged_wnd->Dragable() && start_pos != dragged_wnd->UpperLeft()) {
                    m_curr_drag_wnd_dragged = true;
                    m_curr_drag_wnd = dragged_wnd;
                }
            }
        }

        // notify wnd under cursor of presence of drag-and-drop wnd(s)
        if ((m_curr_drag_wnd_dragged &&
             (dragged_wnd->DragDropDataType() != "") &&
             (mouse_button == 0)) ||
            !m_drag_drop_wnds.empty())
        {
            std::set<Wnd*> ignores;
            auto curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), &ignores);
            m_curr_wnd_under_cursor = curr_wnd_under_cursor;
            std::map<std::shared_ptr<Wnd>, Pt> drag_drop_wnds;
            drag_drop_wnds[dragged_wnd] = m_wnd_drag_offset;
            const auto&& prev_wnd_under_cursor = LockAndResetIfExpired(m_prev_wnd_under_cursor);
            if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) {
                // Wnd under cursor has remained the same for the last two updates
                const auto&& curr_drag_drop_here_wnd = LockAndResetIfExpired(m_curr_drag_drop_here_wnd);
                if (curr_drag_drop_here_wnd == curr_wnd_under_cursor) {
                    // Wnd being dragged over is still being dragged over...
                    WndEvent event(WndEvent::DragDropHere, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                } else {
                    // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
                    WndEvent event(WndEvent::CheckDrops, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // Wnd being dragged over is new; give it an Enter message
                    WndEvent enter_event(WndEvent::DragDropEnter, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(enter_event);
                    m_curr_drag_drop_here_wnd = curr_wnd_under_cursor;
                }
            }
        }

    } else if (dragged_wnd->Resizable()) {
        // send appropriate resize message to window, depending on the position
        // of the cursor within / at the edge of the Wnd being dragged over
        Pt offset_pos = pos + m_wnd_resize_offset;
        if (auto&& parent = dragged_wnd->Parent())
            offset_pos -= parent->ClientUpperLeft();
        GG::Pt rel_lr = dragged_wnd->RelativeLowerRight();
        GG::Pt rel_ul = dragged_wnd->RelativeUpperLeft();

        switch (m_wnd_region)
        {
        case WR_TOPLEFT:
            dragged_wnd->SizeMove(offset_pos,                       rel_lr);
            break;
        case WR_TOP:
            dragged_wnd->SizeMove(Pt(rel_ul.x,      offset_pos.y),  rel_lr);
            break;
        case WR_TOPRIGHT:
            dragged_wnd->SizeMove(Pt(rel_ul.x,      offset_pos.y),  Pt(offset_pos.x,    rel_lr.y));
            break;
        case WR_MIDLEFT:
            dragged_wnd->SizeMove(Pt(offset_pos.x,  rel_ul.y),      rel_lr);
            break;
        case WR_MIDRIGHT:
            dragged_wnd->SizeMove(rel_ul,                           Pt(offset_pos.x,    rel_lr.y));
            break;
        case WR_BOTTOMLEFT:
            dragged_wnd->SizeMove(Pt(offset_pos.x,  rel_ul.y),      Pt(rel_lr.x,        offset_pos.y));
            break;
        case WR_BOTTOM:
            dragged_wnd->SizeMove(rel_ul,                           Pt(rel_lr.x,        offset_pos.y));
            break;
        case WR_BOTTOMRIGHT:
            dragged_wnd->SizeMove(rel_ul,                           offset_pos);
            break;
        default:
            break;
        }
    }
}

void GUIImpl::HandleMouseButtonRelease(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    m_last_mouse_button_down_repeat_time = 0;
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;
    m_prev_wnd_under_cursor_time = curr_ticks;

    const auto&& click_drag_wnd = LockAndResetIfExpired(m_drag_wnds[mouse_button]);
    std::set<Wnd*> ignores;
    if (m_curr_drag_wnd_dragged && click_drag_wnd)
        ignores.insert(click_drag_wnd.get());
    curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), &ignores);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;

    bool in_drag_drop =
        !m_drag_drop_wnds.empty() ||
        (m_curr_drag_wnd_dragged && click_drag_wnd && (click_drag_wnd->DragDropDataType() != "") && (mouse_button == 0));

    m_mouse_button_state[mouse_button] = false;
    m_drag_wnds[mouse_button].reset(); // if the mouse button is released, stop tracking the drag window
    m_wnd_region = WR_NONE;        // and clear this, just in case

    if (!in_drag_drop && click_drag_wnd && curr_wnd_under_cursor == click_drag_wnd) {
        // the release is over the Wnd where the button-down event occurred
        // and that Wnd has not been dragged

        if (m_double_click_time > 0 && m_double_click_wnd == click_drag_wnd.get() &&
            // this is second click over a window that just received an click
            // within the time limit, so it's a double-click, not a click
            m_double_click_button == mouse_button)
        {
            m_double_click_wnd = nullptr;
            m_double_click_start_time = -1;
            m_double_click_time = -1;
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDoubleClick, mouse_button), pos, m_mod_keys));

        } else {
            // just a single click
            if (m_double_click_time > 0) {
                m_double_click_wnd = nullptr;
                m_double_click_start_time = -1;
                m_double_click_time = -1;
            } else {
                m_double_click_start_time = curr_ticks;
                m_double_click_time = 0;
                m_double_click_wnd = click_drag_wnd.get();
                m_double_click_button = mouse_button;
            }
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LClick, mouse_button), pos, m_mod_keys));
        }

    } else if (click_drag_wnd) {
        // drag-dropping
        m_double_click_wnd = nullptr;
        m_double_click_time = -1;
        if (click_drag_wnd)
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonUp, mouse_button), pos, m_mod_keys));

        if (curr_wnd_under_cursor) {
            // dropped onto a Wnd, which can react to the drop

            if (m_drag_drop_wnds.empty()) {
                // dropped a dragged Wnd without having dragged it anywhere yet
                if (click_drag_wnd && click_drag_wnd->DragDropDataType() != "" && mouse_button == 0) {
                    // pass drag-drop-here event to check if the single dragged Wnd is acceptable to drop
                    WndEvent event(WndEvent::CheckDrops, pos, click_drag_wnd.get(), m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // prep / handle end of drag-drop
                    auto&& drag_drop_originating_wnd = click_drag_wnd->Parent();
                    m_drag_drop_originating_wnd = drag_drop_originating_wnd;
                    curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                    m_curr_drag_drop_here_wnd.reset();

                    // put dragged Wnd into container depending on whether it is accepted by the drop target
                    std::vector<std::shared_ptr<Wnd>> accepted_wnds;
                    std::vector<Wnd*> removed_wnds;
                    std::vector<const Wnd*> unaccepted_wnds;
                    if (m_drag_drop_wnds_acceptable[click_drag_wnd.get()]) {
                        accepted_wnds.push_back(click_drag_wnd);
                        removed_wnds.push_back(click_drag_wnd.get());
                    }
                    else
                        unaccepted_wnds.push_back(click_drag_wnd.get());

                    // if dragged Wnd came from somehwere, inform originating
                    // Wnd its child is or is not being dragged away
                    if (drag_drop_originating_wnd) {
                        drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                        drag_drop_originating_wnd->ChildrenDraggedAway(removed_wnds, curr_wnd_under_cursor.get());
                    }
                    // implement drop onto target if the dragged Wnd was accepted
                    if (!accepted_wnds.empty())
                        curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDroppedOn, pos, std::move(accepted_wnds), m_mod_keys));
                }

            } else {
                // dragged one or more Wnds to another location and then dropped them
                // pass checkdrops event to check if the dropped Wnds are acceptable to drop here
                WndEvent event(WndEvent::CheckDrops, pos, std::move(m_drag_drop_wnds), m_mod_keys);
                curr_wnd_under_cursor->HandleEvent(event);
                m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                // prep / handle end of drag-drop
                auto&& drag_drop_originating_wnd = click_drag_wnd->Parent();
                m_drag_drop_originating_wnd = drag_drop_originating_wnd;
                curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                m_curr_drag_drop_here_wnd.reset();

                // put dragged Wnds into containers depending on whether they were accepted by the drop target
                std::vector<std::shared_ptr<Wnd>> accepted_wnds;
                std::vector<Wnd*> removed_wnds;
                std::vector<const Wnd*> unaccepted_wnds;
                for (auto& drop_wnd : m_drag_drop_wnds) {
                    if (m_drag_drop_wnds_acceptable[drop_wnd.first.get()]) {
                        accepted_wnds.push_back(std::move(drop_wnd.first));
                        removed_wnds.push_back(const_cast<Wnd*>(drop_wnd.first.get()));
                    } else
                        unaccepted_wnds.push_back(drop_wnd.first.get());
                }
                // if dragged Wnds came from somehwere, inform originating
                 // Wnd its children are or are not being dragged away
                if (drag_drop_originating_wnd) {
                    drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                    drag_drop_originating_wnd->ChildrenDraggedAway(removed_wnds, curr_wnd_under_cursor.get());
                }
                // implement drop onto target if any of the dragged Wnds were accepted
                if (!accepted_wnds.empty())
                    curr_wnd_under_cursor->HandleEvent(WndEvent(
                        WndEvent::DragDroppedOn, pos, std::move(accepted_wnds), m_mod_keys));
            }
        }
    }

    // Reset drag drop if drop on either originating window or a new window.
    if (click_drag_wnd) {
        m_prev_wnd_drag_position = Pt();
        m_drag_drop_originating_wnd.reset();
        m_drag_drop_wnds.clear();
        m_drag_drop_wnds_acceptable.clear();
        m_curr_drag_wnd_dragged = false;
        m_curr_drag_wnd.reset();
    }

    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleIdle(Flags<ModKey> mod_keys, const GG::Pt& pos, int curr_ticks)
{
    const auto&& curr_wnd_under_cursor = LockAndResetIfExpired(m_curr_wnd_under_cursor);
    if (m_mouse_button_down_repeat_delay != 0 &&
        curr_wnd_under_cursor &&
        curr_wnd_under_cursor == GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys) &&
        curr_wnd_under_cursor->RepeatButtonDown() &&
        LockAndResetIfExpired(m_drag_wnds[0]) == curr_wnd_under_cursor)
    {
        // convert to a key press message after ensuring that timing requirements are met
        if (curr_ticks - m_prev_mouse_button_press_time > m_mouse_button_down_repeat_delay) {
            if (!m_last_mouse_button_down_repeat_time ||
                curr_ticks - m_last_mouse_button_down_repeat_time > m_mouse_button_down_repeat_interval)
            {
                m_last_mouse_button_down_repeat_time = curr_ticks;
                curr_wnd_under_cursor->HandleEvent(WndEvent(
                    WndEvent::LButtonDown, pos, mod_keys));
            }
        }

        return;
    }

    auto&& focus_wnd = FocusWnd();
    if (m_key_press_repeat_delay != 0 &&
        m_last_pressed_key_code_point.first != GGK_UNKNOWN &&
        focus_wnd &&
        focus_wnd->RepeatKeyPress())
    {
        // convert to a key press message after ensuring that timing requirements are met
        if (curr_ticks - m_prev_key_press_time > m_key_press_repeat_delay) {
            if (!m_last_key_press_repeat_time ||
                curr_ticks - m_last_key_press_repeat_time > m_key_press_repeat_interval)
            {
                m_last_key_press_repeat_time = curr_ticks;
                focus_wnd->HandleEvent(
                    WndEvent(WndEvent::KeyPress, m_last_pressed_key_code_point.first,
                             m_last_pressed_key_code_point.second, mod_keys));
            }
        }
        return;
    }

    if (curr_wnd_under_cursor)
        GUI::s_gui->ProcessBrowseInfo();
}

void GUIImpl::HandleKeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks)
{
    key = KeyMappedKey(key, m_key_map);
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = nullptr;
    m_last_pressed_key_code_point = {key, key_code_point};
    m_last_key_press_repeat_time = 0;
    m_prev_key_press_time = curr_ticks;

    bool processed = false;
    // only process accelerators when there are no modal windows active;
    // otherwise, accelerators would be an end-run around modality
    if (m_modal_wnds.empty() || m_allow_modal_accelerator_signals) {
        // the focus_wnd may care about the state of the numlock and
        // capslock, or which side of the keyboard's CTRL, SHIFT, etc.
        // was pressed, but the accelerators don't
        Flags<ModKey> massaged_mods = MassagedAccelModKeys(mod_keys);
        if (m_accelerators.find({key, massaged_mods})
            != m_accelerators.end())
        {
            processed = GUI::s_gui->AcceleratorSignal(key, massaged_mods)();
        }
    }
    auto&& focus_wnd = FocusWnd();
    if (!processed && focus_wnd)
        focus_wnd->HandleEvent(WndEvent(
            WndEvent::KeyPress, key, key_code_point, mod_keys));
}

void GUIImpl::HandleKeyRelease(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks)
{
    key = KeyMappedKey(key, m_key_map);
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point.first = GGK_UNKNOWN;
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = nullptr;
    auto&& focus_wnd = FocusWnd();
    if (focus_wnd)
        focus_wnd->HandleEvent(WndEvent(
            WndEvent::KeyRelease, key, key_code_point, mod_keys));
}

void GUIImpl::HandleTextInput(const std::string* text) {
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = nullptr;
    auto&& focus_wnd = FocusWnd();
    if (focus_wnd)
        focus_wnd->HandleEvent(WndEvent(WndEvent::TextInput, text));
}

void GUIImpl::HandleMouseMove(Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    const auto&& prev_wnd_under_cursor = LockAndResetIfExpired(m_prev_wnd_under_cursor);

    m_mouse_pos = pos; // record mouse position
    m_mouse_rel = rel; // record mouse movement

    const auto&& m_drag_wnds_0 = LockAndResetIfExpired(m_drag_wnds[0]);
    const auto&& m_drag_wnds_1 = LockAndResetIfExpired(m_drag_wnds[1]);
    const auto&& m_drag_wnds_2 = LockAndResetIfExpired(m_drag_wnds[2]);
    if (m_drag_wnds_0 || m_drag_wnds_1 || m_drag_wnds_2) {
        if (m_drag_wnds_0)
            HandleMouseDrag(0, pos, curr_ticks);
        if (m_drag_wnds_1)
            HandleMouseDrag(1, pos, curr_ticks);
        if (m_drag_wnds_2)
            HandleMouseDrag(2, pos, curr_ticks);
    } else if (curr_wnd_under_cursor &&
               prev_wnd_under_cursor == curr_wnd_under_cursor)
    {
        // if !m_drag_wnds[0] and we're moving over the same
        // (valid) object we were during the last iteration
        curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseHere, pos, mod_keys));
        GUI::s_gui->ProcessBrowseInfo();
    }
    if (prev_wnd_under_cursor != curr_wnd_under_cursor) {
        m_browse_info_wnd.reset();
        m_browse_target = nullptr;
        m_prev_wnd_under_cursor_time = curr_ticks;
    }
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseWheel(Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;
    m_prev_wnd_under_cursor_time = curr_ticks;
    // don't send out 0-movement wheel messages
    if (curr_wnd_under_cursor && rel.y)
        curr_wnd_under_cursor->HandleEvent(WndEvent(
            WndEvent::MouseWheel, pos, Value(rel.y), mod_keys));
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseEnter(Flags< ModKey > mod_keys, const GG::Pt& pos, const std::shared_ptr<Wnd>& w)
{
    w->HandleEvent(WndEvent(WndEvent::MouseEnter, pos, mod_keys));
    m_curr_wnd_under_cursor = w;
}

std::shared_ptr<Wnd> GUIImpl::FocusWnd() const
{ return m_modal_wnds.empty() ? m_focus_wnd.lock() : m_modal_wnds.back().second.lock(); }

void GUIImpl::SetFocusWnd(const std::shared_ptr<Wnd>& wnd)
{
    auto&& focus_wnd = FocusWnd();
    if (focus_wnd == wnd)
        return;

    // inform old focus wnd that it is losing focus
    if (focus_wnd)
        focus_wnd->HandleEvent(WndEvent(WndEvent::LosingFocus));

    if (m_modal_wnds.empty())
        m_focus_wnd = wnd;
    else
        // m_modal_wnds stores the window that is modal and the child that has
        // focus separately.
        m_modal_wnds.back().second = wnd;

    // inform new focus wnd that it is gaining focus
    auto&& new_focus_wnd = FocusWnd();
    if (new_focus_wnd)
        new_focus_wnd->HandleEvent(WndEvent(WndEvent::GainingFocus));
}

void GUIImpl::ClearState()
{
    m_focus_wnd.reset();
    m_mouse_pos = GG::Pt(X(-1000), Y(-1000));
    m_mouse_rel = GG::Pt(X(0), Y(0));
    m_mod_keys = Flags<ModKey>();
    m_last_mouse_button_down_repeat_time = 0;
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point = {GGK_UNKNOWN, 0u};

    m_prev_wnd_drag_position = Pt();
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;

    m_prev_mouse_button_press_time = -1;
    m_prev_wnd_under_cursor.reset();
    m_prev_wnd_under_cursor_time = -1;
    m_curr_wnd_under_cursor.reset();

    m_mouse_button_state[0] = m_mouse_button_state[1] = m_mouse_button_state[2] = false;
    m_drag_wnds[0].reset();
    m_drag_wnds[1].reset();
    m_drag_wnds[2].reset();

    m_curr_drag_wnd_dragged = false;
    m_curr_drag_wnd = nullptr;
    m_curr_drag_drop_here_wnd.reset();
    m_wnd_region = WR_NONE;
    m_browse_target = nullptr;
    m_drag_drop_originating_wnd.reset();

    m_delta_t = 0;

    m_double_click_wnd = nullptr;
    m_double_click_start_time = -1;
    m_double_click_time = -1;
}

// static member(s)
GUI*                       GUI::s_gui = nullptr;

// member functions
GUI::GUI(const std::string& app_name) :
    // TODO:: use std::make_unique when switching to C++14
    m_impl(new GUIImpl())
{
    assert(!s_gui);
    s_gui = this;
    m_impl->m_app_name = app_name;
}

GUI::~GUI()
{
    s_gui = nullptr;
    Wnd::s_default_browse_info_wnd.reset();
}

const std::string& GUI::AppName() const
{ return m_impl->m_app_name; }

std::shared_ptr<Wnd> GUI::FocusWnd() const
{ return m_impl->FocusWnd(); }

bool GUI::FocusWndAcceptsTypingInput() const
{
    const auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return dynamic_cast<const Edit*>(focus_wnd.get());    // currently only Edit controls accept text input, so far as I'm aware. Could add a ->AcceptsTypingInput() function to Wnd if needed
}

std::shared_ptr<Wnd> GUI::PrevFocusInteractiveWnd() const
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return focus_wnd;

    auto&& parent_of_focus_wnd = focus_wnd->Parent();
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find previous INTERACTIVE sibling wnd
    const auto& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    const auto& focus_it = std::find(siblings.rbegin(), siblings.rend(), focus_wnd);
    if (focus_it == siblings.rend())
        return focus_wnd;

    // loop around until finding an interactive enabled control sibling or
    // returning to the focus wnd
    auto loop_it = focus_it;
    ++loop_it;
    while (loop_it != focus_it) {
        if (loop_it == siblings.rend()) {
            loop_it = siblings.rbegin();
            continue;
        }

        auto& sibling = *loop_it;
        if (sibling->Interactive()) {
            Control* ctrl = dynamic_cast<Control*>(sibling.get());
            if (ctrl && !ctrl->Disabled()) {
                return sibling;
                break;
            }
        }

        ++loop_it;
    }
    return focus_wnd;
}

std::shared_ptr<Wnd> GUI::NextFocusInteractiveWnd() const
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return focus_wnd;

    auto&& parent_of_focus_wnd = focus_wnd->Parent();
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find next INTERACTIVE sibling wnd
    const auto& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    auto focus_it = std::find(siblings.begin(), siblings.end(), focus_wnd);
    if (focus_it == siblings.end())
        return focus_wnd;

    // loop around until finding an interactive enabled control sibling or
    // returning to the focus wnd
    auto loop_it = focus_it;
    ++loop_it;
    while (loop_it != focus_it) {
        if (loop_it == siblings.end()) {
            loop_it = siblings.begin();
            continue;
        }

        auto& sibling = *loop_it;
        if (sibling->Interactive()) {
            Control* ctrl = dynamic_cast<Control*>(sibling.get());
            if (ctrl && !ctrl->Disabled()) {
                return sibling;
                break;
            }
        }


        ++loop_it;
    }
    return focus_wnd;
}

std::shared_ptr<Wnd> GUI::GetWindowUnder(const Pt& pt) const
{
    auto&& wnd = m_impl->m_zlist.Pick(pt, ModalWindow());
    if (INSTRUMENT_GET_WINDOW_UNDER && wnd)
        std::cerr << "GUI::GetWindowUnder() : " << wnd->Name() << " @ " << wnd << std::endl;

    return wnd;
}

unsigned int GUI::DeltaT() const
{ return m_impl->m_delta_t; }

bool GUI::RenderingDragDropWnds() const
{ return m_impl->m_rendering_drag_drop_wnds; }

bool GUI::FPSEnabled() const
{ return m_impl->m_calc_FPS; }

double GUI::FPS() const
{ return m_impl->m_FPS; }

std::string GUI::FPSString() const
{ return boost::io::str(boost::format("%.2f frames per second") % m_impl->m_FPS); }

double GUI::MaxFPS() const
{ return m_impl->m_max_FPS; }

unsigned int GUI::KeyPressRepeatDelay() const
{ return m_impl->m_key_press_repeat_delay; }

unsigned int GUI::KeyPressRepeatInterval() const
{ return m_impl->m_key_press_repeat_interval; }

unsigned int GUI::ButtonDownRepeatDelay() const
{ return m_impl->m_mouse_button_down_repeat_delay; }

unsigned int GUI::ButtonDownRepeatInterval() const
{ return m_impl->m_mouse_button_down_repeat_interval; }

unsigned int GUI::DoubleClickInterval() const
{ return m_impl->m_double_click_interval; }

unsigned int GUI::MinDragTime() const
{ return m_impl->m_min_drag_time; }

unsigned int GUI::MinDragDistance() const
{ return m_impl->m_min_drag_distance; }

bool GUI::DragWnd(const Wnd* wnd, unsigned int mouse_button) const
{ return wnd && wnd == LockAndResetIfExpired(m_impl->m_drag_wnds[mouse_button < 3 ? mouse_button : 0]).get(); }

bool GUI::DragDropWnd(const Wnd* wnd) const
{
    if (!wnd)
        return false;
    try {
        auto ptr = std::const_pointer_cast<Wnd>(wnd->shared_from_this());
        return m_impl->m_drag_drop_wnds.count(ptr);
    } catch (const std::bad_weak_ptr&) {
        return false;
    }
}

bool GUI::AcceptedDragDropWnd(const Wnd* wnd) const
{
    if (!wnd)
        return false;
    const auto& it = m_impl->m_drag_drop_wnds_acceptable.find(wnd);
    return it != m_impl->m_drag_drop_wnds_acceptable.end() && it->second;
}

bool GUI::MouseButtonDown(unsigned int bn) const
{ return (bn <= 2) ? m_impl->m_mouse_button_state[bn] : false; }

Pt GUI::MousePosition() const
{ return m_impl->m_mouse_pos; }

Pt GUI::MouseMovement() const
{ return m_impl->m_mouse_rel; }

Flags<ModKey> GUI::ModKeys() const
{ return m_impl->m_mod_keys; }

bool GUI::MouseLRSwapped() const
{ return m_impl->m_mouse_lr_swap; }

const std::map<Key, Key>& GUI::KeyMap() const
{ return m_impl->m_key_map; }

std::set<std::pair<CPSize, CPSize>> GUI::FindWords(const std::string& str) const
{
    std::set<std::pair<CPSize, CPSize>> retval;
    utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
    utf8_wchar_iterator last(str.end(), str.begin(), str.end());
    word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
    word_regex_iterator end_it;
    for ( ; it != end_it; ++it) {
        retval.insert(std::pair<CPSize, CPSize>(
                          CPSize(it->position()),
                          CPSize(it->position() + it->length())));
    }
    return retval;
}

std::set<std::pair<StrSize, StrSize>> GUI::FindWordsStringIndices(const std::string& str) const
{
    std::set<std::pair<StrSize, StrSize>> retval;

    utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
    utf8_wchar_iterator last(str.end(), str.begin(), str.end());
    word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
    word_regex_iterator end_it;

    typedef word_regex_iterator::value_type match_result_type;

    for ( ; it != end_it; ++it) {
        match_result_type match_result = *it;
        utf8_wchar_iterator word_pos_it = first;

        std::advance(word_pos_it, match_result.position());
        StrSize start_idx(std::distance(str.begin(), word_pos_it.base()));
        std::advance(word_pos_it, match_result.length());
        StrSize end_idx(std::distance(str.begin(), word_pos_it.base()));

        retval.insert({start_idx, end_idx});
    }
    return retval;
}

bool GUI::ContainsWord(const std::string& str, const std::string& word) const
{
    if (word.empty())
        return false;

    utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
    utf8_wchar_iterator last(str.end(), str.begin(), str.end());
    word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
    word_regex_iterator end_it;

    typedef word_regex_iterator::value_type match_result_type;

    for ( ; it != end_it; ++it) {
        match_result_type match_result = *it;
        utf8_wchar_iterator word_pos_it = first;

        std::advance(word_pos_it, match_result.position());
        auto start_it = word_pos_it.base();
        std::advance(word_pos_it, match_result.length());
        std::string word_in_str(start_it, word_pos_it.base());

        if (boost::iequals(word_in_str, word))
            return true;
    }

    return false;
}

const std::shared_ptr<StyleFactory>& GUI::GetStyleFactory() const
{ return m_impl->m_style_factory; }

bool GUI::RenderCursor() const
{ return m_impl->m_render_cursor; }

const std::shared_ptr<Cursor>& GUI::GetCursor() const
{ return m_impl->m_cursor; }

GUI::const_accel_iterator GUI::accel_begin() const
{ return m_impl->m_accelerators.begin(); }

GUI::const_accel_iterator GUI::accel_end() const
{ return m_impl->m_accelerators.end(); }

GUI::AcceleratorSignalType& GUI::AcceleratorSignal(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/) const
{
    std::shared_ptr<AcceleratorSignalType>& sig_ptr = m_impl->m_accelerator_sigs[{key, mod_keys}];
    if (!sig_ptr)
        sig_ptr.reset(new AcceleratorSignalType());
    if (INSTRUMENT_ALL_SIGNALS)
        sig_ptr->connect(AcceleratorEcho(key, mod_keys));
    return *sig_ptr;
}

bool GUI::ModalAcceleratorSignalsEnabled() const
{ return m_impl->m_allow_modal_accelerator_signals; }

bool GUI::ModalWndsOpen() const
{ return !m_impl->m_modal_wnds.empty(); }

void GUI::SaveWndAsPNG(const Wnd* wnd, const std::string& filename) const
{
    m_impl->m_save_as_png_wnd = wnd;
    m_impl->m_save_as_png_filename = filename;
}

void GUI::operator()()
{ Run(); }

void GUI::HandleGGEvent(EventType event, Key key, std::uint32_t key_code_point,
                        Flags<ModKey> mod_keys, const Pt& pos, const Pt& rel, const std::string* text)
{
    m_impl->m_mod_keys = mod_keys;

    int curr_ticks = Ticks();

    // track double-click time and time-out any pending double-click that has
    // outlived its interval
    if (m_impl->m_double_click_time >= 0) {
        m_impl->m_double_click_time = curr_ticks - m_impl->m_double_click_start_time;
        if (m_impl->m_double_click_time >= m_impl->m_double_click_interval) {
            m_impl->m_double_click_start_time = -1;
            m_impl->m_double_click_time = -1;
            m_impl->m_double_click_wnd = nullptr;
        }
    }

    switch (event) {

    case IDLE:
        m_impl->HandleIdle(mod_keys, pos, curr_ticks);
        break;

    case KEYPRESS:
        m_impl->HandleKeyPress(key, key_code_point, mod_keys, curr_ticks);
        break;

    case KEYRELEASE:
        m_impl->HandleKeyRelease(key, key_code_point, mod_keys, curr_ticks);
        break;

    case TEXTINPUT:
        m_impl->HandleTextInput(text);
        break;

    case MOUSEMOVE:
        m_impl->HandleMouseMove(mod_keys, pos, rel, curr_ticks);
        break;

    case LPRESS:
        m_impl->HandleMouseButtonPress((m_impl->m_mouse_lr_swap ? RPRESS : LPRESS) - LPRESS, pos, curr_ticks);
        break;

    case MPRESS:
        m_impl->HandleMouseButtonPress(MPRESS - LPRESS, pos, curr_ticks);
        break;

    case RPRESS:
        m_impl->HandleMouseButtonPress((m_impl->m_mouse_lr_swap ? LPRESS : RPRESS) - LPRESS, pos, curr_ticks);
        break;

    case LRELEASE:
        m_impl->HandleMouseButtonRelease((m_impl->m_mouse_lr_swap ? RRELEASE : LRELEASE) - LRELEASE, pos, curr_ticks);
        break;

    case MRELEASE:
        m_impl->HandleMouseButtonRelease(MRELEASE - LRELEASE, pos, curr_ticks);
        break;

    case RRELEASE:
        m_impl->HandleMouseButtonRelease((m_impl->m_mouse_lr_swap ? LRELEASE : RRELEASE) - LRELEASE, pos, curr_ticks);
        break;

    case MOUSEWHEEL:
        m_impl->HandleMouseWheel(mod_keys, pos, rel, curr_ticks);
        break;

    default:
        break;
    }
}

void GUI::ClearEventState()
{ m_impl->ClearState(); }

void GUI::SetFocusWnd(const std::shared_ptr<Wnd>& wnd)
{ m_impl->SetFocusWnd(wnd); }

bool GUI::SetPrevFocusWndInCycle()
{
    auto&& prev_wnd = PrevFocusInteractiveWnd();
    if (prev_wnd)
        SetFocusWnd(prev_wnd);
    return true;
}

bool GUI::SetNextFocusWndInCycle()
{
    auto&& next_wnd = NextFocusInteractiveWnd();
    if (next_wnd)
        SetFocusWnd(next_wnd);
    return true;
}

void GUI::Wait(unsigned int ms)
{ std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

void GUI::Wait(std::chrono::microseconds us)
{ std::this_thread::sleep_for(us); }

void GUI::Register(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;

    // Make top level by removing from parent
    if (auto&& parent = wnd->Parent())
        parent->DetachChild(wnd);

    m_impl->m_zlist.Add(std::forward<std::shared_ptr<Wnd>>(wnd));
}

void GUI::RegisterModal(std::shared_ptr<Wnd> wnd)
{
    if (wnd && wnd->Modal()) {
        m_impl->m_zlist.Remove(wnd.get());
        m_impl->m_modal_wnds.push_back({wnd, wnd});
        wnd->HandleEvent(WndEvent(WndEvent::GainingFocus));
    }
}

void GUI::Remove(const std::shared_ptr<Wnd>& wnd)
{
    if (!wnd)
        return;

    if (!m_impl->m_modal_wnds.empty() && m_impl->m_modal_wnds.back().first == wnd) // if it's the current modal window, remove it from the modal list
        m_impl->m_modal_wnds.pop_back();
    else // if it's not a modal window, remove it from the z-order
        m_impl->m_zlist.Remove(wnd);
}

void GUI::EnableFPS(bool b/* = true*/)
{
    m_impl->m_calc_FPS = b;
    if (!b)
        m_impl->m_FPS = -1.0f;
}

void GUI::SetMaxFPS(double max)
{
    if (max && max < 0.1)
        max = 0.1;
    m_impl->m_max_FPS = max;
}

void GUI::MoveUp(const std::shared_ptr<Wnd>& wnd)
{ if (wnd) m_impl->m_zlist.MoveUp(wnd); }

void GUI::MoveDown(const std::shared_ptr<Wnd>& wnd)
{ if (wnd) m_impl->m_zlist.MoveDown(wnd); }

std::shared_ptr<ModalEventPump> GUI::CreateModalEventPump(bool& done)
{ return std::make_shared<ModalEventPump>(done); }

void GUI::RegisterDragDropWnd(std::shared_ptr<Wnd> wnd, const Pt& offset, std::shared_ptr<Wnd> originating_wnd)
{
    assert(wnd);

    // Throw if all dragged wnds are not from the same original wnd.
    const auto&& drag_drop_originating_wnd = LockAndResetIfExpired(m_impl->m_drag_drop_originating_wnd);
    if (!m_impl->m_drag_drop_wnds.empty() && originating_wnd != drag_drop_originating_wnd) {
        std::string m_impl_orig_wnd_name("NULL");
        std::string orig_wnd_name("NULL");
        if (drag_drop_originating_wnd)
            m_impl_orig_wnd_name = drag_drop_originating_wnd->Name();
        if (originating_wnd)
            orig_wnd_name = originating_wnd->Name();
        throw std::runtime_error("GUI::RegisterDragDropWnd() : Attempted to register a drag drop item"
                                "dragged from  one window(" + orig_wnd_name + 
                                "), when another window (" + m_impl_orig_wnd_name +
                                ") already has items being dragged from it.");
    }

    m_impl->m_drag_drop_wnds[wnd] = offset;
    m_impl->m_drag_drop_wnds_acceptable[wnd.get()] = false;
    m_impl->m_drag_drop_originating_wnd = originating_wnd;
}

void GUI::CancelDragDrop()
{
    m_impl->m_drag_drop_wnds.clear();
    m_impl->m_drag_drop_wnds_acceptable.clear();
}

void GUI::RegisterTimer(Timer& timer)
{ m_impl->m_timers.insert(&timer); }

void GUI::RemoveTimer(Timer& timer)
{ m_impl->m_timers.erase(&timer); }

void GUI::EnableKeyPressRepeat(unsigned int delay, unsigned int interval)
{
    if (!delay) { // setting delay = 0 completely disables key press repeat
        m_impl->m_key_press_repeat_delay = 0;
        m_impl->m_key_press_repeat_interval = 0;
    } else {
        m_impl->m_key_press_repeat_delay = delay;
        m_impl->m_key_press_repeat_interval = interval;
    }
}

void GUI::EnableMouseButtonDownRepeat(unsigned int delay, unsigned int interval)
{
    if (!delay) { // setting delay = 0 completely disables mouse button down repeat
        m_impl->m_mouse_button_down_repeat_delay = 0;
        m_impl->m_mouse_button_down_repeat_interval = 0;
    } else {
        m_impl->m_mouse_button_down_repeat_delay = delay;
        m_impl->m_mouse_button_down_repeat_interval = interval;
    }
}

void GUI::SetDoubleClickInterval(unsigned int interval)
{ m_impl->m_double_click_interval = interval; }

void GUI::SetMinDragTime(unsigned int time)
{ m_impl->m_min_drag_time = time; }

void GUI::SetMinDragDistance(unsigned int distance)
{ m_impl->m_min_drag_distance = distance; }

GUI::accel_iterator GUI::accel_begin()
{ return m_impl->m_accelerators.begin(); }

GUI::accel_iterator GUI::accel_end()
{ return m_impl->m_accelerators.end(); }

void GUI::SetAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    m_impl->m_accelerators.insert({key, mod_keys});
}

void GUI::RemoveAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    m_impl->m_accelerators.erase({key, mod_keys});
}

void GUI::RemoveAccelerator(accel_iterator it)
{ m_impl->m_accelerators.erase(it); }

void GUI::EnableModalAcceleratorSignals(bool allow)
{ m_impl->m_allow_modal_accelerator_signals = allow; }

void GUI::SetMouseLRSwapped(bool swapped/* = true*/)
{ m_impl->m_mouse_lr_swap = swapped; }

void GUI::SetKeyMap(const std::map<Key, Key>& key_map)
{ m_impl->m_key_map = key_map; }

std::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts)
{ return GetFontManager().GetFont(font_filename, pts); }

std::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts,
                                   const std::vector<unsigned char>& file_contents)
{ return GetFontManager().GetFont(font_filename, pts, file_contents); }

std::shared_ptr<Font> GUI::GetFont(const std::shared_ptr<Font>& font, unsigned int pts)
{
    std::shared_ptr<Font> retval;
    if (font->FontName() == StyleFactory::DefaultFontName()) {
        retval = GetStyleFactory()->DefaultFont(pts);
    } else {
        retval = GetFont(font->FontName(), font->PointSize(),
                         font->UnicodeCharsets().begin(),
                         font->UnicodeCharsets().end());
    }
    return retval;
}

void GUI::FreeFont(const std::string& font_filename, unsigned int pts)
{ GetFontManager().FreeFont(font_filename, pts); }

std::shared_ptr<Texture> GUI::StoreTexture(Texture* texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

std::shared_ptr<Texture> GUI::StoreTexture(const std::shared_ptr<Texture>& texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

std::shared_ptr<Texture> GUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/)
{ return GetTextureManager().GetTexture(path, mipmap); }

void GUI::FreeTexture(const boost::filesystem::path& path)
{ GetTextureManager().FreeTexture(path); }

void GUI::SetStyleFactory(const std::shared_ptr<StyleFactory>& factory)
{
    m_impl->m_style_factory = factory;
    if (!m_impl->m_style_factory)
        m_impl->m_style_factory.reset(new StyleFactory());
}

void GUI::RenderCursor(bool render)
{ m_impl->m_render_cursor = render; }

void GUI::SetCursor(const std::shared_ptr<Cursor>& cursor)
{ m_impl->m_cursor = cursor; }

std::string GUI::ClipboardText() const
{ return m_impl->m_clipboard_text; }

bool GUI::SetClipboardText(const std::string& text)
{
    m_impl->m_clipboard_text = text;
    return true;
}

bool GUI::CopyFocusWndText()
{
    const auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return CopyWndText(focus_wnd.get());
}

bool GUI::CopyWndText(const Wnd* wnd)
{
    // presently only TextControl copying is supported.

    if (const TextControl* text_control = dynamic_cast<const TextControl*>(wnd)) {
        if (const Edit* edit_control = dynamic_cast<const Edit*>(text_control)) {
            // if TextControl is an Edit, it may have a subset of its text
            // selected. in that case, only copy the selected text. if nothing
            // is selected, revert to copying the full text of the TextControl.
            std::string selected_text = edit_control->SelectedText();
            if (!selected_text.empty()) {
                SetClipboardText(GG::Font::StripTags(selected_text));
                return true;
            }
        }
        SetClipboardText(text_control->Text());
        return true;
    }
    return false;
}

bool GUI::PasteFocusWndText(const std::string& text)
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return PasteWndText(focus_wnd.get(), text);
}

bool GUI::PasteWndText(Wnd* wnd, const std::string& text)
{
    // presently only pasting into Edit wnds is supported
    if (Edit* edit_control = dynamic_cast<Edit*>(wnd)) {
        edit_control->AcceptPastedText(text);
        return true;
    }
    return false;
}

bool GUI::PasteFocusWndClipboardText()
{
    return PasteFocusWndText(ClipboardText());
}

bool GUI::CutFocusWndText()
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return CutWndText(focus_wnd.get());
}

bool GUI::CutWndText(Wnd* wnd)
{
    return CopyWndText(wnd) && PasteWndText(wnd, "");
}

bool GUI::WndSelectAll(Wnd* wnd)
{
    if (!wnd)
        return false;
    if (Edit* edit_control = dynamic_cast<Edit*>(wnd)) {
        edit_control->SelectAll();
        return true;
    } else if (ListBox* list_control = dynamic_cast<ListBox*>(wnd)) {
        list_control->SelectAll(true);
        return true;
    }
    return false;
}

bool GUI::WndDeselect(Wnd* wnd)
{
    if (!wnd)
        return false;
    if (Edit* edit_control = dynamic_cast<Edit*>(wnd)) {
        edit_control->DeselectAll();
        return true;
    } else if (ListBox* list_control = dynamic_cast<ListBox*>(wnd)) {
        list_control->DeselectAll(true);
        return true;
    }
    return false;
}

bool GUI::FocusWndSelectAll()
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return WndSelectAll(focus_wnd.get());
}

bool GUI::FocusWndDeselect()
{
    auto&& focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return WndDeselect(focus_wnd.get());
}

GUI* GUI::GetGUI()
{ return s_gui; }

void GUI::PreRenderWindow(const std::shared_ptr<Wnd>& wnd)
{ PreRenderWindow(wnd.get()); }

void GUI::PreRenderWindow(Wnd* wnd)
{
    if (!wnd || !wnd->Visible())
        return;

    for (auto& child_wnd : wnd->m_children) {
        PreRenderWindow(child_wnd.get());
    }

    if (wnd->PreRenderRequired())
        wnd->PreRender();
}

void GUI::RenderWindow(const std::shared_ptr<Wnd>& wnd)
{ RenderWindow(wnd.get()); }

void GUI::RenderWindow(Wnd* wnd)
{
    if (!wnd || !wnd->Visible())
        return;

    wnd->Render();

    Wnd::ChildClippingMode clip_mode = wnd->GetChildClippingMode();

    if (clip_mode != Wnd::ClipToClientAndWindowSeparately) {
        bool clip = clip_mode != Wnd::DontClip;
        if (clip)
            wnd->BeginClipping();
        for (auto& child_wnd : wnd->m_children) {
            if (child_wnd && child_wnd->Visible())
                RenderWindow(child_wnd.get());
        }
        if (clip)
            wnd->EndClipping();
    } else {
        std::vector<std::shared_ptr<Wnd>> children_copy{wnd->m_children.begin(), wnd->m_children.end()};
        const auto& client_child_begin =
            std::partition(children_copy.begin(), children_copy.end(),
                           boost::bind(&Wnd::NonClientChild, _1));

        if (children_copy.begin() != client_child_begin) {
            wnd->BeginNonclientClipping();
            for (auto it = children_copy.begin(); it != client_child_begin; ++it) {
                if ((*it) && (*it)->Visible())
                    RenderWindow(it->get());
            }
            wnd->EndNonclientClipping();
        }

        if (client_child_begin != children_copy.end()) {
            wnd->BeginClipping();
            for (auto it = client_child_begin; it != children_copy.end(); ++it) {
                if ((*it) && (*it)->Visible())
                    RenderWindow(it->get());
            }
            wnd->EndClipping();
        }
    }

    if (wnd == GetGUI()->m_impl->m_save_as_png_wnd) {
        WriteWndToPNG(GetGUI()->m_impl->m_save_as_png_wnd, GetGUI()->m_impl->m_save_as_png_filename);
        GetGUI()->m_impl->m_save_as_png_wnd = nullptr;
        GetGUI()->m_impl->m_save_as_png_filename.clear();
    }
}

void GUI::RenderDragDropWnds()
{
    // render drag-and-drop windows in arbitrary order (sorted by pointer value)
    m_impl->m_rendering_drag_drop_wnds = true;
    for (const auto drop_wnd : m_impl->m_drag_drop_wnds) {
        bool old_visible = drop_wnd.first->Visible();
        if (!old_visible)
            drop_wnd.first->Show();
        auto&& drop_wnd_parent = drop_wnd.first->Parent();
        Pt parent_offset = drop_wnd_parent ? drop_wnd_parent->ClientUpperLeft() : Pt();
        Pt old_pos = drop_wnd.first->UpperLeft() - parent_offset;
        drop_wnd.first->MoveTo(m_impl->m_mouse_pos - parent_offset - drop_wnd.second);
        RenderWindow(drop_wnd.first.get());
        drop_wnd.first->MoveTo(old_pos);
        if (!old_visible)
            drop_wnd.first->Hide();
    }
    m_impl->m_rendering_drag_drop_wnds = false;
}

void GUI::ProcessBrowseInfo()
{
    auto&& wnd = LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor);
    assert(wnd);

    if (!m_impl->m_mouse_button_state[0] && !m_impl->m_mouse_button_state[1] && !m_impl->m_mouse_button_state[2] &&
        (m_impl->m_modal_wnds.empty() || wnd->RootParent() == m_impl->m_modal_wnds.back().first))
    {
        auto&& parent = wnd->Parent();
        while (!ProcessBrowseInfoImpl(wnd.get())
               && parent
               && (dynamic_cast<Control*>(wnd.get()) || dynamic_cast<Layout*>(wnd.get())))
        {
            wnd = std::move(parent);
            parent = wnd->Parent();
        }
    }
}

void GUI::PreRender()
{
    // pre-render normal windows back-to-front
    for (auto wnd : m_impl->m_zlist.RenderOrder()) {
        PreRenderWindow(wnd.get());
    }

    // pre-render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (const auto modal_wnd : m_impl->m_modal_wnds) {
        PreRenderWindow(modal_wnd.first.get());
    }

    // pre-render the active browse info window, if any
    const auto&& curr_wnd_under_cursor = LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor);
    if (m_impl->m_browse_info_wnd && curr_wnd_under_cursor) {
        assert(m_impl->m_browse_target);
        PreRenderWindow(m_impl->m_browse_info_wnd.get());
    }

    for (const auto& drag_drop_wnd : m_impl->m_drag_drop_wnds) {
        PreRenderWindow(drag_drop_wnd.first.get());
    }
}

void GUI::Render()
{
    // update timers
    int ticks = Ticks();
    for (auto& timer : m_impl->m_timers) {
        timer->Update(ticks);
    }

    Enter2DMode();
    // render normal windows back-to-front
    for (auto wnd : m_impl->m_zlist.RenderOrder()) {
        if (wnd)
            RenderWindow(wnd.get());
    }

    // render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (const auto modal_wnd : m_impl->m_modal_wnds) {
        if (modal_wnd.first)
            RenderWindow(modal_wnd.first.get());
    }

    // render the active browse info window, if any
    if (m_impl->m_browse_info_wnd) {
        const auto&& curr_wnd_under_cursor = LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor);
        if (!curr_wnd_under_cursor) {
            m_impl->m_browse_info_wnd.reset();
            m_impl->m_browse_info_mode = -1;
            m_impl->m_browse_target = nullptr;
            m_impl->m_prev_wnd_under_cursor_time = Ticks();
        } else {
            assert(m_impl->m_browse_target);
            m_impl->m_browse_info_wnd->Update(m_impl->m_browse_info_mode, m_impl->m_browse_target);
            RenderWindow(m_impl->m_browse_info_wnd.get());
        }
    }

    RenderDragDropWnds();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (m_impl->m_render_cursor && m_impl->m_cursor && AppHasMouseFocus())
        m_impl->m_cursor->Render(m_impl->m_mouse_pos);
    Exit2DMode();
}

bool GUI::ProcessBrowseInfoImpl(Wnd* wnd)
{
    bool retval = false;
    const std::vector<Wnd::BrowseInfoMode>& browse_modes = wnd->BrowseModes();
    if (!browse_modes.empty()) {
        unsigned int delta_t = Ticks() - m_impl->m_prev_wnd_under_cursor_time;
        std::size_t i = 0;
        for (auto it = browse_modes.rbegin();
             it != browse_modes.rend();
             ++it, ++i)
        {
            if (it->time < delta_t) {
                if (it->wnd && it->wnd->WndHasBrowseInfo(wnd, i)) {
                    if (m_impl->m_browse_target != wnd || m_impl->m_browse_info_wnd != it->wnd || m_impl->m_browse_info_mode != static_cast<int>(i)) {
                        m_impl->m_browse_target = wnd;
                        m_impl->m_browse_info_wnd = it->wnd;
                        m_impl->m_browse_info_mode = i;
                        m_impl->m_browse_info_wnd->SetCursorPosition(m_impl->m_mouse_pos);
                    }
                    retval = true;
                }
                break;
            }
        }
    }
    return retval;
}

std::shared_ptr<Wnd> GUI::ModalWindow() const
{
    if (!m_impl->m_modal_wnds.empty())
        return m_impl->m_modal_wnds.back().first;
    return nullptr;
}

std::shared_ptr<Wnd> GUI::CheckedGetWindowUnder(const Pt& pt, Flags<ModKey> mod_keys)
{
    const auto&& wnd_under_pt = GetWindowUnder(pt);
    const auto& dragged_wnd = m_impl->m_curr_drag_wnd; // wnd being continuously repositioned / dragged around, not a drag-drop

    //std::cout << "GUI::CheckedGetWindowUnder w: " << w << "  dragged_wnd: " << dragged_wnd << std::endl << std::flush;

    bool unregistered_drag_drop = dragged_wnd && !dragged_wnd->DragDropDataType().empty();
    bool registered_drag_drop = !m_impl->m_drag_drop_wnds.empty();

    const auto&& curr_drag_drop_here_wnd = LockAndResetIfExpired(m_impl->m_curr_drag_drop_here_wnd);
    if (curr_drag_drop_here_wnd && !unregistered_drag_drop && !registered_drag_drop) {
        curr_drag_drop_here_wnd->HandleEvent(WndEvent(WndEvent::DragDropLeave));
        m_impl->m_curr_drag_drop_here_wnd.reset();
    }

    const auto&& curr_wnd_under_cursor = LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor);
    if (wnd_under_pt == curr_wnd_under_cursor)
        return wnd_under_pt;   // same Wnd is under cursor as before; nothing to do

    if (curr_wnd_under_cursor) {
        // inform previous Wnd under the cursor that the cursor has been dragged away
        if (unregistered_drag_drop) {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
            m_impl->m_drag_drop_wnds_acceptable[dragged_wnd.get()] = false;
            m_impl->m_curr_drag_drop_here_wnd.reset();

        } else if (registered_drag_drop) {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
            for (auto& acceptable_wnd : m_impl->m_drag_drop_wnds_acceptable)
            { acceptable_wnd.second = false; }
            m_impl->m_curr_drag_drop_here_wnd.reset();

        } else {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseLeave));
        }
    }

    if (!wnd_under_pt) {
        //std::cout << "CheckedGetWindowUnder returning " << w << std::endl << std::flush;
        return wnd_under_pt;
    }


    // inform new Wnd under cursor that something was dragged over it
    if (unregistered_drag_drop) {
        // pass drag-drop event to check if the single dragged Wnd is acceptable to drop
        WndEvent event(WndEvent::CheckDrops, pt, dragged_wnd.get(), mod_keys);
        wnd_under_pt->HandleEvent(event);
        m_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::DragDropEnter, pt, dragged_wnd.get(), mod_keys);
        wnd_under_pt->HandleEvent(enter_event);
        m_impl->m_curr_drag_drop_here_wnd = wnd_under_pt;

    } else if (registered_drag_drop) {
        // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
        WndEvent event(WndEvent::CheckDrops, pt, m_impl->m_drag_drop_wnds, mod_keys);
        wnd_under_pt->HandleEvent(event);
        m_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::DragDropEnter, pt, m_impl->m_drag_drop_wnds, mod_keys);
        wnd_under_pt->HandleEvent(enter_event);
        m_impl->m_curr_drag_drop_here_wnd = wnd_under_pt;

    } else {
        m_impl->HandleMouseEnter(mod_keys, pt, wnd_under_pt);
    }

    //std::cout << "CheckedGetWindowUnder returning " << w << std::endl << std::flush;
    return wnd_under_pt;
}

void GUI::SetFPS(double FPS)
{ m_impl->m_FPS = FPS; }

void GUI::SetDeltaT(unsigned int delta_t)
{ m_impl->m_delta_t = delta_t; }

bool GG::MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd)
{
    // Note: this does not touch lwnd because it is used in WndDying and it may already be destroyed
    if (rwnd) {
        for (auto w = rwnd; w; w = w->Parent().get()) {
            if (w == lwnd)
                return true;
        }
    } else if (rwnd == lwnd) {
        return true;
    }
    return false;
}

bool GG::MatchesOrContains(const std::shared_ptr<Wnd>& lwnd, const Wnd* rwnd)
{ return MatchesOrContains(lwnd.get(), rwnd);}

bool GG::MatchesOrContains(const Wnd* lwnd, const std::shared_ptr<Wnd>& rwnd)
{ return MatchesOrContains(lwnd, rwnd.get());}

bool GG::MatchesOrContains(const std::shared_ptr<Wnd>& lwnd, const std::shared_ptr<Wnd>& rwnd)
{ return MatchesOrContains(lwnd.get(), rwnd.get());}

Flags<ModKey> GG::MassagedAccelModKeys(Flags<ModKey> mod_keys)
{
    mod_keys &= ~(MOD_KEY_NUM | MOD_KEY_CAPS);
    if (mod_keys & MOD_KEY_CTRL)
        mod_keys |= MOD_KEY_CTRL;
    if (mod_keys & MOD_KEY_SHIFT)
        mod_keys |= MOD_KEY_SHIFT;
    if (mod_keys & MOD_KEY_ALT)
        mod_keys |= MOD_KEY_ALT;
    if (mod_keys & MOD_KEY_META)
        mod_keys |= MOD_KEY_META;
    return mod_keys;
}
