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
#include <GG/utf8/checked.h>

#include <cctype>
#include <iostream>

#include <GG/DrawUtil.h>

#include <boost/format.hpp>

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
        if (sdl_keys & KMOD_LGUI)   retval |= MOD_KEY_LMETA;
        if (sdl_keys & KMOD_RGUI)   retval |= MOD_KEY_RMETA;
        if (sdl_keys & KMOD_NUM)    retval |= MOD_KEY_NUM;
        if (sdl_keys & KMOD_CAPS)   retval |= MOD_KEY_CAPS;
        if (sdl_keys & KMOD_MODE)   retval |= MOD_KEY_MODE;
        return retval;
    }

    // Constructs the map from SDL keycodes to GiGi keys.
    // The map is partial, not all sdl keys have corresponding gigi representations.
    void InitializeKeyMap(std::map<SDL_Keycode, GG::Key>& keys) {
        // These are, at least at the tim eof writing in the same order as
        // in the definition at SDL_keycode.h
        // All values there are present, the ones that don't have
        // a GiGi mapping are simply commented out.
        // This should make it easy to find the right place
        // to add keys should the GiGi key set be enriched.

        keys[SDLK_UNKNOWN] = GGK_UNKNOWN;

        keys[SDLK_RETURN] = GGK_RETURN;
        keys[SDLK_ESCAPE] = GGK_ESCAPE;
        keys[SDLK_BACKSPACE] = GGK_BACKSPACE;
        keys[SDLK_TAB] = GGK_TAB;
        keys[SDLK_SPACE] = GGK_SPACE;
        keys[SDLK_EXCLAIM] = GGK_EXCLAIM;
        keys[SDLK_QUOTEDBL] = GGK_QUOTEDBL;
        keys[SDLK_HASH] = GGK_HASH;
        // keys[SDLK_PERCENT] = GGK_PERCENT;
        keys[SDLK_DOLLAR] = GGK_DOLLAR;
        keys[SDLK_AMPERSAND] = GGK_AMPERSAND;
        keys[SDLK_QUOTE] = GGK_QUOTE;
        keys[SDLK_LEFTPAREN] = GGK_LEFTPAREN;
        keys[SDLK_RIGHTPAREN] = GGK_RIGHTPAREN;
        keys[SDLK_ASTERISK] = GGK_ASTERISK;
        keys[SDLK_PLUS] = GGK_PLUS;
        keys[SDLK_COMMA] = GGK_COMMA;
        keys[SDLK_MINUS] = GGK_MINUS;
        keys[SDLK_PERIOD] = GGK_PERIOD;
        keys[SDLK_SLASH] = GGK_SLASH;
        keys[SDLK_0] = GGK_0;
        keys[SDLK_1] = GGK_1;
        keys[SDLK_2] = GGK_2;
        keys[SDLK_3] = GGK_3;
        keys[SDLK_4] = GGK_4;
        keys[SDLK_5] = GGK_5;
        keys[SDLK_6] = GGK_6;
        keys[SDLK_7] = GGK_7;
        keys[SDLK_8] = GGK_8;
        keys[SDLK_9] = GGK_9;
        keys[SDLK_COLON] = GGK_COLON;
        keys[SDLK_SEMICOLON] = GGK_SEMICOLON;
        keys[SDLK_LESS] = GGK_LESS;
        keys[SDLK_EQUALS] = GGK_EQUALS;
        keys[SDLK_GREATER] = GGK_GREATER;
        keys[SDLK_QUESTION] = GGK_QUESTION;
        keys[SDLK_AT] = GGK_AT;
        /*
         S kip uppercase letters                               *
         */
        keys[SDLK_LEFTBRACKET] = GGK_LEFTBRACKET;
        keys[SDLK_BACKSLASH] = GGK_BACKSLASH;
        keys[SDLK_RIGHTBRACKET] = GGK_RIGHTBRACKET;
        keys[SDLK_CARET] = GGK_CARET;
        keys[SDLK_UNDERSCORE] = GGK_UNDERSCORE;
        keys[SDLK_BACKQUOTE] = GGK_BACKQUOTE;
        keys[SDLK_a] = GGK_a;
        keys[SDLK_b] = GGK_b;
        keys[SDLK_c] = GGK_c;
        keys[SDLK_d] = GGK_d;
        keys[SDLK_e] = GGK_e;
        keys[SDLK_f] = GGK_f;
        keys[SDLK_g] = GGK_g;
        keys[SDLK_h] = GGK_h;
        keys[SDLK_i] = GGK_i;
        keys[SDLK_j] = GGK_j;
        keys[SDLK_k] = GGK_k;
        keys[SDLK_l] = GGK_l;
        keys[SDLK_m] = GGK_m;
        keys[SDLK_n] = GGK_n;
        keys[SDLK_o] = GGK_o;
        keys[SDLK_p] = GGK_p;
        keys[SDLK_q] = GGK_q;
        keys[SDLK_r] = GGK_r;
        keys[SDLK_s] = GGK_s;
        keys[SDLK_t] = GGK_t;
        keys[SDLK_u] = GGK_u;
        keys[SDLK_v] = GGK_v;
        keys[SDLK_w] = GGK_w;
        keys[SDLK_x] = GGK_x;
        keys[SDLK_y] = GGK_y;
        keys[SDLK_z] = GGK_z;

        keys[SDLK_CAPSLOCK] = GGK_CAPSLOCK;

        keys[SDLK_F1] = GGK_F1;
        keys[SDLK_F2] = GGK_F2;
        keys[SDLK_F3] = GGK_F3;
        keys[SDLK_F4] = GGK_F4;
        keys[SDLK_F5] = GGK_F5;
        keys[SDLK_F6] = GGK_F6;
        keys[SDLK_F7] = GGK_F7;
        keys[SDLK_F8] = GGK_F8;
        keys[SDLK_F9] = GGK_F9;
        keys[SDLK_F10] = GGK_F10;
        keys[SDLK_F11] = GGK_F11;
        keys[SDLK_F12] = GGK_F12;

        // keys[SDLK_PRINTSCREEN] = GGK_PRINTSCREEN;
        // keys[SDLK_SCROLLLOCK] = GGK_SCROLLLOCK;
        keys[SDLK_PAUSE] = GGK_PAUSE;
        keys[SDLK_INSERT] = GGK_INSERT;
        keys[SDLK_HOME] = GGK_HOME;
        keys[SDLK_PAGEUP] = GGK_PAGEUP;
        keys[SDLK_DELETE] = GGK_DELETE;
        keys[SDLK_END] = GGK_END;
        keys[SDLK_PAGEDOWN] = GGK_PAGEDOWN;
        keys[SDLK_RIGHT] = GGK_RIGHT;
        keys[SDLK_LEFT] = GGK_LEFT;
        keys[SDLK_DOWN] = GGK_DOWN;
        keys[SDLK_UP] = GGK_UP;

        // keys[SDLK_NUMLOCKCLEAR] = GGK_NUMLOCKCLEAR;
        keys[SDLK_KP_DIVIDE] = GGK_KP_DIVIDE;
        keys[SDLK_KP_MULTIPLY] = GGK_KP_MULTIPLY;
        keys[SDLK_KP_MINUS] = GGK_KP_MINUS;
        keys[SDLK_KP_PLUS] = GGK_KP_PLUS;
        keys[SDLK_KP_ENTER] = GGK_KP_ENTER;
        keys[SDLK_KP_1] = GGK_KP1;
        keys[SDLK_KP_2] = GGK_KP2;
        keys[SDLK_KP_3] = GGK_KP3;
        keys[SDLK_KP_4] = GGK_KP4;
        keys[SDLK_KP_5] = GGK_KP5;
        keys[SDLK_KP_6] = GGK_KP6;
        keys[SDLK_KP_7] = GGK_KP7;
        keys[SDLK_KP_8] = GGK_KP8;
        keys[SDLK_KP_9] = GGK_KP9;
        keys[SDLK_KP_0] = GGK_KP0;
        keys[SDLK_KP_PERIOD] = GGK_KP_PERIOD;

        // keys[SDLK_APPLICATION] = GGK_APPLICATION;
        keys[SDLK_POWER] = GGK_POWER;
        keys[SDLK_KP_EQUALS] = GGK_KP_EQUALS;
        keys[SDLK_F13] = GGK_F13;
        keys[SDLK_F14] = GGK_F14;
        keys[SDLK_F15] = GGK_F15;
        // keys[SDLK_F16] = GGK_F16;
        // keys[SDLK_F17] = GGK_F17;
        // keys[SDLK_F18] = GGK_F18;
        // keys[SDLK_F19] = GGK_F19;
        // keys[SDLK_F20] = GGK_F20;
        // keys[SDLK_F21] = GGK_F21;
        // keys[SDLK_F22] = GGK_F22;
        // keys[SDLK_F23] = GGK_F23;
        // keys[SDLK_F24] = GGK_F24;
        // keys[SDLK_EXECUTE] = GGK_EXECUTE;
        keys[SDLK_HELP] = GGK_HELP;
        keys[SDLK_MENU] = GGK_MENU;
        // keys[SDLK_SELECT] = GGK_SELECT;
        // keys[SDLK_STOP] = GGK_STOP;
        // keys[SDLK_AGAIN] = GGK_AGAIN;
        keys[SDLK_UNDO] = GGK_UNDO;
        // keys[SDLK_CUT] = GGK_CUT;
        // keys[SDLK_COPY] = GGK_COPY;
        // keys[SDLK_PASTE] = GGK_PASTE;
        // keys[SDLK_FIND] = GGK_FIND;
        // keys[SDLK_MUTE] = GGK_MUTE;
        // keys[SDLK_VOLUMEUP] = GGK_VOLUMEUP;
        // keys[SDLK_VOLUMEDOWN] = GGK_VOLUMEDOWN;
        // keys[SDLK_KP_COMMA] = GGK_KP_COMMA;
        //keys[SDLK_KP_EQUALSAS400] = ?

        // keys[SDLK_ALTERASE] = GGK_ALTERASE;
        keys[SDLK_SYSREQ] = GGK_SYSREQ;
        // keys[SDLK_CANCEL] = GGK_CANCEL;
        keys[SDLK_CLEAR] = GGK_CLEAR;
        // keys[SDLK_PRIOR] = GGK_PRIOR;
        // keys[SDLK_RETURN2] = GGK_RETURN2;
        // keys[SDLK_SEPARATOR] = GGK_SEPARATOR;
        // keys[SDLK_OUT] = GGK_OUT;
        // keys[SDLK_OPER] = GGK_OPER;
        // keys[SDLK_CLEARAGAIN] = GGK_CLEARAGAIN;
        // keys[SDLK_CRSEL] = GGK_CRSEL;
        // keys[SDLK_EXSEL] = GGK_EXSEL;

        // keys[SDLK_KP_00] = GGK_KP_00;
        // keys[SDLK_KP_000] = GGK_KP_000;
        // SDLK_THOUSANDSSEPARATOR = ?
        // SDLK_DECIMALSEPARATOR = ?
        // keys[SDLK_CURRENCYUNIT] = GGK_CURRENCYUNIT;
        // SDLK_CURRENCYSUBUNIT = ?
        // keys[SDLK_KP_LEFTPAREN] = GGK_KP_LEFTPAREN;
        // keys[SDLK_KP_RIGHTPAREN] = GGK_KP_RIGHTPAREN;
        // keys[SDLK_KP_LEFTBRACE] = GGK_KP_LEFTBRACE;
        // keys[SDLK_KP_RIGHTBRACE] = GGK_KP_RIGHTBRACE;
        // keys[SDLK_KP_TAB] = GGK_KP_TAB;
        keys[SDLK_KP_BACKSPACE] = GGK_BACKSPACE; // Decreases fidelity
        keys[SDLK_KP_A] = GGK_A;// Decreases fidelity
        keys[SDLK_KP_B] = GGK_B;// Decreases fidelity
        keys[SDLK_KP_C] = GGK_C;// Decreases fidelity
        keys[SDLK_KP_D] = GGK_D;// Decreases fidelity
        keys[SDLK_KP_E] = GGK_E;// Decreases fidelity
        keys[SDLK_KP_F] = GGK_F;// Decreases fidelity
        // keys[SDLK_KP_XOR] = GGK_KP_XOR;
        // keys[SDLK_KP_POWER] = GGK_KP_POWER;
        // keys[SDLK_KP_PERCENT] = GGK_KP_PERCENT;
        // keys[SDLK_KP_LESS] = GGK_KP_LESS;
        // keys[SDLK_KP_GREATER] = GGK_KP_GREATER;
        // keys[SDLK_KP_AMPERSAND] = GGK_KP_AMPERSAND;
        // SDLK_KP_DBLAMPERSAND = ?
        // SDLK_KP_VERTICALBAR =
        // SDLK_KP_DBLVERTICALBAR =
        // keys[SDLK_KP_COLON] = GGK_KP_COLON;
        // keys[SDLK_KP_HASH] = GGK_KP_HASH;
        // keys[SDLK_KP_SPACE] = GGK_KP_SPACE;
        // keys[SDLK_KP_AT] = GGK_KP_AT;
        // keys[SDLK_KP_EXCLAM] = GGK_KP_EXCLAM;
        // keys[SDLK_KP_MEMSTORE] = GGK_KP_MEMSTORE;
        // keys[SDLK_KP_MEMRECALL] = GGK_KP_MEMRECALL;
        // keys[SDLK_KP_MEMCLEAR] = GGK_KP_MEMCLEAR;
        // keys[SDLK_KP_MEMADD] = GGK_KP_MEMADD;
        // SDLK_KP_MEMSUBTRACT = ?
        // SDLK_KP_MEMMULTIPLY = ?
        // keys[SDLK_KP_MEMDIVIDE] = GGK_KP_MEMDIVIDE;
        // keys[SDLK_KP_PLUSMINUS] = GGK_KP_PLUSMINUS;
        // keys[SDLK_KP_CLEAR] = GGK_KP_CLEAR;
        // keys[SDLK_KP_CLEARENTRY] = GGK_KP_CLEARENTRY;
        // keys[SDLK_KP_BINARY] = GGK_KP_BINARY;
        // keys[SDLK_KP_OCTAL] = GGK_KP_OCTAL;
        // keys[SDLK_KP_DECIMAL] = GGK_KP_DECIMAL;
        // SDLK_KP_HEXADECIMAL = ?

        keys[SDLK_LCTRL] = GGK_LCTRL;
        keys[SDLK_LSHIFT] = GGK_LSHIFT;
        keys[SDLK_LALT] = GGK_LALT;
        keys[SDLK_LGUI] = GGK_LSUPER;
        keys[SDLK_RCTRL] = GGK_RCTRL;
        keys[SDLK_RSHIFT] = GGK_RSHIFT;
        keys[SDLK_RALT] = GGK_RALT;
        keys[SDLK_RGUI] = GGK_RSUPER;

        keys[SDLK_MODE] = GGK_MODE;

        // keys[SDLK_AUDIONEXT] = GGK_AUDIONEXT;
        // keys[SDLK_AUDIOPREV] = GGK_AUDIOPREV;
        // keys[SDLK_AUDIOSTOP] = GGK_AUDIOSTOP;
        // keys[SDLK_AUDIOPLAY] = GGK_AUDIOPLAY;
        // keys[SDLK_AUDIOMUTE] = GGK_AUDIOMUTE;
        // keys[SDLK_MEDIASELECT] = GGK_MEDIASELECT;
        // keys[SDLK_WWW] = GGK_WWW;
        // keys[SDLK_MAIL] = GGK_MAIL;
        // keys[SDLK_CALCULATOR] = GGK_CALCULATOR;
        // keys[SDLK_COMPUTER] = GGK_COMPUTER;
        // keys[SDLK_AC_SEARCH] = GGK_AC_SEARCH;
        // keys[SDLK_AC_HOME] = GGK_AC_HOME;
        // keys[SDLK_AC_BACK] = GGK_AC_BACK;
        // keys[SDLK_AC_FORWARD] = GGK_AC_FORWARD;
        // keys[SDLK_AC_STOP] = GGK_AC_STOP;
        // keys[SDLK_AC_REFRESH] = GGK_AC_REFRESH;
        // keys[SDLK_AC_BOOKMARKS] = GGK_AC_BOOKMARKS;

        // keys[SDLK_BRIGHTNESSDOWN] = GGK_BRIGHTNESSDOWN;
        // keys[SDLK_BRIGHTNESSUP] = GGK_BRIGHTNESSUP;
        // keys[SDLK_DISPLAYSWITCH] = GGK_DISPLAYSWITCH;
        // SDLK_KBDILLUMTOGGLE = ?
        // keys[SDLK_KBDILLUMDOWN] = GGK_KBDILLUMDOWN;
        // keys[SDLK_KBDILLUMUP] = GGK_KBDILLUMUP;
        // keys[SDLK_EJECT] = GGK_EJECT;
        // keys[SDLK_SLEEP] = GGK_SLEEP;
    }

    Pt SetSDLFullscreenSize(SDL_Window* window, int display_id, int width, int height) {
        SDL_DisplayMode target;
        target.w = width;
        target.h = height;
        target.format = 0; // DOn't care
        target.driverdata = nullptr;
        target.refresh_rate = 0;
        SDL_DisplayMode closest;
        SDL_GetClosestDisplayMode(display_id, &target, &closest);
        SDL_SetWindowDisplayMode(window, &closest);
        return Pt(X(closest.w), Y(closest.h));
    }

    void Enter2DModeImpl(int width, int height) {
        glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport(0, 0, width, height);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        // This sets up the world coordinate space with the origin in the
        // upper-left corner and +x and +y directions right and down,
        // respectively.
        glOrtho(0.0, width, height, 0.0, -100.0, 100.0);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    struct QuitSignal {
        QuitSignal(int exit_code_) :
            exit_code(exit_code_)
        {}

        int exit_code;
    };

    class FramebufferFailedException : public std::exception {
    public:
        FramebufferFailedException(GLenum status):
            m_status(status)
        {}

        const char* what() const noexcept override {
            switch (m_status) {
                case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
                    return "The requested framebuffer format was unsupported";
                case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
                    return "One of the framebuffer attachments is incomplete.";
                default:
                    std::stringstream ss;
                    ss << "Framebuffer creation failed. Status: " << m_status;
                    return ss.str().c_str();
            }
        }
    private:
        GLenum m_status;
    };
}

