//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/WndEvent.h>


using namespace GG;

///////////////////////////////////////
// ModKeys
///////////////////////////////////////
const ModKey GG::MOD_KEY_NONE         (0x0000);
const ModKey GG::MOD_KEY_LSHIFT       (0x0001);
const ModKey GG::MOD_KEY_RSHIFT       (0x0002);
const ModKey GG::MOD_KEY_LCTRL        (0x0040);
const ModKey GG::MOD_KEY_RCTRL        (0x0080);
const ModKey GG::MOD_KEY_LALT         (0x0100);
const ModKey GG::MOD_KEY_RALT         (0x0200);
const ModKey GG::MOD_KEY_LMETA        (0x0400);
const ModKey GG::MOD_KEY_RMETA        (0x0800);
const ModKey GG::MOD_KEY_NUM          (0x1000);
const ModKey GG::MOD_KEY_CAPS         (0x2000);
const ModKey GG::MOD_KEY_MODE         (0x4000);

namespace {

bool RegisterModKeys()
{
    FlagSpec<ModKey>& spec = FlagSpec<ModKey>::instance();
    spec.insert(MOD_KEY_NONE,   "MOD_KEY_NONE",     true);
    spec.insert(MOD_KEY_LSHIFT, "MOD_KEY_LSHIFT",   true);
    spec.insert(MOD_KEY_RSHIFT, "MOD_KEY_RSHIFT",   true);
    spec.insert(MOD_KEY_LCTRL,  "MOD_KEY_LCTRL",    true);
    spec.insert(MOD_KEY_RCTRL,  "MOD_KEY_RCTRL",    true);
    spec.insert(MOD_KEY_LALT,   "MOD_KEY_LALT",     true);
    spec.insert(MOD_KEY_RALT,   "MOD_KEY_RALT",     true);
    spec.insert(MOD_KEY_LMETA,  "MOD_KEY_LMETA",    true);
    spec.insert(MOD_KEY_RMETA,  "MOD_KEY_RMETA",    true);
    spec.insert(MOD_KEY_NUM,    "MOD_KEY_NUM",      true);
    spec.insert(MOD_KEY_CAPS,   "MOD_KEY_CAPS",     true);
    spec.insert(MOD_KEY_MODE,   "MOD_KEY_MODE",     true);
    return true;
}
bool dummy = RegisterModKeys();

}

const Flags<ModKey> GG::MOD_KEY_CTRL  ((MOD_KEY_LCTRL | MOD_KEY_RCTRL));
const Flags<ModKey> GG::MOD_KEY_SHIFT ((MOD_KEY_LSHIFT | MOD_KEY_RSHIFT));
const Flags<ModKey> GG::MOD_KEY_ALT   ((MOD_KEY_LALT | MOD_KEY_RALT));
const Flags<ModKey> GG::MOD_KEY_META  ((MOD_KEY_LMETA | MOD_KEY_RMETA));

GG_FLAGSPEC_IMPL(ModKey);


///////////////////////////////////////
// class GG::WndEvent
///////////////////////////////////////
WndEvent::WndEvent(EventType type, const Pt& pt, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_mod_keys(mod_keys)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const Pt& move, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_mod_keys(mod_keys),
    m_drag_move(move)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, int move, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_mod_keys(mod_keys),
    m_wheel_move(move)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const std::map<std::shared_ptr<Wnd>, Pt>& drag_drop_wnds, Flags<ModKey> mod_keys) :
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

