#include "TestApp.h"
#include "TestApp_p.h"

#include <GG/StyleFactory.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/Texture.h>
#include <GG/Cursor.h>

#include <GG/SDL/SDLGUI.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

class MinimalGGApp : public GG::SDLGUI {
    public:
        MinimalGGApp (int width, int height, bool calculate_FPS, const std::string& name, int x, int y, bool fullscreen, bool fake_mode_change);

        virtual ~MinimalGGApp();

        virtual void Enter2DMode() override;
        virtual void Exit2DMode() override;
        virtual void GLInit() override;

        virtual void Initialize(){}

    protected:
        virtual void Render();
};

MinimalGGApp::MinimalGGApp (int width, int height, bool calculate_FPS, const std::string& name, int x, int y, bool fullscreen, bool fake_mode_change) :
    SDLGUI (width, height, calculate_FPS, name, x, y, fullscreen, fake_mode_change)
{
    boost::shared_ptr<GG::Texture> cursor_texture = GG::GetTextureManager().GetTexture(
        (boost::filesystem::path("../../default") / "data" / "art" / "cursors" / "default_cursor.png").string(), false);
    SetCursor(boost::shared_ptr<GG::TextureCursor>(new GG::TextureCursor(cursor_texture, GG::Pt(GG::X(6), GG::Y(3)))));
    RenderCursor(true);

    GLInit();
}

MinimalGGApp::~MinimalGGApp() {

}

// The application is assumed to be in "3D mode" (non-orthographic) whenever
// GG is not rendering "in 2D" (orthographic, with coordinates mapped 1-1 onto
// the pixels of the display).  To move into "2D" mode, GG calls Enter2DMode()
// at the beginning of each GG rendering iteration.  This method is
// responsible for preserving any state it alters, then altering the necessary
// state to match what GG requires.  GG requires GL_TEXTURE_2D to be on,
// GL_LIGHTING off, and requires a viewport and orthographic projection as
// shown below.  The other code you see here is usually convenient to working
// with GG, but not required.
void MinimalGGApp::Enter2DMode()
{
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
void MinimalGGApp::Exit2DMode()
{
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

    // DeltaT() returns the time in whole milliseconds since the last frame
    // was rendered (in other words, since this method was last invoked).
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
void MinimalGGApp::GLInit()
{
    double ratio = Value(AppWidth() * 1.0) / Value(AppHeight());

    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0, 0, 0, 0);
    glViewport(0, 0, Value(AppWidth()), Value(AppHeight()));
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, ratio, 1.0, 10.0);
    gluLookAt(0.0, 0.0, 5.0,
              0.0, 0.0, 0.0,
              0.0, 1.0, 0.0);
    glMatrixMode(GL_MODELVIEW);
}


TestApp::TestApp (int argc, char** argv, unsigned width, unsigned height) :
    self (new TestAppImpl (this, argc, argv, width, height)) {

}

void TestApp::Run (GG::Wnd* wnd) {
    self->Run(wnd);
}


TestAppImpl::TestAppImpl (TestApp* q, int argc, char** argv, unsigned width, unsigned height) :
    m_front (q), m_app(NULL){

        std::vector<std::string> args;
        for (int i = 0; i < argc; ++i) {
            args.push_back(argv[i]);
        }

        try {

            bool fullscreen = false;
            std::pair<int, int> left_top(200, 100);
            int left(left_top.first), top(left_top.second);

            m_app = new MinimalGGApp(800, 600, true, "Test", left, top, fullscreen, false);

    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "main() caught unknown exception." << std::endl;
    }
}

TestAppImpl::~TestAppImpl() {
    delete m_app;
}

void TestAppImpl::Run (GG::Wnd* window) {
    try {

        m_app->Register(window);
        (*m_app)();

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
