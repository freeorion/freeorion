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
    m_info = new GG::MultiEdit(20, 20, 450, 400,"This is a textedit with stuff in it but it's not so linewrappy what will this be\nsecondlineist his fawe ifwjiewwej j", ClientUI::FONT, ClientUI::PTS, GG::CLR_WHITE, GG::MultiEdit::READ_ONLY, ClientUI::TEXT_COLOR, GG::CLR_BLACK, GG::Wnd::CLICKABLE | GG::Wnd::DRAG_KEEPER);

    std::ifstream xml_file("default/credits.xml");
    m_credits_doc.ReadDoc(xml_file);
    xml_file.close();

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
   // GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::WND_BORDER_COLOR,true);
    //ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "Galaxy Setup");

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
    m_done = true;
}//OnDone()

void About::OnLicense()
{
   m_info->SetText("License");
}//OnLicense()

void About::OnCredits()
{
   m_info->SetText(m_credits_doc.root_node.Child("CREDITS").Text());
   //m_info->SetText("Credits");
}//OnLicense()

