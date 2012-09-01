/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2007 T. Zachary Laine

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
   
#include <GG/Ogre/Plugins/OISInput.h>

#include <GG/Ogre/OgreGUI.h>

#include <OgreConfigFile.h>
#include <OgreLogManager.h>
#include <OgreRenderWindow.h>
#include <OgreRoot.h>

#include <OIS/OIS.h>

#include <boost/cast.hpp>
#include <boost/lexical_cast.hpp>

#include <cctype>

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif


using namespace GG;

namespace {
    OISInput* gOISInputPlugin = 0;

    const Ogre::String PLUGIN_NAME = "OIS Input Plugin";

    Flags<ModKey> GetModKeys(OIS::Keyboard* keyboard)
    {
        Flags<ModKey> retval;
#ifdef __APPLE__
        UInt32 mod_keys = GetCurrentKeyModifiers();
        if ((mod_keys & shiftKey) > 0)             retval |= MOD_KEY_LSHIFT;
        if ((mod_keys & rightShiftKey) > 0)        retval |= MOD_KEY_RSHIFT;
        if ((mod_keys & controlKey) > 0)           retval |= MOD_KEY_LCTRL;
        if ((mod_keys & rightControlKey) > 0)      retval |= MOD_KEY_RCTRL;
        if ((mod_keys & optionKey) > 0)            retval |= MOD_KEY_LALT;
        if ((mod_keys & rightOptionKey) > 0)       retval |= MOD_KEY_RALT;
        if ((mod_keys & cmdKey) > 0)               retval |= MOD_KEY_LMETA;
#else
        if (keyboard->isKeyDown(OIS::KC_LSHIFT))   retval |= MOD_KEY_LSHIFT;
        if (keyboard->isKeyDown(OIS::KC_RSHIFT))   retval |= MOD_KEY_RSHIFT;
        if (keyboard->isKeyDown(OIS::KC_LCONTROL)) retval |= MOD_KEY_LCTRL;
        if (keyboard->isKeyDown(OIS::KC_RCONTROL)) retval |= MOD_KEY_RCTRL;
        if (keyboard->isKeyDown(OIS::KC_LMENU))    retval |= MOD_KEY_LALT;
        if (keyboard->isKeyDown(OIS::KC_RMENU))    retval |= MOD_KEY_RALT;
        if (keyboard->isKeyDown(OIS::KC_LWIN))     retval |= MOD_KEY_LMETA;
        if (keyboard->isKeyDown(OIS::KC_RWIN))     retval |= MOD_KEY_RMETA;
#endif
#ifdef WIN32
        bool capslock = static_cast<unsigned short>(GetKeyState(0x14)) & 0x0001;
        bool numlock = static_cast<unsigned short>(GetKeyState(0x90)) & 0x0001;
        bool scrolllock = static_cast<unsigned short>(GetKeyState(0x91)) & 0x0001;
        if (capslock)   retval |= MOD_KEY_CAPS;
        if (numlock)    retval |= MOD_KEY_NUM;
        //if (scrolllock) retval |= MOD_KEY_SCROLL;
#else
        if (keyboard->isKeyDown(OIS::KC_NUMLOCK))  retval |= MOD_KEY_NUM;
        if (keyboard->isKeyDown(OIS::KC_CAPITAL))  retval |= MOD_KEY_CAPS;
#endif
        return retval;
    }

