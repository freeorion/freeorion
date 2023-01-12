/** \file SDLGUI.h \brief Contains SDLGUI, the input driver for using SDL with
    GG. */

#ifndef _SDLGUI_h_
#define _SDLGUI_h_

#include <GG/GUI.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_keyboard.h>


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
class SDLGUI : public GG::GUI
{
public:
    explicit SDLGUI(int w = 1024, int h = 768, bool calc_FPS = false, std::string app_name = "GG",
                    int x = SDL_WINDOWPOS_UNDEFINED, int y = SDL_WINDOWPOS_UNDEFINED, bool fullscreen = false,
                    bool fake_mode_change = false);

    virtual ~SDLGUI();

    GG::X AppWidth() const noexcept override { return m_app_width; }
    GG::Y AppHeight() const noexcept override { return m_app_height; }
    unsigned int Ticks() const override;
    std::string ClipboardText() const override;
    std::vector<std::string> GetSupportedResolutions() const override;
    GG::Pt GetDefaultResolution (int display_id) const override;

    bool Fullscreen() const noexcept { return m_fullscreen; }
    bool FakeModeChange() const noexcept { return m_fake_mode_change; }

    void ExitApp(int code = 0) override;
    bool SetClipboardText(std::string text) override;

    void Enter2DMode() override;
    void Exit2DMode() override;

    void HandleSystemEvents() override;

    void RenderBegin() override;
    void RenderEnd() override;

    void Run() override;

    bool AppHasMouseFocus() const override;

    void SetWindowTitle(const std::string& title);
    void SetVideoMode(GG::X width, GG::Y height, bool fullscreen, bool fake_mode_change);

    static SDLGUI*  GetGUI();                             ///< allows any code to access the gui framework by calling SDLGUI::GetGUI()

    static  GG::Pt  GetDefaultResolutionStatic(int display_id);
    static int      NumVideoDisplaysStatic();
    bool            FramebuffersAvailable() const;

    /** Returns the largest possible width if all displays are aligned horizontally.
        Ideally it reports actual desktop width using all displays.*/
    static int MaximumPossibleWidth();
    /** Returns the largest possible height if all displays are aligned vertically.
        Ideally it reports the actual desktop height using all displays.*/
    static int MaximumPossibleHeight();
protected:
    void SetAppSize(GG::Pt size);

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

    void ResetFramebuffer(); ///< Resizes or deletes the framebuffer for fake fullscreen.

    /** Given is_width = true (false) it returns the largest possible window
        width (height) if all displays are aligned horizontally (vertically).
        Ideally it returns the actual width (height) of a multi-monitor display.*/
    static int MaximumPossibleDimension(bool is_width = true);

private:
    void RelayTextInput (const SDL_TextInputEvent& text, GG::Pt mouse_pos);
    /** Bare minimum SDL video initialization to allow queries to display sizes etc.
        If called during static initialization, it will cause OSX to crash on exit. */
    static void SDLMinimalInit();

    GG::X           m_app_width{1024};      ///< application width and height (defaults to 1024 x 768)
    GG::Y           m_app_height{768};
    GG::X           m_initial_x{0};         ///< The initial position of the application window
    GG::Y           m_initial_y{0};
    int             m_display_id = 0;
    SDL_Window*     m_window = nullptr;     ///< The sdl window
    SDL_GLContext   m_gl_context = nullptr; ///< The OpenGL context
    bool            m_fullscreen = false;
    bool            m_fake_mode_change = false;
    bool            m_done = false;         ///< Set true true when we should exit.

    /** Virtual screen for fake fullscreen.  Equals nullptr ifi
        m_fake_mode_change == false. */
    std::unique_ptr<Framebuffer> m_framebuffer;
};

#endif