namespace GG {
    class Framebuffer {
    public:
        /// Construct a framebuffer of dimensions \a size.
        /// \throws FramebufferFailedException if using framebuffers is not going to work.
        Framebuffer(GG::Pt size) :
            m_id(0),
            m_texture(0),
            m_depth_rbo(0)
        {
            int width = Value(size.x);
            int height = Value(size.y);

            // Create the texture to render the image on
            glGenTextures(1, &m_texture);
            glBindTexture(GL_TEXTURE_2D, m_texture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
            glBindTexture(GL_TEXTURE_2D, 0);

            // create a renderbuffer object to store depth and stencil info
            glGenRenderbuffersEXT(1, &m_depth_rbo);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depth_rbo);
            glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width, height);
            glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

            glGenFramebuffersEXT(1, &m_id);
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_id);

            // attach the texture to FBO color attachment point
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,        // 1. fbo target: GL_FRAMEBUFFER_EXT
                                      GL_COLOR_ATTACHMENT0_EXT,  // 2. attachment point
                                      GL_TEXTURE_2D,         // 3. tex target: GL_TEXTURE_2D
                                      m_texture,             // 4. tex ID
                                      0);                    // 5. mipmap level: 0(base)

            // attach the renderbuffer to depth attachment point
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,     // 1. fbo target: GL_FRAMEBUFFER_EXT
                                         GL_DEPTH_ATTACHMENT_EXT,
                                         GL_RENDERBUFFER_EXT,     // 3. rbo target: GL_RENDERBUFFER_EXT
                                         m_depth_rbo);              // 4. rbo ID

            // the same render buffer has the stencil data in other bits
            glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                      GL_STENCIL_ATTACHMENT_EXT,
                                      GL_RENDERBUFFER_EXT,
                                      m_depth_rbo);

            // check FBO status
            GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
            if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
                throw FramebufferFailedException (status);
            }

            // switch back to window-system-provided framebuffer
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
        }

        GLuint OpenGLId()
        { return m_id; }

        GLuint TextureId()
        { return m_texture; }

        ~Framebuffer() {
            glDeleteFramebuffersEXT(1, &m_id);
            glDeleteRenderbuffersEXT(1, &m_depth_rbo);
            glDeleteTextures(1, &m_texture);
        }

    private:
        GLuint m_id;
        GLuint m_texture;
        GLuint m_depth_rbo;
    };
}

