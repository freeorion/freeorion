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
namespace {

bool RegisterModKeys()
{
    FlagSpec<ModKey>& spec = FlagSpec<ModKey>::instance();
    spec.insert(MOD_KEY_NONE,   "MOD_KEY_NONE");
    spec.insert(MOD_KEY_LSHIFT, "MOD_KEY_LSHIFT");
    spec.insert(MOD_KEY_RSHIFT, "MOD_KEY_RSHIFT");
    spec.insert(MOD_KEY_LCTRL,  "MOD_KEY_LCTRL");
    spec.insert(MOD_KEY_RCTRL,  "MOD_KEY_RCTRL");
    spec.insert(MOD_KEY_LALT,   "MOD_KEY_LALT");
    spec.insert(MOD_KEY_RALT,   "MOD_KEY_RALT");
    spec.insert(MOD_KEY_LMETA,  "MOD_KEY_LMETA");
    spec.insert(MOD_KEY_RMETA,  "MOD_KEY_RMETA");
    spec.insert(MOD_KEY_NUM,    "MOD_KEY_NUM");
    spec.insert(MOD_KEY_CAPS,   "MOD_KEY_CAPS");
    spec.insert(MOD_KEY_MODE,   "MOD_KEY_MODE");
    return true;
}
bool dummy = RegisterModKeys();

}

namespace GG {
GG_FLAGSPEC_IMPL(ModKey);
}

///////////////////////////////////////
// class GG::WndEvent
///////////////////////////////////////
std::string_view EventTypeName(const WndEvent& event) noexcept {
    switch (event.Type()) {
    case WndEvent::EventType::LButtonDown:     return "(LButtonDown)";
    case WndEvent::EventType::LDrag:           return "(LDrag)";
    case WndEvent::EventType::LButtonUp:       return "(LButtonUp)";
    case WndEvent::EventType::LClick:          return "(LClick)";
    case WndEvent::EventType::LDoubleClick:    return "(LDoubleClick)";
    case WndEvent::EventType::MButtonDown:     return "(MButtonDown)";
    case WndEvent::EventType::MDrag:           return "(MDrag)";
    case WndEvent::EventType::MButtonUp:       return "(MButtonUp)";
    case WndEvent::EventType::MClick:          return "(MClick)";
    case WndEvent::EventType::MDoubleClick:    return "(MDoubleClick)";
    case WndEvent::EventType::RButtonDown:     return "(RButtonDown)";
    case WndEvent::EventType::RDrag:           return "(RDrag)";
    case WndEvent::EventType::RButtonUp:       return "(RButtonUp)";
    case WndEvent::EventType::RClick:          return "(RClick)";
    case WndEvent::EventType::RDoubleClick:    return "(RDoubleClick)";
    case WndEvent::EventType::MouseEnter:      return "(MouseEnter)";
    case WndEvent::EventType::MouseHere:       return "(MouseHere)";
    case WndEvent::EventType::MouseLeave:      return "(MouseLeave)";
    case WndEvent::EventType::MouseWheel:      return "(MouseWheel)";
    case WndEvent::EventType::DragDropEnter:   return "(DragDropEnter)";
    case WndEvent::EventType::DragDropHere:    return "(DragDropHere)";
    case WndEvent::EventType::CheckDrops:      return "(CheckDrops)";
    case WndEvent::EventType::DragDropLeave:   return "(DragDropLeave)";
    case WndEvent::EventType::DragDroppedOn:   return "(DragDroppedOn)";
    case WndEvent::EventType::KeyPress:        return "(KeyPress)";
    case WndEvent::EventType::KeyRelease:      return "(KeyRelease)";
    case WndEvent::EventType::TextInput:       return "(TextInput)";
    case WndEvent::EventType::GainingFocus:    return "(GainingFocus)";
    case WndEvent::EventType::LosingFocus:     return "(LosingFocus)";
    case WndEvent::EventType::TimerFiring:     return "(TimerFiring)";
    default:                                   return "(Unknown Event Type)";
    }
}
