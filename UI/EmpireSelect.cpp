//EmpireSelect.cpp

#ifndef _EmpireSelect_h_
#include "EmpireSelect.h"
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

EmpireSelect::EmpireSelect():
//    GG::ModalWnd(200,200,645,360),
    CUI_ModalWnd("Empire Selection",200,200,400,200, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE),
    m_end_with_ok(false)
{
   
    //create a temporary texture and static graphic  
    // this will get overwritten during construction
    m_empire_name = new GG::Edit(100, 25, 168, 25, "Tyrethians", ClientUI::FONT, ClientUI::PTS, ClientUI::INNER_BORDER_COLOR/*, GG::CLR_BLACK*/);
    m_empire_name->SetTextColor(ClientUI::TEXT_COLOR);

    m_arrows.Load(ClientUI::ART_DIR + "arrows.png");

    m_left_select = new GG::Button(40,130,15,15,"", ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR);
    m_right_select = new GG::Button(50,130,15,15,"", ClientUI::FONT, ClientUI::PTS, ClientUI::TEXT_COLOR, ClientUI::TEXT_COLOR);

    // Create subtextures of the arrows.png file
    m_la_sub = new GG::SubTexture(&m_arrows, 0, 0, 15, 15);
    m_la_hover_sub = new GG::SubTexture(&m_arrows, 0, 16, 15, 31);
    m_la_pressed_sub = new GG::SubTexture(&m_arrows, 0, 32, 15, 47);
    m_ra_sub = new GG::SubTexture(&m_arrows, 16, 0, 31, 15);
    m_ra_hover_sub = new GG::SubTexture(&m_arrows, 16, 16, 31, 31);
    m_ra_pressed_sub = new GG::SubTexture(&m_arrows, 16, 32, 31, 47);

    // Assign left arrow images to left selector 
    m_left_select->SetUnpressedGraphic(*m_la_sub);
    m_left_select->SetPressedGraphic(*m_la_pressed_sub);
    m_left_select->SetRolloverGraphic(*m_la_hover_sub);

    // Assign right arrow images to right selector
    m_right_select->SetUnpressedGraphic(*m_ra_sub);
    m_right_select->SetPressedGraphic(*m_ra_pressed_sub);
    m_right_select->SetRolloverGraphic(*m_ra_hover_sub);

    m_ok     = new GG::Button(300,90,75,25,"Ok", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_cancel = new GG::Button(210,90,75,25,"Cancel", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);

    m_cur_color = 0;

    Init();    //attaches children and connects signals to slots
}//EmpireSelect()

void EmpireSelect::Init()
{
    //add children
    
    AttachChild(m_empire_name);
    AttachChild(m_ok);
    AttachChild(m_cancel);
    AttachChild(m_left_select);
    AttachChild(m_right_select);

    // Attach static labels
    AttachChild(new GG::StaticText(10,30,"Empire name:",ClientUI::FONT,ClientUI::PTS,ClientUI::TEXT_COLOR));
    
    //attach signals
    GG::Connect(m_ok->ClickedSignal(), &EmpireSelect::OnOK, this);
    GG::Connect(m_cancel->ClickedSignal(), &EmpireSelect::OnCancel, this);
    GG::Connect(m_left_select->ClickedSignal(), &EmpireSelect::OnLeftArrow, this);
    GG::Connect(m_right_select->ClickedSignal(), &EmpireSelect::OnRightArrow, this);
    
}//Init()

EmpireSelect::~EmpireSelect()
{

}//~EmpireSelect

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int EmpireSelect::Render()
{
    CUI_ModalWnd::Render();
   // GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
    //ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "Galaxy Setup");

    //draw square of the selected color
    GG::Clr newcolor(m_cur_color, 0, 0, 255);
    GG::FlatRectangle(UpperLeft().x+30, UpperLeft().y+70, UpperLeft().x+80, UpperLeft().y+120, newcolor, ClientUI::INNER_BORDER_COLOR, 1);
    
    return true;
}//Render()

///////////////////////////////////////////////
//   ACCESSORS
///////////////////////////////////////////////

std::string EmpireSelect::EmpireName() const
{
    return *m_empire_name;
}//EmpireName()

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void EmpireSelect::OnOK()
{
    m_end_with_ok = true;
    
    m_done = true;
}//OnOK()

void EmpireSelect::OnCancel()
{
    m_end_with_ok = false;
    
    m_done = true;
}//OnCancel()

void EmpireSelect::OnLeftArrow()
{

  m_cur_color -= 20;
  Render();

}//OnLeftArrow()

void EmpireSelect::OnRightArrow()
{

  m_cur_color += 20;
  Render();

}//OnRightArrow()
