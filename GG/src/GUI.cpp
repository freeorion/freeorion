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
#include <GG/EventPump.h>
#include <GG/Layout.h>
#include <GG/StyleFactory.h>
#include <GG/Edit.h>
#include <GG/ListBox.h>
#include <GG/Timer.h>
#include <GG/ZList.h>
#include <GG/utf8/checked.h>

#if GG_HAVE_LIBPNG
# if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-local-typedefs"
# endif
# include "GIL/extension/io/png_io.hpp"
# if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 7)
#  pragma GCC diagnostic pop
# endif
#endif

#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive.hpp>

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

    Key           KeyMappedKey(Key key, const std::map<Key, Key>& key_map) {
        std::map<Key, Key>::const_iterator it = key_map.find(key);
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
        png_write_view(filename,
                       flipped_up_down_view(
                           interleaved_view(Value(size.x),
                                            Value(size.y),
                                            static_cast<rgba8_pixel_t*>(static_cast<void*>(&bytes[0])),
                                            Value(size.x) * sizeof(rgba8_pixel_t))));
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
    void HandleKeyPress(         Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);
    void HandleKeyRelease(       Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);
    void HandleTextInput(        const std::string* text);
    void HandleMouseMove(        Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks);
    void HandleMouseWheel(       Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks);
    void HandleMouseEnter(       Flags<ModKey> mod_keys, const GG::Pt& pos, Wnd* w);

    void ClearState();

    std::string  m_app_name;              // the user-defined name of the apllication

    ZList        m_zlist;                 // object that keeps the GUI windows in the correct depth ordering
    Wnd*         m_focus_wnd;             // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)
    std::list<std::pair<Wnd*, Wnd*> >
                 m_modal_wnds;            // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)
    bool         m_allow_modal_accelerator_signals; // iff true: keyboard accelerator signals will be output while modal window(s) is open

    bool         m_mouse_button_state[3]; // the up/down states of the three buttons on the mouse are kept here
    Pt           m_mouse_pos;             // absolute position of mouse, based on last MOUSEMOVE event
    Pt           m_mouse_rel;             // relative position of mouse, based on last MOUSEMOVE event
    Flags<ModKey>m_mod_keys;              // currently-depressed modifier keys, based on last KEYPRESS event

    int          m_key_press_repeat_delay;          // see note above GUI class definition
    int          m_key_press_repeat_interval;
    int          m_last_key_press_repeat_time;      // last time of a simulated key press message
    std::pair<Key, boost::uint32_t>
                 m_last_pressed_key_code_point;
    int          m_prev_key_press_time;             // the time of the most recent key press

    int          m_mouse_button_down_repeat_delay;      // see note above GUI class definition
    int          m_mouse_button_down_repeat_interval;
    int          m_last_mouse_button_down_repeat_time;  // last time of a simulated button-down message

    int          m_double_click_interval; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          m_min_drag_time;         // the minimum amount of time that a drag must be in progress before it is considered a drag, in ms
    int          m_min_drag_distance;     // the minimum distance that a drag must cover before it is considered a drag

    int          m_prev_mouse_button_press_time; // the time of the most recent mouse button press
    Pt           m_prev_mouse_button_press_pos;  // the location of the most recent mouse button press
    Wnd*         m_prev_wnd_under_cursor; // GUI window most recently under the input cursor; may be 0
    int          m_prev_wnd_under_cursor_time; // the time at which prev_wnd_under_cursor was initially set to its current value
    Wnd*         m_curr_wnd_under_cursor; // GUI window currently under the input cursor; may be 0
    Wnd*         m_drag_wnds[3];          // GUI window currently being clicked or dragged by each mouse button
    Pt           m_prev_wnd_drag_position;// the upper-left corner of the dragged window when the last *Drag message was generated
    Pt           m_wnd_drag_offset;       // the offset from the upper left corner of the dragged window to the cursor for the current drag
    bool         m_curr_drag_wnd_dragged; // true iff the currently-pressed window (m_drag_wnds[N]) has actually been dragged some distance (in which case releasing the mouse button is not a click). note that a dragged wnd is one being continuously repositioned by the dragging, and not a wnd being drag-dropped.
    Wnd*         m_curr_drag_wnd;         // nonzero iff m_curr_drag_wnd_dragged is true (that is, we have actually started dragging the Wnd, not just pressed the mouse button); will always be one of m_drag_wnds.
    Wnd*         m_curr_drag_drop_here_wnd;// the Wnd that most recently received a DragDropEnter or DragDropHere message (0 if DragDropLeave was sent as well, or if none)
    Pt           m_wnd_resize_offset;     // offset from the cursor of either the upper-left or lower-right corner of the GUI window currently being resized
    WndRegion    m_wnd_region;            // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    boost::shared_ptr<BrowseInfoWnd>
                 m_browse_info_wnd;       // the current browse info window, if any
    int          m_browse_info_mode;      // the current browse info mode (only valid if browse_info_wnd is non-null)
    Wnd*         m_browse_target;         // the current browse info target

    Wnd*         m_drag_drop_originating_wnd; // the window that originally owned the Wnds in drag_drop_wnds
    std::map<Wnd*, Pt>
                 m_drag_drop_wnds;        // the Wnds (and their offsets) that are being dragged and dropped between Wnds
    std::map<const Wnd*, bool>
                 m_drag_drop_wnds_acceptable; // the Wnds being dragged and dropped, and whether they are acceptable for dropping over their current target.

    std::set<std::pair<Key, Flags<ModKey> > >
                 m_accelerators;          // the keyboard accelerators

    std::map<std::pair<Key, Flags<ModKey> >, boost::shared_ptr<GUI::AcceleratorSignalType> >
                 m_accelerator_sigs;      // the signals emitted by the keyboard accelerators

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

    boost::shared_ptr<StyleFactory> m_style_factory;
    bool                            m_render_cursor;
    boost::shared_ptr<Cursor>       m_cursor;

    std::set<Timer*>  m_timers;

    const Wnd* m_save_as_png_wnd;
    std::string m_save_as_png_filename;

    std::string m_clipboard_text;
};

GUIImpl::GUIImpl() :
    m_focus_wnd(0),
    m_allow_modal_accelerator_signals(false),
    m_mouse_pos(X(-1000), Y(-1000)),
    m_mouse_rel(X(0), Y(0)),
    m_mod_keys(),
    m_key_press_repeat_delay(250),
    m_key_press_repeat_interval(66),
    m_last_key_press_repeat_time(0),
    m_last_pressed_key_code_point(std::make_pair(GGK_UNKNOWN, 0u)),
    m_prev_key_press_time(-1),
    m_mouse_button_down_repeat_delay(250),
    m_mouse_button_down_repeat_interval(66),
    m_last_mouse_button_down_repeat_time(0),
    m_double_click_interval(500),
    m_min_drag_time(250),
    m_min_drag_distance(5),
    m_prev_mouse_button_press_time(-1),
    m_prev_wnd_under_cursor(0),
    m_prev_wnd_under_cursor_time(-1),
    m_curr_wnd_under_cursor(0),
    m_drag_wnds(),
    m_curr_drag_wnd_dragged(false),
    m_curr_drag_wnd(0),
    m_curr_drag_drop_here_wnd(0),
    m_wnd_region(WR_NONE),
    m_browse_info_mode(0),
    m_browse_target(0),
    m_drag_drop_originating_wnd(0),
    m_mouse_lr_swap(false),
    m_delta_t(0),
    m_rendering_drag_drop_wnds(false),
    m_FPS(-1.0),
    m_calc_FPS(false),
    m_max_FPS(0.0),
    m_double_click_wnd(0),
    m_double_click_button(0),
    m_double_click_start_time(-1),
    m_double_click_time(-1),
    m_style_factory(new StyleFactory()),
    m_render_cursor(false),
    m_cursor(),
    m_save_as_png_wnd(0),
    m_clipboard_text()
{
    m_mouse_button_state[0] = m_mouse_button_state[1] = m_mouse_button_state[2] = false;
    m_drag_wnds[0] = m_drag_wnds[1] = m_drag_wnds[2] = 0;
}

