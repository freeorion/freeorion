//GalaxySetupWnd.cpp

#ifndef _GalaxySetupWnd_h_
#include "GalaxySetupWnd.h"
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

////////////////////////////////////////////
//   CONSTRUCTION/DESTRUCTION
////////////////////////////////////////////

GalaxySetupWnd::GalaxySetupWnd():
    GG::ModalWnd(200,200,540,360),
    m_end_with_ok(false)
{
    //initialize size radio group
    m_size_buttons = new GG::RadioButtonGroup(2,2);
    GG::StateButton* temp=NULL;
    
    m_size_buttons->AddButton(temp=new GG::StateButton(5,5,100,50,"Very Small (50 Stars)", ClientUI::FONT, ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_size_buttons->AddButton(temp=new GG::StateButton(5,60,100,50,"Small (100 Stars)", ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_size_buttons->AddButton(temp=new GG::StateButton(5,115,100,50,"Medium (150 Stars)", ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_size_buttons->AddButton(temp=new GG::StateButton(5,170,100,50,"Large (200 Stars)", ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_size_buttons->AddButton(temp=new GG::StateButton(5,225,100,50,"Very Large (250 Stars)", ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_size_buttons->AddButton(temp=new GG::StateButton(5,280,100,50,"Enormous! (300 Stars)", ClientUI::FONT,ClientUI::PTS,0, ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    
    m_type_buttons = new GG::RadioButtonGroup(125,5);
    
    m_type_buttons->AddButton(temp=new GG::StateButton(125,5,100,50,"Spiral, 2-arm", ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,60,100,50,"Spiral, 3-arm", ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,115,100,50,"Spiral, 4-arm", ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,170,100,50,"Cluster", ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    m_type_buttons->AddButton(temp=new GG::StateButton(125,225,100,50,"From file:", ClientUI::FONT,ClientUI::PTS,0,ClientUI::TEXT_COLOR, ClientUI::CTRL_COLOR,GG::StateButton::SBSTYLE_3D_RADIO,-1,-1,ClientUI::PTS+4, ClientUI::PTS+4));
    temp->SetTextColor(ClientUI::TEXT_COLOR);
    
    m_ok     = new GG::Button(460,300,75,25,"OK", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    m_cancel = new GG::Button(370,300,75,25,"Cancel", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR, ClientUI::TEXT_COLOR);
    
        
    //add children
    AttachChild(m_size_buttons);
    AttachChild(m_type_buttons);
    
    AttachChild(m_ok);
    AttachChild(m_cancel);
    
    //attach signals
    GG::Connect(m_size_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeSize, this);
    GG::Connect(m_type_buttons->ButtonChangedSignal(), &GalaxySetupWnd::OnChangeType, this);
    GG::Connect(m_ok->ClickedSignal(), &GalaxySetupWnd::OnOK, this);
    GG::Connect(m_cancel->ClickedSignal(), &GalaxySetupWnd::OnCancel, this);

}//GalaxySetupWnd()

GalaxySetupWnd::GalaxySetupWnd(const GG::XMLElement &elem):
    GG::ModalWnd(elem.Child("GG::ModalWnd"))
{

}//GalaxySetupWnd(XMLElement)

GalaxySetupWnd::~GalaxySetupWnd()
{

}//~GalaxySetupWnd

///////////////////////////////////////////////
//   MUTATORS
///////////////////////////////////////////////

int GalaxySetupWnd::Render()
{
    GG::BeveledRectangle(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y,ClientUI::WND_COLOR,ClientUI::BORDER_COLOR,true);
    
    return true;
}//Render()

GG::XMLElement GalaxySetupWnd::XMLEncode() const
{

}//XMLEncode()

///////////////////////////////////////////////
//   EVENT HANDLERS
///////////////////////////////////////////////

void GalaxySetupWnd::OnOK()
{
    m_end_with_ok = true;
    
    m_done = true;
}//OnOK()

void GalaxySetupWnd::OnCancel()
{
    m_end_with_ok = false;
    
    m_done = true;
}//OnCancel()

void GalaxySetupWnd::OnChangeSize(int index)
{

}//OnChangeSize()

void GalaxySetupWnd::OnChangeType(int index)
{

}//OnChangeType()