// member functions
SDLGUI::SDLGUI(int w/* = 1024*/, int h/* = 768*/, bool calc_FPS/* = false*/, const std::string& app_name/* = "GG"*/,
               int x, int y, bool fullscreen, bool fake_mode_change) :
    GUI(app_name),
    m_app_width(w),
    m_app_height(h),
    m_initial_x(x),
    m_initial_y(y),
    m_fullscreen(fullscreen),
    m_fake_mode_change(fake_mode_change),
    m_display_id(0),
    m_window(nullptr),
    m_gl_context(nullptr),
    m_done(false),
    m_framebuffer(nullptr),
    m_key_map()
{
    SDLInit();
}

SDLGUI::~SDLGUI()
{ SDLQuit(); }

X SDLGUI::AppWidth() const
{ return m_app_width; }

Y SDLGUI::AppHeight() const
{ return m_app_height; }

unsigned int SDLGUI::Ticks() const
{ return SDL_GetTicks(); }

bool SDLGUI::Fullscreen() const
{ return m_fullscreen; }

bool SDLGUI::FakeModeChange() const
{ return m_fake_mode_change; }

std::string SDLGUI::ClipboardText() const {
    if (SDL_HasClipboardText()) {
        char* text = SDL_GetClipboardText();
        if (text) {
            std::string result(text);
            SDL_free(text);
            return result;
        }
    }

    return std::string();
}