    Key GGKeyFromOISKey(const OIS::KeyEvent& event, Flags<ModKey> mods,
                        OIS::Keyboard::TextTranslationMode translation_mode)
    {
        Key retval = GGK_UNKNOWN;

        switch (event.key) {
        case OIS::KC_UNASSIGNED:   retval = GGK_UNKNOWN; break;
        case OIS::KC_ESCAPE:       retval = GGK_ESCAPE; break;
        case OIS::KC_1:            retval = GGK_1; break;
        case OIS::KC_2:            retval = GGK_2; break;
        case OIS::KC_3:            retval = GGK_3; break;
        case OIS::KC_4:            retval = GGK_4; break;
        case OIS::KC_5:            retval = GGK_5; break;
        case OIS::KC_6:            retval = GGK_6; break;
        case OIS::KC_7:            retval = GGK_7; break;
        case OIS::KC_8:            retval = GGK_8; break;
        case OIS::KC_9:            retval = GGK_9; break;
        case OIS::KC_0:            retval = GGK_0; break;
        case OIS::KC_MINUS:        retval = GGK_MINUS; break;
        case OIS::KC_EQUALS:       retval = GGK_EQUALS; break;
        case OIS::KC_BACK:         retval = GGK_BACKSPACE; break;
        case OIS::KC_TAB:          retval = GGK_TAB; break;
        case OIS::KC_Q:            retval = GGK_q; break;
        case OIS::KC_W:            retval = GGK_w; break;
        case OIS::KC_E:            retval = GGK_e; break;
        case OIS::KC_R:            retval = GGK_r; break;
        case OIS::KC_T:            retval = GGK_t; break;
        case OIS::KC_Y:            retval = GGK_y; break;
        case OIS::KC_U:            retval = GGK_u; break;
        case OIS::KC_I:            retval = GGK_i; break;
        case OIS::KC_O:            retval = GGK_o; break;
        case OIS::KC_P:            retval = GGK_p; break;
        case OIS::KC_LBRACKET:     retval = GGK_LEFTBRACKET; break;
        case OIS::KC_RBRACKET:     retval = GGK_RIGHTBRACKET; break;
        case OIS::KC_RETURN:       retval = GGK_RETURN; break;
        case OIS::KC_LCONTROL:     retval = GGK_LCTRL; break;
        case OIS::KC_A:            retval = GGK_a; break;
        case OIS::KC_S:            retval = GGK_s; break;
        case OIS::KC_D:            retval = GGK_d; break;
        case OIS::KC_F:            retval = GGK_f; break;
        case OIS::KC_G:            retval = GGK_g; break;
        case OIS::KC_H:            retval = GGK_h; break;
        case OIS::KC_J:            retval = GGK_j; break;
        case OIS::KC_K:            retval = GGK_k; break;
        case OIS::KC_L:            retval = GGK_l; break;
        case OIS::KC_SEMICOLON:    retval = GGK_SEMICOLON; break;
        case OIS::KC_APOSTROPHE:   retval = GGK_QUOTE; break;
        case OIS::KC_GRAVE:        retval = GGK_BACKQUOTE; break;
        case OIS::KC_LSHIFT:       retval = GGK_LSHIFT; break;
        case OIS::KC_BACKSLASH:    retval = GGK_BACKSLASH; break;
        case OIS::KC_Z:            retval = GGK_z; break;
        case OIS::KC_X:            retval = GGK_x; break;
        case OIS::KC_C:            retval = GGK_c; break;
        case OIS::KC_V:            retval = GGK_v; break;
        case OIS::KC_B:            retval = GGK_b; break;
        case OIS::KC_N:            retval = GGK_n; break;
        case OIS::KC_M:            retval = GGK_m; break;
        case OIS::KC_COMMA:        retval = GGK_COMMA; break;
        case OIS::KC_PERIOD:       retval = GGK_PERIOD; break;
        case OIS::KC_SLASH:        retval = GGK_SLASH; break;
        case OIS::KC_RSHIFT:       retval = GGK_RSHIFT; break;
        case OIS::KC_MULTIPLY:     retval = GGK_KP_MULTIPLY; break;
        case OIS::KC_LMENU:        retval = GGK_LALT; break;
        case OIS::KC_SPACE:        retval = GGK_SPACE; break;
        case OIS::KC_CAPITAL:      retval = GGK_CAPSLOCK; break;
        case OIS::KC_F1:           retval = GGK_F1; break;
        case OIS::KC_F2:           retval = GGK_F2; break;
        case OIS::KC_F3:           retval = GGK_F3; break;
        case OIS::KC_F4:           retval = GGK_F4; break;
        case OIS::KC_F5:           retval = GGK_F5; break;
        case OIS::KC_F6:           retval = GGK_F6; break;
        case OIS::KC_F7:           retval = GGK_F7; break;
        case OIS::KC_F8:           retval = GGK_F8; break;
        case OIS::KC_F9:           retval = GGK_F9; break;
        case OIS::KC_F10:          retval = GGK_F10; break;
        case OIS::KC_NUMLOCK:      retval = GGK_NUMLOCK; break;
        case OIS::KC_SCROLL:       retval = GGK_SCROLLOCK; break;
        case OIS::KC_NUMPAD7:      retval = GGK_KP7; break;
        case OIS::KC_NUMPAD8:      retval = GGK_KP8; break;
        case OIS::KC_NUMPAD9:      retval = GGK_KP9; break;
        case OIS::KC_SUBTRACT:     retval = GGK_KP_MINUS; break;
        case OIS::KC_NUMPAD4:      retval = GGK_KP4; break;
        case OIS::KC_NUMPAD5:      retval = GGK_KP5; break;
        case OIS::KC_NUMPAD6:      retval = GGK_KP6; break;
        case OIS::KC_ADD:          retval = GGK_KP_PLUS; break;
        case OIS::KC_NUMPAD1:      retval = GGK_KP1; break;
        case OIS::KC_NUMPAD2:      retval = GGK_KP2; break;
        case OIS::KC_NUMPAD3:      retval = GGK_KP3; break;
        case OIS::KC_NUMPAD0:      retval = GGK_KP0; break;
        case OIS::KC_DECIMAL:      retval = GGK_KP_PERIOD; break;
        case OIS::KC_OEM_102:      retval = GGK_UNKNOWN; break;
        case OIS::KC_F11:          retval = GGK_F11; break;
        case OIS::KC_F12:          retval = GGK_F12; break;
        case OIS::KC_F13:          retval = GGK_F13; break;
        case OIS::KC_F14:          retval = GGK_F14; break;
        case OIS::KC_F15:          retval = GGK_F15; break;
        case OIS::KC_KANA:         retval = GGK_UNKNOWN; break;
        case OIS::KC_ABNT_C1:      retval = GGK_SLASH; break;
        case OIS::KC_CONVERT:      retval = GGK_UNKNOWN; break;
        case OIS::KC_NOCONVERT:    retval = GGK_UNKNOWN; break;
        case OIS::KC_YEN:          retval = GGK_UNKNOWN; break;
        case OIS::KC_ABNT_C2:      retval = GGK_KP_PERIOD; break;
        case OIS::KC_NUMPADEQUALS: retval = GGK_KP_EQUALS; break;
        case OIS::KC_PREVTRACK:    retval = GGK_UNKNOWN; break;
        case OIS::KC_AT:           retval = GGK_AT; break;
        case OIS::KC_COLON:        retval = GGK_COLON; break;
        case OIS::KC_UNDERLINE:    retval = GGK_UNDERSCORE; break;
        case OIS::KC_KANJI:        retval = GGK_UNKNOWN; break;
        case OIS::KC_STOP:         retval = GGK_UNKNOWN; break;
        case OIS::KC_AX:           retval = GGK_UNKNOWN; break;
        case OIS::KC_UNLABELED:    retval = GGK_UNKNOWN; break;
        case OIS::KC_NEXTTRACK:    retval = GGK_UNKNOWN; break;
        case OIS::KC_NUMPADENTER:  retval = GGK_KP_ENTER; break;
        case OIS::KC_RCONTROL:     retval = GGK_RCTRL; break;
        case OIS::KC_MUTE:         retval = GGK_UNKNOWN; break;
        case OIS::KC_CALCULATOR:   retval = GGK_UNKNOWN; break;
        case OIS::KC_PLAYPAUSE:    retval = GGK_UNKNOWN; break;
        case OIS::KC_MEDIASTOP:    retval = GGK_UNKNOWN; break;
        case OIS::KC_VOLUMEDOWN:   retval = GGK_UNKNOWN; break;
        case OIS::KC_VOLUMEUP:     retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBHOME:      retval = GGK_UNKNOWN; break;
        case OIS::KC_NUMPADCOMMA:  retval = GGK_KP_EQUALS; break;
        case OIS::KC_DIVIDE:       retval = GGK_KP_DIVIDE; break;
        case OIS::KC_SYSRQ:        retval = GGK_SYSREQ; break;
        case OIS::KC_RMENU:        retval = GGK_RALT; break;
        case OIS::KC_PAUSE:        retval = GGK_PAUSE; break;
        case OIS::KC_HOME:         retval = GGK_HOME; break;
        case OIS::KC_UP:           retval = GGK_UP; break;
        case OIS::KC_PGUP:         retval = GGK_PAGEUP; break;
        case OIS::KC_LEFT:         retval = GGK_LEFT; break;
        case OIS::KC_RIGHT:        retval = GGK_RIGHT; break;
        case OIS::KC_END:          retval = GGK_END; break;
        case OIS::KC_DOWN:         retval = GGK_DOWN; break;
        case OIS::KC_PGDOWN:       retval = GGK_PAGEDOWN; break;
        case OIS::KC_INSERT:       retval = GGK_INSERT; break;
        case OIS::KC_DELETE:       retval = GGK_DELETE; break;
        case OIS::KC_LWIN:         retval = GGK_LMETA; break;
        case OIS::KC_RWIN:         retval = GGK_RMETA; break;
        case OIS::KC_APPS:         retval = GGK_UNKNOWN; break;
        case OIS::KC_POWER:        retval = GGK_POWER; break;
        case OIS::KC_SLEEP:        retval = GGK_UNKNOWN; break;
        case OIS::KC_WAKE:         retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBSEARCH:    retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBFAVORITES: retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBREFRESH:   retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBSTOP:      retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBFORWARD:   retval = GGK_UNKNOWN; break;
        case OIS::KC_WEBBACK:      retval = GGK_UNKNOWN; break;
        case OIS::KC_MYCOMPUTER:   retval = GGK_UNKNOWN; break;
        case OIS::KC_MAIL:         retval = GGK_UNKNOWN; break;
        case OIS::KC_MEDIASELECT:  retval = GGK_UNKNOWN; break;
        }

        // this code works because GG::Key maps (at least partially) to the
        // printable ASCII characters
        bool shift = mods & MOD_KEY_SHIFT;
        bool caps_lock = mods & MOD_KEY_CAPS;
        if (shift || caps_lock) {
            if (shift != caps_lock && ('a' <= retval && retval <= 'z')) {
                retval = Key(std::toupper(retval));
            } else if (shift) { // the caps lock key should not affect these
                // this assumes a US keyboard layout
                switch (retval) {
                case '`': retval = Key('~'); break;
                case '1': retval = Key('!'); break;
                case '2': retval = Key('@'); break;
                case '3': retval = Key('#'); break;
                case '4': retval = Key('$'); break;
                case '5': retval = Key('%'); break;
                case '6': retval = Key('^'); break;
                case '7': retval = Key('&'); break;
                case '8': retval = Key('*'); break;
                case '9': retval = Key('('); break;
                case '0': retval = Key(')'); break;
                case '-': retval = Key('_'); break;
                case '=': retval = Key('+'); break;
                case '[': retval = Key('{'); break;
                case ']': retval = Key('}'); break;
                case '\\': retval = Key('|'); break;
                case ';': retval = Key(':'); break;
                case '\'': retval = Key('"'); break;
                case ',': retval = Key('<'); break;
                case '.': retval = Key('>'); break;
                case '/': retval = Key('?'); break;
                default: break;
                }
            }
        }

        return retval;
    }
}

