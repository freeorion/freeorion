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

/** \file WndEvent.h \brief Contains the WndEvent class, which is used
    internally by GG to dispatch event messages to Wnds. */

#ifndef _GG_WndEvent_h_
#define _GG_WndEvent_h_

#include <GG/Base.h>
#include <GG/Exception.h>
#include <GG/Flags.h>

#include <map>
#include <vector>


namespace GG {

class Timer;
class Wnd;

// Adpated from SDLKey enum in SDL_keysym.h of the SDL library.
GG_FLAG_TYPE(ModKey);
extern GG_API const ModKey MOD_KEY_NONE;         ///< No modifier key.
extern GG_API const ModKey MOD_KEY_LSHIFT;       ///< The left Shift key.
extern GG_API const ModKey MOD_KEY_RSHIFT;       ///< The right Shift key.
extern GG_API const ModKey MOD_KEY_LCTRL;        ///< The left Control key.
extern GG_API const ModKey MOD_KEY_RCTRL;        ///< The right Control key.
extern GG_API const ModKey MOD_KEY_LALT;         ///< The left Alt key.
extern GG_API const ModKey MOD_KEY_RALT;         ///< The right Alt key.
extern GG_API const ModKey MOD_KEY_LMETA;        ///< The left Meta key.
extern GG_API const ModKey MOD_KEY_RMETA;        ///< The right Meta key.
extern GG_API const ModKey MOD_KEY_NUM;          ///< The Num Lock key.
extern GG_API const ModKey MOD_KEY_CAPS;         ///< The Caps Lock key.
extern GG_API const ModKey MOD_KEY_MODE;         ///< The Mode key.
extern GG_API const Flags<ModKey> MOD_KEY_CTRL;  ///< Either Control key.
extern GG_API const Flags<ModKey> MOD_KEY_SHIFT; ///< Either Shift key.
extern GG_API const Flags<ModKey> MOD_KEY_ALT;   ///< Either Alt key.
extern GG_API const Flags<ModKey> MOD_KEY_META;  ///< Either Meta key.

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
    enum EventType {
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

    /** \name Structors */ ///@{
    WndEvent(EventType type, const Pt& pt, Flags<ModKey> mod_keys);
    WndEvent(EventType type, const Pt& pt, const Pt& move, Flags<ModKey> mod_keys);
    WndEvent(EventType type, const Pt& pt, int move, Flags<ModKey> mod_keys);
    WndEvent(EventType type, const Pt& pt, const std::vector<std::shared_ptr<Wnd>>& drag_drop_wnds, Flags<ModKey> mod_keys);
    WndEvent(EventType type, const Pt& pt, const std::map<std::shared_ptr<Wnd>, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys);
    WndEvent(EventType type, const Pt& pt, const Wnd* const drag_wnd, Flags<ModKey> mod_keys);
    WndEvent(EventType type, Key key, std::uint32_t code_point, Flags<ModKey> mod_keys);
    WndEvent(EventType type, unsigned int ticks, Timer* timer);
    WndEvent(EventType type, const std::string* text);
    explicit WndEvent(EventType type);
    //@}

    /** \name Accessors */ ///@{
    EventType                   Type() const;           ///< returns the type of the WndEvent
    const Pt&                   Point() const;          ///< returns the point at which the event took place, if any
    Key                         GetKey() const;         ///< returns the key pressed or released in the WndEvent, if any

    /** Returns the Unicode code point for the key pressed or released in the
        WndEvent, if any.

        @note  This may be zero, even in a KeyPress or KeyRelease event, if
        Unicode support is unavailable. */
    std::uint32_t KeyCodePoint() const;

    Flags<ModKey>               ModKeys() const;        ///< returns the modifiers to the WndEvent's keypress, if any
    const Pt&                   DragMove() const;       ///< returns the amount of drag movement represented by the WndEvent, if any
    int                         WheelMove() const;      ///< returns the ammount of mouse wheel movement represented by the WndEvent, if any
    unsigned int                Ticks() const;          ///< returns the number of ticks represented by the WndEvent. if any
    Timer*                      GetTimer() const;       ///< returns the Timer represented by the WndEvent. if any
    const std::string*          GetText() const;        ///< returns the utf8 text represented by the WndEvent, if any
    const std::map<const Wnd* const, Pt>&   DragDropWnds() const;   ///< returns the drag-and-drop wnds represented by the WndEvent, if any
    std::vector<std::shared_ptr<Wnd>>&          GetDragDropWnds() const;
    std::map<const Wnd*, bool>& GetAcceptableDropWnds() const;
    //@}

private:
    EventType                   m_type;
    Pt                          m_point;
    Key                         m_key;

    std::uint32_t m_key_code_point;

    Flags<ModKey>               m_mod_keys;
    Pt                          m_drag_move;
    int                         m_wheel_move;

    std::map<const Wnd* const, Pt>            m_drag_drop_wnds;

    unsigned int                m_ticks;
    Timer*                      m_timer;
    const std::string*          m_text;

    mutable std::vector<std::shared_ptr<Wnd>> m_dropped_wnds;
    mutable std::map<const Wnd*, bool>  m_acceptable_drop_wnds;
};

GG_API std::string EventTypeName(const GG::WndEvent& event);

} // namespace GG

#endif