WndEvent::WndEvent(EventType type, const Pt& pt, const std::vector<std::shared_ptr<Wnd>>& drag_drop_wnds, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_mod_keys(mod_keys),
    m_dropped_wnds(drag_drop_wnds)
{}

WndEvent::WndEvent(EventType type, const Pt& pt, const Wnd* const drag_wnd, Flags<ModKey> mod_keys) :
    m_type(type),
    m_point(pt),
    m_mod_keys(mod_keys)
{
    // initialize storage for single dragged Wnd
    m_drag_drop_wnds[drag_wnd] = pt;
    m_acceptable_drop_wnds[drag_wnd] = false;
}

WndEvent::WndEvent(EventType type, Key key, std::uint32_t code_point, Flags<ModKey> mod_keys) :
    m_type(type),
    m_key(key),
    m_key_code_point(code_point),
    m_mod_keys(mod_keys)
{}

WndEvent::WndEvent(EventType type, unsigned int ticks, Timer* timer) :
    m_type(type),
    m_ticks(ticks),
    m_timer(timer)
{}

WndEvent::WndEvent (WndEvent::EventType type, const std::string* text):
    m_type(type),
    m_text(text)
{}

WndEvent::WndEvent(EventType type) :
    m_type(type)
{}

WndEvent::EventType WndEvent::Type() const
{ return m_type; }

const Pt& WndEvent::Point() const
{ return m_point; }

Key WndEvent::GetKey() const
{ return m_key; }

std::uint32_t WndEvent::KeyCodePoint() const
{ return m_key_code_point; }

Flags<ModKey> WndEvent::ModKeys() const
{ return m_mod_keys; }

const Pt& WndEvent::DragMove() const
{ return m_drag_move; }

int WndEvent::WheelMove() const
{ return m_wheel_move; }

const std::map<const Wnd* const, Pt>& WndEvent::DragDropWnds() const
{ return m_drag_drop_wnds; }

std::vector<std::shared_ptr<Wnd>>& WndEvent::GetDragDropWnds() const
{ return m_dropped_wnds; }

std::map<const Wnd*, bool>& WndEvent::GetAcceptableDropWnds() const
{ return m_acceptable_drop_wnds; }

unsigned int WndEvent::Ticks() const
{ return m_ticks; }

Timer* WndEvent::GetTimer() const
{ return m_timer; }

const std::string* WndEvent::GetText() const
{ return m_text; }

std::string EventTypeName(const WndEvent& event) {
    switch (event.Type()) {
    case WndEvent::LButtonDown:     return "(LButtonDown)";
    case WndEvent::LDrag:           return "(LDrag)";
    case WndEvent::LButtonUp:       return "(LButtonUp)";
    case WndEvent::LClick:          return "(LClick)";
    case WndEvent::LDoubleClick:    return "(LDoubleClick)";
    case WndEvent::MButtonDown:     return "(MButtonDown)";
    case WndEvent::MDrag:           return "(MDrag)";
    case WndEvent::MButtonUp:       return "(MButtonUp)";
    case WndEvent::MClick:          return "(MClick)";
    case WndEvent::MDoubleClick:    return "(MDoubleClick)";
    case WndEvent::RButtonDown:     return "(RButtonDown)";
    case WndEvent::RDrag:           return "(RDrag)";
    case WndEvent::RButtonUp:       return "(RButtonUp)";
    case WndEvent::RClick:          return "(RClick)";
    case WndEvent::RDoubleClick:    return "(RDoubleClick)";
    case WndEvent::MouseEnter:      return "(MouseEnter)";
    case WndEvent::MouseHere:       return "(MouseHere)";
    case WndEvent::MouseLeave:      return "(MouseLeave)";
    case WndEvent::MouseWheel:      return "(MouseWheel)";
    case WndEvent::DragDropEnter:   return "(DragDropEnter)";
    case WndEvent::DragDropHere:    return "(DragDropHere)";
    case WndEvent::CheckDrops:      return "(CheckDrops)";
    case WndEvent::DragDropLeave:   return "(DragDropLeave)";
    case WndEvent::DragDroppedOn:   return "(DragDroppedOn)";
    case WndEvent::KeyPress:        return "(KeyPress)";
    case WndEvent::KeyRelease:      return "(KeyRelease)";
    case WndEvent::TextInput:       return "(TextInput)";
    case WndEvent::GainingFocus:    return "(GainingFocus)";
    case WndEvent::LosingFocus:     return "(LosingFocus)";
    case WndEvent::TimerFiring:     return "(TimerFiring)";
    default:                        return "(Unknown Event Type)";
    }
}
