//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/Config.h>
#include <cassert>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <thread>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/xpressive/xpressive.hpp>
#if GG_HAVE_LIBPNG
# include <boost/gil/extension/io/png.hpp>
#endif
#include <GG/BrowseInfoWnd.h>
#include <GG/Cursor.h>
#include <GG/Edit.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/ListBox.h>
#include <GG/StyleFactory.h>
#include <GG/Timer.h>
#include <GG/utf8/checked.h>
#include <GG/ZList.h>


using namespace GG;
namespace gil = boost::gil;

namespace {

constexpr bool INSTRUMENT_GET_WINDOW_UNDER = false;

struct AcceleratorEcho
{
    AcceleratorEcho(Key key, Flags<ModKey> mod_keys) :
        m_str(std::string{"GG SIGNAL : GUI::AcceleratorSignal(key="}.append(to_string(key))
              .append(" mod_keys=").append(to_string(mod_keys)).append(")"))
    {}
    bool operator()()
    {
        std::cerr << m_str << std::endl;
        return false;
    }
    std::string m_str;
};

// calculates WndEvent::EventType corresponding to a given mouse button
// and a given left mouse button event type. For example, given the 
// left mouse button drag and button 2 (the right mouse button),
// this will return right button drag.
WndEvent::EventType ButtonEvent(WndEvent::EventType left_type, unsigned int mouse_button)
{
    return WndEvent::EventType(int(left_type) +
        (int(WndEvent::EventType::MButtonDown) - int(WndEvent::EventType::LButtonDown)) * mouse_button);
}

namespace {
    using utf8_wchar_iterator = utf8::iterator<std::string_view::const_iterator, wchar_t> ;
    using word_regex = boost::xpressive::basic_regex<utf8_wchar_iterator>;
    using word_regex_iterator = boost::xpressive::regex_iterator<utf8_wchar_iterator>;

    constexpr wchar_t WIDE_DASH = u'-';
    const word_regex DEFAULT_WORD_REGEX =
        +boost::xpressive::set[boost::xpressive::_w | WIDE_DASH];
}

void WriteWndToPNG(const Wnd* wnd, const std::string& filename)
{
#if GG_HAVE_LIBPNG
    const Pt ul = wnd->UpperLeft();
    const Pt size = wnd->Size();

    std::vector<GLubyte> bytes(static_cast<std::size_t>(Value(size.x) * Value(size.y) * 4));

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

    gil::write_view(
        filename,
        gil::flipped_up_down_view(
            gil::interleaved_view(
                Value(size.x),
                Value(size.y),
                static_cast<gil::rgba8_pixel_t*>(static_cast<void*>(bytes.data())),
                Value(size.x) * sizeof(gil::rgba8_pixel_t))),
        gil::png_tag());
#endif
}

}


// implementation data types
struct GG::GUIImpl
{
    explicit GUIImpl(std::string app_name);

    void HandleMouseButtonPress(  unsigned int mouse_button, Pt pos, int curr_ticks);
    void HandleMouseDrag(         unsigned int mouse_button, Pt pos, int curr_ticks);
    void HandleMouseButtonRelease(unsigned int mouse_button, Pt pos, int curr_ticks);
    void HandleIdle(              Flags<ModKey> mod_keys, Pt pos, int curr_ticks);

    void HandleKeyPress(          Key key, uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);

    void HandleKeyRelease(        Key key, uint32_t key_code_point, Flags<ModKey> mod_keys, int curr_ticks);

    void HandleTextInput(         std::string text);
    void HandleMouseMove(         Flags<ModKey> mod_keys, Pt pos, Pt rel, int curr_ticks);
    void HandleMouseWheel(        Flags<ModKey> mod_keys, Pt pos, Pt rel, int curr_ticks);
    void HandleMouseEnter(        Flags<ModKey> mod_keys, Pt pos, std::shared_ptr<Wnd> w);

    void ClearState();

    [[nodiscard]] std::shared_ptr<Wnd> FocusWnd() const;
    void SetFocusWnd(const std::shared_ptr<Wnd>& wnd);

    void GouvernFPS();

    std::string  m_app_name;            // the user-defined name of the application

    ZList               m_zlist;        // object that keeps the GUI windows in the correct depth ordering
    std::weak_ptr<Wnd>  m_focus_wnd;    // GUI window that currently has the input focus (this is the base level focus window, used when no modal windows are active)

    std::vector<std::pair<std::shared_ptr<Wnd>, std::weak_ptr<Wnd>>>
                        m_modal_wnds;                               // modal GUI windows, and the window with focus for that modality (only the one in back is active, simulating a stack but allowing traversal of the list)
    bool                m_allow_modal_accelerator_signals = false;  // iff true: keyboard accelerator signals will be output while modal window(s) is open

    bool         m_mouse_button_state[3] = {false, false, false};   // the up/down states of the three buttons on the mouse are kept here
    Pt           m_mouse_pos{GG::X(-1000), GG::Y(-1000)};           // absolute position of mouse, based on last MOUSEMOVE event
    Pt           m_mouse_rel{GG::X0, GG::Y0};                       // relative position of mouse, based on last MOUSEMOVE event
    Flags<ModKey>m_mod_keys;                                        // currently-depressed modifier keys, based on last KEYPRESS event

    int          m_key_press_repeat_delay = 250;            // see note above GUI class definition
    int          m_key_press_repeat_interval = 66;
    int          m_last_key_press_repeat_time = 0;          // last time of a simulated key press message

    std::pair<Key, uint32_t> m_last_pressed_key_code_point{Key::GGK_NONE, 0u};

    int          m_prev_key_press_time = -1;                // the time of the most recent key press

    int          m_mouse_button_down_repeat_delay = 250;    // see note above GUI class definition
    int          m_mouse_button_down_repeat_interval = 66;
    int          m_last_mouse_button_down_repeat_time= 0;   // last time of a simulated button-down message

    int          m_double_click_interval = 500; // the maximum interval allowed between clicks that is still considered a double-click, in ms
    int          m_min_drag_time = 250;         // the minimum amount of time that a drag must be in progress before it is considered a drag, in ms
    int          m_min_drag_distance = 5;       // the minimum distance that a drag must cover before it is considered a drag

