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
#include <GG/PluginInterface.h>
#include <GG/StyleFactory.h>
#include <GG/Edit.h>
#include <GG/Timer.h>
#include <GG/ZList.h>
#include <GG/utf8/checked.h>

#if GG_HAVE_LIBPNG
#include "GIL/extension/io/png_io.hpp"
#endif

#include <boost/format.hpp>
#include <boost/thread.hpp>
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
                  ")\n")
            {}
        bool operator()()
            {
                std::cerr << m_str;
                return false;
            }
        std::string m_str;
    };

    /* returns the storage value of mod_keys that should be used with keyboard accelerators the accelerators don't care
       which side of the keyboard you use for CTRL, SHIFT, etc., and whether or not the numlock or capslock are
       engaged.*/
    Flags<ModKey> MassagedAccelModKeys(Flags<ModKey> mod_keys)
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

    Key           KeyMappedKey(Key key, const std::map<Key, Key>& key_map) {
        std::map<Key, Key>::const_iterator it = key_map.find(key);
        if (it != key_map.end())
            return it->second;
        return key;
    }

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
                     Value(GUI::GetGUI()->AppHeight() - wnd->LowerRight().y),
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

    void HandlePress(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleDrag(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void HandleRelease(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks);
    void ClearState();

    std::string  m_app_name;              // the user-defined name of the apllication

    ZList        m_zlist;                 // object that keeps the GUI windows in the correct depth ordering
    Wnd*         m_focus_wnd;             // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)
    std::list<std::pair<Wnd*, Wnd*> >
                 m_modal_wnds;            // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)

    bool         m_button_state[3];       // the up/down states of the three buttons on the mouse are kept here
    Pt           m_mouse_pos;             // absolute position of mouse, based on last MOUSEMOVE event
    Pt           m_mouse_rel;             // relative position of mouse, based on last MOUSEMOVE event
    Flags<ModKey>m_mod_keys;              // currently-depressed modifier keys, based on last KEYPRESS event

    int          m_button_down_repeat_delay;     // see note above GUI class definition
    int          m_button_down_repeat_interval;
    int          m_last_button_down_repeat_time; // last time of a simulated button-down message

    int          m_double_click_interval; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          m_min_drag_time;         // the minimum amount of time that a drag must be in progress before it is considered a drag, in ms
    int          m_min_drag_distance;     // the minimum distance that a drag must cover before it is considered a drag

    int          m_prev_button_press_time;// the time of the most recent mouse button press
    Pt           m_prev_button_press_pos; // the location of the most recent mouse button press
    Wnd*         m_prev_wnd_under_cursor; // GUI window most recently under the input cursor; may be 0
    int          m_prev_wnd_under_cursor_time; // the time at which prev_wnd_under_cursor was initially set to its current value
    Wnd*         m_curr_wnd_under_cursor; // GUI window currently under the input cursor; may be 0
    Wnd*         m_drag_wnds[3];          // GUI window currently being clicked or dragged by each mouse button
    Pt           m_prev_wnd_drag_position;// the upper-left corner of the dragged window when the last *Drag message was generated
    Pt           m_wnd_drag_offset;       // the offset from the upper left corner of the dragged window to the cursor for the current drag
    bool         m_curr_drag_wnd_dragged; // true iff the currently-pressed window (m_drag_wnds[N]) has actually been dragged some distance (in which case releasing the mouse button is not a click)
    Wnd*         m_curr_drag_wnd;         // nonzero iff m_curr_drag_wnd_dragged is true (that is, we have actually started dragging the Wnd, not just pressed the mouse button); will always be one of m_drag_wnds
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
    m_mouse_pos(X(-1000), Y(-1000)),
    m_mouse_rel(X(0), Y(0)),
    m_mod_keys(),
    m_button_down_repeat_delay(250),
    m_button_down_repeat_interval(66),
    m_last_button_down_repeat_time(0),
    m_double_click_interval(500),
    m_min_drag_time(250),
    m_min_drag_distance(5),
    m_prev_button_press_time(-1),
    m_prev_wnd_under_cursor(0),
    m_prev_wnd_under_cursor_time(-1),
    m_curr_wnd_under_cursor(0),
    m_drag_wnds(),
    m_curr_drag_wnd_dragged(false),
    m_curr_drag_wnd(0),
    m_curr_drag_drop_here_wnd(0),
    m_wnd_region(WR_NONE),
    m_browse_target(0),
    m_drag_drop_originating_wnd(0),
    m_mouse_lr_swap(false),
    m_delta_t(0),
    m_rendering_drag_drop_wnds(false),
    m_FPS(-1.0),
    m_calc_FPS(false),
    m_max_FPS(0.0),
    m_double_click_wnd(0),
    m_double_click_start_time(-1),
    m_double_click_time(-1),
    m_style_factory(new StyleFactory()),
    m_render_cursor(false),
    m_cursor(),
    m_save_as_png_wnd(0),
    m_clipboard_text()
{
    m_button_state[0] = m_button_state[1] = m_button_state[2] = false;
    m_drag_wnds[0] = m_drag_wnds[1] = m_drag_wnds[2] = 0;
}

void GUIImpl::HandlePress(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_last_button_down_repeat_time = 0;
    m_prev_wnd_drag_position = Pt();
    m_wnd_drag_offset = Pt();
    m_prev_button_press_time = 0;
    m_browse_info_wnd.reset();
    m_browse_target = 0;
    m_prev_wnd_under_cursor_time = curr_ticks;
    m_prev_button_press_time = curr_ticks;
    m_prev_button_press_pos = pos;

    m_button_state[mouse_button] = true;
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
            m_wnd_resize_offset.x = m_drag_wnds[mouse_button]->UpperLeft().x - pos.x;
        else
            m_wnd_resize_offset.x = m_drag_wnds[mouse_button]->LowerRight().x - pos.x;
        if (m_wnd_region < 3) // top regions
            m_wnd_resize_offset.y = m_drag_wnds[mouse_button]->UpperLeft().y - pos.y;
        else
            m_wnd_resize_offset.y = m_drag_wnds[mouse_button]->LowerRight().y - pos.y;
        Wnd* drag_wnds_root_parent = m_drag_wnds[mouse_button]->RootParent();
        GUI::s_gui->MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : m_drag_wnds[mouse_button]);
        m_drag_wnds[mouse_button]->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonDown, mouse_button), pos, m_mod_keys));
    }

    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleDrag(unsigned int mouse_button, const Pt& pos, int curr_ticks)
{
    if (m_wnd_region == WR_MIDDLE || m_wnd_region == WR_NONE) { // send drag message to window or initiate drag-and-drop
        Pt diff = m_prev_button_press_pos - pos;
        int drag_distance = Value(diff.x * diff.x) + Value(diff.y * diff.y);
        // ensure that the minimum drag requirements are met
        if (m_min_drag_time < (curr_ticks - m_prev_button_press_time) &&
            (m_min_drag_distance * m_min_drag_distance < drag_distance) &&
            m_drag_drop_wnds.find(m_drag_wnds[mouse_button]) == m_drag_drop_wnds.end())
        {
            if (!m_drag_wnds[mouse_button]->Dragable() &&
                m_drag_wnds[mouse_button]->DragDropDataType() != "" &&
                mouse_button == 0)
            {
                Wnd* parent = m_drag_wnds[mouse_button]->Parent();
                Pt offset = m_prev_button_press_pos - m_drag_wnds[mouse_button]->UpperLeft();
                GUI::s_gui->RegisterDragDropWnd(m_drag_wnds[mouse_button], offset, parent);
                if (parent)
                    parent->StartingChildDragDrop(m_drag_wnds[mouse_button], offset);
            } else {
                Pt start_pos = m_drag_wnds[mouse_button]->UpperLeft();
                Pt move = (pos - m_wnd_drag_offset) - m_prev_wnd_drag_position;
                m_drag_wnds[mouse_button]->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDrag, mouse_button), pos, move, m_mod_keys));
                m_prev_wnd_drag_position = m_drag_wnds[mouse_button]->UpperLeft();
                if (start_pos != m_drag_wnds[mouse_button]->UpperLeft()) {
                    m_curr_drag_wnd_dragged = true;
                    m_curr_drag_wnd = m_drag_wnds[mouse_button];
                }
            }
        }
        // notify wnd under cursor of presence of drag-and-drop wnd(s)
        if (m_curr_drag_wnd_dragged &&
            m_drag_wnds[mouse_button]->DragDropDataType() != "" &&
            mouse_button == 0 ||
            !m_drag_drop_wnds.empty()) {
            bool unregistered_drag = m_drag_drop_wnds.empty();
            std::set<Wnd*> ignores;
            if (unregistered_drag)
                ignores.insert(m_drag_wnds[mouse_button]);
            m_curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), &ignores);
            std::map<Wnd*, Pt> drag_drop_wnds;
            drag_drop_wnds[m_drag_wnds[mouse_button]] = m_wnd_drag_offset;
            std::map<Wnd*, Pt>& drag_drop_wnds_to_use = unregistered_drag ? drag_drop_wnds : m_drag_drop_wnds;
            if (m_curr_wnd_under_cursor && m_prev_wnd_under_cursor == m_curr_wnd_under_cursor) {
                if (m_curr_drag_drop_here_wnd) {
                    assert(m_curr_wnd_under_cursor == m_curr_drag_drop_here_wnd);
                    m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropHere, pos, drag_drop_wnds_to_use, m_mod_keys));
                    m_curr_wnd_under_cursor->DropsAcceptable(m_drag_drop_wnds_acceptable.begin(),
                                                             m_drag_drop_wnds_acceptable.end(),
                                                             pos);
                } else {
                    m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropEnter, pos, drag_drop_wnds_to_use, m_mod_keys));
                    m_curr_wnd_under_cursor->DropsAcceptable(m_drag_drop_wnds_acceptable.begin(),
                                                             m_drag_drop_wnds_acceptable.end(),
                                                             pos);
                    m_curr_drag_drop_here_wnd = m_curr_wnd_under_cursor;
                }
            }
        }
    } else if (m_drag_wnds[mouse_button]->Resizable()) { // send appropriate resize message to window
        Pt offset_pos = pos + m_wnd_resize_offset;
        if (Wnd* parent = m_drag_wnds[mouse_button]->Parent())
            offset_pos -= parent->ClientUpperLeft();
        switch (m_wnd_region)
        {
        case WR_TOPLEFT:
            m_drag_wnds[mouse_button]->SizeMove(
                offset_pos,
                m_drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_TOP:
            m_drag_wnds[mouse_button]->SizeMove(
                Pt(m_drag_wnds[mouse_button]->RelativeUpperLeft().x,
                   offset_pos.y),
                m_drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_TOPRIGHT:
            m_drag_wnds[mouse_button]->SizeMove(
                Pt(m_drag_wnds[mouse_button]->RelativeUpperLeft().x,
                   offset_pos.y),
                Pt(offset_pos.x,
                   m_drag_wnds[mouse_button]->RelativeLowerRight().y));
            break;
        case WR_MIDLEFT:
            m_drag_wnds[mouse_button]->SizeMove(
                Pt(offset_pos.x,
                   m_drag_wnds[mouse_button]->RelativeUpperLeft().y),
                m_drag_wnds[mouse_button]->RelativeLowerRight());
            break;
        case WR_MIDRIGHT:
            m_drag_wnds[mouse_button]->SizeMove(
                m_drag_wnds[mouse_button]->RelativeUpperLeft(),
                Pt(offset_pos.x,
                   m_drag_wnds[mouse_button]->RelativeLowerRight().y));
            break;
        case WR_BOTTOMLEFT:
            m_drag_wnds[mouse_button]->SizeMove(
                Pt(offset_pos.x,
                   m_drag_wnds[mouse_button]->RelativeUpperLeft().y),
                Pt(m_drag_wnds[mouse_button]->RelativeLowerRight().x,
                   offset_pos.y));
            break;
        case WR_BOTTOM:
            m_drag_wnds[mouse_button]->SizeMove(
                m_drag_wnds[mouse_button]->RelativeUpperLeft(),
                Pt(m_drag_wnds[mouse_button]->RelativeLowerRight().x,
                   offset_pos.y));
            break;
        case WR_BOTTOMRIGHT:
            m_drag_wnds[mouse_button]->SizeMove(
                m_drag_wnds[mouse_button]->RelativeUpperLeft(),
                offset_pos);
            break;
        default:
            break;
        }
    }
}

void GUIImpl::HandleRelease(unsigned int mouse_button, const GG::Pt& pos, int curr_ticks)
{
    m_curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_last_button_down_repeat_time = 0;
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

    m_button_state[mouse_button] = false;
    m_drag_wnds[mouse_button] = 0; // if the mouse button is released, stop the tracking the drag window
    m_wnd_region = WR_NONE;        // and clear this, just in case
    // if the release is over the Wnd where the button-down event occurred, and that Wnd has not been dragged
    if (!in_drag_drop && click_wnd && m_curr_wnd_under_cursor == click_wnd) {
        // if this is second click over a window that just received an click within
        // the time limit -- it's a double-click, not a click
        if (m_double_click_time > 0 && m_double_click_wnd == click_wnd &&
            m_double_click_button == mouse_button) {
            m_double_click_wnd = 0;
            m_double_click_start_time = -1;
            m_double_click_time = -1;
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LDoubleClick, mouse_button), pos, m_mod_keys));
        } else {
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
        m_double_click_wnd = 0;
        m_double_click_time = -1;
        if (click_wnd)
            click_wnd->HandleEvent(WndEvent(ButtonEvent(WndEvent::LButtonUp, mouse_button), pos, m_mod_keys));
        if (m_curr_wnd_under_cursor) {
            if (m_drag_drop_wnds.empty()) {
                if (click_wnd && click_wnd->DragDropDataType() != "" && mouse_button == 0) {
                    m_drag_drop_originating_wnd = click_wnd->Parent();
                    m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                    m_curr_drag_drop_here_wnd = 0;
                    m_drag_drop_wnds_acceptable[click_wnd] = false;
                    m_curr_wnd_under_cursor->DropsAcceptable(m_drag_drop_wnds_acceptable.begin(),
                                                             m_drag_drop_wnds_acceptable.end(),
                                                             pos);
                    std::vector<Wnd*> accepted_wnds;
                    std::vector<const Wnd*> unaccepted_wnds;
                    if (m_drag_drop_wnds_acceptable[click_wnd])
                        accepted_wnds.push_back(click_wnd);
                    else
                        unaccepted_wnds.push_back(click_wnd);
                    if (m_drag_drop_originating_wnd) {
                        m_drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                        m_drag_drop_originating_wnd->ChildrenDraggedAway(accepted_wnds, m_curr_wnd_under_cursor);
                    }
                    if (!accepted_wnds.empty())
                        m_curr_wnd_under_cursor->AcceptDrops(accepted_wnds, pos);
                }
            } else {
                m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                m_curr_drag_drop_here_wnd = 0;
                m_curr_wnd_under_cursor->DropsAcceptable(m_drag_drop_wnds_acceptable.begin(),
                                                         m_drag_drop_wnds_acceptable.end(),
                                                         pos);
                std::vector<Wnd*> accepted_wnds;
                std::vector<const Wnd*> unaccepted_wnds;
                for (std::map<const Wnd*, bool>::iterator it = m_drag_drop_wnds_acceptable.begin();
                     it != m_drag_drop_wnds_acceptable.end();
                     ++it) {
                    if (it->second)
                        accepted_wnds.push_back(const_cast<Wnd*>(it->first));
                    else
                        unaccepted_wnds.push_back(it->first);
                }
                if (m_drag_drop_originating_wnd) {
                    m_drag_drop_originating_wnd->CancellingChildDragDrop(unaccepted_wnds);
                    m_drag_drop_originating_wnd->ChildrenDraggedAway(accepted_wnds, m_curr_wnd_under_cursor);
                }
                if (!accepted_wnds.empty())
                    m_curr_wnd_under_cursor->AcceptDrops(accepted_wnds, pos);
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

void GUIImpl::ClearState()
{
    m_focus_wnd = 0;
    m_mouse_pos = GG::Pt(X(-1000), Y(-1000));
    m_mouse_rel = GG::Pt(X(0), Y(0));
    m_mod_keys = Flags<ModKey>();
    m_last_button_down_repeat_time = 0;

    m_prev_wnd_drag_position = Pt();
    m_browse_info_wnd.reset();
    m_browse_target = 0;

    m_prev_button_press_time = -1;
    m_prev_wnd_under_cursor = 0;
    m_prev_wnd_under_cursor_time = -1;
    m_curr_wnd_under_cursor = 0;

    m_button_state[0] = m_button_state[1] = m_button_state[2] = false;
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

Wnd* GUI::FocusWnd() const
{ return s_impl->m_modal_wnds.empty() ? s_impl->m_focus_wnd : s_impl->m_modal_wnds.back().second; }

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

unsigned int GUI::ButtonDownRepeatDelay() const
{ return s_impl->m_button_down_repeat_delay; }

unsigned int GUI::ButtonDownRepeatInterval() const
{ return s_impl->m_button_down_repeat_interval; }

unsigned int GUI::DoubleClickInterval() const
{ return s_impl->m_double_click_interval; }

unsigned int GUI::MinDragTime() const
{ return s_impl->m_min_drag_time; }

unsigned int GUI::MinDragDistance() const
{ return s_impl->m_min_drag_distance; }

bool GUI::DragDropWnd(const Wnd* wnd) const
{ return s_impl->m_drag_drop_wnds.find(const_cast<Wnd*>(wnd)) != s_impl->m_drag_drop_wnds.end(); }

bool GUI::AcceptedDragDropWnd(const Wnd* wnd) const
{
    std::map<const Wnd*, bool>::const_iterator it = s_impl->m_drag_drop_wnds_acceptable.find(wnd);
    return it != s_impl->m_drag_drop_wnds_acceptable.end() && it->second;
}

bool GUI::MouseButtonDown(unsigned int bn) const
{ return (bn <= 2) ? s_impl->m_button_state[bn] : false; }

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

void GUI::SaveWndAsPNG(const Wnd* wnd, const std::string& filename) const
{
    s_impl->m_save_as_png_wnd = wnd;
    s_impl->m_save_as_png_filename = filename;
}

void GUI::operator()()
{ Run(); }

void GUI::HandleGGEvent(EventType event, Key key, boost::uint32_t key_code_point, Flags<ModKey> mod_keys, const Pt& pos, const Pt& rel)
{
    s_impl->m_mod_keys = mod_keys;

    int curr_ticks = Ticks();

    // track double-click time and time-out any pending double-click that has outlived its interval
    if (s_impl->m_double_click_time >= 0) {
        s_impl->m_double_click_time = curr_ticks - s_impl->m_double_click_start_time;
        if (s_impl->m_double_click_time >= s_impl->m_double_click_interval) {
            s_impl->m_double_click_start_time = -1;
            s_impl->m_double_click_time = -1;
            s_impl->m_double_click_wnd = 0;
        }
    }

    switch (event) {
    case IDLE: {
        if ((s_impl->m_curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys))) {
            if (s_impl->m_button_down_repeat_delay && s_impl->m_curr_wnd_under_cursor->RepeatButtonDown() &&
                s_impl->m_drag_wnds[0] == s_impl->m_curr_wnd_under_cursor) { // convert to a button-down message
                // ensure that the timing requirements are met
                if (curr_ticks - s_impl->m_prev_button_press_time > s_impl->m_button_down_repeat_delay) {
                    if (!s_impl->m_last_button_down_repeat_time ||
                        curr_ticks - s_impl->m_last_button_down_repeat_time > s_impl->m_button_down_repeat_interval) {
                        s_impl->m_last_button_down_repeat_time = curr_ticks;
                        s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::LButtonDown, pos, mod_keys));
                    }
                }
            } else {
                ProcessBrowseInfo();
            }
        }
        break; }
    case KEYPRESS: {
        key = KeyMappedKey(key, s_impl->m_key_map);
        s_impl->m_browse_info_wnd.reset();
        s_impl->m_browse_info_mode = -1;
        s_impl->m_browse_target = 0;
        bool processed = false;
        // only process accelerators when there are no modal windows active; otherwise, accelerators would be an end-run
        // around modality
        if (s_impl->m_modal_wnds.empty()) {
            // the focus_wnd may care about the state of the numlock and capslock, or which side of the keyboard's CTRL,
            // SHIFT, etc. was pressed, but the accelerators don't
            Flags<ModKey> massaged_mods = MassagedAccelModKeys(mod_keys);
            if (s_impl->m_accelerators.find(std::make_pair(key, massaged_mods)) != s_impl->m_accelerators.end())
                processed = AcceleratorSignal(key, massaged_mods)();
        }
        if (!processed && FocusWnd())
            FocusWnd()->HandleEvent(WndEvent(WndEvent::KeyPress, key, key_code_point, mod_keys));
        break; }
    case KEYRELEASE: {
        key = KeyMappedKey(key, s_impl->m_key_map);
        s_impl->m_browse_info_wnd.reset();
        s_impl->m_browse_info_mode = -1;
        s_impl->m_browse_target = 0;
        if (FocusWnd())
            FocusWnd()->HandleEvent(WndEvent(WndEvent::KeyRelease, key, key_code_point, mod_keys));
        break; }
    case MOUSEMOVE: {
        s_impl->m_curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys);

        s_impl->m_mouse_pos = pos; // record mouse position
        s_impl->m_mouse_rel = rel; // record mouse movement

        if (s_impl->m_drag_wnds[0] || s_impl->m_drag_wnds[1] || s_impl->m_drag_wnds[2]) {
            if (s_impl->m_drag_wnds[0])
                s_impl->HandleDrag(0, pos, curr_ticks);
            if (s_impl->m_drag_wnds[1])
                s_impl->HandleDrag(1, pos, curr_ticks);
            if (s_impl->m_drag_wnds[2])
                s_impl->HandleDrag(2, pos, curr_ticks);
        } else if (s_impl->m_curr_wnd_under_cursor && s_impl->m_prev_wnd_under_cursor == s_impl->m_curr_wnd_under_cursor) { // if !s_impl->m_drag_wnds[0] and we're moving over the same (valid) object we were during the last iteration
            s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseHere, pos, mod_keys));
            ProcessBrowseInfo();
        }
        if (s_impl->m_prev_wnd_under_cursor != s_impl->m_curr_wnd_under_cursor) {
            s_impl->m_browse_info_wnd.reset();
            s_impl->m_browse_target = 0;
            s_impl->m_prev_wnd_under_cursor_time = curr_ticks;
        }
        s_impl->m_prev_wnd_under_cursor = s_impl->m_curr_wnd_under_cursor; // update this for the next time around
        break; }
    case LPRESS:
        s_impl->HandlePress((s_impl->m_mouse_lr_swap ? RPRESS : LPRESS) - LPRESS, pos, curr_ticks);
        break;
    case MPRESS:
        s_impl->HandlePress(MPRESS - LPRESS, pos, curr_ticks);
        break;
    case RPRESS:
        s_impl->HandlePress((s_impl->m_mouse_lr_swap ? LPRESS : RPRESS) - LPRESS, pos, curr_ticks);
        break;
    case LRELEASE:
        s_impl->HandleRelease((s_impl->m_mouse_lr_swap ? RRELEASE : LRELEASE) - LRELEASE, pos, curr_ticks);
        break;
    case MRELEASE:
        s_impl->HandleRelease(MRELEASE - LRELEASE, pos, curr_ticks);
        break;
    case RRELEASE:
        s_impl->HandleRelease((s_impl->m_mouse_lr_swap ? LRELEASE : RRELEASE) - LRELEASE, pos, curr_ticks);
        break;
    case MOUSEWHEEL: {
        s_impl->m_curr_wnd_under_cursor = CheckedGetWindowUnder(pos, mod_keys);
        s_impl->m_browse_info_wnd.reset();
        s_impl->m_browse_target = 0;
        s_impl->m_prev_wnd_under_cursor_time = curr_ticks;
        // don't send out 0-movement wheel messages
        if (s_impl->m_curr_wnd_under_cursor && rel.y)
            s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseWheel, pos, Value(rel.y), mod_keys));
        s_impl->m_prev_wnd_under_cursor = s_impl->m_curr_wnd_under_cursor; // update this for the next time around
        break; }
    default:
        break;
    }
}