void SDLGUI::operator()()
{ GUI::operator()(); }

void SDLGUI::ExitApp(int code)
{ throw QuitSignal(code); }

void SDLGUI::SetWindowTitle(const std::string& title)
{ SDL_SetWindowTitle(m_window, title.c_str()); }

void SDLGUI::SetVideoMode(X width, Y height, bool fullscreen, bool fake_mode_change)
{
    m_fullscreen = fullscreen;
    // Only allow fake mode change if the necessary extensions are supported
    m_fake_mode_change = fake_mode_change && FramebuffersAvailable();
    m_app_width = width;
    m_app_height = height;
    SDL_SetWindowFullscreen(m_window, 0);
    glViewport(0, 0, Value(width), Value(height));
    if (fullscreen) {
        if (!m_fake_mode_change) {
            Pt resulting_size = SetSDLFullscreenSize(m_window, m_display_id, Value(width), Value(height));
            m_app_width = resulting_size.x;
            m_app_height = resulting_size.y;
        }
        SDL_SetWindowFullscreen(m_window, m_fake_mode_change?SDL_WINDOW_FULLSCREEN_DESKTOP:SDL_WINDOW_FULLSCREEN);
    } else {
        SDL_SetWindowSize(m_window, Value(width), Value(height));
        SDL_RestoreWindow(m_window);
    }
    ResetFramebuffer();
}

