//About.cpp

#include "About.h"

#include "ClientUI.h"
#include "CUIControls.h"
#include "GGApp.h"
#include "GGClr.h"
#include "GGDrawUtil.h"
#include "dialogs/GGFileDlg.h"

#include <fstream>

////////////////////////////////////////////
//   CONSTANTS
////////////////////////////////////////////


////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

About::About():
    CUI_Wnd(ClientUI::String("ABOUT_WINDOW_TITLE"),100,100,500,500, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{

    m_done_btn = new CUIButton(400,440,75,ClientUI::String("DONE"));
    m_license = new CUIButton(310,440,75,ClientUI::String("LICENSE"));
    m_credits = new CUIButton(220,440,75,ClientUI::String("CREDITS"));
    std::string bob="FreeOrion, an open-source game based o­n Master of Orion, is a turn-based game of epic space strategy that builds o­n the classic '4X' model by incorporating the nation-building elements of games such as Europa Universalis 2 and a versatile tactical combat engine. While its modular, open-source design allows for a significant degree of customization of the game engine and the story elements by the community, the FreeOrion team is dedicated to the construction of a living, breathing universe in a 'grand campaign' model.";
    m_info = new CUIMultiEdit(20, 20, 450, 400, bob, GG::TF_WORDBREAK | GG::MultiEdit::READ_ONLY, ClientUI::FONT,
                 ClientUI::PTS, ClientUI::CTRL_BORDER_COLOR, ClientUI::TEXT_COLOR,
                 ClientUI::MULTIEDIT_INT_COLOR, CLICKABLE | DRAG_KEEPER);

    std::ifstream xml_file("default/credits.xml");
    m_credits_doc.ReadDoc(xml_file);
    xml_file.close();

    // Read in the credits from a file
    std::fstream fin, fin2;
    fin.open("default/credits.txt", std::ios::in);
    if (!fin.is_open()) return;

    std::string temp_str;
    while (!fin.eof())
    {
      std::getline(fin, temp_str, '\n');
      m_credits_str.append(temp_str);
      m_credits_str.append("\n");	// To ensure new lines are read
    }
    fin.close();

    fin2.open("default/COPYING", std::ios::in);
    if (!fin2.is_open()) return;
    while (!fin2.eof())
    {
      std::getline(fin2, temp_str, '\n');
      m_license_str.append(temp_str);
      m_license_str.append("\n");	// To ensure new lines are read
    }
    fin2.close();

    Init();    //attaches children and connects signals to slots
}//About()

void About::Init()
{
    //add children
    AttachChild(m_done_btn);
    AttachChild(m_license);
    AttachChild(m_credits);
    AttachChild(m_info);

    //attach signals
    GG::Connect(m_done_btn->ClickedSignal(), &About::OnDone, this);
    GG::Connect(m_license->ClickedSignal(), &About::OnLicense, this);
    GG::Connect(m_credits->ClickedSignal(), &About::OnCredits, this);

}//Init()

About::~About()
{

}//~About

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int About::Render()
{
    CUI_Wnd::Render();

    return true;
}//Render()

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void About::OnDone()
{
    m_info->SetText(m_credits_str);	//TODO this is a hack to stop a segfault
    m_done = true;
}//OnDone()

void About::OnLicense()
{
   m_info->SetText(m_license_str);
}//OnLicense()

void About::OnCredits()
{
   m_info->SetText(m_credits_str);
}//OnLicense()

