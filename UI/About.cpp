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
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

About::About():
    CUIWnd(UserString("ABOUT_WINDOW_TITLE"),80,130,600,500, GG::CLICKABLE | GG::DRAGABLE | GG::MODAL),
    m_end_with_done(false)
{
    m_done_btn = new CUIButton(400,440,75,UserString("DONE"));
    m_license = new CUIButton(310,440,75,UserString("LICENSE"));
    m_vision = new CUIButton(220,440,75,UserString("VISION"));
    m_info = new CUIMultiEdit(20, 20, 550, 400, UserString("FREEORION_VISION"), 
                              GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY,
                              GG::GUI::GetGUI()->GetFont(ClientUI::FONT, ClientUI::PTS),
                              ClientUI::CTRL_BORDER_COLOR, ClientUI::TEXT_COLOR,
                              ClientUI::MULTIEDIT_INT_COLOR, GG::CLICKABLE);
    GG::Layout* layout = new GG::Layout(0, 0, ClientWidth(), ClientHeight(), 2, 6, 5);
    layout->SetMinimumRowHeight(1, m_license->Height() + 5);
    layout->SetRowStretch(0, 1);
    layout->Add(m_info, 0, 0, 1, 6);
    layout->Add(m_vision, 1, 3);
    layout->Add(m_license, 1, 4);
    layout->Add(m_done_btn, 1, 5);
    AttachChild(layout);

    // Read in the copyright info from a file
#ifdef FREEORION_LINUX
    boost::filesystem::ifstream fin(GetGlobalDir() / "default/COPYING");
#else
    std::ifstream fin("default/COPYING");
#endif
    if (!fin.is_open()) return;
    std::string temp_str;
    while (!fin.eof())
    {
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
{

}

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

void About::Render()
{
    CUIWnd::Render();
}

void About::KeyPress (GG::Key key, Uint32 key_mods)
{
    if ((key == GG::GGK_RETURN) || (key == GG::GGK_ESCAPE)) // Same behaviour as if "done" was pressed
    {
      OnDone();
    }
}

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

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