OISInput::OISInput() :
    m_input_manager(0),
    m_mouse(0),
    m_keyboard(0)
{}

OISInput::~OISInput()
{
    CleanupInputManager();
    DisconnectHandlers();
}

const Ogre::String& OISInput::getName() const
{ return PLUGIN_NAME; }

void OISInput::install()
{}

void OISInput::initialise()
{
    Ogre::RenderWindow* window = GetRenderWindow();

    typedef OIS::ParamList::value_type ParamType;

    OIS::ParamList param_list;
    std::size_t window_handle = 0;
    std::ostringstream window_handle_string;
    window->getCustomAttribute("WINDOW", &window_handle);
    window_handle_string << window_handle;
    param_list.insert(ParamType(std::string("WINDOW"), window_handle_string.str()));

    OgreGUI* gui = OgreGUI::GetGUI();
    assert(gui);
    const Ogre::SharedPtr<Ogre::DataStream>& config_file_stream = gui->ConfigFileStream();
    if (!config_file_stream.isNull()) {
        Ogre::ConfigFile config_file;
        config_file.load(config_file_stream);
        for (Ogre::ConfigFile::SettingsIterator it = config_file.getSettingsIterator();
             it.hasMoreElements();
             it.getNext()) {
            param_list.insert(ParamType(it.peekNextKey(), it.peekNextValue()));
            Ogre::LogManager::getSingleton().logMessage("OISPlugin using config setting " + it.peekNextKey() + "=" + it.peekNextValue());
        }
    }

    m_input_manager = OIS::InputManager::createInputSystem(param_list);
    m_keyboard = boost::polymorphic_downcast<OIS::Keyboard*>(
        m_input_manager->createInputObject(OIS::OISKeyboard, true));
    m_keyboard->setEventCallback(this);
    m_mouse = boost::polymorphic_downcast<OIS::Mouse*>(
        m_input_manager->createInputObject(OIS::OISMouse, true));
    m_mouse->setEventCallback(this);

    const OIS::MouseState& mouse_state = m_mouse->getMouseState();
    mouse_state.width = Value(gui->AppWidth());
    mouse_state.height = Value(gui->AppHeight());

    ConnectHandlers();
}

