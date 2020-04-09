//About.cpp

#include "About.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/Directories.h"
#include "../util/i18n.h"

#include <GG/GUI.h>

#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>


namespace {

boost::optional<std::string> ReadFile(const boost::filesystem::path& file_path) {
    boost::filesystem::ifstream fin(file_path);
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

    m_done = Wnd::Create<CUIButton>(UserString("DONE"));
    m_license = Wnd::Create<CUIButton>(UserString("LICENSE"));
    m_vision = Wnd::Create<CUIButton>(UserString("VISION"));
    m_info = GG::Wnd::Create<CUIMultiEdit>(UserString("FREEORION_VISION"), GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY);
    AttachChild(m_info);
    AttachChild(m_vision);
    AttachChild(m_license);
    AttachChild(m_done);

    DoLayout();

    // Read in the copyright info from a file
    // this is not GetResourceDir() / "COPYING" because if a mod or scenario is loaded
    // that changes the settings directory, the copyright notice should be unchanged
    m_license_str = ReadFile(GetRootDataDir() / "default" / "COPYING").value_or("");

    m_done->LeftClickedSignal.connect(
        boost::bind(&GG::Wnd::EndRun, this));
    m_license->LeftClickedSignal.connect(
        boost::bind(&About::ShowLicense, this));
    m_vision->LeftClickedSignal.connect(
        boost::bind(&About::ShowVision, this));
}

void About::KeyPress(GG::Key key, std::uint32_t key_code_point,
                     GG::Flags<GG::ModKey> mod_keys)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE))
        EndRun();
}

void About::ShowLicense()
{ m_info->SetText(m_license_str); }

void About::ShowVision()
{ m_info->SetText(UserString("FREEORION_VISION")); }

void About::DoLayout() {
    const GG::X BUTTONS_HORIZONTAL_SPACING(5);
    const GG::Y CONTENT_GROUPS_VERTICAL_SPACING(5);
    const GG::Pt BORDERS_SIZE { GG::X {5}, GG::Y {5} };
    const GG::Pt BUTTON_SIZE {
        std::max({ m_vision->MinUsableSize().x, m_license->MinUsableSize().x, m_done->MinUsableSize().x }),
        std::max({ m_vision->MinUsableSize().y, m_license->MinUsableSize().y, m_done->MinUsableSize().y }),
    };

    auto const window_lr = ScreenToClient(ClientLowerRight());
    auto const content_lr = window_lr - BORDERS_SIZE;
    auto const content_ul = BORDERS_SIZE;

    GG::Pt draw_point = content_lr;

    for (auto& button : { m_done, m_vision, m_license }) {
        GG::Pt button_ul = draw_point - BUTTON_SIZE;
        button->SizeMove(button_ul, draw_point);

        draw_point.x -= BUTTON_SIZE.x + BUTTONS_HORIZONTAL_SPACING;
    }

    draw_point.x = content_lr.x;
    draw_point.y -= BUTTON_SIZE.y + CONTENT_GROUPS_VERTICAL_SPACING;

    m_info->SizeMove(content_ul, draw_point);
}
