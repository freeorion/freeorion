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
    CUI_Wnd(ClientUI::String("ESELECT_WINDOW_TITLE"),200,200,400,200, GG::Wnd::CLICKABLE | GG::Wnd::DRAGABLE | GG::Wnd::MODAL),
    m_end_with_ok(false)
{
   
    //create a temporary texture and static graphic  
    // this will get overwritten during construction
    m_empire_name = new GG::Edit(100, 25, 168, 25, "Tyrethians", ClientUI::FONT, ClientUI::PTS, ClientUI::INNER_BORDER_COLOR/*, GG::CLR_BLACK*/);
    m_empire_name->SetTextColor(ClientUI::TEXT_COLOR);
//uncomment this line once arrows.png is properly committed
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

    m_ok     = new GG::Button(300,90,75,25,ClientUI::String("OK"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_cancel = new GG::Button(210,90,75,25,ClientUI::String("CANCEL"), ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);

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
    AttachChild(new GG::TextControl(10,30,ClientUI::String("ESELECT_EMPIRE_NAME"),ClientUI::FONT,ClientUI::PTS,ClientUI::TEXT_COLOR));
    
    //attach signals
    GG::Connect(m_ok->ClickedSignal(), &EmpireSelect::OnOK, this);
    GG::Connect(m_cancel->ClickedSignal(), &EmpireSelect::OnCancel, this);
    GG::Connect(m_left_select->ClickedSignal(), &EmpireSelect::OnLeftArrow, this);
    GG::Connect(m_right_select->ClickedSignal(), &EmpireSelect::OnRightArrow, this);
    
}//Init()

GG::Clr EmpireSelect::SelectColor(int colnum)
{

   // TEMPORARY - This is just a placeholder to get something working quickly

   switch (colnum) {
   case 0:
   {
      GG::Clr newcolor(100,50,200,255);
      return newcolor;
   }
   case 1:
   {
      GG::Clr newcolor(100,100,100,255);
      return newcolor;
   }
   case 2:
   {
      GG::Clr newcolor(100,0,0,255);
      return newcolor;
   }
   case 3:
   {
      GG::Clr newcolor(0,100,0,255);
      return newcolor;
   }
   case 4:
   {
      GG::Clr newcolor(0,0,100,255);
      return newcolor;
   }
   case 5:
   {
      GG::Clr newcolor(100,100,0,255);
      return newcolor;
   }
   case 6:
   {
      GG::Clr newcolor(100,0,100,255);
      return newcolor;
   }
   case 7:
   {
      GG::Clr newcolor(0,100,100,255);
      return newcolor;
   }
   case 8:
   {
      GG::Clr newcolor(200,100,0,255);
      return newcolor;
   }
   case 9:
   {
      GG::Clr newcolor(200,0,100,255);
      return newcolor;
   }
   case 10:
   {
      GG::Clr newcolor(100,200,0,255);
      return newcolor;
   }
   default:
   {
      GG::Clr newcolor(0,0,0,255);
      return newcolor;
   }
   }

}//SelectColor()

EmpireSelect::~EmpireSelect()
{

}//~EmpireSelect

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int EmpireSelect::Render()
{
    CUI_Wnd::Render();
   // GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
    //ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, "Galaxy Setup");

    //draw square of the selected color
    GG::Clr newcolor(SelectColor(m_cur_color));
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

int EmpireSelect::EmpireColor()
{
   return m_cur_color;
}//EmpireColor()

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
  if (m_cur_color <= 0)
    m_cur_color = 10;
  else
    m_cur_color -= 1;
  Render();

}//OnLeftArrow()

void EmpireSelect::OnRightArrow()
{
  if (m_cur_color >= 10)
    m_cur_color = 0;
  else
    m_cur_color += 1;
  Render();

}//OnRightArrow()