bool SDLGUI::SetClipboardText(const std::string& text)
{ return SDL_SetClipboardText(text.c_str()) == 0; }

SDLGUI* SDLGUI::GetGUI()
{ return dynamic_cast<SDLGUI*>(GUI::GetGUI()); }

Key SDLGUI::GGKeyFromSDLKey(const SDL_Keysym& key)
{
    Key retval = GGK_UNKNOWN;
    if (m_key_map.count(key.sym))
        retval = m_key_map[key.sym];
    int shift = key.mod & KMOD_SHIFT;
    int caps_lock = key.mod & KMOD_CAPS;

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
    InitializeKeyMap(m_key_map);

    SDLMinimalInit();

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Create the window with a temporary size.
    // Use the same code for the real size initialization that is used for resizing the window
    // to avoid duplicated effort.
    m_window = SDL_CreateWindow(AppName().c_str(), Value(m_initial_x), Value(m_initial_y),
                                Value(m_app_width), Value(m_app_height), SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (m_window)
        m_gl_context = SDL_GL_CreateContext(m_window);
    GLenum glew_status = glewInit();

    if (!m_window || !m_gl_context || GLEW_OK != glew_status) {
        std::string msg;
        if (!m_window) {
            msg = "Unable to create window.";
            msg += "\n\nSDL reported:\n";
            msg += SDL_GetError();
        } else if (!m_gl_context) {
            msg = "Unable to create accelerated OpenGL 2.0 context.";
            msg += "\n\nSDL reported:\n";
            msg += SDL_GetError();
        } else {
            msg = "Unable to load OpenGL entry points.";
            msg += "\n\nGLEW reported:\n";
            msg += reinterpret_cast<const char*>(glewGetErrorString(glew_status));
        }

        SDL_ShowSimpleMessageBox(
            SDL_MESSAGEBOX_ERROR, "OpenGL initialization error", msg.c_str(), nullptr);
        std::cerr << msg << std::endl;
        ExitApp(1);
    }

    SDL_ShowWindow(m_window);

    SDL_ShowCursor(false);

    ResetFramebuffer();

    GLInit();

    // Now we can use the standard resizing call to make our window the size it belongs.
    SetVideoMode(m_app_width, m_app_height, m_fullscreen, m_fake_mode_change);

    SDL_EnableScreenSaver();
}

void SDLGUI::GLInit()
{
    double ratio = Value(m_app_width) * 1.0 / Value(m_app_height);

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
        std::uint32_t key_code_point = 0;
        Flags<ModKey> mod_keys = GetSDLModKeys();
        // In GiGi some events contain mouse position info,
        // where the corresponding sdl event does not.
        // Therefore we need to get the position,
        int mouse_x = 0;
        int mouse_y = 0;
        SDL_GetMouseState(&mouse_x, &mouse_y);
        Pt mouse_pos = Pt(X(mouse_x), Y(mouse_y));
        Pt mouse_rel(X(event.motion.xrel), Y(event.motion.yrel));

        switch (event.type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            key = GGKeyFromSDLKey(event.key.keysym);
            key_code_point = event.key.keysym.sym;
            if (key < GGK_NUMLOCK)
                send_to_gg = true;
            gg_event = (event.type == SDL_KEYDOWN) ? KEYPRESS : KEYRELEASE;
            break;

        case SDL_TEXTINPUT:
            RelayTextInput(event.text, mouse_pos);  // calls HandleGGEvent repeatedly to process
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

        case SDL_MOUSEWHEEL:
            send_to_gg = true;
            gg_event = MOUSEWHEEL;
            mouse_rel = Pt(X(event.wheel.x), Y(event.wheel.y));
            mod_keys = GetSDLModKeys();
            break;

        case SDL_WINDOWEVENT:
            send_to_gg = false;
            switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    // Alt-tabbing and other things give dubious resize events while in fullscreen mode.
                    // ignore them
                    if (!m_fullscreen) {
                        m_app_width = X(event.window.data1);
                        m_app_height = Y(event.window.data2);
                    }
                    // If faking resolution change, we need to listen to this event
                    // to size the buffer correctly for the screen.
                    if (m_fullscreen && m_fake_mode_change) {
                        ResetFramebuffer();
                    }
                case SDL_WINDOWEVENT_RESIZED:
                    // Alt-tabbing and other things give dubious resize events while in fullscreen mode.
                    // ignore them
                    if (!m_fullscreen) {
                        WindowResizedSignal(X(event.window.data1), Y(event.window.data2));
                    }
                    break;
                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    FocusChangedSignal(true);
                    break;
                case SDL_WINDOWEVENT_FOCUS_LOST:
                    FocusChangedSignal(false);
                    break;
                case SDL_WINDOWEVENT_MOVED:
                    WindowMovedSignal(X(event.window.data1), Y(event.window.data2));
                    break;
                case SDL_WINDOWEVENT_MINIMIZED:
                case SDL_WINDOWEVENT_MAXIMIZED:
                case SDL_WINDOWEVENT_RESTORED:
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    WindowClosingSignal();
                    break;
            }
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
        AppQuittingSignal();
        break;
    }
}

