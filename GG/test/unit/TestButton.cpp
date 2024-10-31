//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2016 Marcel Metz
//!  Copyright (C) 2018-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

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

    unsigned int Ticks() const noexcept override { return 0; }

    GG::X AppWidth() const noexcept override { return GG::X(800); }

    GG::Y AppHeight() const noexcept override { return GG::Y(600); }

    void ExitApp(int code) noexcept override {}

    void Enter2DMode() noexcept override {}

    void Exit2DMode() noexcept override {}

    void RenderBegin() noexcept override {}

    void RenderEnd() noexcept override {}

    std::vector<std::string> GetSupportedResolutions() const override
    { return std::vector<std::string>(1, "800x600"); }

    GG::Pt GetDefaultResolution(int display_id) const override
    { return GG::Pt(GG::X(800), GG::Y(600)); }

    void HandleSystemEvents() noexcept override {}

    void Run() noexcept override {}
};

class MockFont : public GG::Font
{};

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
    GG::WndEvent::EventType::MouseEnter, GG::Pt(GG::X(10), GG::Y(10)), GG::MOD_KEY_NONE);
GG::WndEvent mousehere_ev(
    GG::WndEvent::EventType::MouseHere, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselpress_ev(
    GG::WndEvent::EventType::LButtonDown, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouserpress_ev(
    GG::WndEvent::EventType::RButtonDown, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselclick_ev(
    GG::WndEvent::EventType::LClick, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouselrelease_ev(
    GG::WndEvent::EventType::LButtonUp, GG::Pt(GG::X(25), GG::Y(25)), GG::MOD_KEY_NONE);
GG::WndEvent mouseldrag_ev(
    GG::WndEvent::EventType::LDrag, GG::Pt(GG::X(30), GG::Y(30)), GG::MOD_KEY_NONE);
GG::WndEvent mouseleave_ev(
    GG::WndEvent::EventType::MouseLeave, GG::Pt(GG::X(40), GG::Y(40)), GG::MOD_KEY_NONE);


BOOST_AUTO_TEST_SUITE(TestButton)

BOOST_AUTO_TEST_CASE( constructor )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    auto buttonp = GG::Wnd::Create<InspectButton>("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);
    auto& button = *buttonp;

    BOOST_CHECK(button.Text() == "Test");
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
    BOOST_CHECK(button.MinUsableSize() == GG::Pt0);
    BOOST_CHECK(button.Color() == GG::CLR_WHITE);
    BOOST_CHECK(button.GetLabel().TextColor() == GG::CLR_GREEN);
    BOOST_CHECK(button.UnpressedGraphic().Empty());
    BOOST_CHECK(button.PressedGraphic().Empty());
    BOOST_CHECK(button.RolloverGraphic().Empty());
    BOOST_CHECK(button.LeftClickedSignal.empty());
    BOOST_CHECK(button.RightClickedSignal.empty());
    BOOST_CHECK(!button.Disabled());
    BOOST_CHECK(button.Interactive());
    BOOST_CHECK(!button.RepeatKeyPress());
    BOOST_CHECK(!button.RepeatButtonDown());
    BOOST_CHECK(!button.Dragable());
    BOOST_CHECK(!button.Resizable());
    BOOST_CHECK(!button.OnTop());
    BOOST_CHECK(!button.Modal());
    BOOST_CHECK(button.GetChildClippingMode() == GG::Wnd::ChildClippingMode::DontClip);
    BOOST_CHECK(!button.NonClientChild());
    BOOST_CHECK(button.Visible());
    BOOST_CHECK(!button.PreRenderRequired());
    BOOST_CHECK(button.Name() == "");
    BOOST_CHECK(button.DragDropDataType() == "");
    BOOST_CHECK(button.UpperLeft() == GG::Pt0);
    BOOST_CHECK(button.RelativeUpperLeft() == GG::Pt0);
    BOOST_CHECK(button.ClientUpperLeft() == GG::Pt0);
    BOOST_CHECK(button.Left() == GG::X0);
    BOOST_CHECK(button.Top() == GG::Y0);
    BOOST_CHECK(button.LowerRight() == GG::Pt(GG::X1, GG::Y1));
    BOOST_CHECK(button.RelativeLowerRight() == GG::Pt(GG::X1, GG::Y1));
    BOOST_CHECK(button.ClientLowerRight() == GG::Pt(GG::X1, GG::Y1));
    BOOST_CHECK(button.Right() == GG::X1);
    BOOST_CHECK(button.Bottom() == GG::Y1);
    BOOST_CHECK(button.Width() == GG::X1);
    BOOST_CHECK(button.ClientWidth() == GG::X1);
    BOOST_CHECK(button.Height() == GG::Y1);
    BOOST_CHECK(button.ClientHeight() == GG::Y1);
    BOOST_CHECK(button.Size() == GG::Pt(GG::X1, GG::Y1));
    BOOST_CHECK(button.ClientSize() == GG::Pt(GG::X1, GG::Y1));
    BOOST_CHECK(button.MinSize() == GG::Pt0);
    BOOST_CHECK(button.MinUsableSize() == GG::Pt0);
    BOOST_CHECK(button.ScreenToWindow(GG::Pt0) == GG::Pt0);
    BOOST_CHECK(button.ScreenToClient(GG::Pt0) == GG::Pt0);
    BOOST_CHECK(button.InWindow(GG::Pt0));
    BOOST_CHECK(button.InClient(GG::Pt0));
    BOOST_CHECK(button.Children().size() == 2);
    BOOST_CHECK(!button.Parent());
    BOOST_CHECK(!button.RootParent());
    BOOST_CHECK(!button.GetLayout());
    BOOST_CHECK(!button.ContainingLayout());
    BOOST_CHECK(button.BrowseModes().size() == 1);
    BOOST_CHECK(&button.GetStyleFactory() == &gui.GetStyleFactory());
    BOOST_CHECK(button.WindowRegion(GG::Pt0) == GG::WndRegion::WR_NONE);
}

BOOST_AUTO_TEST_CASE( resize )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);

    button.SizeMove(GG::Pt(GG::X(10), GG::Y(10)), GG::Pt(GG::X(40), GG::Y(40)));

    BOOST_CHECK(button.UpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_CHECK(button.RelativeUpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_CHECK(button.ClientUpperLeft() == GG::Pt(GG::X(10), GG::Y(10)));
    BOOST_CHECK(button.Left() == GG::X(10));
    BOOST_CHECK(button.Top() == GG::Y(10));
    BOOST_CHECK(button.LowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_CHECK(button.RelativeLowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_CHECK(button.ClientLowerRight() == GG::Pt(GG::X(40), GG::Y(40)));
    BOOST_CHECK(button.Right() == GG::X(40));
    BOOST_CHECK(button.Bottom() == GG::Y(40));
    BOOST_CHECK(button.Width() == GG::X(30));
    BOOST_CHECK(button.ClientWidth() == GG::X(30));
    BOOST_CHECK(button.Height() == GG::Y(30));
    BOOST_CHECK(button.ClientHeight() == GG::Y(30));
    BOOST_CHECK(button.Size() == GG::Pt(GG::X(30), GG::Y(30)));
    BOOST_CHECK(button.ClientSize() == GG::Pt(GG::X(30), GG::Y(30)));
    BOOST_CHECK(button.MinSize() == GG::Pt0);
    BOOST_CHECK(button.MinUsableSize() == GG::Pt0);
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

    BOOST_CHECK(!button.Disabled());

    BOOST_CHECK(catcher.count == 0);
    button.HandleEventE(mouselpress_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mouserpress_ev);

    BOOST_CHECK(catcher.count == 0);

    button.LeftClickedSignal.connect(
        boost::bind(&MouseClickCatcher::CountClick, &catcher));
    BOOST_CHECK(button.LeftClickedSignal.num_slots() == 1);

    button.HandleEventE(mouseenter_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mousehere_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mousehere_ev);
    button.HandleEventE(mouselpress_ev);
    button.HandleEventE(mouseldrag_ev);
    button.HandleEventE(mouseleave_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mouserpress_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mouseldrag_ev);
    button.HandleEventE(mouselrelease_ev);

    BOOST_CHECK(catcher.count == 0);

    button.HandleEventE(mouselclick_ev);

    BOOST_CHECK(catcher.count == 1);
}

BOOST_AUTO_TEST_CASE( stateChange )
{
    std::shared_ptr<GG::Font> font(new MockFont());
    InspectButton button("Test", font, GG::CLR_WHITE, GG::CLR_GREEN);

    button.SizeMove(GG::Pt(GG::X(10), GG::Y(10)), GG::Pt(GG::X(40), GG::Y(40)));

    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
    button.HandleEventE(mouseenter_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_ROLLOVER);
    button.HandleEventE(mouseleave_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);

    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
    button.HandleEventE(mousehere_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_ROLLOVER);
    button.HandleEventE(mouseleave_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);

    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
    button.HandleEventE(mousehere_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_ROLLOVER);
    button.HandleEventE(mouselpress_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_PRESSED);
    button.HandleEventE(mouseldrag_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_PRESSED);
    button.HandleEventE(mouseleave_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);

    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
    button.HandleEventE(mouseldrag_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_PRESSED);
    button.HandleEventE(mouselrelease_ev);
    BOOST_CHECK(button.State() == GG::Button::ButtonState::BN_UNPRESSED);
}

BOOST_AUTO_TEST_SUITE_END()
