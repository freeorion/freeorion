#include <GG/AdamDlg.h>
#include <GG/Layout.h>
#include <GG/dialogs/ThreeButtonDlg.h>
#include <GG/SDL/SDLGUI.h>

#include <iostream>


class AdamGGApp :
    public GG::SDLGUI
{
public:
    AdamGGApp();

    virtual void Enter2DMode();
    virtual void Exit2DMode();

protected:
    virtual void Render();

private:
    virtual void GLInit();
    virtual void Initialize();
    virtual void FinalCleanup();
};

enum PathTypes {
    NONE,
    PATH_1
};

std::ostream& operator<<(std::ostream& os, PathTypes p);
std::istream& operator>>(std::istream& os, PathTypes& p);

namespace GG {

    struct AnySlotImplBase
    {
        virtual AnySlotImplBase* Clone() const = 0;
    };

    template <class Signature>
    struct AnySlotImpl :
        AnySlotImplBase
    {
        AnySlotImpl(const boost::function<Signature>& slot) :
            m_slot(slot)
            {}

        virtual AnySlotImplBase* Clone() const
            { return new AnySlotImpl(*this); }

        boost::function<Signature> m_slot;
    };

    GG_EXCEPTION(BadAnySlotCast);

    class AnySlot
    {
    public:
        AnySlot() :
            m_impl(0)
            {}

        template <class Signature>
        AnySlot(const boost::function<Signature>& slot) :
            m_impl(new AnySlotImpl<Signature>(slot))
            {}

        AnySlot(const AnySlot& rhs) :
            m_impl(rhs.m_impl ? rhs.m_impl->Clone() : 0)
            {}

        AnySlot& operator=(const AnySlot& rhs)
            {
                delete m_impl;
                m_impl = rhs.m_impl ? rhs.m_impl->Clone() : 0;
                return *this;
            }

        ~AnySlot()
            { delete m_impl; }

        template <class Signature>
        boost::function<Signature>& Cast()
            {
                assert(m_impl);
                AnySlotImpl<Signature>* derived_impl =
                    dynamic_cast<AnySlotImpl<Signature>*>(m_impl);
                if (!derived_impl)
                    throw BadAnySlotCast();
                return derived_impl->m_slot;
            }

    private:
        AnySlotImplBase* m_impl;
    };

    struct AnySignalImplBase
    {
        virtual AnySignalImplBase* Clone() const = 0;
        virtual boost::signals::connection Connect(AnySlot& slot) = 0;
    };

    template <class Signature>
    struct AnySignalImpl :
        AnySignalImplBase
    {
        AnySignalImpl(boost::signal<Signature>& signal) :
            m_signal(&signal)
            {}

        virtual AnySignalImplBase* Clone() const
            { return new AnySignalImpl(*this); }

        virtual boost::signals::connection Connect(AnySlot& slot)
            {
                boost::function<Signature>& fn = slot.Cast<Signature>();
                return m_signal->connect(fn);
            }

        boost::signal<Signature>* m_signal;
    };

    class AnySignal
    {
    public:
        AnySignal() :
            m_impl(0)
            {}

        template <class Signature>
        AnySignal(boost::signal<Signature>& signal) :
            m_impl(new AnySignalImpl<Signature>(signal))
            {}

        AnySignal(const AnySignal& rhs) :
            m_impl(rhs.m_impl ? rhs.m_impl->Clone() : 0)
            {}

        AnySignal& operator=(const AnySignal& rhs)
            {
                delete m_impl;
                m_impl = rhs.m_impl ? rhs.m_impl->Clone() : 0;
                return *this;
            }

        ~AnySignal()
            { delete m_impl; }

        boost::signals::connection Connect(AnySlot& slot)
            {
                assert(m_impl);
                return m_impl->Connect(slot);
            }

    private:
        AnySignalImplBase* m_impl;
    };

    struct Wnd_
    {
        typedef adobe::closed_hash_map<adobe::name_t, AnySignal> SignalsMap;
        typedef adobe::closed_hash_map<adobe::name_t, AnySlot> SlotsMap;

        SignalsMap m_signals_map;
        SlotsMap m_slots_map;
    };