void GUIImpl::HandleMouseButtonPress(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_last_mouse_button_down_repeat_time = 0;
    m_prev_wnd_drag_position = Pt();
    m_wnd_drag_offset = Pt();
    m_prev_mouse_button_press_time = 0;
    m_browse_info_wnd.reset();
    m_browse_target = 0;
    m_prev_wnd_under_cursor_time = curr_ticks;
    m_prev_mouse_button_press_time = curr_ticks;
    m_prev_mouse_button_press_pos = pos;

    m_mouse_button_state[mouse_button] = true;
    m_drag_wnds[mouse_button] = m_curr_wnd_under_cursor; // track this window as the one being dragged by this mouse button
    if (m_curr_wnd_under_cursor) {
        m_prev_wnd_drag_position = m_drag_wnds[mouse_button]->UpperLeft();
        m_wnd_drag_offset = pos - m_prev_wnd_drag_position;
    }

    // if this window is not a disabled Control window, it becomes the focus window
    Control* control = 0;
    if (m_drag_wnds[mouse_button] && (!(control = dynamic_cast<Control*>(m_drag_wnds[mouse_button])) || !control->Disabled()))
        GUI::s_gui->SetFocusWnd(m_drag_wnds[mouse_button]);

    if (m_drag_wnds[mouse_button]) {
        m_wnd_region = m_drag_wnds[mouse_button]->WindowRegion(pos); // and determine whether a resize-region of it is being dragged
        if (m_wnd_region % 3 == 0) // left regions
            m_wnd_resize_offset.x = m_drag_wnds[mouse_button]->Left() - pos.x;
        else
            m_wnd_resize_offset.x = m_drag_wnds[mouse_button]->Right() - pos.x;
        if (m_wnd_region < 3) // top regions
            m_wnd_resize_offset.y = m_drag_wnds[mouse_button]->Top() - pos.y;
        else
            m_wnd_resize_offset.y = m_drag_wnds[mouse_button]->Bottom() - pos.y;
        Wnd* drag_wnds_root_parent = m_drag_wnds[mouse_button]->RootParent();
        GUI::s_gui->MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : m_drag_wnds[mouse_button]);
        m_drag_wnds[mouse_button]->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonDown, mouse_button), pos, m_mod_keys));
    }

    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseDrag(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    GG::Wnd* dragged_wnd = m_drag_wnds[mouse_button];
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
            m_drag_drop_wnds.find(dragged_wnd) == m_drag_drop_wnds.end())
        {
            // several conditions to allow drag-and-drop to occur:
            if (!dragged_wnd->Dragable() &&             // normal-dragable non-drop wnds can't be drag-dropped
                dragged_wnd->DragDropDataType() != "" &&// Wnd must have a defined drag-drop data type to be drag-dropped
                mouse_button == 0)                      // left mouse button drag-drop only
            {
                Wnd* parent = dragged_wnd->Parent();
                Pt offset = m_prev_mouse_button_press_pos - dragged_wnd->UpperLeft();
                // start drag
                GUI::s_gui->RegisterDragDropWnd(dragged_wnd, offset, parent);
                // inform parent
                if (parent)
                    parent->StartingChildDragDrop(dragged_wnd, offset);
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
        if (m_curr_drag_wnd_dragged &&
            dragged_wnd->DragDropDataType() != "" &&
            mouse_button == 0 ||
            !m_drag_drop_wnds.empty())
        {
            std::set<Wnd*> ignores;
            m_curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), &ignores);
            std::map<Wnd*, Pt> drag_drop_wnds;
            drag_drop_wnds[dragged_wnd] = m_wnd_drag_offset;

            if (m_curr_wnd_under_cursor && m_prev_wnd_under_cursor == m_curr_wnd_under_cursor) {
                // Wnd under cursor has remained the same for the last two updates
                if (m_curr_drag_drop_here_wnd && m_curr_drag_drop_here_wnd == m_curr_wnd_under_cursor) {
                    // Wnd being dragged over is still being dragged over...
                    WndEvent event(WndEvent::DragDropHere, pos, m_drag_drop_wnds, m_mod_keys);
                    m_curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                } else {
                    // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
                    WndEvent event(WndEvent::CheckDrops, pos, m_drag_drop_wnds, m_mod_keys);
                    m_curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // Wnd being dragged over is new; give it an Enter message
                    WndEvent enter_event(WndEvent::DragDropEnter, pos, m_drag_drop_wnds, m_mod_keys);
                    m_curr_wnd_under_cursor->HandleEvent(enter_event);
                    m_curr_drag_drop_here_wnd = m_curr_wnd_under_cursor;
                }
            }
        }

    } else if (dragged_wnd->Resizable()) {
        // send appropriate resize message to window, depending on the position
        // of the cursor within / at the edge of the Wnd being dragged over
        Pt offset_pos = pos + m_wnd_resize_offset;
        if (Wnd* parent = dragged_wnd->Parent())
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
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_last_mouse_button_down_repeat_time = 0;
    m_prev_wnd_drag_position = Pt();
    m_browse_info_wnd.reset();
    m_browse_target = 0;
    m_prev_wnd_under_cursor_time = curr_ticks;

    Wnd* click_wnd = m_drag_wnds[mouse_button];
    std::set<Wnd*> ignores;
    if (m_curr_drag_wnd_dragged)
        ignores.insert(click_wnd);
    m_curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), &ignores);

    bool in_drag_drop =
        !m_drag_drop_wnds.empty() ||
        m_curr_drag_wnd_dragged && click_wnd && click_wnd->DragDropDataType() != "" && mouse_button == 0;

    m_mouse_button_state[mouse_button] = false;
    m_drag_wnds[mouse_button] = 0; // if the mouse button is released, stop the tracking the drag window
    m_wnd_region = WR_NONE;        // and clear this, just in case

    if (!in_drag_drop && click_wnd && m_curr_wnd_under_cursor == click_wnd) {
        // the release is over the Wnd where the button-down event occurred
        // and that Wnd has not been dragged

        if (m_double_click_time > 0 && m_double_click_wnd == click_wnd &&
            // this is second click over a window that just received an click
            // within the time limit, so it's a double-click, not a click
            m_double_click_button == mouse_button)
        {
            m_double_click_wnd = 0;
            m_double_click_start_time = -1;
            m_double_click_time = -1;
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDoubleClick, mouse_button), pos, m_mod_keys));

        } else {
            // just a single click
            if (m_double_click_time > 0) {
                m_double_click_wnd = 0;
                m_double_click_start_time = -1;
                m_double_click_time = -1;
            } else {
                m_double_click_start_time = curr_ticks;
                m_double_click_time = 0;
                m_double_click_wnd = click_wnd;
                m_double_click_button = mouse_button;
            }
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LClick, mouse_button), pos, m_mod_keys));
        }

    } else {
        // drag-dropping
        m_double_click_wnd = 0;
        m_double_click_time = -1;
        if (click_wnd)
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonUp, mouse_button), pos, m_mod_keys));

        if (m_curr_wnd_under_cursor) {
            // dropped onto a Wnd, which can react to the drop

            if (m_drag_drop_wnds.empty()) {
                // dropped a dragged Wnd without having dragged it anywhere yet
                if (click_wnd && click_wnd->DragDropDataType() != "" && mouse_button == 0) {
                    // pass drag-drop-here event to check if the single dragged Wnd is acceptable to drop
                    WndEvent event(WndEvent::CheckDrops, pos, click_wnd, m_mod_keys);
                    m_curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // prep / handle end of drag-drop
                    m_drag_drop_originating_wnd = click_wnd->Parent();
                    m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                    m_curr_drag_drop_here_wnd = 0;

                    // put dragged Wnd into container depending on whether it is accepted by the drop target
                    std::vector<Wnd*> accepted_wnds;
                    std::vector<const Wnd*> unaccepted_wnds;
                    if (m_drag_drop_wnds_acceptable[click_wnd])
                        accepted_wnds.push_back(click_wnd);
                    else
                        unaccepted_wnds.push_back(click_wnd);

                    // if dragged Wnd came from somehwere, inform originating
                    // Wnd its child is or is not being dragged away
                    if (m_drag_drop_originating_wnd) {
                        m_drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                        m_drag_drop_originating_wnd->ChildrenDraggedAway(accepted_wnds, m_curr_wnd_under_cursor);
                    }
                    // implement drop onto target if the dragged Wnd was accepted
                    if (!accepted_wnds.empty())
                        m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDroppedOn, pos, accepted_wnds, m_mod_keys));
                }

            } else {
                // dragged one or more Wnds to another location and then dropped them
                // pass checkdrops event to check if the dropped Wnds are acceptable to drop here
                WndEvent event(WndEvent::CheckDrops, pos, m_drag_drop_wnds, m_mod_keys);
                m_curr_wnd_under_cursor->HandleEvent(event);
                m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                // prep / handle end of drag-drop
                m_drag_drop_originating_wnd = click_wnd->Parent();
                m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                m_curr_drag_drop_here_wnd = 0;

                // put dragged Wnds into containers depending on whether they were accepted by the drop target
                std::vector<Wnd*> accepted_wnds;
                std::vector<const Wnd*> unaccepted_wnds;
                for (std::map<const Wnd*, bool>::iterator it = m_drag_drop_wnds_acceptable.begin();
                     it != m_drag_drop_wnds_acceptable.end(); ++it)
                {
                    if (it->second)
                        accepted_wnds.push_back(const_cast<Wnd*>(it->first));
                    else
                        unaccepted_wnds.push_back(it->first);
                }
                // if dragged Wnds came from somehwere, inform originating
                 // Wnd its children are or are not being dragged away
                if (m_drag_drop_originating_wnd) {
                    m_drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                    m_drag_drop_originating_wnd->ChildrenDraggedAway(accepted_wnds, m_curr_wnd_under_cursor);
                }
                // implement drop onto target if any of the dragged Wnds were accepted
                if (!accepted_wnds.empty())
                    m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDroppedOn, pos,
                                                                  accepted_wnds, m_mod_keys));
            }
        }
    }
    m_drag_drop_originating_wnd = 0;
    m_drag_drop_wnds.clear();
    m_drag_drop_wnds_acceptable.clear();
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
    m_curr_drag_wnd_dragged = false;
    m_curr_drag_wnd = 0;
}

