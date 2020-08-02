//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/Base.h
//!
//! Contains enums, utility classes, and free functions used throughout GG.

#ifndef _GG_Base_h_
#define _GG_Base_h_


#include <memory>
#include <GL/glew.h>
#include <GG/Clr.h>
#include <GG/Enum.h>
#include <GG/Export.h>
#include <GG/PtRect.h>


/** \namespace GG \brief The namespace that encloses all GG classes,
    functions, typedefs, enums, etc. */
namespace GG {

extern GG_API const bool INSTRUMENT_ALL_SIGNALS;

template <typename FlagType>
class Flags;
class ModKey;

template <typename T>
class ScopedAssign
{
public:
    ScopedAssign(T& t, T val) :
        m_old_val(t),
        m_t(t)
        { m_t = val; }
    ~ScopedAssign()
        { m_t = m_old_val; }

private:
    T m_old_val;
    T& m_t;
};

/** Apply \p process to each weak_ptr in \p container and then remove any that are expired.*/

// vector
template <typename Container>
void ProcessThenRemoveExpiredPtrs(
    Container& container,
    const std::function<void(std::shared_ptr<typename Container::value_type::element_type>&)>& process,
    typename std::add_pointer<decltype(std::declval<Container>().at(std::declval<typename Container::size_type>()))>::type = nullptr)
{
    // Process
    for (const auto& weak : container)
        if (auto wnd = weak.lock())
            process(wnd);

    // Remove if the process caused the pointer to expire.
    Container not_expired;
    for (auto& weak : container) {
        if (!weak.expired()) {
            // Swap them to avoid another reference count check
            not_expired.push_back(std::shared_ptr<typename Container::value_type::element_type>());
            not_expired.back().swap(weak);
        }
    }
    container.swap(not_expired);
}

// set, unordered_set
template <typename Container>
void ProcessThenRemoveExpiredPtrs(
    Container& container,
    const std::function<void(std::shared_ptr<typename Container::value_type::element_type>&)>& process,
    decltype(std::declval<Container>().erase(std::declval<typename Container::iterator>()))* = nullptr,
    decltype(std::declval<Container>().equal_range(std::declval<
                                                   typename std::add_const<
                                                   typename std::add_lvalue_reference<
                                                   typename Container::key_type>::type>::type>()))* = nullptr)
{
    auto it = container.begin();
    while (it != container.end()) {
        // Process
        if (auto wnd = it->lock())
            process(wnd);

        // Remove if the process caused the pointer to expire.
        if (!it->expired())
            ++it;
        else
            it = container.erase(it);
    }
}

// list types
template <typename Container>
void ProcessThenRemoveExpiredPtrs(
    Container& container,
    const std::function<void(std::shared_ptr<typename Container::value_type::element_type>&)>& process,
    decltype(std::declval<Container>().erase(std::declval<typename Container::iterator>()))* = nullptr,
    decltype(std::declval<Container>().splice(std::declval<typename Container::const_iterator>(),
                                              std::declval<typename std::add_lvalue_reference<Container>::type>()))* = nullptr)
{
    auto it = container.begin();
    while (it != container.end()) {
        // Process
        if (auto wnd = it->lock())
            process(wnd);

        // Remove if the process caused the pointer to expire.
        if (!it->expired())
            ++it;
        else
            it = container.erase(it);
    }
}

/** "Regions" of a window; used e.g. to determine direction(s) of drag when a
    window that has a drag-frame is clicked. */
GG_ENUM(WndRegion,
    WR_NONE = -1, 
    WR_TOPLEFT = 0, 
    WR_TOP, 
    WR_TOPRIGHT, 
    WR_MIDLEFT, 
    WR_MIDDLE, 
    WR_MIDRIGHT, 
    WR_BOTTOMLEFT, 
    WR_BOTTOM, 
    WR_BOTTOMRIGHT
)

/** The orientations for scrollbars, sliders, etc. */
GG_ENUM(Orientation,
    VERTICAL,  ///< Vertical orientation.
    HORIZONTAL ///< Horizontal orientation.
)

/// Represents true keys on a keyboard (scancodes).  The values of this enum
/// are based on the Universal Serial BUS HID Usage Table 0x07
/// (Keyboard/Keypad page).
///
/// https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf
GG_ENUM(Key,
    GGK_NONE            = 0x00,
    GGK_a               = 0x04,
    GGK_b               = 0x05,
    GGK_c               = 0x06,
    GGK_d               = 0x07,
    GGK_e               = 0x08,
    GGK_f               = 0x09,
    GGK_g               = 0x0a,
    GGK_h               = 0x0b,
    GGK_i               = 0x0c,
    GGK_j               = 0x0d,
    GGK_k               = 0x0e,
    GGK_l               = 0x0f,
    GGK_m               = 0x10,
    GGK_n               = 0x11,
    GGK_o               = 0x12,
    GGK_p               = 0x13,
    GGK_q               = 0x14,
    GGK_r               = 0x15,
    GGK_s               = 0x16,
    GGK_t               = 0x17,
    GGK_u               = 0x18,
    GGK_v               = 0x19,
    GGK_w               = 0x1a,
    GGK_x               = 0x1b,
    GGK_y               = 0x1c,
    GGK_z               = 0x1d,
    GGK_1               = 0x1e,
    GGK_2               = 0x1f,
    GGK_3               = 0x20,
    GGK_4               = 0x21,
    GGK_5               = 0x22,
    GGK_6               = 0x23,
    GGK_7               = 0x24,
    GGK_8               = 0x25,
    GGK_9               = 0x26,
    GGK_0               = 0x27,
    GGK_RETURN          = 0x28,
    GGK_ESCAPE          = 0x29,
    GGK_BACKSPACE       = 0x2a,
    GGK_TAB             = 0x2b,
    GGK_SPACE           = 0x2c,
    GGK_MINUS           = 0x2d,
    GGK_EQUALS          = 0x2e,
    GGK_LBRACKET        = 0x2f,
    GGK_RBRACKET        = 0x30,
    GGK_BACKSLASH       = 0x31,
    GGK_NONUSHASH       = 0x32,
    GGK_SEMICOLON       = 0x33,
    GGK_APOSTROPHE      = 0x34,
    GGK_GRAVE           = 0x35,
    GGK_COMMA           = 0x36,
    GGK_PERIOD          = 0x37,
    GGK_SLASH           = 0x38,
    GGK_CAPSLOCK        = 0x39,
    GGK_F1              = 0x3a,
    GGK_F2              = 0x3b,
    GGK_F3              = 0x3c,
    GGK_F4              = 0x3d,
    GGK_F5              = 0x3e,
    GGK_F6              = 0x3f,
    GGK_F7              = 0x40,
    GGK_F8              = 0x41,
    GGK_F9              = 0x42,
    GGK_F10             = 0x43,
    GGK_F11             = 0x44,
    GGK_F12             = 0x45,
    GGK_PRINTSCREEN     = 0x46,
    GGK_SCROLLLOCK      = 0x47,
    GGK_PAUSE           = 0x48,
    GGK_INSERT          = 0x49,
    GGK_HOME            = 0x4a,
    GGK_PAGEUP          = 0x4b,
    GGK_DELETE          = 0x4c,
    GGK_END             = 0x4d,
    GGK_PAGEDOWN        = 0x4e,
    GGK_RIGHT           = 0x4f,
    GGK_LEFT            = 0x50,
    GGK_DOWN            = 0x51,
    GGK_UP              = 0x52,
    GGK_KP_NUMLOCK      = 0x53,
    GGK_KP_DIVIDE       = 0x54,
    GGK_KP_MULTIPLY     = 0x55,
    GGK_KP_MINUS        = 0x56,
    GGK_KP_PLUS         = 0x57,
    GGK_KP_ENTER        = 0x58,
    GGK_KP1             = 0x59,
    GGK_KP2             = 0x5a,
    GGK_KP3             = 0x5b,
    GGK_KP4             = 0x5c,
    GGK_KP5             = 0x5d,
    GGK_KP6             = 0x5e,
    GGK_KP7             = 0x5f,
    GGK_KP8             = 0x60,
    GGK_KP9             = 0x61,
    GGK_KP0             = 0x62,
    GGK_KP_PERIOD       = 0x63,
    GGK_NONUSBACKSLASH  = 0x64,
    GGK_APPLICATION     = 0x65,
    GGK_POWER           = 0x66,  /// only a status flag according to USB HID
    GGK_KP_EQUALS       = 0x67,
    GGK_F13             = 0x68,
    GGK_F14             = 0x69,
    GGK_F15             = 0x6a,
    GGK_F16             = 0x6b,
    GGK_F17             = 0x6c,
    GGK_F18             = 0x6d,
    GGK_F19             = 0x6e,
    GGK_F20             = 0x6f,
    GGK_F21             = 0x70,
    GGK_F22             = 0x71,
    GGK_F23             = 0x72,
    GGK_F24             = 0x73,
    GGK_EXECUTE         = 0x74,
    GGK_HELP            = 0x75,
    GGK_MENU            = 0x76,
    GGK_SELECT          = 0x77,
    GGK_STOP            = 0x78,
    GGK_AGAIN           = 0x79,
    GGK_UNDO            = 0x7a,
    GGK_CUT             = 0x7b,
    GGK_COPY            = 0x7c,
    GGK_PASTE           = 0x7d,
    GGK_FIND            = 0x7e,
    GGK_MUTE            = 0x7f,
    GGK_VOLUMEUP        = 0x80,
    GGK_VOLUMEDOWN      = 0x81,
    GGK_LOCKINGCAPS     = 0x82,
    GGK_LOCKINGNUM      = 0x83,
    GGK_LOCKINGSCROLL   = 0x84,
    GGK_KP_COMMA        = 0x85,
    GGK_KP_EQUALSAS400  = 0x86,
    GGK_INTERNATIONAL1  = 0x87,
    GGK_INTERNATIONAL2  = 0x88,
    GGK_INTERNATIONAL3  = 0x89,
    GGK_INTERNATIONAL4  = 0x8a,
    GGK_INTERNATIONAL5  = 0x8b,
    GGK_INTERNATIONAL6  = 0x8c,
    GGK_INTERNATIONAL7  = 0x8d,
    GGK_INTERNATIONAL8  = 0x8e,
    GGK_INTERNATIONAL9  = 0x8f,
    GGK_LANGUAGE1       = 0x90,
    GGK_LANGUAGE2       = 0x91,
    GGK_LANGUAGE3       = 0x92,
    GGK_LANGUAGE4       = 0x93,
    GGK_LANGUAGE5       = 0x94,
    GGK_LANGUAGE6       = 0x95,
    GGK_LANGUAGE7       = 0x96,
    GGK_LANGUAGE8       = 0x97,
    GGK_LANGUAGE9       = 0x98,
    GGK_ERASE_ALT       = 0x99,
    GGK_SYSTEMREQUEST   = 0x9a,
    GGK_CANCEL          = 0x9b,
    GGK_CLEAR           = 0x9c,
    GGK_PRIOR           = 0x9d,
    GGK_RETURN_ALT      = 0x9e,
    GGK_SEPARATOR       = 0x9f,
    GGK_OUT             = 0xa0,
    GGK_OPER            = 0xa1,
    GGK_CLEARAGAIN      = 0xa2,
    GGK_CRSEL           = 0xa3,
    GGK_EXSEL           = 0xa4,
    // Reserved block     0xa5 - 0xaf
    GGK_KP00            = 0xb0,
    GGK_KP000           = 0xb1,
    GGK_THOUSANDSSEP    = 0xb2,
    GGK_DECIMALSEP      = 0xb3,
    GGK_CURRENCYUNIT    = 0xb4,
    GGK_CURRENCYSUBUNIT = 0xb5,
    GGK_KP_LPARENTHESIS = 0xb6,
    GGK_KP_RPARENTHESIS = 0xb7,
    GGK_KP_LBRACE       = 0xb8,
    GGK_KP_RBRACE       = 0xb9,
    GGK_KP_TAB          = 0xba,
    GGK_KP_BACKSPACE    = 0xbb,
    GGK_KP_A            = 0xbc,
    GGK_KP_B            = 0xbd,
    GGK_KP_C            = 0xbe,
    GGK_KP_D            = 0xbf,
    GGK_KP_E            = 0xc0,
    GGK_KP_F            = 0xc1,
    GGK_KP_XOR          = 0xc2,
    GGK_KP_POWER        = 0xc3,
    GGK_KP_PERCENT      = 0xc4,
    GGK_KP_LESS         = 0xc5,
    GGK_KP_GREATER      = 0xc6,
    GGK_KP_AMPERSAND    = 0xc7,
    GGK_KP_DBLAMPERSAND = 0xc8,
    GGK_KP_BAR          = 0xc9,
    GGK_KP_DBLBAR       = 0xca,
    GGK_KP_COLON        = 0xcb,
    GGK_KP_HASH         = 0xcc,
    GGK_KP_SPACE        = 0xcd,
    GGK_KP_AT           = 0xce,
    GGK_KP_EXCLAMATION  = 0xcf,
    GGK_KP_MEMSTORE     = 0xd0,
    GGK_KP_MEMRECALL    = 0xd1,
    GGK_KP_MEMCLEAR     = 0xd2,
    GGK_KP_MEMADD       = 0xd3,
    GGK_KP_MEMSUBTRACT  = 0xd4,
    GGK_KP_MEMMULTIPLY  = 0xd5,
    GGK_KP_MEMDIVIDE    = 0xd6,
    GGK_KP_PLUSMINUS    = 0xd7,
    GGK_KP_CLEAR        = 0xd8,
    GGK_KP_CLEARENTRY   = 0xd9,
    GGK_KP_BINARY       = 0xda,
    GGK_KP_OCTAL        = 0xdb,
    GGK_KP_DECIMAL      = 0xdc,
    GGK_KP_HEXADECIMAL  = 0xdd,
    // Reserved block     0xde - 0xdf
    GGK_LCONTROL        = 0xe0,
    GGK_LSHIFT          = 0xe1,
    GGK_LALT            = 0xe2,
    GGK_LGUI            = 0xe3,
    GGK_RCONTROL        = 0xe4,
    GGK_RSHIFT          = 0xe5,
    GGK_RALT            = 0xe6,
    GGK_RGUI            = 0xe7,
    // Reserved block     0xe8 - 0xffff
    GGK_LAST
)

}


#endif
