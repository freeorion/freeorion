//About.cpp

#ifndef _About_h_
#include "About.h"
#endif

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

#ifndef _GGDrawUtil_h_
#include "GGDrawUtil.h"
#endif

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"
#endif

#ifndef _GGFileDlg_h_
#include "dialogs/GGFileDlg.h"
#endif

#include <fstream>

////////////////////////////////////////////
//   CONSTANTS
////////////////////////////////////////////


////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

About::About():
    CUI_Wnd(ClientUI::String("ABOUT_WINDOW_TITLE"),200,200,300,300, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_done(false)
{

    m_done_btn = new GG::Button(200,240,75,25,ClientUI::String("DONE"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_license = new GG::Button(110,240,75,25,ClientUI::String("LICENSE"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);

    Init();    //attaches children and connects signals to slots
}//About()

void About::Init()
{
    //add children

    AttachChild(m_done_btn);
    AttachChild(m_license);

    // Attach static labels
    //AttachChild(new GG::StaticText(10,30,ClientUI::String("ESELECT_EMPIRE_NAME"),ClientUI::FONT,ClientUI::PTS,ClientUI::TEXT_COLOR));

    //attach signals
    GG::Connect(m_done_btn->ClickedSignal(), &About::OnDone, this);
    GG::Connect(m_license->ClickedSignal(), &About::OnLicense, this);

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
   // GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
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

}//OnLicense()