    struct ControlA :
        public Wnd_
    {
        ControlA()
            {
                m_signals_map[adobe::name_t("foo_signal")] = AnySignal(foo_signal);
                m_signals_map[adobe::name_t("bar_signal")] = AnySignal(bar_signal);
                m_signals_map[adobe::name_t("baz_signal")] = AnySignal(baz_signal);
            }

        boost::signal<void (const int, double)> foo_signal;
        boost::signal<void (const int&, double&)> bar_signal;
        boost::signal<void (const int*, double*)> baz_signal;
    };

    struct ControlB :
        public Wnd_
    {
        ControlB()
            {
                m_slots_map[adobe::name_t("foo")] =
                    AnySlot(
                        boost::function<void (const int, double)>(
                            boost::bind(&ControlB::foo, this, _1, _2)
                        )
                    );
                m_slots_map[adobe::name_t("bar")] =
                    AnySlot(
                        boost::function<void (const int&, double&)>(
                            boost::bind(&ControlB::bar, this, _1, _2)
                        )
                    );
                m_slots_map[adobe::name_t("baz")] =
                    AnySlot(
                        boost::function<void (const int*, double*)>(
                            boost::bind(&ControlB::baz, this, _1, _2)
                        )
                    );
            }

        void foo(const int i, double d)
            { std::cerr << "foo(" << i << ", " << d << ")\n"; }
        void bar(const int& i, double& d)
            { std::cerr << "bar(" << i << ", " << d << ")\n"; }
        void baz(const int* i, double* d)
            { std::cerr << "baz(" << *i << ", " << *d << ")\n"; }
    };

    boost::signals::connection
    Connect(Wnd_& signal_wnd, adobe::name_t signal_name, Wnd_& slot_wnd, adobe::name_t slot_name)
    {
        AnySignal signal = signal_wnd.m_signals_map[signal_name];
        AnySlot slot = slot_wnd.m_slots_map[slot_name];
        return signal.Connect(slot);
    }

    bool TestNewConnections()
    {
        ControlA* control_a = new ControlA;
        ControlB* control_b = new ControlB;

#if 0
        boost::signals::connection old_style_foo_connection =
            Connect(control_a->foo_signal, &ControlB::foo, control_b);
#endif

        boost::signals::connection foo_connection =
            Connect(*control_a, adobe::name_t("foo_signal"), *control_b, adobe::name_t("foo"));
        boost::signals::connection bar_connection =
            Connect(*control_a, adobe::name_t("bar_signal"), *control_b, adobe::name_t("bar"));
        boost::signals::connection baz_connection =
            Connect(*control_a, adobe::name_t("baz_signal"), *control_b, adobe::name_t("baz"));

        int i = 180;
        double d = 3.14159;

        control_a->foo_signal(i, d);
        control_a->bar_signal(i, d);
        control_a->baz_signal(&i, &d);

        return false;
    }

    //bool dummy = TestNewConnections();

}

class AdamDialog :
    public GG::Wnd
{
private:
    static const GG::X WIDTH;
    static const GG::Y HEIGHT;

public:
    AdamDialog();

    adobe::dictionary_t Result();

    virtual void Render();
    virtual bool Run();

private:
    bool HandleActions (adobe::name_t name, const adobe::any_regular_t&);

    boost::shared_ptr<GG::Font> m_font;
    GG::Spin<PathTypes>* m_path_spin;
    GG::Edit* m_flatness_edit;
    GG::Button* m_ok;

    GG::AdamModalDialog m_adam_modal_dialog;
};


// implementations

std::ostream& operator<<(std::ostream& os, PathTypes p)
{
    os << (p == NONE ? "None" : "Path_1");
    return os;
}

std::istream& operator>>(std::istream& is, PathTypes& p)
{
    std::string path_str;
    is >> path_str;
    p = path_str == "None" ? NONE : PATH_1;
    return is;
}

const GG::X AdamDialog::WIDTH(250);
const GG::Y AdamDialog::HEIGHT(75);

