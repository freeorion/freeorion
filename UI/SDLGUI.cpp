#include "SDLGUI.h"

#include <GG/WndEvent.h>
#include <GG/utf8/checked.h>

#include <cctype>
#include <iostream>

#include <boost/format.hpp>

using namespace GG;

namespace {
    Flags<ModKey> GetSDLModKeys() {
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

    Pt SetSDLFullscreenSize(SDL_Window* window, int display_id, int width, int height) {
        SDL_DisplayMode target{};
        target.w = width;
        target.h = height;
        target.format = 0; // DOn't care
        target.driverdata = nullptr;
        target.refresh_rate = 0;
        SDL_DisplayMode closest{};
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
                case GL_FRAMEBUFFER_UNDEFINED:
                    return "The default framebuffer does not exist.";
                case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
                    return "At least one picture must be attached to the framebuffer.";
                case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                    return "All populated color attachments are not from textures of the same target.";
                default:
                    return "Framebuffer creation failed with an unhandled exception.";
            }
        }
    private:
        GLenum m_status;
    };
}

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

// member functions
SDLGUI::SDLGUI(int w, int h, bool calc_FPS, std::string app_name, int x, int y,
               bool fullscreen, bool fake_mode_change) :
    GUI(std::move(app_name)),
    m_app_width{w},
    m_app_height{h},
    m_initial_x{x},
    m_initial_y{y},
    m_fullscreen(fullscreen),
    m_fake_mode_change(fake_mode_change)
{
    SDLInit();
}

SDLGUI::~SDLGUI()
{ SDLQuit(); }

unsigned int SDLGUI::Ticks() const
{ return SDL_GetTicks(); }

std::string SDLGUI::ClipboardText() const {
    if (SDL_HasClipboardText()) {
        char* text = SDL_GetClipboardText();
        if (text) {
            std::string result{text};
            SDL_free(text);
            return result;
        }
    }

    return std::string{};
}

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

bool SDLGUI::SetClipboardText(std::string text)
{ return SDL_SetClipboardText(text.c_str()) == 0; }

SDLGUI* SDLGUI::GetGUI()
{ return dynamic_cast<SDLGUI*>(GUI::GetGUI()); }

void SDLGUI::SetAppSize(Pt size) {
    m_app_width = size.x;
    m_app_height = size.y;
}

void SDLGUI::SDLInit() {
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

    const char* glew_error_string = reinterpret_cast<const char*>(glewGetErrorString(glew_status));
    if (!m_window || !m_gl_context || ((GLEW_OK != glew_status) && (strcmp(glew_error_string, "Unknown Error")==0))) {
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
    if (GLEW_OK != glew_status) {
        std::cerr << "[error] Ignored GLEW error when setting up OpenGL: " << glew_error_string << std::endl;
    }

    SDL_ShowWindow(m_window);

    SDL_ShowCursor(false);

    ResetFramebuffer();

    GLInit();

    // Now we can use the standard resizing call to make our window the size it belongs.
    SetVideoMode(m_app_width, m_app_height, m_fullscreen, m_fake_mode_change);

    SDL_EnableScreenSaver();
}

void SDLGUI::GLInit() {
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

    // set up perspective with vertical FOV of 50°. 1:1 application
    // window ratio, near plane of 1.0 and far plane of 10.0
    const float ratio = Value(m_app_width) * 1.0f / Value(m_app_height);

    static constexpr float cot = 0.839099631177f; // radians = 50.0f * pi / 180.0f; cot = cos(radians) / sin(radians);
    const float cor = cot / ratio;
    static constexpr float near = 1.0f;
    static constexpr float far = 10.0f;
    static constexpr float fpnonmn = -((far + near) / (far - near));
    static constexpr float ttftnofmn = -((2.0f * far * near) / (far - near));
    const std::array<std::array<float, 4>, 4> projection{{
        { cor, 0.0f,      0.0f,  0.0f},
        {0.0f, cot,       0.0f,  0.0f},
        {0.0f, 0.0f,   fpnonmn, -1.0f},
        {0.0f, cot,  ttftnofmn,  0.0f}}};

    glMultMatrixf(projection.front().data());
}

void SDLGUI::HandleSystemEvents() {
    // handle events
    SDL_Event event;
    while (0 < SDL_PollEvent(&event)) {
        bool send_to_gg = false;
        EventType gg_event = EventType::MOUSEMOVE;
        Key key = Key::GGK_NONE;
        uint32_t key_code_point = 0;
        GG::Flags<GG::ModKey> mod_keys = GetSDLModKeys();
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
            send_to_gg = true;
            key = static_cast<Key>(event.key.keysym.scancode);
            key_code_point = event.key.keysym.sym;
            gg_event = (event.type == SDL_KEYDOWN) ? EventType::KEYPRESS : EventType::KEYRELEASE;
            break;

        case SDL_TEXTINPUT:
            RelayTextInput(event.text, mouse_pos);  // calls HandleGGEvent repeatedly to process
            break;

        case SDL_MOUSEMOTION:
            send_to_gg = true;
            gg_event = EventType::MOUSEMOVE;
            break;

        case SDL_MOUSEBUTTONDOWN:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:   gg_event = EventType::LPRESS; break;
                case SDL_BUTTON_MIDDLE: gg_event = EventType::MPRESS; break;
                case SDL_BUTTON_RIGHT:  gg_event = EventType::RPRESS; break;
            }
            mod_keys = GetSDLModKeys();
            break;

        case SDL_MOUSEBUTTONUP:
            send_to_gg = true;
            switch (event.button.button) {
                case SDL_BUTTON_LEFT:   gg_event = EventType::LRELEASE; break;
                case SDL_BUTTON_MIDDLE: gg_event = EventType::MRELEASE; break;
                case SDL_BUTTON_RIGHT:  gg_event = EventType::RRELEASE; break;
            }
            mod_keys = GetSDLModKeys();
            break;

        case SDL_MOUSEWHEEL:
            send_to_gg = true;
            gg_event = EventType::MOUSEWHEEL;
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
                    if (m_fullscreen && m_fake_mode_change)
                        ResetFramebuffer();
                    [[fallthrough]]; // not sure if this is inteded to be a fall through to the next case, but it seems plausible...
                case SDL_WINDOWEVENT_RESIZED:
                    // Alt-tabbing and other things give dubious resize events while in fullscreen mode.
                    // ignore them
                    if (!m_fullscreen)
                        WindowResizedSignal(X(event.window.data1), Y(event.window.data2));
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
            HandleGGEvent(gg_event, key, key_code_point, mod_keys, mouse_pos, mouse_rel, std::string());
        else
            HandleNonGGEvent(event);
    }
}