    int                 m_prev_mouse_button_press_time = -1;// the time of the most recent mouse button press
    Pt                  m_prev_mouse_button_press_pos;      // the location of the most recent mouse button press
    std::weak_ptr<Wnd>  m_prev_wnd_under_cursor;            // GUI window most recently under the input cursor; may be 0
    int                 m_prev_wnd_under_cursor_time = -1;  // the time at which prev_wnd_under_cursor was initially set to its current value
    std::weak_ptr<Wnd>  m_curr_wnd_under_cursor;            // GUI window currently under the input cursor; may be 0
    std::weak_ptr<Wnd>  m_drag_wnds[3];                     // GUI window currently being clicked or dragged by each mouse button
    Pt                  m_prev_wnd_drag_position;           // the upper-left corner of the dragged window when the last *Drag message was generated
    Pt                  m_wnd_drag_offset;                  // the offset from the upper left corner of the dragged window to the cursor for the current drag
    bool                m_curr_drag_wnd_dragged = false;    // true iff the currently-pressed window (m_drag_wnds[N]) has actually been dragged some distance (in which case releasing the mouse button is not a click). note that a dragged wnd is one being continuously repositioned by the dragging, and not a wnd being drag-dropped.
    std::shared_ptr<Wnd>m_curr_drag_wnd;                    // nonzero iff m_curr_drag_wnd_dragged is true (that is, we have actually started dragging the Wnd, not just pressed the mouse button); will always be one of m_drag_wnds.
    std::weak_ptr<Wnd>  m_curr_drag_drop_here_wnd;          // the Wnd that most recently received a DragDropEnter or DragDropHere message (0 if DragDropLeave was sent as well, or if none)
    Pt                  m_wnd_resize_offset;                // offset from the cursor of either the upper-left or lower-right corner of the GUI window currently being resized
    WndRegion           m_wnd_region = WndRegion::WR_NONE;  // window region currently being dragged or clicked; for non-frame windows, this will always be WR_NONE

    /** The current browse info window, if any. */
    std::shared_ptr<BrowseInfoWnd> m_browse_info_wnd;

    int                 m_browse_info_mode = 0;      // the current browse info mode (only valid if browse_info_wnd is non-null)
    Wnd*                m_browse_target = nullptr;   // the current browse info target

    std::weak_ptr<Wnd>  m_drag_drop_originating_wnd; // the window that originally owned the Wnds in drag_drop_wnds

    /** The Wnds currently being dragged and dropped. They are owned by the GUI and rendered separately.*/
    std::map<std::shared_ptr<Wnd>, Pt> m_drag_drop_wnds;

    /** Tracks whether Wnd is acceptable for dropping on the current target Wnd.*/
    std::map<const Wnd*, bool> m_drag_drop_wnds_acceptable;

    std::vector<std::pair<Key, Flags<ModKey>>> m_accelerators; // the keyboard accelerators

    /** The signals emitted by the keyboard accelerators. */
    std::vector<std::pair<std::pair<Key, Flags<ModKey>>,
                          std::unique_ptr<GUI::AcceleratorSignalType>>> m_accelerator_sigs;

    bool m_mouse_lr_swap = false; // treat left and right mouse events as each other

    bool m_rendering_drag_drop_wnds = false;

    //! The most recent calculation of the frames per second rendering speed (-1.0 if calcs are disabled)
    double m_FPS = 0.0;         //! true iff FPS calcs are to be done
    bool m_calc_FPS = false;    //! true iff FPS calcs are to be done
    double m_max_FPS = 60.0;    //! The maximum allowed frames per second rendering speed

    std::chrono::high_resolution_clock::time_point m_last_FPS_time;     //! The last time an FPS calculation was done.
    std::chrono::high_resolution_clock::time_point m_last_frame_time;   //! The time of the last frame rendered.

    std::size_t  m_frames = 0;                  //! The number of frames rendered since \a m_last_frame_time.

    Wnd*         m_double_click_wnd = nullptr;  //! GUI window most recently clicked
    unsigned int m_double_click_button = 0;     // the index of the mouse button used in the last click
    int          m_double_click_start_time = -1;//! the time from which we started measuring double_click_time, in ms
    int          m_double_click_time = -1;      //! time elapsed since last click, in ms

    std::unique_ptr<const StyleFactory> m_style_factory;
    std::unique_ptr<const Cursor>       m_cursor;
    bool                                m_render_cursor = false;

    std::set<Timer*> m_timers;

    const Wnd* m_save_as_png_wnd = nullptr;
    std::string m_save_as_png_filename;

    std::string m_clipboard_text;
};

GUIImpl::GUIImpl(std::string app_name) :
    m_app_name(std::move(app_name)),
    m_last_FPS_time(std::chrono::high_resolution_clock::now()),
    m_last_frame_time(std::chrono::high_resolution_clock::now())
{}