AdamDialog::AdamDialog() :
    Wnd((AdamGGApp::GetGUI()->AppWidth() - WIDTH) / 2,
        (AdamGGApp::GetGUI()->AppHeight() - HEIGHT) / 2,
        WIDTH, HEIGHT, GG::INTERACTIVE | GG::MODAL),
    m_font(AdamGGApp::GetGUI()->GetStyleFactory()->DefaultFont()),
    m_path_spin(new GG::Spin<PathTypes>(GG::X0, GG::Y0, GG::X(50),
                                        NONE, PathTypes(1), NONE, PATH_1,
                                        false, m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_flatness_edit(new GG::Edit(GG::X0, GG::Y0, GG::X(50),
                                 "0.0", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_ok(new GG::Button(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                        "Ok", m_font, GG::CLR_SHADOW, GG::CLR_WHITE)),
    m_adam_modal_dialog("sheet clipping_path"
                        "{"
                        "output:"
                        "    result                  <== { path: path, flatness: flatness };"
                        ""
                        "interface:"
                        "    unlink flatness : 0.0   <== (path == 0) ? 0.0 : flatness;"
                        "    path            : 1;"
                        "}",
                        adobe::dictionary_t(),
                        adobe::dictionary_t(),
                        GG::ADAM_DIALOG_DISPLAY_ALWAYS,
                        this,
                        boost::bind(&AdamDialog::HandleActions, this, _1, _2),
                        boost::filesystem::path())
{
    GG::TextControl* path_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Path:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);
    GG::TextControl* flatness_label =
        new GG::TextControl(GG::X0, GG::Y0, GG::X(50), GG::Y(25),
                            "Flatness:", m_font, GG::CLR_WHITE, GG::FORMAT_RIGHT);

    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, WIDTH, HEIGHT, 2u, 3u);
    layout->SetBorderMargin(2);
    layout->SetCellMargin(4);
    layout->Add(path_label, 0, 0);
    layout->Add(m_path_spin, 0, 1);
    layout->Add(m_ok, 0, 2);
    layout->Add(flatness_label, 1, 0);
    layout->Add(m_flatness_edit, 1, 1);
    SetLayout(layout);

    m_adam_modal_dialog.BindCell<double, PathTypes>(*m_path_spin, adobe::name_t("path"));
    m_adam_modal_dialog.BindCell<double, double>(*m_flatness_edit, adobe::name_t("flatness"));

    GG::Connect(m_ok->ClickedSignal,
                boost::bind(boost::ref(m_adam_modal_dialog.DialogActionSignal),
                            adobe::name_t("ok"),
                            adobe::any_regular_t()));
}

adobe::dictionary_t AdamDialog::Result()
{ return m_adam_modal_dialog.Result().m_result_values; }

void AdamDialog::Render()
{ FlatRectangle(UpperLeft(), LowerRight(), GG::CLR_SHADOW, GG::CLR_SHADOW, 1); }

bool AdamDialog::Run()
{
    if (m_adam_modal_dialog.NeedUI())
        return Wnd::Run();
    return true;
}

bool AdamDialog::HandleActions (adobe::name_t name, const adobe::any_regular_t&)
{ return name == adobe::static_name_t("ok"); }


AdamGGApp::AdamGGApp() : 
    SDLGUI(1024, 768, false, "Adam App")
{}

void AdamGGApp::Enter2DMode()
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

void AdamGGApp::Exit2DMode()
{
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glPopAttrib();
}

void AdamGGApp::Render()
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

void AdamGGApp::GLInit()
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

void AdamGGApp::Initialize()
{
    SDL_WM_SetCaption("Adam GG App", "Adam GG App");

    AdamDialog adam_dlg;
    adam_dlg.Run();

    adobe::dictionary_t dictionary = adam_dlg.Result();

    std::ostringstream results_str;
    results_str << "result:\n"
                << "path = " << dictionary[adobe::name_t("path")].cast<double>() << "\n"
                << "flatness = " << dictionary[adobe::name_t("flatness")].cast<double>();

    GG::ThreeButtonDlg results_dlg(GG::X(200), GG::Y(100), results_str.str(),
                                   GetStyleFactory()->DefaultFont(), GG::CLR_SHADOW, 
                                   GG::CLR_SHADOW, GG::CLR_SHADOW, GG::CLR_WHITE, 1);
    results_dlg.Run();

    Exit(0);
}

// This gets called as the application is exit()ing, and as the name says,
// performs all necessary cleanup at the end of the app's run.
void AdamGGApp::FinalCleanup()
{}

extern "C" // Note the use of C-linkage, as required by SDL.
int main(int argc, char* argv[])
{
    AdamGGApp app;

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