void GUIImpl::HandleIdle(Flags<ModKey> mod_keys, const GG::Pt& pos, int curr_ticks)
{
    if (m_mouse_button_down_repeat_delay != 0 &&
        m_curr_wnd_under_cursor &&
        m_curr_wnd_under_cursor == GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys) &&
        m_curr_wnd_under_cursor->RepeatButtonDown() &&
        m_drag_wnds[0] == m_curr_wnd_under_cursor)
    {
        // convert to a key press message after ensuring that timing requirements are met
        if (curr_ticks - m_prev_mouse_button_press_time > m_mouse_button_down_repeat_delay) {
            if (!m_last_mouse_button_down_repeat_time ||
                curr_ticks - m_last_mouse_button_down_repeat_time > m_mouse_button_down_repeat_interval)
            {
                m_last_mouse_button_down_repeat_time = curr_ticks;
                m_curr_wnd_under_cursor->HandleEvent(WndEvent(
                    WndEvent::LButtonDown, pos, mod_keys));
            }
        }

    } else if (
        m_key_press_repeat_delay != 0 &&
        m_last_pressed_key_code_point.first != GGK_UNKNOWN &&
        GUI::s_gui->FocusWnd() &&
        GUI::s_gui->FocusWnd()->RepeatKeyPress())
    {
        // convert to a key press message after ensuring that timing requirements are met
        if (curr_ticks - m_prev_key_press_time > m_key_press_repeat_delay) {
            if (!m_last_key_press_repeat_time ||
                curr_ticks - m_last_key_press_repeat_time > m_key_press_repeat_interval)
            {
                m_last_key_press_repeat_time = curr_ticks;
                GUI::s_gui->FocusWnd()->HandleEvent(WndEvent(
                    WndEvent::KeyPress, m_last_pressed_key_code_point.first,
                    m_last_pressed_key_code_point.second, mod_keys));
            }
        }

    } else if (m_curr_wnd_under_cursor) {
        GUI::s_gui->ProcessBrowseInfo();
    }
}

void GUIImpl::HandleKeyPress(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks)
{
    key = KeyMappedKey(key, m_key_map);
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = 0;
    m_last_pressed_key_code_point = std::make_pair(key, key_code_point);
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
        if (m_accelerators.find(std::make_pair(key, massaged_mods))
            != m_accelerators.end())
        {
            processed = GUI::s_gui->AcceleratorSignal(key, massaged_mods)();
        }
    }
    if (!processed && GUI::s_gui->FocusWnd())
        GUI::s_gui->FocusWnd()->HandleEvent(WndEvent(
            WndEvent::KeyPress, key, key_code_point, mod_keys));
}

void GUIImpl::HandleKeyRelease(Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks)
{
    key = KeyMappedKey(key, m_key_map);
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point.first = GGK_UNKNOWN;
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = 0;
    if (GUI::s_gui->FocusWnd())
        GUI::s_gui->FocusWnd()->HandleEvent(WndEvent(
            WndEvent::KeyRelease, key, key_code_point, mod_keys));
}

void GUIImpl::HandleTextInput(const std::string* text) {
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = 0;
    if (GUI::s_gui->FocusWnd())
        GUI::s_gui->FocusWnd()->HandleEvent(WndEvent(WndEvent::TextInput, text));
}