void GUIImpl::HandleMouseButtonPress(unsigned int mouse_button, Pt pos, int curr_ticks)
{
    const auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
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
        if (int(m_wnd_region) % 3 == 0) // left regions
            m_wnd_resize_offset.x = curr_wnd_under_cursor->Left() - pos.x;
        else
            m_wnd_resize_offset.x = curr_wnd_under_cursor->Right() - pos.x;
        if (int(m_wnd_region) < 3) // top regions
            m_wnd_resize_offset.y = curr_wnd_under_cursor->Top() - pos.y;
        else
            m_wnd_resize_offset.y = curr_wnd_under_cursor->Bottom() - pos.y;
        auto&& drag_wnds_root_parent = curr_wnd_under_cursor->RootParent();
        GUI::s_gui->MoveUp(drag_wnds_root_parent ? drag_wnds_root_parent : curr_wnd_under_cursor);
        curr_wnd_under_cursor->HandleEvent(WndEvent(
            ButtonEvent(WndEvent::EventType::LButtonDown, mouse_button), pos, m_mod_keys));
    }

    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseDrag(unsigned int mouse_button, Pt pos, int curr_ticks)
{
    const auto dragged_wnd = LockAndResetIfExpired(m_drag_wnds[mouse_button]);
    if (!dragged_wnd)
        return;

    if (m_wnd_region == WndRegion::WR_MIDDLE || m_wnd_region == WndRegion::WR_NONE) {
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
                dragged_wnd->HandleEvent(WndEvent(ButtonEvent(
                    WndEvent::EventType::LDrag, mouse_button), pos, move, m_mod_keys));

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
            auto curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow());
            m_curr_wnd_under_cursor = curr_wnd_under_cursor;
            std::map<std::shared_ptr<Wnd>, Pt> drag_drop_wnds;
            drag_drop_wnds[dragged_wnd] = m_wnd_drag_offset;
            const auto prev_wnd_under_cursor = LockAndResetIfExpired(m_prev_wnd_under_cursor);
            if (curr_wnd_under_cursor && prev_wnd_under_cursor == curr_wnd_under_cursor) {
                // Wnd under cursor has remained the same for the last two updates
                const auto curr_drag_drop_here_wnd = LockAndResetIfExpired(m_curr_drag_drop_here_wnd);
                if (curr_drag_drop_here_wnd == curr_wnd_under_cursor) {
                    // Wnd being dragged over is still being dragged over...
                    WndEvent event(WndEvent::EventType::DragDropHere, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                } else {
                    // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
                    WndEvent event(WndEvent::EventType::CheckDrops, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // Wnd being dragged over is new; give it an Enter message
                    WndEvent enter_event(WndEvent::EventType::DragDropEnter, pos, m_drag_drop_wnds, m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(enter_event);
                    m_curr_drag_drop_here_wnd = curr_wnd_under_cursor;
                }
            }
        }

    } else if (dragged_wnd->Resizable()) {
        // send appropriate resize message to window, depending on the position
        // of the cursor within / at the edge of the Wnd being dragged over
        Pt offset_pos = pos + m_wnd_resize_offset;
        if (auto parent = dragged_wnd->Parent())
            offset_pos -= parent->ClientUpperLeft();
        const GG::Pt rel_lr = dragged_wnd->RelativeLowerRight();
        const GG::Pt rel_ul = dragged_wnd->RelativeUpperLeft();

        switch (m_wnd_region)
        {
        case WndRegion::WR_TOPLEFT:
            dragged_wnd->SizeMove(offset_pos,                       rel_lr);
            break;
        case WndRegion::WR_TOP:
            dragged_wnd->SizeMove(Pt(rel_ul.x,      offset_pos.y),  rel_lr);
            break;
        case WndRegion::WR_TOPRIGHT:
            dragged_wnd->SizeMove(Pt(rel_ul.x,      offset_pos.y),  Pt(offset_pos.x,    rel_lr.y));
            break;
        case WndRegion::WR_MIDLEFT:
            dragged_wnd->SizeMove(Pt(offset_pos.x,  rel_ul.y),      rel_lr);
            break;
        case WndRegion::WR_MIDRIGHT:
            dragged_wnd->SizeMove(rel_ul,                           Pt(offset_pos.x,    rel_lr.y));
            break;
        case WndRegion::WR_BOTTOMLEFT:
            dragged_wnd->SizeMove(Pt(offset_pos.x,  rel_ul.y),      Pt(rel_lr.x,        offset_pos.y));
            break;
        case WndRegion::WR_BOTTOM:
            dragged_wnd->SizeMove(rel_ul,                           Pt(rel_lr.x,        offset_pos.y));
            break;
        case WndRegion::WR_BOTTOMRIGHT:
            dragged_wnd->SizeMove(rel_ul,                           offset_pos);
            break;
        default:
            break;
        }
    }
}

void GUIImpl::HandleMouseButtonRelease(unsigned int mouse_button, Pt pos, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    m_last_mouse_button_down_repeat_time = 0;
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;
    m_prev_wnd_under_cursor_time = curr_ticks;

    const auto click_drag_wnd = LockAndResetIfExpired(m_drag_wnds[mouse_button]);

    std::vector<const Wnd*> ignores;
    if (m_curr_drag_wnd_dragged && click_drag_wnd)
        ignores.push_back(click_drag_wnd.get());
    curr_wnd_under_cursor = m_zlist.Pick(pos, GUI::s_gui->ModalWindow(), ignores);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;

    bool in_drag_drop =
        !m_drag_drop_wnds.empty() ||
        (m_curr_drag_wnd_dragged && click_drag_wnd && (click_drag_wnd->DragDropDataType() != "") && (mouse_button == 0));

    m_mouse_button_state[mouse_button] = false;
    m_drag_wnds[mouse_button].reset(); // if the mouse button is released, stop tracking the drag window
    m_wnd_region = WndRegion::WR_NONE; // and clear this, just in case

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
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(
                WndEvent::EventType::LDoubleClick, mouse_button), pos, m_mod_keys));

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
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(
                WndEvent::EventType::LClick, mouse_button), pos, m_mod_keys));
        }

    } else if (click_drag_wnd) {
        // drag-dropping
        m_double_click_wnd = nullptr;
        m_double_click_time = -1;
        if (click_drag_wnd)
            click_drag_wnd->HandleEvent(WndEvent(ButtonEvent(
                WndEvent::EventType::LButtonUp, mouse_button), pos, m_mod_keys));

        if (curr_wnd_under_cursor) {
            // dropped onto a Wnd, which can react to the drop

            if (m_drag_drop_wnds.empty()) {
                // dropped a dragged Wnd without having dragged it anywhere yet
                if (click_drag_wnd && click_drag_wnd->DragDropDataType() != "" && mouse_button == 0) {
                    // pass drag-drop-here event to check if the single dragged Wnd is acceptable to drop
                    WndEvent event(WndEvent::EventType::CheckDrops, pos,
                                   click_drag_wnd.get(), m_mod_keys);
                    curr_wnd_under_cursor->HandleEvent(event);
                    m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                    // prep / handle end of drag-drop
                    auto&& drag_drop_originating_wnd = click_drag_wnd->Parent();
                    m_drag_drop_originating_wnd = drag_drop_originating_wnd;
                    curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::DragDropLeave));
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
                        curr_wnd_under_cursor->HandleEvent(WndEvent(
                            WndEvent::EventType::DragDroppedOn, pos,
                            std::move(accepted_wnds), m_mod_keys));
                }

            } else {
                // dragged one or more Wnds to another location and then dropped them
                // pass checkdrops event to check if the dropped Wnds are acceptable to drop here
                WndEvent event(WndEvent::EventType::CheckDrops, pos, m_drag_drop_wnds, m_mod_keys);
                curr_wnd_under_cursor->HandleEvent(event);
                m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

                // prep / handle end of drag-drop
                auto&& drag_drop_originating_wnd = click_drag_wnd->Parent();
                m_drag_drop_originating_wnd = drag_drop_originating_wnd;
                curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::DragDropLeave));
                m_curr_drag_drop_here_wnd.reset();

                // put dragged Wnds into containers depending on whether they were accepted by the drop target
                std::vector<std::shared_ptr<Wnd>> accepted_wnds;
                std::vector<Wnd*> removed_wnds;
                std::vector<const Wnd*> unaccepted_wnds;
                for (auto& drop_wnd : m_drag_drop_wnds) {
                    if (m_drag_drop_wnds_acceptable[drop_wnd.first.get()]) {
                        accepted_wnds.emplace_back(drop_wnd.first);
                        removed_wnds.emplace_back(const_cast<Wnd*>(drop_wnd.first.get()));
                    } else
                        unaccepted_wnds.emplace_back(drop_wnd.first.get());
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
                        WndEvent::EventType::DragDroppedOn, pos, std::move(accepted_wnds), m_mod_keys));
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

