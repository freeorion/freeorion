#include <GG/StyleFactory.h>
#include <GG/SDL/SDLGUI.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/dialogs/FileDlg.h>

#include <iostream>

// Tutorial 1: Minimal

// This contains the minimal interesting GG application.  It contains 3D as
// well as GUI elements in the same scene, and demonstrates how to use the
// default SDL input driver, SDLGUI.


// This is the minimal interface (or nearly so) to SDLGUI required to produce
// a functional GG-over-SDL application.  Note that GG does not require SDL,
// and even when using SDL, it does not require you to use SDLGUI.  However,
// using SDLGUI as an application framework makes using GG extremely easy, as
// you can see.
class MinimalGGApp : public GG::SDLGUI
{
public:
    MinimalGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

// The constructor must call SDLGUI with the desired application width and
// height, a flag indicating whether FPS statistics should be generated, amd
// the application's name.  Note that SDLGUIs run in windowed mode by default,
// and note that the application name does not appear in the title bar, but
// must be set separately.
MinimalGGApp::MinimalGGApp() : 
    SDLGUI(1024, 768, false, "Minimal GG App")
{
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
void MinimalGGApp::Render()
{
    const double RPM = 4;
    const double DEGREES_PER_MS = 360.0 * RPM / 60000.0;

    // DeltaT() returns the time in whole milliseconds since the last frame
    // was rendered (in other words, since this method was last invoked).
    glRotated(DeltaT() * DEGREES_PER_MS, 0.0, 1.0, 0.0);

    glBegin(GL_QUADS);

    glColor3d(0.0, 1.0, 0.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(1.0, 1.0, 1.0);

    glColor3d(1.0, 0.5, 0.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(-1.0, -1.0,-1.0);
    glVertex3d(1.0, -1.0,-1.0);

    glColor3d(1.0, 0.0, 0.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);

    glColor3d(1.0, 1.0, 0.0);
    glVertex3d(1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, -1.0);

    glColor3d(0.0, 0.0, 1.0);
    glVertex3d(-1.0, 1.0, 1.0);
    glVertex3d(-1.0, 1.0, -1.0);
    glVertex3d(-1.0, -1.0, -1.0);
    glVertex3d(-1.0, -1.0, 1.0);

    glColor3d(1.0, 0.0, 1.0);
    glVertex3d(1.0, 1.0, -1.0);
    glVertex3d(1.0, 1.0, 1.0);
    glVertex3d(1.0, -1.0, 1.0);
    glVertex3d(1.0, -1.0, -1.0);

    glEnd();

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

// This is the launch point for your GG app.  This is where you should place
// your main GG::Wnd(s) that should appear when the application starts, if
// any.
void MinimalGGApp::Initialize()
{
    // This sets the caption seen in the application's window, when operating
    // in windowed mode.  Note that this is not related in any way to the app
    // name set previously.  Typically this goes in SDLInit(), but since that
    // was otherwise exactly what we want, I placed it here instead of copying
    // the entire method here again just to add one function call.
    SDL_WM_SetCaption("Minimal SDL GG App", "Minimal SDL GG App");

    // Create a modal dialog and execute it.  This will show GG operating on
    // top of a "real 3D" scene.  Note that if you want "real" 3D objects
    // (i.e. drawn in a non-orthographic space) inside of GG windows, you can
    // add whatever OpenGL calls you like to a GG::Wnd's Render() method,
    // sandwiched between Exit2DMode() and Enter2DMode().

    const std::string message = "Are we Готово yet?"; // That Russian word means "Done", ha.
    const std::set<GG::UnicodeCharset> charsets_ = GG::UnicodeCharsetsToRender(message);
    const std::vector<GG::UnicodeCharset> charsets(charsets_.begin(), charsets_.end());

    const boost::shared_ptr<GG::Font> font =
        GetStyleFactory()->DefaultFont(12, &charsets[0], &charsets[0] + charsets.size());

    GG::Wnd* quit_dlg =
        new GG::ThreeButtonDlg(GG::X(200), GG::Y(100), message, font, GG::CLR_SHADOW, 
                               GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);
    quit_dlg->Run();

    // Now that we're back from the modal dialog, we can exit normally, since
    // that's what closing the dialog indicates.  Exit() calls all the cleanup
    // methods for GG::SDLGUI.
    Exit(0);
}

// This gets called as the application is exit()ing, and as the name says,
// performs all necessary cleanup at the end of the app's run.
void MinimalGGApp::FinalCleanup()
{
}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    MinimalGGApp app;

    // The try-catch block is not strictly necessary, but it sure helps to see
    // what exception crashed your app in the log file.
    try {
        // This, however, is necessary.  This executes the GG event loop until the app is terminated.
        app();
    } catch (const std::invalid_argument& e) {
        std::cerr << "main() caught exception(std::invalid_arg): " << e.what();
    } catch (const std::runtime_error& e) {
        std::cerr << "main() caught exception(std::runtime_error): " << e.what();
    } catch (const std::exception& e) {
        std::cerr << "main() caught exception(std::exception): " << e.what();
    }
    return 0;
}
