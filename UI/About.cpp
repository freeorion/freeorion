//About.cpp

#include "About.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "../util/MultiplayerCommon.h"
#include "../util/Directories.h"

#include <GG/GUI.h>
#include <GG/Clr.h>
#include <GG/Layout.h>

#include <boost/filesystem/fstream.hpp>



////////////////////////////////////////////
//   About
////////////////////////////////////////////
About::About():
    CUIWnd(UserString("ABOUT_WINDOW_TITLE"), GG::X(80), GG::Y(130), GG::X(600), GG::Y(500),
           GG::INTERACTIVE | GG::DRAGABLE | GG::MODAL),
    m_end_with_done(false)
{
    m_done_btn = new CUIButton(GG::X(400), GG::Y(440), GG::X(75), UserString("DONE"));
    m_license = new CUIButton(GG::X(310), GG::Y(440), GG::X(75), UserString("LICENSE"));
    m_vision = new CUIButton(GG::X(220), GG::Y(440), GG::X(75), UserString("VISION"));
    m_info = new CUIMultiEdit(GG::X(20), GG::Y(20), GG::X(550), GG::Y(400), UserString("FREEORION_VISION"), 
                              GG::MULTI_WORDBREAK | GG::MULTI_READ_ONLY,
                              ClientUI::GetFont(),
                              ClientUI::CtrlBorderColor(), ClientUI::TextColor(),
                              ClientUI::CtrlColor(), GG::INTERACTIVE);
    GG::Layout* layout = new GG::Layout(GG::X0, GG::Y0, ClientWidth(), ClientHeight(), 2, 6, 5);
    layout->SetMinimumRowHeight(1, m_license->Height() + 5);
    layout->SetRowStretch(0, 1);
    layout->Add(m_info, 0, 0, 1, 6);
    layout->Add(m_vision, 1, 3);
    layout->Add(m_license, 1, 4);
    layout->Add(m_done_btn, 1, 5);
    AttachChild(layout);

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

    Init();
}

void About::Init()
{
    GG::Connect(m_done_btn->ClickedSignal, &About::OnDone, this);
    GG::Connect(m_license->ClickedSignal, &About::OnLicense, this);
    GG::Connect(m_vision->ClickedSignal, &About::OnVision, this);
}

About::~About()
{}

void About::Render()
{
    CUIWnd::Render();
}

void About::KeyPress (GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE)) {
        OnDone();
    }
}

void About::OnDone()
{
    m_done = true;
}

void About::OnLicense()
{
   m_info->SetText(m_license_str);
}

void About::OnVision()
{
   m_info->SetText(UserString("FREEORION_VISION"));
}

