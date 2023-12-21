//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2023 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/WndEvent.h
//!
//! Contains the WndEvent class, which is used internally by GG to dispatch
//! event messages to Wnds.

#ifndef _GG_WndEvent_h_
#define _GG_WndEvent_h_


#include <map>
#include <vector>
#include <GG/Base.h>
#include <GG/Exception.h>
#include <GG/Flags.h>


namespace GG {

class Timer;
class Wnd;

// Adpated from SDLKey enum in SDL_keysym.h of the SDL library.
GG_FLAG_TYPE(ModKey);

constexpr ModKey MOD_KEY_NONE     (0x0000); ///< No modifier key.
constexpr ModKey MOD_KEY_LSHIFT   (0x0001); ///< The left Shift key.
constexpr ModKey MOD_KEY_RSHIFT   (0x0002); ///< The right Shift key.
constexpr ModKey MOD_KEY_LCTRL    (0x0040); ///< The left Control key.
constexpr ModKey MOD_KEY_RCTRL    (0x0080); ///< The right Control key.
constexpr ModKey MOD_KEY_LALT     (0x0100); ///< The left Alt key.
constexpr ModKey MOD_KEY_RALT     (0x0200); ///< The right Alt key.
constexpr ModKey MOD_KEY_LMETA    (0x0400); ///< The left Meta key.
constexpr ModKey MOD_KEY_RMETA    (0x0800); ///< The right Meta key.
constexpr ModKey MOD_KEY_NUM      (0x1000); ///< The Num Lock key.
constexpr ModKey MOD_KEY_CAPS     (0x2000); ///< The Caps Lock key.
constexpr ModKey MOD_KEY_MODE     (0x4000); ///< The Mode key.

constexpr Flags<ModKey> MOD_KEY_CTRL  ((MOD_KEY_LCTRL | MOD_KEY_RCTRL));    ///< Either Control key.
constexpr Flags<ModKey> MOD_KEY_SHIFT ((MOD_KEY_LSHIFT | MOD_KEY_RSHIFT));  ///< Either Shift key.
constexpr Flags<ModKey> MOD_KEY_ALT   ((MOD_KEY_LALT | MOD_KEY_RALT));      ///< Either Alt key.
constexpr Flags<ModKey> MOD_KEY_META  ((MOD_KEY_LMETA | MOD_KEY_RMETA));    ///< Either Meta key.


/** \brief Encapsulates a Wnd event that is passed from the singleton GUI to a
    Wnd.

    The various types of WndEvents correspond to the various message member
    functions of Wnd, some of which have different parameterizations.  Rather
    than have a less-efficient but more-easily-extensible hierarchy of
    WndEvent types, a single WndEvent type exists that has all possible
    parameters to a Wnd message function call.  Therefore, not all of
    WndEvent's accessors will return sensical results, depending on the
    WndEventType of the WndEvent.  Note that Wnd events may be filtered before
    they actually reach the target Wnd \see Wnd */
class GG_API WndEvent
{
public:
    /** The types of Wnd events.  Each of these corresponds to a Wnd member
        function of the same name. */
    enum class EventType : uint8_t {
        LButtonDown,
        LDrag,
        LButtonUp,
        LClick,
        LDoubleClick,
        MButtonDown,
        MDrag,
        MButtonUp,
        MClick,
        MDoubleClick,
        RButtonDown,
        RDrag,
        RButtonUp,
        RClick,
        RDoubleClick,
        MouseEnter,
        MouseHere,
        MouseLeave,
        MouseWheel,
        DragDropEnter,
        DragDropHere,
        CheckDrops,
        DragDropLeave,
        DragDroppedOn,
        KeyPress,
        KeyRelease,
        TextInput,
        GainingFocus,
        LosingFocus,
        TimerFiring
    };

    WndEvent(EventType type, Pt pt, Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys)
    {}

    WndEvent(EventType type, Pt pt, Pt move, Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys),
        m_drag_move(move)
    {}

    WndEvent(EventType type, Pt pt, int move, Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys),
        m_wheel_move(move)
    {}

    WndEvent(EventType type, Pt pt, std::vector<std::shared_ptr<Wnd>> drag_drop_wnds, Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys),
        m_dropped_wnds(std::move(drag_drop_wnds))
    {}