void SDLGUI::RenderBegin()
{
    if (m_fake_mode_change && m_fullscreen) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer->OpenGLId());
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, Value(m_app_width), Value(m_app_height));
}

void SDLGUI::RenderEnd()
{
    if (m_fake_mode_change && m_fullscreen) {
        // Return to rendering on the real screen
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        // Clear the real screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        int width, height;
        SDL_GetWindowSize(m_window, &width, &height);
        Enter2DModeImpl(width, height);
        // Disable blending, we want a direct copy
        glDisable(GL_BLEND);
        // Draw the virtual screen on the real screen
        glBindTexture(GL_TEXTURE_2D, m_framebuffer->TextureId());
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
            glTexCoord2f(0.0, 1.0);
            glVertex2i(0, 0);
            glTexCoord2f(1.0, 1.0);
            glVertex2i(width, 0);
            glTexCoord2f(1.0, 0.0);
            glVertex2i(width, height);
            glTexCoord2f(0.0, 0.0);
            glVertex2i(0, height);
        glEnd();
        glEnable(GL_BLEND);
        Exit2DMode();
    }
    SDL_GL_SwapWindow(m_window);
}

void SDLGUI::FinalCleanup()
{
    if (m_gl_context){
        SDL_GL_DeleteContext(m_gl_context);
        m_gl_context = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void SDLGUI::SDLQuit()
{
    // Ensure that the dektop resolution is restored on linux
    // By returning to windowed mode before exit.
    SetVideoMode(m_app_width, m_app_height, false, false);
    FinalCleanup();
    SDL_Quit();
}

void SDLGUI::Run()
{
    try {
        Initialize();
        ModalEventPump pump(m_done);
        pump();
    } catch (const QuitSignal& e) {
        if (e.exit_code != 0)
            throw;

        // This is the normal exit path from Run()
        // Do not put exit(0) or ExitApp(0) here.
        return;
    }
}

bool SDLGUI::AppHasMouseFocus() const {
    auto window_flags = SDL_GetWindowFlags(m_window);

    return window_flags & SDL_WINDOW_MOUSE_FOCUS;
}

std::vector<std::string> SDLGUI::GetSupportedResolutions() const
{
    std::vector<std::string> mode_vec;

    SDLMinimalInit();

    unsigned valid_mode_count = SDL_GetNumDisplayModes(m_display_id);

    /* Check if our resolution is restricted */
    if ( valid_mode_count < 1 ) {
        // This is bad.
    } else {
        for (unsigned i = 0; i < valid_mode_count; ++i) {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(m_display_id, i, &mode) != 0) {
                SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
            } else {
                mode_vec.push_back(boost::io::str(boost::format("%1% x %2%") % mode.w % mode.h));
            }
        }
    }

    return mode_vec;
}

void SDLGUI::SDLMinimalInit()
{
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }
    }
}

