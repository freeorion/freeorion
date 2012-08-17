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

#include <GG/SDL/SDLGUI.h>
#include <GG/EventPump.h>
#include <GG/WndEvent.h>

#include <cctype>
#include <iostream>


using namespace GG;

namespace {
    Flags<ModKey> GetSDLModKeys()
    {
        Flags<ModKey> retval;
        Uint32 sdl_keys = SDL_GetModState();
        if (sdl_keys & KMOD_LSHIFT) retval |= MOD_KEY_LSHIFT;
        if (sdl_keys & KMOD_RSHIFT) retval |= MOD_KEY_RSHIFT;
        if (sdl_keys & KMOD_LCTRL)  retval |= MOD_KEY_LCTRL;
        if (sdl_keys & KMOD_RCTRL)  retval |= MOD_KEY_RCTRL;
        if (sdl_keys & KMOD_LALT)   retval |= MOD_KEY_LALT;
        if (sdl_keys & KMOD_RALT)   retval |= MOD_KEY_RALT;
        if (sdl_keys & KMOD_LMETA)  retval |= MOD_KEY_LMETA;
        if (sdl_keys & KMOD_RMETA)  retval |= MOD_KEY_RMETA;
        if (sdl_keys & KMOD_NUM)    retval |= MOD_KEY_NUM;
        if (sdl_keys & KMOD_CAPS)   retval |= MOD_KEY_CAPS;
        if (sdl_keys & KMOD_MODE)   retval |= MOD_KEY_MODE;
        return retval;
    }
}

// member functions
SDLGUI::SDLGUI(int w/* = 1024*/, int h/* = 768*/, bool calc_FPS/* = false*/, const std::string& app_name/* = "GG"*/) :
    GUI(app_name),
    m_app_width(w),
    m_app_height(h)
{}

SDLGUI::~SDLGUI()
{ SDLQuit(); }

X SDLGUI::AppWidth() const
{ return m_app_width; }

Y SDLGUI::AppHeight() const
{ return m_app_height; }

unsigned int SDLGUI::Ticks() const
{ return SDL_GetTicks(); }

void SDLGUI::operator()()
{ GUI::operator()(); }

void SDLGUI::Exit(int code)
{
    if (code)
        std::cerr << "Initiating Exit (code " << code << " - error termination)";
    SDLQuit();
    exit(code);
}

SDLGUI* SDLGUI::GetGUI()
{ return dynamic_cast<SDLGUI*>(GUI::GetGUI()); }

Key SDLGUI::GGKeyFromSDLKey(const SDL_keysym& key)
{
    Key retval = Key(key.sym);
    bool shift = key.mod & KMOD_SHIFT;
    bool caps_lock = key.mod & KMOD_CAPS;

    // this code works because both SDLKey and Key map (at least
    // partially) to the printable ASCII characters
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

void SDLGUI::SetAppSize(const Pt& size)
{
    m_app_width = size.x;
    m_app_height = size.y;
}

void SDLGUI::SDLInit()
{
    const SDL_VideoInfo* vid_info = 0;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError();
        Exit(1);
    }

    vid_info = SDL_GetVideoInfo();

    if (!vid_info) {
        std::cerr << "Video info query failed: " << SDL_GetError();
        Exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    if (SDL_SetVideoMode(Value(m_app_width), Value(m_app_height), 16, SDL_OPENGL) == 0) {
        std::cerr << "Video mode set failed: " << SDL_GetError();
        Exit(1);
    }

    SDL_EnableUNICODE(1);

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    EnableMouseButtonDownRepeat(SDL_DEFAULT_REPEAT_DELAY / 2, SDL_DEFAULT_REPEAT_INTERVAL / 2);

    GLInit();
}

void SDLGUI::GLInit()
{
    double ratio = Value(m_app_width * 1.0) / Value(m_app_height);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glShadeModel(GL_SMOOTH);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, Value(m_app_width), Value(m_app_height));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);
}

void SDLGUI::HandleSystemEvents()
{
    // handle events
    SDL_Event event;
    while (0 < SDL_PollEvent(&event)) {
        bool send_to_gg = false;
        EventType gg_event = MOUSEMOVE;
        Key key = GGK_UNKNOWN;
        boost::uint32_t key_code_point = 0;
        Flags<ModKey> mod_keys = GetSDLModKeys();
        Pt mouse_pos(X(event.motion.x), Y(event.motion.y));
        Pt mouse_rel(X(event.motion.xrel), Y(event.motion.yrel));

        switch (event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            key = GGKeyFromSDLKey(event.key.keysym);
            key_code_point = event.key.keysym.unicode;
            if (key < GGK_NUMLOCK)
                send_to_gg = true;
            gg_event = (event.type == SDL_KEYDOWN) ? KEYPRESS : KEYRELEASE;
            break;
        case SDL_MOUSEMOTION:
            send_to_gg = true;
            gg_event = MOUSEMOVE;
            break;
        case SDL_MOUSEBUTTONDOWN:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:      gg_event = LPRESS; break;
                case SDL_BUTTON_MIDDLE:    gg_event = MPRESS; break;
                case SDL_BUTTON_RIGHT:     gg_event = RPRESS; break;
                case SDL_BUTTON_WHEELUP:   gg_event = MOUSEWHEEL; mouse_rel = Pt(X0, Y1); break;
                case SDL_BUTTON_WHEELDOWN: gg_event = MOUSEWHEEL; mouse_rel = Pt(X0, -Y1); break;
            }
            mod_keys = GetSDLModKeys();
            break;
        case SDL_MOUSEBUTTONUP:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:   gg_event = LRELEASE; break;
                case SDL_BUTTON_MIDDLE: gg_event = MRELEASE; break;
                case SDL_BUTTON_RIGHT:  gg_event = RRELEASE; break;
            }
            mod_keys = GetSDLModKeys();
            break;
        }

        if (send_to_gg)
            HandleGGEvent(gg_event, key, key_code_point, mod_keys, mouse_pos, mouse_rel);
        else
            HandleNonGGEvent(event);
    }
}

void SDLGUI::HandleNonGGEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_QUIT:
        Exit(0);
        break;
    }
}

void SDLGUI::RenderBegin()
{ glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

void SDLGUI::RenderEnd()
{ SDL_GL_SwapBuffers(); }

void SDLGUI::FinalCleanup()
{}

void SDLGUI::SDLQuit()
{
    FinalCleanup();
    SDL_Quit();
}

void SDLGUI::Run()
{
    try {
        SDLInit();
        Initialize();
        EventPump pump;
        pump();
    } catch (const std::invalid_argument& e) {
        std::cerr << "std::invalid_argument exception caught in GUI::Run(): " << e.what();
        Exit(1);
    } catch (const std::runtime_error& e) {
        std::cerr << "std::runtime_error exception caught in GUI::Run(): " << e.what();
        Exit(1);
    } catch (const ExceptionBase& e) {
        std::cerr << "GG exception (subclass " << e.type() << ") caught in GUI::Run(): " << e.what();
        Exit(1);
    }
}

