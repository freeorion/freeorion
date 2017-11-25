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

/** \file SDLGUI.h \brief Contains SDLGUI, the input driver for using SDL with
    GG. */

#ifndef _GG_SDLGUI_h_
#define _GG_SDLGUI_h_

#include <GG/GUI.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>


#ifdef _MSC_VER
# ifdef GiGiSDL_EXPORTS
#  define GG_SDL_API __declspec(dllexport)
# else
#  define GG_SDL_API __declspec(dllimport)
# endif
#else
# define GG_SDL_API
#endif

namespace GG {

class Framebuffer;

/** \brief This is an abstract singleton class that represents the GUI
    framework of an SDL OpenGL application.

    <p>Usage:

    <br>Any application including an object of this class should declare that
    object as a local variable in main(). The name of this variable will
    herein be assumed to be "gui". It should be allocated on the stack; if it
    is created dynamically a leak may occur. SDLGUI is designed so the main()
    of the application can consist of just the one line "gui();".

    <p>To do this, the user needs only to override the Initialize() and
    FinalCleanup() methods, and ensure that the program does not terminate
    abnormally; this ensures FinalCleanup() is called when gui's destructor is
    invoked. ExitApp() can also perform cleanup and terminate the application
    cleanly.

    <p>Most of the member methods of SDLGUI have been declared virtual, to
    give the user great control when subclassing. The virtual function calls
    are usually not a performance issue, since none of the methods is called
    repeatedly, except HandleEvent(); if this is a problem, just create a new
    function in your subclass and call that from within Run() instead of
    HandleEvent(). Note that though the bulk of the program execution takes
    place within Run(), Run() itself is also only called once.

    <p>SDLGUI takes a two-tiered approach to event handling.  The event pump
    calls HandleSystemEvents(), which polls for SDL events and handles them by
    first determining whether the event is GG-related, or some other non-GG
    event, such as SDL_QUIT, etc.  GG events and non-GG events are passed to
    HandleGGEvent() and HandleNonGGEvent(), respectively.  For most uses,
    there should be no need to override the behavior of HandleSDLEvents().
    However, the HandleNonGGEvent() default implementation only responds to
    SDL_QUIT events, and so should be overridden in most cases. */
class GG_SDL_API SDLGUI : public GG::GUI
{
public:
    /** \name Structors */ ///@{
    explicit SDLGUI(int w = 1024, int h = 768, bool calc_FPS = false, const std::string& app_name = "GG",
                    int x = SDL_WINDOWPOS_UNDEFINED, int y = SDL_WINDOWPOS_UNDEFINED, bool fullscreen = false,
                    bool fake_mode_change = false);

    virtual ~SDLGUI();
    //@}

    /** \name Accessors */ ///@{
    X AppWidth() const override;
    Y AppHeight() const override;
    unsigned int Ticks() const override;
    std::string ClipboardText() const override;
    virtual std::vector<std::string> GetSupportedResolutions() const override;
    virtual Pt GetDefaultResolution (int display_id) const override;

    virtual bool Fullscreen() const final;
    virtual bool FakeModeChange() const final;
    //@}

    /** \name Mutators */ ///@{
    void ExitApp(int code = 0) override;
    bool SetClipboardText(const std::string& text) override;

    void Enter2DMode() override;
    void Exit2DMode() override;

    void HandleSystemEvents() override;

    void RenderBegin() override;
    void RenderEnd() override;

    void Run() override;

    bool AppHasMouseFocus() const override;

    void            operator()();      ///< external interface to Run()

    void            SetWindowTitle(const std::string& title);
    void            SetVideoMode(X width, Y height, bool fullscreen, bool fake_mode_change);
    //@}

    static SDLGUI*  GetGUI();                             ///< allows any code to access the gui framework by calling SDLGUI::GetGUI()
    GG::Key         GGKeyFromSDLKey(const SDL_Keysym& key); ///< gives the GGKey equivalent of key

    static  Pt      GetDefaultResolutionStatic(int display_id);
    static int      NumVideoDisplaysStatic();
    bool            FramebuffersAvailable() const;

    /** Returns the largest possible width if all displays are aligned horizontally.
        Ideally it reports actual desktop width using all displays.*/
    static int MaximumPossibleWidth();
    /** Returns the largest possible height if all displays are aligned vertically.
        Ideally it reports the actual desktop height using all displays.*/
    static int MaximumPossibleHeight();
protected:
    void SetAppSize(const GG::Pt& size);

    // these are called at the beginning of the gui's execution
    /** Initializes SDL, FE, and SDL OpenGL functionality. */
    void SDLInit();

    /** Allows user to specify OpenGL initialization code;
        called at the end of SDLInit(). */
    void GLInit();

    /** Provides one-time gui initialization. */
    virtual void Initialize() = 0;

    /** event handler for all SDL events that are not GG-related. */
    void HandleNonGGEvent(const SDL_Event& event);

    // these are called at the end of the gui's execution
    /** Provides one-time gui cleanup. */
    void FinalCleanup();

    /** Cleans up SDL and (if used) FE. */
    void SDLQuit();

    void            ResetFramebuffer(); ///< Resizes or deletes the framebuffer for fake fullscreen.

    /** Given is_width = true (false) it returns the largest possible window
        width (height) if all displays are aligned horizontally (vertically).
        Ideally it returns the actual width (height) of a multi-monitor display.*/
    static int MaximumPossibleDimension(bool is_width = true);

private:
    void            RelayTextInput (const SDL_TextInputEvent& text, Pt mouse_pos);
    /** Bare minimum SDL video initialization to allow queries to display sizes etc.
        If called during static initialization, it will cause OSX to crash on exit. */
    static void     SDLMinimalInit();

    X         m_app_width;      ///< application width and height (defaults to 1024 x 768)
    Y         m_app_height;
    X         m_initial_x; ///< The initial position of the application window
    Y         m_initial_y;
    bool      m_fullscreen;
    bool      m_fake_mode_change;
    int       m_display_id;
    SDL_Window*     m_window; ///< The sdl window
    SDL_GLContext   m_gl_context; //< The OpenGL context
    bool            m_done; //< Set true true when we should exit.

    /** Virtual screen for fake fullscreen.  Equals nullptr ifi
        m_fake_mode_change == false. */
    std::unique_ptr<Framebuffer> m_framebuffer;

    std::map<SDL_Keycode, Key>      m_key_map; //< a mapping from sdl keycodes to GiGi keys
};

} // namespace GG

#endif