Pt SDLGUI::GetDefaultResolution(int display_id) const
{ return GetDefaultResolutionStatic(display_id); }

Pt SDLGUI::GetDefaultResolutionStatic(int display_id)
{
    SDLMinimalInit();

    if (display_id >= 0 && display_id < SDL_GetNumVideoDisplays()) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(display_id, &mode);
        Pt resolution(X(mode.w), Y(mode.h));
        return resolution;
    } else {
        return Pt(X0, Y0);
    }
}

int SDLGUI::NumVideoDisplaysStatic()
{
    SDLMinimalInit();
    return SDL_GetNumVideoDisplays();
}

int SDLGUI::MaximumPossibleDimension(bool is_width) {
    int dim = 0;

    int num_displays = NumVideoDisplaysStatic();
    for (int i_display = 0; i_display < num_displays; ++i_display) {
        SDL_Rect r;
        if (SDL_GetDisplayBounds(i_display, &r) == 0) {
            dim += is_width ? r.w : r.h;
        }
    }
    return dim;
}

int SDLGUI::MaximumPossibleWidth()
{ return MaximumPossibleDimension(true); }

int SDLGUI::MaximumPossibleHeight()
{ return MaximumPossibleDimension(false); }

void SDLGUI::RelayTextInput(const SDL_TextInputEvent& text, GG::Pt mouse_pos)
{
    const char *current = text.text;
    const char *last = current;
    // text is zero terminated, find the end
    while (*last)
    { ++last; }
    std::string text_string(current, last);

    // pass each utf-8 character as a separate event
    while (current != last) {
        HandleGGEvent(TEXTINPUT, GGK_UNKNOWN, utf8::next(current, last), Flags<ModKey>(),
                      mouse_pos, Pt(X0, Y0), &text_string);
    }
}

void SDLGUI::ResetFramebuffer()
{
    m_framebuffer.reset();
    if (m_fake_mode_change && m_fullscreen) {
        try {
            m_framebuffer.reset(new Framebuffer(Pt(m_app_width, m_app_height)));
        } catch (const FramebufferFailedException& ex) {
            std::cerr << "Fake resolution change failed. Reason: \"" << ex.what() << "\". Reverting to real resolution change." << std::endl;
            m_fake_mode_change = false;
        }
    }
}

void SDLGUI::Enter2DMode()
{ Enter2DModeImpl(Value(AppWidth()), Value(AppHeight())); }

void SDLGUI::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

bool SDLGUI::FramebuffersAvailable() const
{ return GLEW_EXT_framebuffer_object && GLEW_EXT_packed_depth_stencil; }