void GUIImpl::HandleIdle(Flags<ModKey> mod_keys, const Pt pos, int curr_ticks)
{
    const auto curr_wnd_under_cursor = LockAndResetIfExpired(m_curr_wnd_under_cursor);
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
                    WndEvent::EventType::LButtonDown, pos, mod_keys));
            }
        }

        return;
    }


    if (m_key_press_repeat_delay != 0 &&
        m_last_pressed_key_code_point.first != Key::GGK_NONE)
    {
        const auto focus_wnd = FocusWnd();
        if (focus_wnd && focus_wnd->RepeatKeyPress()) {
            // convert to a key press message after ensuring that timing requirements are met
            if (curr_ticks - m_prev_key_press_time > m_key_press_repeat_delay) {
                if (!m_last_key_press_repeat_time ||
                    curr_ticks - m_last_key_press_repeat_time > m_key_press_repeat_interval)
                {
                    m_last_key_press_repeat_time = curr_ticks;
                    focus_wnd->HandleEvent(
                        WndEvent(WndEvent::EventType::KeyPress, m_last_pressed_key_code_point.first,
                                 m_last_pressed_key_code_point.second, mod_keys));
                }
            }
            return;
        }
    }

    if (curr_wnd_under_cursor)
        GUI::s_gui->ProcessBrowseInfo();
}

void GUIImpl::HandleKeyPress(Key key, uint32_t key_code_point,
                             Flags<ModKey> mod_keys, int curr_ticks)
{
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
        const std::pair key_mods{key, MassagedAccelModKeys(mod_keys)};
        if (std::find(m_accelerators.begin(), m_accelerators.end(), key_mods) != m_accelerators.end())
            processed = GUI::s_gui->AcceleratorSignal(key, key_mods.second)();
    }
    if (!processed)
        if (auto focus_wnd = FocusWnd())
            focus_wnd->HandleEvent(WndEvent(WndEvent::EventType::KeyPress, key, key_code_point, mod_keys));
}

void GUIImpl::HandleKeyRelease(Key key, uint32_t key_code_point,
                               Flags<ModKey> mod_keys, int curr_ticks)
{
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point.first = Key::GGK_NONE;
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = nullptr;
    auto&& focus_wnd = FocusWnd();
    if (focus_wnd)
        focus_wnd->HandleEvent(WndEvent(
            WndEvent::EventType::KeyRelease, key, key_code_point, mod_keys));
}

void GUIImpl::HandleTextInput(std::string text) {
    m_browse_info_wnd.reset();
    m_browse_info_mode = -1;
    m_browse_target = nullptr;
    auto&& focus_wnd = FocusWnd();
    if (focus_wnd)
        focus_wnd->HandleEvent(WndEvent(WndEvent::EventType::TextInput, std::move(text)));
}

void GUIImpl::HandleMouseMove(Flags<ModKey> mod_keys, Pt pos, Pt rel,
                              int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    const auto&& prev_wnd_under_cursor = LockAndResetIfExpired(m_prev_wnd_under_cursor);

    m_mouse_pos = pos; // record mouse position
    m_mouse_rel = rel; // record mouse movement

    const auto m_drag_wnds_0 = LockAndResetIfExpired(m_drag_wnds[0]);
    const auto m_drag_wnds_1 = LockAndResetIfExpired(m_drag_wnds[1]);
    const auto m_drag_wnds_2 = LockAndResetIfExpired(m_drag_wnds[2]);
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
        curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::MouseHere, pos, mod_keys));
        GUI::s_gui->ProcessBrowseInfo();
    }
    if (prev_wnd_under_cursor != curr_wnd_under_cursor) {
        m_browse_info_wnd.reset();
        m_browse_target = nullptr;
        m_prev_wnd_under_cursor_time = curr_ticks;
    }
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseWheel(Flags<ModKey> mod_keys, Pt pos, Pt rel, int curr_ticks)
{
    auto curr_wnd_under_cursor = GUI::s_gui->CheckedGetWindowUnder(pos, m_mod_keys);
    m_curr_wnd_under_cursor = curr_wnd_under_cursor;
    m_browse_info_wnd.reset();
    m_browse_target = nullptr;
    m_prev_wnd_under_cursor_time = curr_ticks;
    // don't send out 0-movement wheel messages
    if (curr_wnd_under_cursor && rel.y != Y0)
        curr_wnd_under_cursor->HandleEvent(WndEvent(
            WndEvent::EventType::MouseWheel, pos, Value(rel.y), mod_keys));
    m_prev_wnd_under_cursor = m_curr_wnd_under_cursor; // update this for the next time around
}

void GUIImpl::HandleMouseEnter(Flags<ModKey> mod_keys, Pt pos, std::shared_ptr<Wnd> w)
{
    w->HandleEvent(WndEvent(WndEvent::EventType::MouseEnter, pos, mod_keys));
    m_curr_wnd_under_cursor = std::move(w);
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
        focus_wnd->HandleEvent(WndEvent(WndEvent::EventType::LosingFocus));

    if (m_modal_wnds.empty())
        m_focus_wnd = wnd;
    else
        // m_modal_wnds stores the window that is modal and the child that has
        // focus separately.
        m_modal_wnds.back().second = wnd;

    // inform new focus wnd that it is gaining focus
    auto&& new_focus_wnd = FocusWnd();
    if (new_focus_wnd)
        new_focus_wnd->HandleEvent(WndEvent(WndEvent::EventType::GainingFocus));
}

void GUIImpl::ClearState()
{
    m_focus_wnd.reset();
    m_mouse_pos = GG::Pt(X(-1000), Y(-1000));
    m_mouse_rel = GG::Pt(X(0), Y(0));
    m_mod_keys = Flags<ModKey>();
    m_last_mouse_button_down_repeat_time = 0;
    m_last_key_press_repeat_time = 0;
    m_last_pressed_key_code_point = {Key::GGK_NONE, 0u};

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
    m_wnd_region = WndRegion::WR_NONE;
    m_browse_target = nullptr;
    m_drag_drop_originating_wnd.reset();

    m_double_click_wnd = nullptr;
    m_double_click_start_time = -1;
    m_double_click_time = -1;
}