void SDLGUI::HandleNonGGEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_QUIT:
        AppQuittingSignal();
        break;
    }
}

void SDLGUI::RenderBegin() {
    if (m_fake_mode_change && m_fullscreen) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_framebuffer->OpenGLId());
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, Value(m_app_width), Value(m_app_height));
}

void SDLGUI::RenderEnd() {
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

void SDLGUI::FinalCleanup() {
    if (m_gl_context){
        SDL_GL_DeleteContext(m_gl_context);
        m_gl_context = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void SDLGUI::SDLQuit() {
    // Ensure that the dektop resolution is restored on linux
    // By returning to windowed mode before exit.
    SetVideoMode(m_app_width, m_app_height, false, false);
    FinalCleanup();
    SDL_Quit();
}

void SDLGUI::Run() {
    try {
        Initialize();
        RunModal(m_done);
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

std::vector<std::string> SDLGUI::GetSupportedResolutions() const {
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

void SDLGUI::SDLMinimalInit() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            throw std::runtime_error("Failed to initialize SDL");
        }
    }
}

Pt SDLGUI::GetDefaultResolution(int display_id) const
{ return GetDefaultResolutionStatic(display_id); }

Pt SDLGUI::GetDefaultResolutionStatic(int display_id) {
    SDLMinimalInit();

    if (display_id >= 0 && display_id < SDL_GetNumVideoDisplays()) {
        SDL_DisplayMode mode;
        SDL_GetDesktopDisplayMode(display_id, &mode);
        Pt resolution(X(mode.w), Y(mode.h));
        return resolution;
    } else {
        return Pt0;
    }
}

int SDLGUI::NumVideoDisplaysStatic() {
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

void SDLGUI::RelayTextInput(const SDL_TextInputEvent& text, GG::Pt mouse_pos) {
    const char* current = text.text;
    const char* end = current + SDL_TEXTEDITINGEVENT_TEXT_SIZE;
    while (current != end && *current)
        ++current;
    HandleGGEvent(EventType::TEXTINPUT, Key::GGK_NONE, 0u, GG::Flags<GG::ModKey>(), mouse_pos, Pt0,
                  std::string(text.text, current));
}

void SDLGUI::ResetFramebuffer() {
    m_framebuffer.reset();
    if (m_fake_mode_change && m_fullscreen) {
        try {
            m_framebuffer = std::make_unique<Framebuffer>(Pt(m_app_width, m_app_height));
        } catch (const FramebufferFailedException& ex) {
            std::cerr << "Fake resolution change failed. Reason: \"" << ex.what() << "\". Reverting to real resolution change." << std::endl;
            m_fake_mode_change = false;
        }
    }
}

void SDLGUI::Enter2DMode()
{ Enter2DModeImpl(Value(AppWidth()), Value(AppHeight())); }

void SDLGUI::Exit2DMode() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

bool SDLGUI::FramebuffersAvailable() const
{ return GLEW_EXT_framebuffer_object && GLEW_EXT_packed_depth_stencil; }