void OISInput::shutdown()
{
    CleanupInputManager();
    DisconnectHandlers();
}

void OISInput::uninstall()
{}

void OISInput::HandleSystemEvents()
{
    assert(m_mouse->buffered());
    assert(m_keyboard->buffered());
    m_mouse->capture();
    m_keyboard->capture();
}

void OISInput::HandleWindowResize(X width, Y height)
{
    const OIS::MouseState& mouse_state = m_mouse->getMouseState();
    mouse_state.width = Value(width);
    mouse_state.height = Value(height);
}

void OISInput::HandleWindowClose()
{
    CleanupInputManager();
    DisconnectHandlers();
}

bool OISInput::mouseMoved(const OIS::MouseEvent &event)
{
    Pt mouse_pos(X(event.state.X.abs), Y(event.state.Y.abs));
    assert(OgreGUI::GetGUI());
    if (event.state.Z.rel)
        OgreGUI::GetGUI()->HandleGGEvent(GUI::MOUSEWHEEL, GGK_UNKNOWN, 0, GetModKeys(m_keyboard), mouse_pos, Pt(X0, 0 < event.state.Z.rel ? Y1 : -Y1));
    else
        OgreGUI::GetGUI()->HandleGGEvent(GUI::MOUSEMOVE, GGK_UNKNOWN, 0, GetModKeys(m_keyboard), mouse_pos, Pt(X(event.state.X.rel), Y(event.state.Y.rel)));
    return true;
}

