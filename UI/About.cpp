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
    const GG::X HORIZONTAL_SPACING(5);
    const GG::Y VERTICAL_SPACING(5);

    GG::Pt BUTTON_SIZE = m_vision->MinUsableSize();
    BUTTON_SIZE.x = std::max(BUTTON_SIZE.x, m_license->MinUsableSize().x);
    BUTTON_SIZE.x = std::max(BUTTON_SIZE.x, m_done->MinUsableSize().x);
    BUTTON_SIZE.y = std::max(BUTTON_SIZE.y, m_license->MinUsableSize().y);
    BUTTON_SIZE.y = std::max(BUTTON_SIZE.y, m_done->MinUsableSize().y);

    GG::Pt buttons_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING);
    GG::Pt buttons_ul = buttons_lr - BUTTON_SIZE;
    m_done->SizeMove(buttons_ul, buttons_lr);

    buttons_lr.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    buttons_ul.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    m_vision->SizeMove(buttons_ul, buttons_lr);

    buttons_lr.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    buttons_ul.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    m_license->SizeMove(buttons_ul, buttons_lr);

    GG::Pt text_area_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING + BUTTON_SIZE.y + VERTICAL_SPACING);
    m_info->SizeMove(GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING), text_area_lr);
}