void GUIImpl::HandleMouseMove(Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks)
{
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys);

    m_mouse_pos = pos; // record mouse position
    m_mouse_rel = rel; // record mouse movement

    if (m_drag_wnds[0] || m_drag_wnds[1] || m_drag_wnds[2]) {
        if (m_drag_wnds[0])
            HandleMouseDrag(0, pos, curr_ticks);
        if (m_drag_wnds[1])
            HandleMouseDrag(1, pos, curr_ticks);
        if (m_drag_wnds[2])
            HandleMouseDrag(2, pos, curr_ticks);
    } else if (m_curr_wnd_under_cursor &&
               m_prev_wnd_under_cursor == m_curr_wnd_under_cursor)
    {
        // if !m_drag_wnds[0] and we're moving over the same
        // (valid) object we were during the last iteration
        m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseHere, pos, mod_keys));
        GUI::s_gui->ProcessBrowseInfo();
    }
    if (m_prev_wnd_under_cursor != m_curr_wnd_under_cursor) {
        m_browse_info_wnd.reset();
        m_browse_target = 0;
        m_prev_wnd_under_cursor_time = curr_ticks;
    }
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseWheel(Flags<ModKey> mod_keys, const GG::Pt& pos, const Pt& rel, int curr_ticks)
{
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, mod_keys);
    m_browse_info_wnd.reset();
    m_browse_target = 0;
    m_prev_wnd_under_cursor_time = curr_ticks;
    // don't send out 0-movement wheel messages
    if (m_curr_wnd_under_cursor && rel.y)
        m_curr_wnd_under_cursor->HandleEvent(WndEvent(
            WndEvent::MouseWheel, pos, Value(rel.y), mod_keys));
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseEnter(Flags< ModKey > mod_keys, const GG::Pt& pos, Wnd* w)
{
    w->HandleEvent(WndEvent(WndEvent::MouseEnter, pos, mod_keys));
    m_curr_wnd_under_cursor = w;
}

void GUIImpl::ClearState()
{
    m_focus_wnd = 0;
    m_mouse_pos = GG::Pt(X(-1000), Y(-1000));
    m_mouse_rel = GG::Pt(X(0), Y(0));
    m_mod_keys = Flags<ModKey>();
    m_last_mouse_button_down_repeat_time = 0;
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point = std::make_pair(GGK_UNKNOWN, 0u);

    m_prev_wnd_drag_position = Pt();
    m_browse_info_wnd.reset();
    m_browse_target = 0;

    m_prev_mouse_button_press_time = -1;
    m_prev_wnd_under_cursor = 0;
    m_prev_wnd_under_cursor_time = -1;
    m_curr_wnd_under_cursor = 0;

    m_mouse_button_state[0] = m_mouse_button_state[1] = m_mouse_button_state[2] = false;
    m_drag_wnds[0] = m_drag_wnds[1] = m_drag_wnds[2] = 0;

    m_curr_drag_wnd_dragged = false;
    m_curr_drag_wnd = 0;
    m_curr_drag_drop_here_wnd = 0;
    m_wnd_region = WR_NONE;
    m_browse_target = 0;
    m_drag_drop_originating_wnd = 0;
    m_curr_wnd_under_cursor = 0;

    m_delta_t = 0;

    m_double_click_wnd = 0;
    m_double_click_start_time = -1;
    m_double_click_time = -1;
}

// static member(s)
GUI*                       GUI::s_gui = 0;
boost::shared_ptr<GUIImpl> GUI::s_impl;

// member functions
GUI::GUI(const std::string& app_name)
{
    assert(!s_gui);
    s_gui = this;
    assert(!s_impl);
    s_impl.reset(new GUIImpl());
    s_impl->m_app_name = app_name;
}

GUI::~GUI()
{ Wnd::s_default_browse_info_wnd.reset(); }

const std::string& GUI::AppName() const
{ return s_impl->m_app_name; }

Wnd* GUI::FocusWnd() const
{ return s_impl->m_modal_wnds.empty() ? s_impl->m_focus_wnd : s_impl->m_modal_wnds.back().second; }

bool GUI::FocusWndAcceptsTypingInput() const
{
    const Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return dynamic_cast<const Edit*>(focus_wnd);    // currently only Edit controls accept text input, so far as I'm aware. Could add a ->AcceptsTypingInput() function to Wnd if needed
}

Wnd* GUI::PrevFocusInteractiveWnd() const
{
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return focus_wnd;

    Wnd* parent_of_focus_wnd = focus_wnd->Parent();
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find previous INTERACTIVE sibling wnd
    const std::list<Wnd*>& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    std::list<Wnd*>::const_reverse_iterator focus_it =
        std::find(siblings.rbegin(), siblings.rend(), focus_wnd);
    if (focus_it == siblings.rend())
        return focus_wnd;

    // loop around until finding an interactive enabled control sibling or
    // returning to the focus wnd
    std::list<Wnd*>::const_reverse_iterator loop_it = focus_it;
    ++loop_it;
    while (loop_it != focus_it) {
        if (loop_it == siblings.rend()) {
            loop_it = siblings.rbegin();
            continue;
        }

        Wnd* sibling = *loop_it;
        if (sibling->Interactive()) {
            Control* ctrl = dynamic_cast<Control*>(sibling);
            if (ctrl && !ctrl->Disabled()) {
                return sibling;
                break;
            }
        }

        ++loop_it;
    }
    return focus_wnd;
}

Wnd* GUI::NextFocusInteractiveWnd() const
{
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return focus_wnd;

    Wnd* parent_of_focus_wnd = focus_wnd->Parent();
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find next INTERACTIVE sibling wnd
    const std::list<Wnd*>& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    std::list<Wnd*>::const_iterator focus_it =
        std::find(siblings.begin(), siblings.end(), focus_wnd);
    if (focus_it == siblings.end())
        return focus_wnd;

    // loop around until finding an interactive enabled control sibling or
    // returning to the focus wnd
    std::list<Wnd*>::const_iterator loop_it = focus_it;
    ++loop_it;
    while (loop_it != focus_it) {
        if (loop_it == siblings.end()) {
            loop_it = siblings.begin();
            continue;
        }

        Wnd* sibling = *loop_it;
        if (sibling->Interactive()) {
            Control* ctrl = dynamic_cast<Control*>(sibling);
            if (ctrl && !ctrl->Disabled()) {
                return sibling;
                break;
            }
        }


        ++loop_it;
    }
    return focus_wnd;
}

Wnd* GUI::GetWindowUnder(const Pt& pt) const
{
    if (INSTRUMENT_GET_WINDOW_UNDER) {
        if (Wnd* w = s_impl->m_zlist.Pick(pt, ModalWindow()))
            std::cerr << "GUI::GetWindowUnder() : " << w->Name() << " @ " << w << std::endl;
    }
    return s_impl->m_zlist.Pick(pt, ModalWindow());
}

unsigned int GUI::DeltaT() const
{ return s_impl->m_delta_t; }

bool GUI::RenderingDragDropWnds() const
{ return s_impl->m_rendering_drag_drop_wnds; }

bool GUI::FPSEnabled() const
{ return s_impl->m_calc_FPS; }

double GUI::FPS() const
{ return s_impl->m_FPS; }

std::string GUI::FPSString() const
{ return boost::io::str(boost::format("%.2f frames per second") % s_impl->m_FPS); }

double GUI::MaxFPS() const
{ return s_impl->m_max_FPS; }