bool OISInput::mousePressed(const OIS::MouseEvent &event, OIS::MouseButtonID id)
{
    Pt mouse_pos(X(event.state.X.abs), Y(event.state.Y.abs));
    GUI::EventType gg_event = GUI::IDLE;
    switch (id) {
    case OIS::MB_Left:   gg_event = GUI::LPRESS; break;
    case OIS::MB_Right:  gg_event = GUI::RPRESS; break;
    case OIS::MB_Middle: gg_event = GUI::MPRESS; break;
    default: break;
    }
    assert(OgreGUI::GetGUI());
    if (gg_event != GUI::IDLE)
        OgreGUI::GetGUI()->HandleGGEvent(gg_event, GGK_UNKNOWN, 0, GetModKeys(m_keyboard), mouse_pos, Pt());
    return true;
}

bool OISInput::mouseReleased(const OIS::MouseEvent &event, OIS::MouseButtonID id)
{
    Pt mouse_pos(X(event.state.X.abs), Y(event.state.Y.abs));
    GUI::EventType gg_event = GUI::IDLE;
    switch (id) {
    case OIS::MB_Left:   gg_event = GUI::LRELEASE; break;
    case OIS::MB_Right:  gg_event = GUI::RRELEASE; break;
    case OIS::MB_Middle: gg_event = GUI::MRELEASE; break;
    default: break;
    }
    assert(OgreGUI::GetGUI());
    if (gg_event != GUI::IDLE)
        OgreGUI::GetGUI()->HandleGGEvent(gg_event, GGK_UNKNOWN, 0, GetModKeys(m_keyboard), mouse_pos, Pt());
    return true;
}

bool OISInput::keyPressed(const OIS::KeyEvent& event)
{
    Flags<ModKey> mods = GetModKeys(m_keyboard);
    Key key = GGKeyFromOISKey(event, mods, m_keyboard->getTextTranslation());
    assert(OgreGUI::GetGUI());
    if (key != GGK_UNKNOWN)
        OgreGUI::GetGUI()->HandleGGEvent(GUI::KEYPRESS, key, 0, mods, Pt(), Pt());
    return true;
}

bool OISInput::keyReleased(const OIS::KeyEvent& event)
{
    Flags<ModKey> mods = GetModKeys(m_keyboard);
    Key key = GGKeyFromOISKey(event, mods, m_keyboard->getTextTranslation());
    assert(OgreGUI::GetGUI());
    if (key != GGK_UNKNOWN)
        OgreGUI::GetGUI()->HandleGGEvent(GUI::KEYRELEASE, key, 0, mods, Pt(), Pt());
    return true;
}

void OISInput::CleanupInputManager()
{
    if (m_input_manager) {
        m_input_manager->destroyInputObject(m_mouse);
        m_input_manager->destroyInputObject(m_keyboard);
        OIS::InputManager::destroyInputSystem(m_input_manager);
        m_input_manager = 0;
    }
}

#ifndef OGRE_STATIC_LIB
extern "C" void GG_OGRE_PLUGIN_API dllStartPlugin(void)
{
    gOISInputPlugin = new OISInput();
    Ogre::Root::getSingleton().installPlugin(gOISInputPlugin);
}

extern "C" void GG_OGRE_PLUGIN_API dllStopPlugin(void)
{
        Ogre::Root::getSingleton().uninstallPlugin(gOISInputPlugin);
    delete gOISInputPlugin;
}
#endif
