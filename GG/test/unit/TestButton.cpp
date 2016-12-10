#include <boost/test/unit_test.hpp>

#include <boost/make_shared.hpp>
#include <GG/Button.h>
#include <GG/GUI.h>

class MockGUI : public GG::GUI
{
public:
    MockGUI() :
        GUI("MockGUI")
    {}

    unsigned int Ticks() const override
    { return 0; }

    GG::X AppWidth() const override
    { return GG::X(800); }

    GG::Y AppHeight() const override
    { return GG::Y(600); }

    void Exit(int code) override
    {}

    void Enter2DMode() override
    {}

    void Exit2DMode() override
    {}

    void RenderBegin() override
    {}

    void RenderEnd() override
    {}

    std::vector<std::string> GetSupportedResolutions() const override
    { return std::vector<std::string>(1, "800x600"); }

    GG::Pt GetDefaultResolution(int display_id) const override
    { return GG::Pt(GG::X(800), GG::Y(600)); }

    void HandleSystemEvents() override
    {}

    void Run() override
    {}
};

class MockFont : public GG::Font
{
};

class InspectButton : public GG::Button
{
public:
    using GG::Button::Button;

    GG::Label& GetLabel() const
    { return *m_label; }

    void HandleEventE(const GG::WndEvent& event)
    {
        Button::HandleEvent(event);
    }
};

namespace GG
{
    inline std::ostream& operator<<(std::ostream& stream, const Clr& color)
    {
        stream << "<GG::Clr(" << color.r
               << ", " << color.g
               << ", " << color.b
               << ", " << color.a << ")@" << &color << ">";
        return stream;
    }
}

MockGUI gui;

GG::WndEvent mouseenter_ev(
    GG::WndEvent::MouseEnter, GG::Pt(GG::X(10), GG::Y(10)), GG::MOD_KEY_NONE);
