#include "Application.h"

#include <GG/Texture.h>
#include <GG/Cursor.h>

#include "../../../UI/SDLGUI.h"

#include <boost/format.hpp>


extern const unsigned char cursor_data[16 * 16 * 4];

class Application::Impl {
    public:
        Impl(int argc, char** argv, unsigned width, unsigned height);

        virtual ~Impl();

        /// The given window will be made visible.
        /// Then the event pump is started.
        /// This method only returns once the application quits
        void Run(std::shared_ptr<GG::Wnd> wnd);

    private:
        std::unique_ptr<class MinimalGGApp> m_app;
};


class MinimalGGApp : public SDLGUI {
    public:
        MinimalGGApp(int width, int height, bool calculate_FPS, std::string name,
                     int x, int y, bool fullscreen, bool fake_mode_change);

        virtual ~MinimalGGApp();

        void Enter2DMode() override;
        void Exit2DMode() override;
        virtual void GLInit();

        void Initialize() override
        {}

    protected:
        void Render() override;
};

MinimalGGApp::MinimalGGApp(int width, int height, bool calculate_FPS, std::string name,
                           int x, int y, bool fullscreen, bool fake_mode_change) :
    SDLGUI(width, height, calculate_FPS, std::move(name), x, y, fullscreen, fake_mode_change)
{
    auto cursor_texture = std::make_shared<GG::Texture>();

    cursor_texture->Init(GG::X(16), GG::Y(16), cursor_data, GL_RGBA, GL_UNSIGNED_BYTE, 1);

    GG::GetTextureManager().StoreTexture(cursor_texture, "test_cursor");
    SetCursor(std::make_unique<GG::TextureCursor>(std::move(cursor_texture), GG::Pt(GG::X(1), GG::Y(1))));
    RenderCursor(true);

    GLInit();
}

MinimalGGApp::~MinimalGGApp() = default;