void GUIImpl::GouvernFPS()
{
    using namespace std::chrono;

    high_resolution_clock::time_point time = high_resolution_clock::now();

    // govern FPS speed if needed
    if (m_max_FPS) {
        microseconds min_us_per_frame = duration_cast<microseconds>(duration<double>(1.0 / (m_max_FPS + 1)));
        microseconds us_elapsed = duration_cast<microseconds>(time - m_last_frame_time);
        microseconds us_to_wait = (min_us_per_frame - us_elapsed);
        if (microseconds(0) < us_to_wait) {
            std::this_thread::sleep_for(us_to_wait);
            time = high_resolution_clock::now();
        }
    }

    m_last_frame_time = time;

    // track FPS if needed
    if (m_calc_FPS) {
        ++m_frames;
        if (seconds(1) < time - m_last_FPS_time) { // calculate FPS at most once a second
            double time_since_last_FPS = duration_cast<microseconds>(
                time - m_last_FPS_time).count() / 1000000.0;
            m_FPS = m_frames / time_since_last_FPS;
            m_last_FPS_time = time;
            m_frames = 0;
        }
    }
}


// static member(s)
GUI* GUI::s_gui = nullptr;

// member functions
GUI::GUI(std::string app_name) :
    m_impl(std::make_unique<GUIImpl>(std::move(app_name)))
{
    assert(!s_gui);
    s_gui = this;
}

GUI::~GUI()
{
    s_gui = nullptr; // probly optimized away :/
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
    auto focus_wnd{FocusWnd()};
    if (!focus_wnd)
        return focus_wnd;

    auto parent_of_focus_wnd{focus_wnd->Parent()};
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find previous INTERACTIVE sibling wnd
    const auto& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    const auto focus_it = std::find(siblings.rbegin(), siblings.rend(), focus_wnd);
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
    auto focus_wnd{FocusWnd()};
    if (!focus_wnd)
        return focus_wnd;

    auto parent_of_focus_wnd{focus_wnd->Parent()};
    if (!parent_of_focus_wnd)
        return focus_wnd;

    // find next INTERACTIVE sibling wnd
    const auto& siblings = parent_of_focus_wnd->Children();

    // find current focus wnd in siblings...
    const auto focus_it = std::find(siblings.begin(), siblings.end(), focus_wnd);
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

std::shared_ptr<Wnd> GUI::GetWindowUnder(Pt pt) const
{
    auto wnd{m_impl->m_zlist.Pick(pt, ModalWindow())};
    if constexpr (INSTRUMENT_GET_WINDOW_UNDER && wnd)
        std::cerr << "GUI::GetWindowUnder() : " << wnd->Name() << " @ " << wnd << std::endl;
    return wnd;
}

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
    const auto it = m_impl->m_drag_drop_wnds_acceptable.find(wnd);
    return it != m_impl->m_drag_drop_wnds_acceptable.end() && it->second;
}

bool GUI::MouseButtonDown(unsigned int bn) const
{ return (bn <= 2) ? m_impl->m_mouse_button_state[bn] : false; }

Pt GUI::MousePosition() const noexcept
{ return m_impl->m_mouse_pos; }

Pt GUI::MouseMovement() const noexcept
{ return m_impl->m_mouse_rel; }

Flags<ModKey> GUI::ModKeys() const noexcept
{ return m_impl->m_mod_keys; }

bool GUI::MouseLRSwapped() const noexcept
{ return m_impl->m_mouse_lr_swap; }

std::vector<std::pair<CPSize, CPSize>> GUI::FindWords(std::string_view str) const
{
    std::vector<std::pair<CPSize, CPSize>> retval;

    try {
        const utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
        const utf8_wchar_iterator last(str.end(), str.begin(), str.end());
        const word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
        const word_regex_iterator end_it;

        std::transform(it, end_it, std::back_inserter(retval),
                       [](const word_regex_iterator::value_type& match_result) -> std::pair<CPSize, CPSize>
        { return { CPSize(match_result.position()), CPSize(match_result.position() + match_result.length()) }; });

    } catch (...) {}

    return retval;
}

std::vector<std::pair<StrSize, StrSize>> GUI::FindWordsStringIndices(std::string_view str) const
{
    std::vector<std::pair<StrSize, StrSize>> retval;

    try {
        const utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
        const utf8_wchar_iterator last(str.end(), str.begin(), str.end());
        const word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
        const word_regex_iterator end_it;

        std::transform(it, end_it, std::back_inserter(retval),
                       [first, begin{str.begin()}](const word_regex_iterator::value_type& match_result) -> std::pair<StrSize, StrSize>
        {
            auto word_pos_it = first;
            std::advance(word_pos_it, match_result.position());
            StrSize start_idx{static_cast<std::size_t>(std::distance(begin, word_pos_it.base()))};
            std::advance(word_pos_it, match_result.length());
            StrSize end_idx{static_cast<std::size_t>(std::distance(begin, word_pos_it.base()))};

            return {start_idx, end_idx};
        });
    } catch (...) {}

    return retval;
}

std::vector<std::string_view> GUI::FindWordsStringViews(std::string_view str) const
{
    std::vector<std::string_view> retval;

    try {
        const utf8_wchar_iterator first(str.begin(), str.begin(), str.end());
        const utf8_wchar_iterator last(str.end(), str.begin(), str.end());
        const word_regex_iterator it(first, last, DEFAULT_WORD_REGEX);
        const word_regex_iterator end_it;

        std::transform(it, end_it, std::back_inserter(retval),
                      [str, first, begin{str.begin()}](const word_regex_iterator::value_type& match_result)
        {
            auto word_pos_it = first;
            std::advance(word_pos_it, match_result.position());
            auto start_idx(std::distance(begin, word_pos_it.base()));

            auto word_end_it = word_pos_it;
            std::advance(word_end_it, match_result.length());
            auto len = std::distance(word_pos_it.base(), word_end_it.base());

            return str.substr(start_idx, len);
        });
    } catch (...) {}

    return retval;
}

namespace {
#if defined(__cpp_constexpr) && (__cpp_constexpr >= 201907L)
    constexpr StyleFactory default_stylefactory;
    constexpr Cursor default_cursor;
#else
    const StyleFactory default_stylefactory;
    const Cursor default_cursor;
#endif
}

const StyleFactory& GUI::GetStyleFactory() const noexcept
{ return m_impl->m_style_factory ? *m_impl->m_style_factory : default_stylefactory; }

bool GUI::RenderCursor() const
{ return m_impl->m_render_cursor; }

const Cursor& GUI::GetCursor() const noexcept
{ return m_impl->m_cursor ? *m_impl->m_cursor : default_cursor; }

