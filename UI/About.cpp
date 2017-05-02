//About.cpp

#include "About.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/Directories.h"
#include "../util/i18n.h"

#include <GG/GUI.h>
#include <GG/SignalsAndSlots.h>

#include <boost/filesystem/fstream.hpp>


////////////////////////////////////////////
//   About
////////////////////////////////////////////
About::About():
    CUIWnd(UserString("ABOUT_WINDOW_TITLE"), GG::X(80), GG::Y(130), GG::X(600), GG::Y(500),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL)
{
    m_done_btn = new CUIButton(UserString("DONE"));
    m_license = new CUIButton(UserString("LICENSE"));
    m_vision = new CUIButton(UserString("VISION"));
    m_info = new CUIMultiEdit(UserString("FREEORION_VISION"), GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY);
    AttachChild(m_info);
    AttachChild(m_vision);
    AttachChild(m_license);
    AttachChild(m_done_btn);

    DoLayout();

    // Read in the copyright info from a file
    boost::filesystem::ifstream fin(GetRootDataDir() / "default" / "COPYING");    // this is not GetResourceDir() / "COPYING" because if a mod or scenario is loaded that changes the settings directory, the copyright notice should be unchanged
    if (!fin.is_open()) return;
    std::string temp_str;
    while (!fin.eof()) {
        std::getline(fin, temp_str, '\n');
        m_license_str.append(temp_str);
        m_license_str.append("\n"); // To ensure new lines are read
    }
    fin.close();

    GG::Connect(m_done_btn->LeftClickedSignal, boost::bind(&GG::Wnd::EndRun, this));
    GG::Connect(m_license->LeftClickedSignal, &About::ShowLicense, this);
    GG::Connect(m_vision->LeftClickedSignal, &About::ShowVision, this);
}

About::~About()
{}

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
    BUTTON_SIZE.x = std::max(BUTTON_SIZE.x, m_done_btn->MinUsableSize().x);
    BUTTON_SIZE.y = std::max(BUTTON_SIZE.y, m_license->MinUsableSize().y);
    BUTTON_SIZE.y = std::max(BUTTON_SIZE.y, m_done_btn->MinUsableSize().y);

    GG::Pt buttons_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING);
    GG::Pt buttons_ul = buttons_lr - BUTTON_SIZE;
    m_done_btn->SizeMove(buttons_ul, buttons_lr);

    buttons_lr.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    buttons_ul.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    m_vision->SizeMove(buttons_ul, buttons_lr);

    buttons_lr.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    buttons_ul.x -= BUTTON_SIZE.x + HORIZONTAL_SPACING;
    m_license->SizeMove(buttons_ul, buttons_lr);

    GG::Pt text_area_lr = ScreenToClient(ClientLowerRight()) - GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING + BUTTON_SIZE.y + VERTICAL_SPACING);
    m_info->SizeMove(GG::Pt(HORIZONTAL_SPACING, VERTICAL_SPACING), text_area_lr);
}
