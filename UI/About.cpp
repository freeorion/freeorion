#include "About.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/Directories.h"
#include "../util/i18n.h"
#include "../client/human/GGHumanClientApp.h"

#include <GG/GUI.h>

#include <boost/optional.hpp>
#include <filesystem>
#include <fstream>


namespace {

boost::optional<std::string> ReadFile(const std::filesystem::path& file_path) {
    std::ifstream fin(file_path);
    if (!fin.is_open())
        return boost::none;

    std::stringstream buffer;
    buffer << fin.rdbuf();

    return buffer.str();
}

}

////////////////////////////////////////////
//   About
////////////////////////////////////////////
About::About():
    CUIWnd(UserString("ABOUT_WINDOW_TITLE"), GG::X(80), GG::Y(130), GG::X(600), GG::Y(500),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL)
{}

void About::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    if ((m_done = Wnd::Create<CUIButton>(UserString("DONE")))) {
        AttachChild(m_done);
        m_done->LeftClickedSignal.connect([this]() { EndRun(); });
    }

    if ((m_license = Wnd::Create<CUIButton>(UserString("LICENSE")))) {
        AttachChild(m_license);
        m_license->LeftClickedSignal.connect([this]() { ShowLicense(); });
    }

    if ((m_vision = Wnd::Create<CUIButton>(UserString("VISION")))) {
        AttachChild(m_vision);
        m_vision->LeftClickedSignal.connect([this]() { ShowVision(); });
    }

    if ((m_info = GG::Wnd::Create<CUIMultiEdit>(UserString("FREEORION_VISION"), GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY)))
        AttachChild(m_info);

    DoLayout();

    // Read in the copyright info from a file
    // this is not GetResourceDir() / "COPYING" because if a mod or scenario is loaded
    // that changes the settings directory, the copyright notice should be unchanged
    m_license_str = ReadFile(GetRootDataDir() / "default" / "COPYING").value_or("");
}

void About::KeyPress(GG::Key key, uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    if ((key == GG::Key::GGK_RETURN) || (key == GG::Key::GGK_ESCAPE))
        EndRun();
}

void About::ShowLicense()
{ m_info->SetText(m_license_str); }

void About::ShowVision()
{ m_info->SetText(UserString("FREEORION_VISION")); }

void About::DoLayout() {
    static constexpr GG::X BUTTONS_HORIZONTAL_SPACING{5};
    static constexpr GG::Y CONTENT_GROUPS_VERTICAL_SPACING{5};
    static constexpr GG::Pt BORDERS_SIZE{ GG::X{5}, GG::Y{5} };
    const GG::Pt BUTTON_SIZE {
        std::max({ m_vision ? m_vision->MinUsableSize().x : GG::X0,
                   m_license ? m_license->MinUsableSize().x : GG::X0,
                   m_done ? m_done->MinUsableSize().x : GG::X0}),
        std::max({ m_vision ? m_vision->MinUsableSize().y : GG::Y0,
                   m_license ? m_license->MinUsableSize().y : GG::Y0,
                   m_done ? m_done->MinUsableSize().y : GG::Y0}),
    };

    auto const window_lr = ScreenToClient(ClientLowerRight());
    auto const content_lr = window_lr - BORDERS_SIZE;
    auto const content_ul = BORDERS_SIZE;

    GG::Pt draw_point = content_lr;

    for (auto& button : { m_done, m_vision, m_license }) {
        GG::Pt button_ul = draw_point - BUTTON_SIZE;
        if (button)
            button->SizeMove(button_ul, draw_point);
        draw_point.x -= BUTTON_SIZE.x + BUTTONS_HORIZONTAL_SPACING;
    }

    draw_point.x = content_lr.x;
    draw_point.y -= BUTTON_SIZE.y + CONTENT_GROUPS_VERTICAL_SPACING;

    m_info->SizeMove(content_ul, draw_point);
}