GUI::const_accel_iterator GUI::accel_begin() const noexcept
{ return m_impl->m_accelerators.begin(); }

GUI::const_accel_iterator GUI::accel_end() const noexcept
{ return m_impl->m_accelerators.end(); }

GUI::AcceleratorSignalType& GUI::AcceleratorSignal(Key key, Flags<ModKey> mod_keys) const
{
    auto& sigs = m_impl->m_accelerator_sigs;
    std::pair key_mod{key, mod_keys};
    const auto is_key_mod = [key_mod](const auto& entry) { return entry.first == key_mod; };

    auto it = std::find_if(sigs.begin(), sigs.end(), is_key_mod);

    using sig_t = std::decay_t<decltype(*it->second)>;

    sig_t& sig = (it != sigs.end()) ? *it->second :
        *sigs.emplace_back(key_mod, std::make_unique<sig_t>()).second;

    if (INSTRUMENT_ALL_SIGNALS)
        sig.connect(AcceleratorEcho(key, mod_keys));
    return sig;
}

bool GUI::ModalAcceleratorSignalsEnabled() const noexcept
{ return m_impl->m_allow_modal_accelerator_signals; }

bool GUI::ModalWndsOpen() const
{ return !m_impl->m_modal_wnds.empty(); }

void GUI::SaveWndAsPNG(const Wnd* wnd, const std::string& filename) const
{
    m_impl->m_save_as_png_wnd = wnd;
    m_impl->m_save_as_png_filename = filename;
}

void GUI::HandleGGEvent(EventType event, Key key, uint32_t key_code_point,
                        Flags<ModKey> mod_keys, Pt pos, Pt rel, std::string text)
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

    case EventType::IDLE:
        m_impl->HandleIdle(mod_keys, pos, curr_ticks);
        break;

    case EventType::KEYPRESS:
        m_impl->HandleKeyPress(key, key_code_point, mod_keys, curr_ticks);
        break;

    case EventType::KEYRELEASE:
        m_impl->HandleKeyRelease(key, key_code_point, mod_keys, curr_ticks);
        break;

    case EventType::TEXTINPUT:
        m_impl->HandleTextInput(std::move(text));
        break;

    case EventType::MOUSEMOVE:
        m_impl->HandleMouseMove(mod_keys, pos, rel, curr_ticks);
        break;

    case EventType::LPRESS:
        m_impl->HandleMouseButtonPress(
            int(m_impl->m_mouse_lr_swap ? EventType::RPRESS : EventType::LPRESS) - int(EventType::LPRESS),
            pos, curr_ticks);
        break;

    case EventType::MPRESS:
        m_impl->HandleMouseButtonPress(int(EventType::MPRESS) - int(EventType::LPRESS), pos, curr_ticks);
        break;

    case EventType::RPRESS:
        m_impl->HandleMouseButtonPress(
            int(m_impl->m_mouse_lr_swap ? EventType::LPRESS : EventType::RPRESS) - int(EventType::LPRESS),
            pos, curr_ticks);
        break;

    case EventType::LRELEASE:
        m_impl->HandleMouseButtonRelease(
            int(m_impl->m_mouse_lr_swap ? EventType::RRELEASE : EventType::LRELEASE) - int(EventType::LRELEASE),
            pos, curr_ticks);
        break;

    case EventType::MRELEASE:
        m_impl->HandleMouseButtonRelease(int(EventType::MRELEASE) - int(EventType::LRELEASE),
                                         pos, curr_ticks);
        break;

    case EventType::RRELEASE:
        m_impl->HandleMouseButtonRelease(
            int(m_impl->m_mouse_lr_swap ? EventType::LRELEASE : EventType::RRELEASE) - int(EventType::LRELEASE),
            pos, curr_ticks);
        break;

    case EventType::MOUSEWHEEL:
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

    m_impl->m_zlist.Add(std::move(wnd));
}

void GUI::RegisterModal(std::shared_ptr<Wnd> wnd)
{
    if (wnd && wnd->Modal()) {
        m_impl->m_zlist.Remove(wnd.get());
        m_impl->m_modal_wnds.emplace_back(wnd, wnd);
        wnd->HandleEvent(WndEvent(WndEvent::EventType::GainingFocus));
    }
}

void GUI::RunModal(const bool& done)
{
    while (!done) {
        HandleSystemEvents();
        // send an idle message, so that the gui has timely updates for triggering browse info windows, etc.
        HandleGGEvent(GUI::EventType::IDLE, Key::GGK_NONE, 0, m_impl->m_mod_keys, m_impl->m_mouse_pos, Pt());
        PreRender();
        RenderBegin();
        Render();
        RenderEnd();
        m_impl->GouvernFPS();
    }
}