void GUI::ClearEventState()
{ s_impl->ClearState(); }

void GUI::SetFocusWnd(Wnd* wnd)
{
    // inform old focus wnd that it is losing focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::LosingFocus));

    (s_impl->m_modal_wnds.empty() ? s_impl->m_focus_wnd : s_impl->m_modal_wnds.back().second) = wnd;

    // inform new focus wnd that it is gaining focus
    if (FocusWnd())
        FocusWnd()->HandleEvent(WndEvent(WndEvent::GainingFocus));
}

void GUI::Wait(unsigned int ms)
{
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms));
}

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
    if (wnd) {
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

void GUI::EnableMouseButtonDownRepeat(unsigned int delay, unsigned int interval)
{
    if (!delay) { // setting delay = 0 completely disables mouse drag repeat
        s_impl->m_button_down_repeat_delay = 0;
        s_impl->m_button_down_repeat_interval = 0;
    } else {
        s_impl->m_button_down_repeat_delay = delay;
        s_impl->m_button_down_repeat_interval = interval;
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

boost::shared_ptr<Texture> GUI::GetTexture(const std::string& name, bool mipmap/* = false*/)
{ return GetTextureManager().GetTexture(name, mipmap); }

void GUI::FreeTexture(const std::string& name)
{ GetTextureManager().FreeTexture(name); }

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

const std::string& GUI::ClipboardText() const
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
                SetClipboardText(selected_text);
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
    return PasteFocusWndText(s_impl->m_clipboard_text);
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

GUI* GUI::GetGUI()
{ return s_gui; }

void GUI::RenderWindow(Wnd* wnd)
{
    if (wnd && wnd->Visible()) {
        wnd->Render();

        Wnd::ChildClippingMode clip_mode = wnd->GetChildClippingMode();

        if (clip_mode != Wnd::ClipToClientAndWindowSeparately) {
            bool clip = clip_mode != Wnd::DontClip;
            if (clip)
                wnd->BeginClipping();
            for (std::list<Wnd*>::iterator it = wnd->m_children.begin(); it != wnd->m_children.end(); ++it) {
                if ((*it)->Visible())
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
                    if ((*it)->Visible())
                        RenderWindow(*it);
                }
                wnd->EndNonclientClipping();
            }

            if (client_child_begin != children_copy.end()) {
                wnd->BeginClipping();
                for (std::vector<Wnd*>::iterator it = client_child_begin; it != children_copy.end(); ++it) {
                    if ((*it)->Visible())
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

void GUI::ProcessBrowseInfo()
{
    assert(s_impl->m_curr_wnd_under_cursor);
    if (!s_impl->m_button_state[0] && !s_impl->m_button_state[1] && !s_impl->m_button_state[2] &&
        (s_impl->m_modal_wnds.empty() || s_impl->m_curr_wnd_under_cursor->RootParent() == s_impl->m_modal_wnds.back().first)) {
        Wnd* wnd = s_impl->m_curr_wnd_under_cursor;
        while (!ProcessBrowseInfoImpl(wnd) && wnd->Parent() && (dynamic_cast<Control*>(wnd) || dynamic_cast<Layout*>(wnd))) {
            wnd = wnd->Parent();
        }
    }
}

void GUI::Render()
{
    // handle timers
    int ticks = Ticks();
    for (std::set<Timer*>::iterator it = s_impl->m_timers.begin(); it != s_impl->m_timers.end(); ++it) {
        (*it)->Update(ticks);
    }

    Enter2DMode();
    // render normal windows back-to-front
    for (ZList::reverse_iterator it = s_impl->m_zlist.rbegin(); it != s_impl->m_zlist.rend(); ++it) {
        RenderWindow(*it);
    }
    // render modal windows back-to-front
    for (std::list<std::pair<Wnd*, Wnd*> >::iterator it = s_impl->m_modal_wnds.begin(); it != s_impl->m_modal_wnds.end(); ++it) {
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
    Wnd* dragged_wnd = s_impl->m_curr_drag_wnd;
    bool unregistered_drag_drop =
        dragged_wnd && !dragged_wnd->DragDropDataType().empty();
    bool registered_drag_drop = !s_impl->m_drag_drop_wnds.empty();
    std::map<Wnd*, Pt> drag_drop_wnds;
    drag_drop_wnds[dragged_wnd] = s_impl->m_wnd_drag_offset;
    if (s_impl->m_curr_drag_drop_here_wnd && !unregistered_drag_drop && !registered_drag_drop) {
        s_impl->m_curr_drag_drop_here_wnd->HandleEvent(WndEvent(WndEvent::DragDropLeave));
        s_impl->m_curr_drag_drop_here_wnd = 0;
    }
    if (w != s_impl->m_curr_wnd_under_cursor) {
        if (s_impl->m_curr_wnd_under_cursor) {
            if (unregistered_drag_drop) {
                s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                s_impl->m_drag_drop_wnds_acceptable[dragged_wnd] = false;
                s_impl->m_curr_drag_drop_here_wnd = 0;
            } else if (registered_drag_drop) {
                s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::DragDropLeave));
                for (std::map<const Wnd*, bool>::iterator it = s_impl->m_drag_drop_wnds_acceptable.begin();
                     it != s_impl->m_drag_drop_wnds_acceptable.end();
                     ++it) {
                    it->second = false;
                }
                s_impl->m_curr_drag_drop_here_wnd = 0;
            } else {
                s_impl->m_curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::MouseLeave));
            }
        }
        if (w) {
            if (unregistered_drag_drop) {
                w->HandleEvent(WndEvent(WndEvent::DragDropEnter, pt, drag_drop_wnds, mod_keys));
                s_impl->m_drag_drop_wnds_acceptable[dragged_wnd] = false;
                w->DropsAcceptable(s_impl->m_drag_drop_wnds_acceptable.begin(),
                                   s_impl->m_drag_drop_wnds_acceptable.end(),
                                   pt);
                s_impl->m_curr_drag_drop_here_wnd = w;
            } else if (registered_drag_drop) {
                w->HandleEvent(WndEvent(WndEvent::DragDropEnter, pt, s_impl->m_drag_drop_wnds, mod_keys));
                w->DropsAcceptable(s_impl->m_drag_drop_wnds_acceptable.begin(),
                                   s_impl->m_drag_drop_wnds_acceptable.end(),
                                   pt);
                s_impl->m_curr_drag_drop_here_wnd = w;
            } else {
                w->HandleEvent(WndEvent(WndEvent::MouseEnter, pt, mod_keys));
            }
        }
    }
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
