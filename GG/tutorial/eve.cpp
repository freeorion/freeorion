#include <GG/StyleFactory.h>
#include <GG/EveGlue.h>
#include <GG/SDL/SDLGUI.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/dialogs/FileDlg.h>
#include <GG/adobe/name.hpp>
#include <GG/adobe/any_regular.hpp>

#include <iostream>


class EveGGApp : public GG::SDLGUI
{
public:
    EveGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

EveGGApp::EveGGApp() : 
    SDLGUI(1024, 768, false, "Eve GG App")
{}

void EveGGApp::Enter2DMode()
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

    glOrtho(0.0, Value(AppWidth()), Value(AppHeight()), 0.0, 0.0, Value(AppWidth()));

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void EveGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void EveGGApp::Render()
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

void EveGGApp::GLInit()
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

bool OkHandler(adobe::name_t name, const adobe::any_regular_t&)
{ return name == adobe::name_t("ok"); }

void EveGGApp::Initialize()
{
    SDL_WM_SetCaption("Eve SDL GG App", "Eve SDL GG App");

    std::istringstream eve(
"layout alert_dialog\n"
"{\n"
"    view dialog(name: 'Alert', placement: place_row)\n"
"    {\n"
"        image(image: 'stop.png');     \n"
"\n"
"        column(vertical: align_fill)\n"
"        {\n"
"            static_text(name: 'Unfortunately, something drastic has happened. If you would like we can try to continue with the operation, but there is a chance you will blow up your computer. Would you like to try?', characters: 25);\n"
"            row(vertical: align_bottom, horizontal: align_right)\n"
"            {\n"
"                button(name: 'Cancel', action: @cancel, cancel: true);\n"
"                button(name: 'OK', bind: @result, action: @ok, default: true);\n"
"            }\n"
"        }\n"
"    }\n"
"}");

    std::istringstream adam(
"sheet alert_dialog\n"
"{\n"
"output:\n"
"    result <== { dummy_value: 42 };\n"
"}");

    GG::ExecuteModalDialog(eve, adam, &OkHandler);

    Exit(0);
}

void EveGGApp::FinalCleanup()
{}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    EveGGApp app;

    try {
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