unsigned int GUI::KeyPressRepeatDelay() const
{ return s_impl->m_key_press_repeat_delay; }

unsigned int GUI::KeyPressRepeatInterval() const
{ return s_impl->m_key_press_repeat_interval; }

unsigned int GUI::ButtonDownRepeatDelay() const
{ return s_impl->m_mouse_button_down_repeat_delay; }

unsigned int GUI::ButtonDownRepeatInterval() const
{ return s_impl->m_mouse_button_down_repeat_interval; }

unsigned int GUI::DoubleClickInterval() const
{ return s_impl->m_double_click_interval; }

unsigned int GUI::MinDragTime() const
{ return s_impl->m_min_drag_time; }

unsigned int GUI::MinDragDistance() const
{ return s_impl->m_min_drag_distance; }

bool GUI::DragWnd(const Wnd* wnd, unsigned int mouse_button) const
{ return wnd == s_impl->m_drag_wnds[mouse_button < 3 ? mouse_button : 0]; }

bool GUI::DragDropWnd(const Wnd* wnd) const
{ return s_impl->m_drag_drop_wnds.find(const_cast<Wnd*>(wnd)) != s_impl->m_drag_drop_wnds.end(); }

bool GUI::AcceptedDragDropWnd(const Wnd* wnd) const
{
    std::map<const Wnd*, bool>::const_iterator it = s_impl->m_drag_drop_wnds_acceptable.find(wnd);
    return it != s_impl->m_drag_drop_wnds_acceptable.end() && it->second;
}

bool GUI::MouseButtonDown(unsigned int bn) const
{ return (bn <= 2) ? s_impl->m_mouse_button_state[bn] : false; }

Pt GUI::MousePosition() const
{ return s_impl->m_mouse_pos; }

Pt GUI::MouseMovement() const
{ return s_impl->m_mouse_rel; }

Flags<ModKey> GUI::ModKeys() const
{ return s_impl->m_mod_keys; }

bool GUI::MouseLRSwapped() const
{ return s_impl->m_mouse_lr_swap; }

const std::map<Key, Key>& GUI::KeyMap() const
{ return s_impl->m_key_map; }

std::set<std::pair<CPSize, CPSize> > GUI::FindWords(const std::string& str) const
{
    std::set<std::pair<CPSize, CPSize> > retval;
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

std::set<std::pair<StrSize, StrSize> > GUI::FindWordsStringIndices(const std::string& str) const
{
    std::set<std::pair<StrSize, StrSize> > retval;

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

        retval.insert(std::make_pair(start_idx, end_idx));
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
        std::string::const_iterator start_it = word_pos_it.base();
        std::advance(word_pos_it, match_result.length());
        std::string word_in_str(start_it, word_pos_it.base());

        if (boost::iequals(word_in_str, word))
            return true;
    }

    return false;
}

const boost::shared_ptr<StyleFactory>& GUI::GetStyleFactory() const
{ return s_impl->m_style_factory; }

bool GUI::RenderCursor() const
{ return s_impl->m_render_cursor; }

const boost::shared_ptr<Cursor>& GUI::GetCursor() const
{ return s_impl->m_cursor; }

GUI::const_accel_iterator GUI::accel_begin() const
{ return s_impl->m_accelerators.begin(); }

GUI::const_accel_iterator GUI::accel_end() const
{ return s_impl->m_accelerators.end(); }

GUI::AcceleratorSignalType& GUI::AcceleratorSignal(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/) const
{
    boost::shared_ptr<AcceleratorSignalType>& sig_ptr = s_impl->m_accelerator_sigs[std::make_pair(key, mod_keys)];
    if (!sig_ptr)
        sig_ptr.reset(new AcceleratorSignalType());
    if (INSTRUMENT_ALL_SIGNALS)
        Connect(*sig_ptr, AcceleratorEcho(key, mod_keys));
    return *sig_ptr;
}

bool GUI::ModalAcceleratorSignalsEnabled() const
{ return s_impl->m_allow_modal_accelerator_signals; }

bool GUI::ModalWndsOpen() const
{ return !s_impl->m_modal_wnds.empty(); }

void GUI::SaveWndAsPNG(const Wnd* wnd, const std::string& filename) const
{
    s_impl->m_save_as_png_wnd = wnd;
    s_impl->m_save_as_png_filename = filename;
}

void GUI::operator()()
{ Run(); }

void GUI::HandleGGEvent(EventType event, Key key, boost::uint32_t key_code_point,
                        Flags<ModKey> mod_keys, const Pt& pos, const Pt& rel, const std::string* text)
{
    s_impl->m_mod_keys = mod_keys;

    int curr_ticks = Ticks();

    // track double-click time and time-out any pending double-click that has
    // outlived its interval
    if (s_impl->m_double_click_time >= 0) {
        s_impl->m_double_click_time = curr_ticks - s_impl->m_double_click_start_time;
        if (s_impl->m_double_click_time >= s_impl->m_double_click_interval) {
            s_impl->m_double_click_start_time = -1;
            s_impl->m_double_click_time = -1;
            s_impl->m_double_click_wnd = 0;
        }
    }

    switch (event) {

    case IDLE:
        s_impl->HandleIdle(mod_keys, pos, curr_ticks);
        break;

    case KEYPRESS:
        s_impl->HandleKeyPress(key, key_code_point, mod_keys, curr_ticks);
        break;

    case KEYRELEASE:
        s_impl->HandleKeyRelease(key, key_code_point, mod_keys, curr_ticks);
        break;

    case TEXTINPUT:
        s_impl->HandleTextInput(text);
        break;

    case MOUSEMOVE:
        s_impl->HandleMouseMove(mod_keys, pos, rel, curr_ticks);
        break;

    case LPRESS:
        s_impl->HandleMouseButtonPress((s_impl->m_mouse_lr_swap ? RPRESS : LPRESS) - LPRESS, pos, curr_ticks);
        break;

    case MPRESS:
        s_impl->HandleMouseButtonPress(MPRESS - LPRESS, pos, curr_ticks);
        break;

    case RPRESS:
        s_impl->HandleMouseButtonPress((s_impl->m_mouse_lr_swap ? LPRESS : RPRESS) - LPRESS, pos, curr_ticks);
        break;

    case LRELEASE:
        s_impl->HandleMouseButtonRelease((s_impl->m_mouse_lr_swap ? RRELEASE : LRELEASE) - LRELEASE, pos, curr_ticks);
        break;

    case MRELEASE:
        s_impl->HandleMouseButtonRelease(MRELEASE - LRELEASE, pos, curr_ticks);
        break;

    case RRELEASE:
        s_impl->HandleMouseButtonRelease((s_impl->m_mouse_lr_swap ? LRELEASE : RRELEASE) - LRELEASE, pos, curr_ticks);
        break;

    case MOUSEWHEEL:
        s_impl->HandleMouseWheel(mod_keys, pos, rel, curr_ticks);
        break;

    default:
        break;
    }
}

void GUI::ClearEventState()
{ s_impl->ClearState(); }

void GUI::SetFocusWnd(Wnd* wnd)
{
    if (FocusWnd() == wnd)
        return;

    // inform old focus wnd that it is losing focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::LosingFocus));

    (s_impl->m_modal_wnds.empty() ? s_impl->m_focus_wnd : s_impl->m_modal_wnds.back().second) = wnd;

    // inform new focus wnd that it is gaining focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::GainingFocus));
}