// The application is assumed to be in "3D mode" (non-orthographic) whenever
// GG is not rendering "in 2D" (orthographic, with coordinates mapped 1-1 onto
// the pixels of the display).  To move into "2D" mode, GG calls Enter2DMode()
// at the beginning of each GG rendering iteration.  This method is
// responsible for preserving any state it alters, then altering the necessary
// state to match what GG requires.  GG requires GL_TEXTURE_2D to be on,
// GL_LIGHTING off, and requires a viewport and orthographic projection as
// shown below.  The other code you see here is usually convenient to working
// with GG, but not required.
void MinimalGGApp::Enter2DMode() {
    glPushAttrib(GL_ENABLE_BIT | GL_PIXEL_MODE_BIT | GL_TEXTURE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    // This sets up the world coordinate space with the origin in the
    // upper-left corner and +x and +y directions right and down,
    // respectively.  Note that this call leaves the depth of the viewing
    // volume is only 1 (from 0.0 to 1.0), which is fine for GG's purposes.
    glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    //glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// At the end of a single iteration of the GG rendering cycle, GG attempts to
// return GL state to what it was before the call to Enter2DMode() at the
// beginning of the iteration.  That is accomplished by calling Exit2DMode().
// Note that Enter- and Exit2DMode() depend entirely on the details of your
// application; there is no default implementation for either of these methods
// in SDLGUI.
void MinimalGGApp::Exit2DMode() {
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}


// This gets called once per frame, and should call GG::GUI::Render() at its
// end.  Before this call to GG::GUI::Render(), all "3D" (non-GG,
// non-orthographic) rendering should be done.  If you don't plan on doing
// anything but using GG's windows and controls, you won't need to override
// this method at all.  Note that GG::GUI::Render() calls Enter2DMode() at tis
// beginning and Exit2DMode() at its end.
void MinimalGGApp::Render() {
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

    glPushMatrix();

    glRotated ( (Ticks() % 60000) * DEGREES_PER_MS, 0.0, 1.0, 0.0);

    glBegin (GL_LINES);

    glColor3d (0.0, 0.5, 0.0);

    glVertex3d (1.0, 1.0, 1.0);
    glVertex3d (1.0, -1.0, 1.0);

    glVertex3d (1.0, -1.0, 1.0);
    glVertex3d (1.0, -1.0, -1.0);

    glVertex3d (1.0, -1.0, -1.0);
    glVertex3d (1.0, 1.0, -1.0);

    glVertex3d (1.0, 1.0, -1.0);
    glVertex3d (1.0, 1.0, 1.0);

    glVertex3d (1.0, 1.0, 1.0);
    glVertex3d (-1.0, 1.0, 1.0);

    glVertex3d (1.0, -1.0, 1.0);
    glVertex3d (-1.0, -1.0, 1.0);

    glVertex3d (1.0, -1.0, -1.0);
    glVertex3d (-1.0, -1.0, -1.0);

    glVertex3d (1.0, 1.0, -1.0);
    glVertex3d (-1.0, 1.0, -1.0);

    glEnd();

    glPopMatrix();

    GG::GUI::Render();
}

// This is where you put any OpenGL initialization code you need to execute at
// the start of the application.  This should set up the initial "3D"
// environment, the one that exists before Enter2DMode() and after
// Exit2DMode().  As with Enter- and Exit2DMode(), the needed code is entirely
// application-dependent.  Note that this method is called before Render() is
// ever called.
void MinimalGGApp::GLInit() {
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // set up perspective with vertical FOV of 50°. 1:1 application
    // window ratio, near plane of 1.0 and far plane of 10.0
    float ratio = AppWidth() / static_cast<float>(Value(AppHeight()));
    float radians = 50.0f * M_PI / 180.f;
    float near = 1.0f;
    float far = 10.0f;
    float cotangent = std::cos(radians) / std::sin(radians);

    float projection[4][4] = {{0.0f}};
    projection[0][0] = cotangent / ratio;
    projection[1][1] = cotangent;
    projection[2][2] = -((far + near) / (far - near));
    projection[2][3] = -1.0f;
    projection[3][2] = -((2.0f * far * near) / (far - near));
    projection[3][3] = 0.0f;

    glMultMatrixf(&projection[0][0]);

    // set up camera in -5.0 z offset from origin looking at origin
    GLfloat view[4][4] = {{0.0}};
    view[0][0] = -1.0;
    view[1][1] = -1.0;
    view[2][2] = 1.0;
    view[3][3] = 1.0;

    glMultMatrixf(&view[0][0]);
    glTranslated(-0.0, -0.0, -5.0);
    glMatrixMode(GL_MODELVIEW);
}


Application::Application(int argc, char** argv, unsigned width, unsigned height)
{ self = std::make_unique<Application::Impl>(argc, argv, width, height); }

Application::~Application() = default;

void Application::Run(std::shared_ptr<GG::Wnd> wnd)
{ self->Run(std::forward<std::shared_ptr<GG::Wnd>>(wnd)); }

Application::Impl::Impl(int argc, char** argv, unsigned width, unsigned height) {
    //std::vector<std::string> args;
    //for (int i = 0; i < argc; ++i)
    //    args.push_back(argv[i]);

    try {
        bool fullscreen = false;
        int left = 200;
        int top = 100;

        m_app = std::make_unique<MinimalGGApp>(800, 600, true, "Test", left, top, fullscreen, false);

    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "main() caught unknown exception." << std::endl;
    }
}

Application::Impl::~Impl() = default;

void Application::Impl::Run(std::shared_ptr<GG::Wnd> window) {
    try {

        m_app->Register(std::forward<std::shared_ptr<GG::Wnd>>(window));
        m_app->Run();

    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what() << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what() << std::endl;
    } catch (const  boost::io::format_error& e) {
        std::cerr << "main() caught exception(boost::io::format_error): " << e.what() << std::endl;
    } catch (const GG::ExceptionBase& e) {
        std::cerr << "main() caught exception(" << e.type() << "): " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "main() caught unknown exception." << std::endl;
    }
}