GG::WndEvent mousehere_ev(
    GG::WndEvent::MouseHere, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselpress_ev(
    GG::WndEvent::LButtonDown, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouserpress_ev(
    GG::WndEvent::RButtonDown, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselclick_ev(
    GG::WndEvent::LClick, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselrelease_ev(
    GG::WndEvent::LButtonUp, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouseldrag_ev(
    GG::WndEvent::LDrag, GG::Pt(GG::X(30), GG::Y(30)), GG::MOD_KEY_NONE);
GG::WndEvent mouseleave_ev(
    GG::WndEvent::MouseLeave, GG::Pt(GG::X(40), GG::Y(40)), GG::MOD_KEY_NONE);


BOOST_AUTO_TEST_SUITE(TestButton)

BOOST_AUTO_TEST_CASE( constructor )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);

    BOOST_TEST(button.Text() == "Test");
    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
    BOOST_TEST(button.MinUsableSize() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.Color() == GG::CLR_WHITE);
    BOOST_TEST(button.GetLabel().TextColor() == GG::CLR_GREEN);
    BOOST_TEST(button.UnpressedGraphic().Empty());
    BOOST_TEST(button.PressedGraphic().Empty());
    BOOST_TEST(button.RolloverGraphic().Empty());
    BOOST_TEST(button.LeftClickedSignal.empty());
    BOOST_TEST(button.RightClickedSignal.empty());
    BOOST_TEST(!button.Disabled());
    BOOST_TEST(button.Interactive());
    BOOST_TEST(!button.RepeatKeyPress());
    BOOST_TEST(!button.RepeatButtonDown());
    BOOST_TEST(!button.Dragable());
    BOOST_TEST(!button.Resizable());
    BOOST_TEST(!button.OnTop());
    BOOST_TEST(!button.Modal());
    BOOST_TEST(button.GetChildClippingMode() == GG::Wnd::DontClip);
    BOOST_TEST(!button.NonClientChild());
    BOOST_TEST(button.Visible());
    BOOST_TEST(button.PreRenderRequired());
    BOOST_TEST(button.Name() == "");
    BOOST_TEST(button.DragDropDataType() == "");
    BOOST_TEST(button.UpperLeft() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.RelativeUpperLeft() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.ClientUpperLeft() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.Left() == GG::X(0));
    BOOST_TEST(button.Top() == GG::Y(0));
    BOOST_TEST(button.LowerRight() == GG::Pt(GG::X(1), GG::Y(1)));
    BOOST_TEST(button.RelativeLowerRight() == GG::Pt(GG::X(1), GG::Y(1)));
    BOOST_TEST(button.ClientLowerRight() == GG::Pt(GG::X(1), GG::Y(1)));
    BOOST_TEST(button.Right() == GG::X(1));
    BOOST_TEST(button.Bottom() == GG::Y(1));
    BOOST_TEST(button.Width() == GG::X(1));
    BOOST_TEST(button.ClientWidth() == GG::X(1));
    BOOST_TEST(button.Height() == GG::Y(1));
    BOOST_TEST(button.ClientHeight() == GG::Y(1));
    BOOST_TEST(button.Size() == GG::Pt(GG::X(1), GG::Y(1)));
    BOOST_TEST(button.ClientSize() == GG::Pt(GG::X(1), GG::Y(1)));
    BOOST_TEST(button.MinSize() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.MinUsableSize() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.ScreenToWindow(GG::Pt(GG::X(0), GG::Y(0))) == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.ScreenToClient(GG::Pt(GG::X(0), GG::Y(0))) == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.InWindow(GG::Pt(GG::X(0), GG::Y(0))));
    BOOST_TEST(button.InClient(GG::Pt(GG::X(0), GG::Y(0))));
    BOOST_TEST(button.Children().size() == 1);
    BOOST_TEST(!button.Parent());
    BOOST_TEST(!button.RootParent());
    BOOST_TEST(!button.GetLayout());
    BOOST_TEST(!button.ContainingLayout());
    BOOST_TEST(button.BrowseModes().size() == 1);
    BOOST_TEST(button.GetStyleFactory() == gui.GetStyleFactory());
    BOOST_TEST(button.WindowRegion(GG::Pt(GG::X(0), GG::Y(0))) == GG::WR_NONE);
}

BOOST_AUTO_TEST_CASE( resize )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);

    button.SizeMove(GG::Pt(GG::X(10), GG::Y(10)), GG::Pt(GG::X(40), GG::Y(40)));

    BOOST_TEST(button.UpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_TEST(button.RelativeUpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_TEST(button.ClientUpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_TEST(button.Left() == GG::X(10));
    BOOST_TEST(button.Top() == GG::Y(10));
    BOOST_TEST(button.LowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_TEST(button.RelativeLowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_TEST(button.ClientLowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_TEST(button.Right() == GG::X(40));
    BOOST_TEST(button.Bottom() == GG::Y(40));
    BOOST_TEST(button.Width() == GG::X(30));
    BOOST_TEST(button.ClientWidth() == GG::X(30));
    BOOST_TEST(button.Height() == GG::Y(30));
    BOOST_TEST(button.ClientHeight() == GG::Y(30));
    BOOST_TEST(button.Size() == GG::Pt(GG::X(30), GG::Y(30)));
    BOOST_TEST(button.ClientSize() == GG::Pt(GG::X(30), GG::Y(30)));
    BOOST_TEST(button.MinSize() == GG::Pt(GG::X(0), GG::Y(0)));
    BOOST_TEST(button.MinUsableSize() == GG::Pt(GG::X(0), GG::Y(0)));
}

struct MouseClickCatcher
{
    void CountClick() {
        count++;
    }

    int count = 0;
};

BOOST_AUTO_TEST_CASE( click )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);
    MouseClickCatcher catcher;

    button.SizeMove(GG::Pt(GG::X(10), GG::Y(10)), GG::Pt(GG::X(40), GG::Y(40)));

    BOOST_TEST(!button.Disabled());

    BOOST_TEST(catcher.count == 0);
    button.HandleEventE(mouselpress_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mouserpress_ev);

    BOOST_TEST(catcher.count == 0);

    GG::Connect(button.LeftClickedSignal, &MouseClickCatcher::CountClick, &catcher);
    BOOST_TEST(button.LeftClickedSignal.num_slots() == 1);

    button.HandleEventE(mouseenter_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mousehere_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mousehere_ev);
    button.HandleEventE(mouselpress_ev);
    button.HandleEventE(mouseldrag_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mouserpress_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mouseldrag_ev);
    button.HandleEventE(mouselrelease_ev);

    BOOST_TEST(catcher.count == 0);

    button.HandleEventE(mouselclick_ev);

    BOOST_TEST(catcher.count == 1);
}

BOOST_AUTO_TEST_CASE( stateChange )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);

    button.SizeMove(GG::Pt(GG::X(10), GG::Y(10)), GG::Pt(GG::X(40), GG::Y(40)));

    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
    button.HandleEventE(mouseenter_ev);
    BOOST_TEST(button.State() == GG::Button::BN_ROLLOVER);
    button.HandleEventE(mouseleave_ev);
    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);

    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
    button.HandleEventE(mousehere_ev);
    BOOST_TEST(button.State() == GG::Button::BN_ROLLOVER);
    button.HandleEventE(mouseleave_ev);
    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);

    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
    button.HandleEventE(mousehere_ev);
    BOOST_TEST(button.State() == GG::Button::BN_ROLLOVER);
    button.HandleEventE(mouselpress_ev);
    BOOST_TEST(button.State() == GG::Button::BN_PRESSED);
    button.HandleEventE(mouseldrag_ev);
    BOOST_TEST(button.State() == GG::Button::BN_PRESSED);
    button.HandleEventE(mouseleave_ev);
    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);

    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
    button.HandleEventE(mouseldrag_ev);
    BOOST_TEST(button.State() == GG::Button::BN_PRESSED);
    button.HandleEventE(mouselrelease_ev);
    BOOST_TEST(button.State() == GG::Button::BN_UNPRESSED);
}

BOOST_AUTO_TEST_SUITE_END()