bool GUI::SetPrevFocusWndInCycle()
{
    Wnd* prev_wnd = PrevFocusInteractiveWnd();
    if (prev_wnd)
        SetFocusWnd(prev_wnd);
    return true;
}

bool GUI::SetNextFocusWndInCycle()
{
    Wnd* next_wnd = NextFocusInteractiveWnd();
    if (next_wnd)
        SetFocusWnd(next_wnd);
    return true;
}

void GUI::Wait(unsigned int ms)
{ boost::this_thread::sleep_for(boost::chrono::milliseconds(ms)); }

void GUI::Register(Wnd* wnd)
{ if (wnd) s_impl->m_zlist.Add(wnd); }

void GUI::RegisterModal(Wnd* wnd)
{
    if (wnd && wnd->Modal()) {
        s_impl->m_modal_wnds.push_back(std::make_pair(wnd, wnd));
        wnd->HandleEvent(WndEvent(WndEvent::GainingFocus));
    }
}

void GUI::Remove(Wnd* wnd)
{
    if (wnd) {
        if (!s_impl->m_modal_wnds.empty() && s_impl->m_modal_wnds.back().first == wnd) // if it's the current modal window, remove it from the modal list
            s_impl->m_modal_wnds.pop_back();
        else // if it's not a modal window, remove it from the z-order
            s_impl->m_zlist.Remove(wnd);
    }
}

void GUI::WndDying(Wnd* wnd)
{
    if (!wnd)
        return;

    Remove(wnd);
    if (MatchesOrContains(wnd, s_impl->m_focus_wnd))
        s_impl->m_focus_wnd = 0;
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->m_modal_wnds.begin(); it != s_impl->m_modal_wnds.end(); ++it) {
        if (MatchesOrContains(wnd, it->second)) {
            if (MatchesOrContains(wnd, it->first)) {
                it->second = 0;
            } else { // if the modal window for the removed window's focus level is available, revert focus to the modal window
                if ((it->second = it->first))
                    it->first->HandleEvent(WndEvent(WndEvent::GainingFocus));
            }
        }
    }
    if (MatchesOrContains(wnd, s_impl->m_prev_wnd_under_cursor))
        s_impl->m_prev_wnd_under_cursor = 0;
    if (MatchesOrContains(wnd, s_impl->m_curr_wnd_under_cursor))
        s_impl->m_curr_wnd_under_cursor = 0;
    if (MatchesOrContains(wnd, s_impl->m_drag_wnds[0])) {
        s_impl->m_drag_wnds[0] = 0;
        s_impl->m_wnd_region = WR_NONE;
    }
    if (MatchesOrContains(wnd, s_impl->m_drag_wnds[1])) {
        s_impl->m_drag_wnds[1] = 0;
        s_impl->m_wnd_region = WR_NONE;
    }
    if (MatchesOrContains(wnd, s_impl->m_drag_wnds[2])) {
        s_impl->m_drag_wnds[2] = 0;
        s_impl->m_wnd_region = WR_NONE;
    }
    if (MatchesOrContains(wnd, s_impl->m_curr_drag_drop_here_wnd))
        s_impl->m_curr_drag_drop_here_wnd = 0;
    if (MatchesOrContains(wnd, s_impl->m_drag_drop_originating_wnd))
        s_impl->m_drag_drop_originating_wnd = 0;
    s_impl->m_drag_drop_wnds.erase(wnd);
    s_impl->m_drag_drop_wnds_acceptable.erase(wnd);
    if (MatchesOrContains(wnd, s_impl->m_double_click_wnd)) {
        s_impl->m_double_click_wnd = 0;
        s_impl->m_double_click_start_time = -1;
        s_impl->m_double_click_time = -1;
    }
}

void GUI::EnableFPS(bool b/* = true*/)
{
    s_impl->m_calc_FPS = b;
    if (!b)
        s_impl->m_FPS = -1.0f;
}

void GUI::SetMaxFPS(double max)
{
    if (max && max < 0.1)
        max = 0.1;
    s_impl->m_max_FPS = max;
}

void GUI::MoveUp(Wnd* wnd)
{ if (wnd) s_impl->m_zlist.MoveUp(wnd); }

void GUI::MoveDown(Wnd* wnd)
{ if (wnd) s_impl->m_zlist.MoveDown(wnd); }

boost::shared_ptr<ModalEventPump> GUI::CreateModalEventPump(bool& done)
{ return boost::shared_ptr<ModalEventPump>(new ModalEventPump(done)); }

void GUI::RegisterDragDropWnd(Wnd* wnd, const Pt& offset, Wnd* originating_wnd)
{
    assert(wnd);
    if (!s_impl->m_drag_drop_wnds.empty() && originating_wnd != s_impl->m_drag_drop_originating_wnd) {
        std::string s_impl_orig_wnd_name("NULL");
        std::string orig_wnd_name("NULL");
        if (s_impl->m_drag_drop_originating_wnd)
            s_impl_orig_wnd_name = s_impl->m_drag_drop_originating_wnd->Name();
        if (originating_wnd)
            orig_wnd_name = originating_wnd->Name();
        throw std::runtime_error("GUI::RegisterDragDropWnd() : Attempted to register a drag drop item"
                                "dragged from  one window(" + orig_wnd_name + 
                                "), when another window (" + s_impl_orig_wnd_name +
                                ") already has items being dragged from it.");
    }
    s_impl->m_drag_drop_wnds[wnd] = offset;
    s_impl->m_drag_drop_wnds_acceptable[wnd] = false;
    s_impl->m_drag_drop_originating_wnd = originating_wnd;
}

void GUI::CancelDragDrop()
{
    s_impl->m_drag_drop_wnds.clear();
    s_impl->m_drag_drop_wnds_acceptable.clear();
}

void GUI::RegisterTimer(Timer& timer)
{ s_impl->m_timers.insert(&timer); }

void GUI::RemoveTimer(Timer& timer)
{ s_impl->m_timers.erase(&timer); }

void GUI::EnableKeyPressRepeat(unsigned int delay, unsigned int interval)
{
    if (!delay) { // setting delay = 0 completely disables key press repeat
        s_impl->m_key_press_repeat_delay = 0;
        s_impl->m_key_press_repeat_interval = 0;
    } else {
        s_impl->m_key_press_repeat_delay = delay;
        s_impl->m_key_press_repeat_interval = interval;
    }
}

void GUI::EnableMouseButtonDownRepeat(unsigned int delay, unsigned int interval)
{
    if (!delay) { // setting delay = 0 completely disables mouse button down repeat
        s_impl->m_mouse_button_down_repeat_delay = 0;
        s_impl->m_mouse_button_down_repeat_interval = 0;
    } else {
        s_impl->m_mouse_button_down_repeat_delay = delay;
        s_impl->m_mouse_button_down_repeat_interval = interval;
    }
}

void GUI::SetDoubleClickInterval(unsigned int interval)
{ s_impl->m_double_click_interval = interval; }

void GUI::SetMinDragTime(unsigned int time)
{ s_impl->m_min_drag_time = time; }

void GUI::SetMinDragDistance(unsigned int distance)
{ s_impl->m_min_drag_distance = distance; }