    WndEvent(EventType type, Pt pt, const std::map<std::shared_ptr<Wnd>, Pt>& drag_drop_wnds,
             Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys)
    {
        // initialize storage for acceptable Wnds
        for (const auto& drag_drop_wnd : drag_drop_wnds) {
            m_drag_drop_wnds[drag_drop_wnd.first.get()] = drag_drop_wnd.second;
            m_acceptable_drop_wnds[drag_drop_wnd.first.get()] = false;
        }
    }

    WndEvent(EventType type, Key key, uint32_t code_point, Flags<ModKey> mod_keys) :
        m_type(type),
        m_key_code_point(code_point),
        m_mod_keys(mod_keys),
        m_key(key)
    {}

    WndEvent(EventType type, unsigned int ticks, Timer* timer) :
        m_type(type),
        m_ticks(ticks),
        m_timer(timer)
    {}

    WndEvent (WndEvent::EventType type, std::string text):
        m_type(type),
        m_text(std::move(text))
    {}

    explicit WndEvent(EventType type) :
        m_type(type)
    {}

    WndEvent(EventType type, Pt pt, const Wnd* const drag_wnd, Flags<ModKey> mod_keys) :
        m_type(type),
        m_point(pt),
        m_mod_keys(mod_keys)
    {
        // initialize storage for single dragged Wnd
        m_drag_drop_wnds[drag_wnd] = pt;
        m_acceptable_drop_wnds[drag_wnd] = false;
    }


    [[nodiscard]] EventType Type() const noexcept { return m_type; }   ///< type of WndEvent
    [[nodiscard]] Pt        Point() const noexcept { return m_point; } ///< point where the event took place, if any
    [[nodiscard]] Key       GetKey() const noexcept { return m_key; }  ///< key pressed or released in the WndEvent, if any

    /** Returns the Unicode code point for the key pressed or released in the
        WndEvent, if any.

        @note  This may be zero, even in a KeyPress or KeyRelease event, if
        Unicode support is unavailable. */
    [[nodiscard]] uint32_t KeyCodePoint() const noexcept { return m_key_code_point; }

    [[nodiscard]] Flags<ModKey> ModKeys() const noexcept { return m_mod_keys; }   ///< modifiers to the WndEvent's keypress, if any
    [[nodiscard]] Pt            DragMove() const noexcept { return m_drag_move; } ///< amount of drag movement represented by the WndEvent, if any
    [[nodiscard]] int           WheelMove() const noexcept { return m_wheel_move;}///< amount of mouse wheel movement represented by the WndEvent, if any
    [[nodiscard]] unsigned int  Ticks() const noexcept { return m_ticks; }        ///< number of ticks represented by the WndEvent. if any
    [[nodiscard]] Timer*        GetTimer() const noexcept { return m_timer; }     ///< Timer represented by the WndEvent. if any
    [[nodiscard]] auto&         GetText() const noexcept { return m_text; }       ///< utf8 text represented by the WndEvent, if any
    [[nodiscard]] auto&         DragDropWnds() const noexcept { return m_drag_drop_wnds; } ///< drag-and-drop wnds represented by the WndEvent, if any
    [[nodiscard]] auto&         GetDragDropWnds() const noexcept { return m_dropped_wnds; }
    [[nodiscard]] auto&         GetAcceptableDropWnds() const noexcept { return m_acceptable_drop_wnds; }

private:
    EventType                       m_type;
    Pt                              m_point;

    uint32_t                   m_key_code_point = 0;

    Flags<ModKey>                   m_mod_keys;
    Pt                              m_drag_move;
    int                             m_wheel_move = 0;

    std::map<const Wnd* const, Pt>  m_drag_drop_wnds;

    unsigned int                    m_ticks = 0;
    Timer*                          m_timer = nullptr;
    std::string                     m_text;

    mutable std::vector<std::shared_ptr<Wnd>>   m_dropped_wnds;
    mutable std::map<const Wnd*, bool>          m_acceptable_drop_wnds;

    Key                             m_key{Key::GGK_NONE};
};

}


#endif