void GUI::RunModal(std::shared_ptr<Wnd> wnd)
{
    if (!wnd)
        return;
    //std::cout << "RunModal start on " << wnd->Name() << "  at: " << &*wnd << "\n";
    while (!wnd->ModalDone()) {
        HandleSystemEvents();
        // send an idle message, so that the gui has timely updates for triggering browse info windows, etc.
        HandleGGEvent(GUI::EventType::IDLE, Key::GGK_NONE, 0, m_impl->m_mod_keys, m_impl->m_mouse_pos, Pt());
        PreRender();
        RenderBegin();
        Render();
        RenderEnd();
        m_impl->GouvernFPS();
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

void GUI::EnableFPS(bool b)
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

void GUI::RegisterDragDropWnd(std::shared_ptr<Wnd> wnd, Pt offset,
                              std::shared_ptr<Wnd> originating_wnd)
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
{ m_impl->m_timers.emplace(&timer); }

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

void GUI::SetAccelerator(Key key, Flags<ModKey> mod_keys)
{ m_impl->m_accelerators.emplace_back(key, MassagedAccelModKeys(mod_keys)); }

void GUI::RemoveAccelerator(Key key, Flags<ModKey> mod_keys)
{
    auto& acs = m_impl->m_accelerators;
    auto it = std::find(acs.begin(), acs.end(), std::pair{key, MassagedAccelModKeys(mod_keys)});
    if (it != acs.end())
        acs.erase(it);
}

void GUI::RemoveAccelerator(accel_iterator it)
{ m_impl->m_accelerators.erase(it); }

void GUI::EnableModalAcceleratorSignals(bool allow)
{ m_impl->m_allow_modal_accelerator_signals = allow; }

void GUI::SetMouseLRSwapped(bool swapped)
{ m_impl->m_mouse_lr_swap = swapped; }

std::shared_ptr<Font> GUI::GetFont(std::string_view font_filename, unsigned int pts)
{ return GetFontManager().GetFont(font_filename, pts); }

std::shared_ptr<Font> GUI::GetFont(std::string_view font_filename, unsigned int pts,
                                   const std::vector<uint8_t>& file_contents)
{ return GetFontManager().GetFont(font_filename, pts, file_contents); }

std::shared_ptr<Font> GUI::GetFont(const std::shared_ptr<Font>& font, unsigned int pts)
{
    std::shared_ptr<Font> retval;
    if (font->FontName() == StyleFactory::DefaultFontName()) {
        retval = GetStyleFactory().DefaultFont(pts);
    } else {
        retval = GetFont(font->FontName(), font->PointSize(),
                         font->UnicodeCharsets().begin(),
                         font->UnicodeCharsets().end());
    }
    return retval;
}

void GUI::FreeFont(std::string_view font_filename, unsigned int pts)
{ GetFontManager().FreeFont(font_filename, pts); }

std::shared_ptr<Texture> GUI::StoreTexture(Texture* texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

std::shared_ptr<Texture> GUI::StoreTexture(const std::shared_ptr<Texture>& texture, const std::string& texture_name)
{ return GetTextureManager().StoreTexture(texture, texture_name); }

std::shared_ptr<Texture> GUI::GetTexture(const boost::filesystem::path& path, bool mipmap)
{ return GetTextureManager().GetTexture(path, mipmap); }

void GUI::FreeTexture(const boost::filesystem::path& path)
{ GetTextureManager().FreeTexture(path); }

void GUI::SetStyleFactory(std::unique_ptr<StyleFactory>&& factory) noexcept
{ m_impl->m_style_factory = std::move(factory); }

void GUI::RenderCursor(bool render) noexcept
{ m_impl->m_render_cursor = render; }

void GUI::SetCursor(std::unique_ptr<Cursor>&& cursor) noexcept
{ m_impl->m_cursor = std::move(cursor); }

std::string GUI::ClipboardText() const
{ return m_impl->m_clipboard_text; }

bool GUI::SetClipboardText(std::string text)
{
    m_impl->m_clipboard_text = std::move(text);
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
            auto selected_text = edit_control->SelectedText();
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

GUI* GUI::GetGUI() noexcept
{ return s_gui; }

void GUI::PreRenderWindow(const std::shared_ptr<Wnd>& wnd, bool even_if_not_visible)
{ PreRenderWindow(wnd.get(), even_if_not_visible); }

void GUI::PreRenderWindow(Wnd* wnd, bool even_if_not_visible)
{
    if (!wnd || (!even_if_not_visible && !wnd->Visible()))
        return;

    for (auto& child_wnd : wnd->m_children)
        PreRenderWindow(child_wnd.get(), even_if_not_visible);

    if (wnd->PreRenderRequired())
        wnd->PreRender();
}

void GUI::RenderWindow(const std::shared_ptr<Wnd>& wnd)
{ RenderWindow(wnd.get()); }

namespace {
    bool WndClippedOut(const Rect clipped_rect, const Wnd* clipped_wnd, const Wnd* clipping_wnd)
    {
        const auto client_clipped_out = [clipped_rect](const Wnd* clipping_wnd) noexcept
        { return !clipping_wnd->InClient(clipped_rect); };
        const auto wnd_clipped_out = [clipped_rect](const Wnd* clipping_wnd)
        { return !clipping_wnd->InWindow(clipped_rect); };

        switch (clipping_wnd->GetChildClippingMode()) {
        case Wnd::ChildClippingMode::DontClip:
            return false;
            break;
        case Wnd::ChildClippingMode::ClipToClient:
            return client_clipped_out(clipping_wnd);
            break;
        case Wnd::ChildClippingMode::ClipToWindow:
            return wnd_clipped_out(clipping_wnd);
            break;
        case Wnd::ChildClippingMode::ClipToClientAndWindowSeparately:
            return clipped_wnd->NonClientChild() ?
                wnd_clipped_out(clipping_wnd) : client_clipped_out(clipping_wnd);
            break;
        case Wnd::ChildClippingMode::ClipToAncestorClient:
            return false;
        }
        return false;
    };
}

void GUI::RenderWindow(Wnd* wnd)
{
    if (!wnd || !wnd->Visible())
        return;
    wnd->Render();

    const auto clip_mode = wnd->GetChildClippingMode();

    if (clip_mode == Wnd::ChildClippingMode::DontClip) {
        for (auto& child : wnd->Children())
            if (child && child->Visible())
                RenderWindow(child);

    } else if (clip_mode == Wnd::ChildClippingMode::ClipToAncestorClient) {
        for (auto& child_wnd : wnd->Children()) {
            Wnd* const child  = child_wnd.get();
            if (child && child->Visible()) {
                const Rect clipped_rect{child->UpperLeft(), child->LowerRight()};
                bool clipped_out = false;
                const Wnd* clipping_wnd = wnd;
                while (clipping_wnd && !clipped_out) {
                    if (WndClippedOut(clipped_rect, child, clipping_wnd))
                        clipped_out = true;
                    else
                        clipping_wnd = clipping_wnd->Parent().get();
                }
                if (!clipped_out)
                    RenderWindow(child);
            }
        }

    } else if (clip_mode != Wnd::ChildClippingMode::ClipToClientAndWindowSeparately) {
        wnd->BeginClipping();
        for (auto& child : wnd->Children())
            if (child && child->Visible())
                RenderWindow(child);
        wnd->EndClipping();

    } else { // clip_mode == Wnd::ChildClippingMode::ClipToClientAndWindowSeparately
        const auto& wnd_children = wnd->Children();
        std::vector<Wnd*> children;
        children.reserve(wnd->Children().size());
        std::transform(wnd_children.begin(), wnd_children.end(), std::back_inserter(children),
                       [](const auto& child) { return child.get(); });

        const auto client_child_begin =
            std::partition(children.begin(), children.end(),
                           [](const auto& child) { return child->NonClientChild(); });

        if (children.begin() != client_child_begin) {
            wnd->BeginNonclientClipping();
            for (auto it = children.begin(); it != client_child_begin; ++it) {
                Wnd* const child = *it;
                if (child && child->Visible())
                    RenderWindow(child);
            }
            wnd->EndNonclientClipping();
        }

        if (client_child_begin != children.end()) {
            wnd->BeginClipping();
            for (auto it = client_child_begin; it != children.end(); ++it) {
                Wnd* const child = *it;
                if (child && child->Visible())
                    RenderWindow(child);
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
    for (const auto& drop_wnd : m_impl->m_drag_drop_wnds) {
        bool old_visible = drop_wnd.first->Visible();
        if (!old_visible)
            drop_wnd.first->Show();
        auto drop_wnd_parent{drop_wnd.first->Parent()};
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
    for (auto& wnd : m_impl->m_zlist.RenderOrder())
        PreRenderWindow(wnd.get());

    // pre-render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (const auto& modal_wnd : m_impl->m_modal_wnds)
        PreRenderWindow(modal_wnd.first.get());

    // pre-render the active browse info window, if any
    const auto curr_wnd_under_cursor{LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor)};
    if (m_impl->m_browse_info_wnd && curr_wnd_under_cursor) {
        assert(m_impl->m_browse_target);
        PreRenderWindow(m_impl->m_browse_info_wnd.get());
    }

    for (const auto& drag_drop_wnd : m_impl->m_drag_drop_wnds)
        PreRenderWindow(drag_drop_wnd.first.get());
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
    for (auto& wnd : m_impl->m_zlist.RenderOrder()) {
        if (wnd)
            RenderWindow(wnd.get());
    }

    // render modal windows back-to-front (on top of non-modal Wnds rendered above)
    for (const auto& modal_wnd : m_impl->m_modal_wnds) {
        if (modal_wnd.first)
            RenderWindow(modal_wnd.first.get());
    }

    // render the active browse info window, if any
    if (m_impl->m_browse_info_wnd) {
        const auto curr_wnd_under_cursor{LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor)};
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

    if (m_impl && m_impl->m_render_cursor && m_impl->m_cursor && AppHasMouseFocus())
        m_impl->m_cursor->Render(m_impl->m_mouse_pos);
    Exit2DMode();
}

bool GUI::ProcessBrowseInfoImpl(Wnd* wnd)
{
    bool retval = false;
    const auto& browse_modes = wnd->BrowseModes();
    if (browse_modes.empty())
        return retval;

    const auto delta_t = Ticks() - m_impl->m_prev_wnd_under_cursor_time;
    std::size_t i = 0;
    for (auto it = browse_modes.rbegin(); it != browse_modes.rend(); ++it, ++i)
    {
        if (it->time < delta_t) {
            if (it->wnd && it->wnd->WndHasBrowseInfo(wnd, i)) {
                if (m_impl->m_browse_target != wnd || m_impl->m_browse_info_wnd != it->wnd || m_impl->m_browse_info_mode != static_cast<int>(i)) {
                    m_impl->m_browse_target = wnd;
                    m_impl->m_browse_info_wnd = it->wnd;
                    m_impl->m_browse_info_mode = static_cast<decltype(m_impl->m_browse_info_mode)>(i);
                    m_impl->m_browse_info_wnd->SetCursorPosition(m_impl->m_mouse_pos);
                }
                retval = true;
            }
            break;
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

std::shared_ptr<Wnd> GUI::CheckedGetWindowUnder(Pt pt, Flags<ModKey> mod_keys)
{
    auto wnd_under_pt = GetWindowUnder(pt);
    const auto& dragged_wnd = m_impl->m_curr_drag_wnd; // wnd being continuously repositioned / dragged around, not a drag-drop

    //std::cout << "GUI::CheckedGetWindowUnder w: " << w << "  dragged_wnd: " << dragged_wnd << std::endl;

    bool unregistered_drag_drop = dragged_wnd && !dragged_wnd->DragDropDataType().empty();
    bool registered_drag_drop = !m_impl->m_drag_drop_wnds.empty();

    const auto&& curr_drag_drop_here_wnd = LockAndResetIfExpired(m_impl->m_curr_drag_drop_here_wnd);
    if (curr_drag_drop_here_wnd && !unregistered_drag_drop && !registered_drag_drop) {
        curr_drag_drop_here_wnd->HandleEvent(WndEvent(WndEvent::EventType::DragDropLeave));
        m_impl->m_curr_drag_drop_here_wnd.reset();
    }

    const auto&& curr_wnd_under_cursor = LockAndResetIfExpired(m_impl->m_curr_wnd_under_cursor);
    if (wnd_under_pt == curr_wnd_under_cursor)
        return wnd_under_pt;   // same Wnd is under cursor as before; nothing to do

    if (curr_wnd_under_cursor) {
        // inform previous Wnd under the cursor that the cursor has been dragged away
        if (unregistered_drag_drop) {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::DragDropLeave));
            m_impl->m_drag_drop_wnds_acceptable[dragged_wnd.get()] = false;
            m_impl->m_curr_drag_drop_here_wnd.reset();

        } else if (registered_drag_drop) {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::DragDropLeave));
            for (auto& acceptable_wnd : m_impl->m_drag_drop_wnds_acceptable)
            { acceptable_wnd.second = false; }
            m_impl->m_curr_drag_drop_here_wnd.reset();

        } else {
            curr_wnd_under_cursor->HandleEvent(WndEvent(WndEvent::EventType::MouseLeave));
        }
    }

    if (!wnd_under_pt) {
        //std::cout << "CheckedGetWindowUnder returning " << w << std::endl;
        return wnd_under_pt;
    }


    // inform new Wnd under cursor that something was dragged over it
    if (unregistered_drag_drop) {
        // pass drag-drop event to check if the single dragged Wnd is acceptable to drop
        WndEvent event(WndEvent::EventType::CheckDrops, pt, dragged_wnd.get(), mod_keys);
        wnd_under_pt->HandleEvent(event);
        m_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::EventType::DragDropEnter, pt, dragged_wnd.get(), mod_keys);
        wnd_under_pt->HandleEvent(enter_event);
        m_impl->m_curr_drag_drop_here_wnd = wnd_under_pt;

    } else if (registered_drag_drop) {
        // pass drag-drop event to check if the various dragged Wnds are acceptable to drop
        WndEvent event(WndEvent::EventType::CheckDrops, pt, m_impl->m_drag_drop_wnds, mod_keys);
        wnd_under_pt->HandleEvent(event);
        m_impl->m_drag_drop_wnds_acceptable = event.GetAcceptableDropWnds();

        // Wnd being dragged over is new; give it an Enter message
        WndEvent enter_event(WndEvent::EventType::DragDropEnter, pt, m_impl->m_drag_drop_wnds, mod_keys);
        wnd_under_pt->HandleEvent(enter_event);
        m_impl->m_curr_drag_drop_here_wnd = wnd_under_pt;

    } else {
        m_impl->HandleMouseEnter(mod_keys, pt, wnd_under_pt);
    }

    //std::cout << "CheckedGetWindowUnder returning " << w << std::endl;
    return wnd_under_pt;
}

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