GUI::accel_iterator GUI::accel_begin()
{ return s_impl->m_accelerators.begin(); }

GUI::accel_iterator GUI::accel_end()
{ return s_impl->m_accelerators.end(); }

void GUI::SetAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    s_impl->m_accelerators.insert(std::make_pair(key, mod_keys));
}

void GUI::RemoveAccelerator(Key key, Flags<ModKey> mod_keys/* = MOD_KEY_NONE*/)
{
    mod_keys = MassagedAccelModKeys(mod_keys);
    s_impl->m_accelerators.erase(std::make_pair(key, mod_keys));
}

void GUI::RemoveAccelerator(accel_iterator it)
{ s_impl->m_accelerators.erase(it); }

void GUI::EnableModalAcceleratorSignals(bool allow)
{ s_impl->m_allow_modal_accelerator_signals = allow; }

void GUI::SetMouseLRSwapped(bool swapped/* = true*/)
{ s_impl->m_mouse_lr_swap = swapped; }

void GUI::SetKeyMap(const std::map<Key, Key>& key_map)
{ s_impl->m_key_map = key_map; }

boost::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts)
{ return GetFontManager().GetFont(font_filename, pts); }

boost::shared_ptr<Font> GUI::GetFont(const std::string& font_filename, unsigned int pts,
                                     const std::vector<unsigned char>& file_contents)
{ return GetFontManager().GetFont(font_filename, pts, file_contents); }

boost::shared_ptr<Font> GUI::GetFont(const boost::shared_ptr<Font>& font, unsigned int pts)
{
    boost::shared_ptr<Font> retval;
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

boost::shared_ptr<Texture> GUI::StoreTexture(Texture* texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

boost::shared_ptr<Texture> GUI::StoreTexture(const boost::shared_ptr<Texture>& texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

boost::shared_ptr<Texture> GUI::GetTexture(const boost::filesystem::path& path, bool mipmap/* = false*/)
{ return GetTextureManager().GetTexture(path, mipmap); }

void GUI::FreeTexture(const boost::filesystem::path& path)
{ GetTextureManager().FreeTexture(path); }

void GUI::SetStyleFactory(const boost::shared_ptr<StyleFactory>& factory)
{
    s_impl->m_style_factory = factory;
    if (!s_impl->m_style_factory)
        s_impl->m_style_factory.reset(new StyleFactory());
}

void GUI::RenderCursor(bool render)
{ s_impl->m_render_cursor = render; }

void GUI::SetCursor(const boost::shared_ptr<Cursor>& cursor)
{ s_impl->m_cursor = cursor; }

std::string GUI::ClipboardText() const
{ return s_impl->m_clipboard_text; }

bool GUI::SetClipboardText(const std::string& text)
{
    s_impl->m_clipboard_text = text;
    return true;
}

bool GUI::CopyFocusWndText()
{
    const Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return CopyWndText(focus_wnd);
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
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return PasteWndText(focus_wnd, text);
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
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return CutWndText(focus_wnd);
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
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return WndSelectAll(focus_wnd);
}

bool GUI::FocusWndDeselect()
{
    Wnd* focus_wnd = FocusWnd();
    if (!focus_wnd)
        return false;
    return WndDeselect(focus_wnd);
}

GUI* GUI::GetGUI()
{ return s_gui; }

void GUI::PreRenderWindow(Wnd* wnd)
{
    if (!wnd || !wnd->Visible())
        return;

    for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != wnd->m_children.end(); ++it) {
        PreRenderWindow(*it);
    }

    if (wnd->PreRenderRequired())
        wnd->PreRender();
}

void GUI::RenderWindow(Wnd* wnd)
{
    if (!wnd)
        return;

    if (wnd->Visible()) {
        wnd->Render();

        Wnd::ChildClippingMode clip_mode = wnd->GetChildClippingMode();

        if (clip_mode != Wnd::ClipToClientAndWindowSeparately) {
            bool clip = clip_mode != Wnd::DontClip;
            if (clip)
                wnd->BeginClipping();
            for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != wnd->m_children.end(); ++it) {
                if ((*it) && (*it)->Visible())
                    RenderWindow(*it);
            }
            if (clip)
                wnd->EndClipping();
        } else {
            std::vector<Wnd*> children_copy(wnd->m_children.begin(), wnd->m_children.end());
            std::vector<Wnd*>::iterator client_child_begin =
                std::partition(children_copy.begin(), children_copy.end(),
                               boost::bind(&Wnd::NonClientChild, _1));

            if (children_copy.begin() != client_child_begin) {
                wnd->BeginNonclientClipping();
                for (std::vector<Wnd*>::iterator it = children_copy.begin(); it != client_child_begin; ++it) {
                    if ((*it) && (*it)->Visible())
                        RenderWindow(*it);
                }
                wnd->EndNonclientClipping();
            }

            if (client_child_begin != children_copy.end()) {
                wnd->BeginClipping();
                for (std::vector<Wnd*>::iterator it = client_child_begin; it != children_copy.end(); ++it) {
                    if ((*it) && (*it)->Visible())
                        RenderWindow(*it);
                }
                wnd->EndClipping();
            }
        }
    }

    if (wnd == s_impl->m_save_as_png_wnd) {
        WriteWndToPNG(s_impl->m_save_as_png_wnd, s_impl->m_save_as_png_filename);
        s_impl->m_save_as_png_wnd = 0;
        s_impl->m_save_as_png_filename.clear();
    }
}

void GUI::RenderDragDropWnds()
{
    // render drag-and-drop windows in arbitrary order (sorted by pointer value)
    s_impl->m_rendering_drag_drop_wnds = true;
    for (std::map<Wnd*, Pt>::const_iterator it = s_impl->m_drag_drop_wnds.begin(); it != s_impl->m_drag_drop_wnds.end(); ++it) {
        bool old_visible = it->first->Visible();
        if (!old_visible)
            it->first->Show();
        Pt parent_offset = it->first->Parent() ? it->first->Parent()->ClientUpperLeft() : Pt();
        Pt old_pos = it->first->UpperLeft() - parent_offset;
        it->first->MoveTo(s_impl->m_mouse_pos - parent_offset - it->second);
        RenderWindow(it->first);
        it->first->MoveTo(old_pos);
        if (!old_visible)
            it->first->Hide();
    }
    s_impl->m_rendering_drag_drop_wnds = false;
}

void GUI::ProcessBrowseInfo()
{
    assert(s_impl->m_curr_wnd_under_cursor);
    if (!s_impl->m_mouse_button_state[0] && !s_impl->m_mouse_button_state[1] && !s_impl->m_mouse_button_state[2] &&
        (s_impl->m_modal_wnds.empty() || s_impl->m_curr_wnd_under_cursor->RootParent() == s_impl->m_modal_wnds.back().first))
    {
        Wnd* wnd = s_impl->m_curr_wnd_under_cursor;
        while (!ProcessBrowseInfoImpl(wnd) &&
               wnd->Parent() &&
               (dynamic_cast<Control*>(wnd) || dynamic_cast<Layout*>(wnd)))
        { wnd = wnd->Parent(); }
    }
}

void GUI::PreRender()
{
    // pre-render normal windows back-to-front
    for (ZList::reverse_iterator it = s_impl->m_zlist.rbegin(); it != s_impl->m_zlist.rend(); ++it) {
        PreRenderWindow(*it);
    }

    // pre-render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->m_modal_wnds.begin();
         it != s_impl->m_modal_wnds.end(); ++it)
    {
        PreRenderWindow(it->first);
    }

    // pre-render the active browse info window, if any
    if (s_impl->m_browse_info_wnd && s_impl->m_curr_wnd_under_cursor) {
        assert(s_impl->m_browse_target);
        PreRenderWindow(s_impl->m_browse_info_wnd.get());
    }

    for (std::map<Wnd*, Pt>::const_iterator it = s_impl->m_drag_drop_wnds.begin(); it != s_impl->m_drag_drop_wnds.end(); ++it) {
        PreRenderWindow(it->first);
    }
}

void GUI::Render()
{
    // update timers
    int ticks = Ticks();
    for (std::set<Timer*>::iterator it = s_impl->m_timers.begin(); it != s_impl->m_timers.end(); ++it) {
        (*it)->Update(ticks);
    }

    Enter2DMode();
    // render normal windows back-to-front
    for (ZList::reverse_iterator it = s_impl->m_zlist.rbegin(); it != s_impl->m_zlist.rend(); ++it) {
        if (*it)
            RenderWindow(*it);
    }

    // render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->m_modal_wnds.begin();
         it != s_impl->m_modal_wnds.end(); ++it)
    {
        if (it->first)
            RenderWindow(it->first);
    }

    // render the active browse info window, if any
    if (s_impl->m_browse_info_wnd) {
        if (!s_impl->m_curr_wnd_under_cursor) {
            s_impl->m_browse_info_wnd.reset();
            s_impl->m_browse_info_mode = -1;
            s_impl->m_browse_target = 0;
            s_impl->m_prev_wnd_under_cursor_time = Ticks();
        } else {
            assert(s_impl->m_browse_target);
            s_impl->m_browse_info_wnd->Update(s_impl->m_browse_info_mode, s_impl->m_browse_target);
            RenderWindow(s_impl->m_browse_info_wnd.get());
        }
    }

    RenderDragDropWnds();

    if (s_impl->m_render_cursor && s_impl->m_cursor)
        s_impl->m_cursor->Render(s_impl->m_mouse_pos);
    Exit2DMode();
}

bool GUI::ProcessBrowseInfoImpl(Wnd* wnd)
{
    bool retval = false;
    const std::vector<Wnd::BrowseInfoMode>& browse_modes = wnd->BrowseModes();
    if (!browse_modes.empty()) {
        unsigned int delta_t = Ticks() - s_impl->m_prev_wnd_under_cursor_time;
        std::size_t i = 0;
        for (std::vector<Wnd::BrowseInfoMode>::const_reverse_iterator it = browse_modes.rbegin();
             it != browse_modes.rend();
             ++it, ++i) {
            if (it->time < delta_t) {
                if (it->wnd && it->wnd->WndHasBrowseInfo(wnd, i)) {
                    if (s_impl->m_browse_target != wnd || s_impl->m_browse_info_wnd != it->wnd || s_impl->m_browse_info_mode != static_cast<int>(i)) {
                        s_impl->m_browse_target = wnd;
                        s_impl->m_browse_info_wnd = it->wnd;
                        s_impl->m_browse_info_mode = i;
                        s_impl->m_browse_info_wnd->SetCursorPosition(s_impl->m_mouse_pos);
                    }
                    retval = true;
                }
                break;
            }
        }
    }
    return retval;
}

Wnd* GUI::ModalWindow() const
{
    Wnd* retval = 0;
    if (!s_impl->m_modal_wnds.empty())
        retval = s_impl->m_modal_wnds.back().first;
    return retval;
}

Wnd* GUI::CheckedGetWindowUnder(const Pt& pt, Flags<ModKey> mod_keys)
{
    Wnd* w = GetWindowUnder(pt);
    Wnd* dragged_wnd = s_impl->m_curr_drag_wnd; // wnd being continuously repositioned / dragged around, not a drag-drop

    //std::cout << "GUI::CheckedGetWindowUnder w: " << w << "  dragged_wnd: " << dragged_wnd << std::endl << std::flush;

    bool unregistered_drag_drop = dragged_wnd && !dragged_wnd->DragDropDataType().empty();
    bool registered_drag_drop = !s_impl->m_drag_drop_wnds.empty();

    if (s_impl->m_curr_drag_drop_here_wnd && !unregistered_drag_drop && !registered_drag_drop) {
        s_impl->m_curr_drag_drop_here_wnd->HandleEvent(WndEvent(WndEvent::DragDropLeave));
        s_impl->m_curr_drag_drop_here_wnd = 0;
    }

    if (w == s_impl->m_curr_wnd_under_cursor)
        return w;   // same Wnd is under cursor as before; nothing to do

    if (s_impl->m_curr_wnd_under_cursor) {
        // inform previous Wnd under the cursor that the cursor has been dragged away
        if (unregistered_drag_drop) {
            s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
            s_impl->m_drag_drop_wnds_acceptable[dragged_wnd] = false;
            s_impl->m_curr_drag_drop_here_wnd = 0;

        } else if (registered_drag_drop) {
            s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
            for (std::map<const Wnd*, bool>::iterator it = s_impl->m_drag_drop_wnds_acceptable.begin();
                 it != s_impl->m_drag_drop_wnds_acceptable.end(); ++it)
            { it->second = false; }
            s_impl->m_curr_drag_drop_here_wnd = 0;

        } else {
            s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseLeave));
        }
    }

    if (!w) {
        //std::cout << "CheckedGetWindowUnder returning " << w << std::endl << std::flush;
        return w;
    }


    // inform new Wnd under cursor that something was dragged over it
    if (unregistered_drag_drop) {
        // pass drag-drop event to check if the single dragged Wnd is acceptable to drop
        WndEvent event(WndEvent::CheckDrops, pt, dragged_wnd, mod_keys);
        w->HandleEvent(event);
        s_impl->m_drag_drop_wnds_acceptable[dragged_wnd] = false;
        s_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::DragDropEnter, pt, dragged_wnd, mod_keys);
        w->HandleEvent(enter_event);
        s_impl->m_curr_drag_drop_here_wnd = w;

    } else if (registered_drag_drop) {
        // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
        WndEvent event(WndEvent::CheckDrops, pt, s_impl->m_drag_drop_wnds, mod_keys);
        w->HandleEvent(event);
        s_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::DragDropEnter, pt, s_impl->m_drag_drop_wnds, mod_keys);
        w->HandleEvent(enter_event);
        s_impl->m_curr_drag_drop_here_wnd = w;

    } else {
        s_impl->HandleMouseEnter(mod_keys, pt, w);
    }

    //std::cout << "CheckedGetWindowUnder returning " << w << std::endl << std::flush;
    return w;
}

void GUI::SetFPS(double FPS)
{ s_impl->m_FPS = FPS; }

void GUI::SetDeltaT(unsigned int delta_t)
{ s_impl->m_delta_t = delta_t; }

bool GG::MatchesOrContains(const Wnd* lwnd, const Wnd* rwnd)
{
    if (rwnd) {
        for (const Wnd* w = rwnd; w; w = w->Parent()) {
            if (w == lwnd)
                return true;
        }
    } else if (rwnd == lwnd) {
        return true;
    }
    return false;
}

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
