//CUI_Wnd.cpp

#ifndef _CUI_Wnd_h_
#include "CUI_Wnd.h"
#endif

#ifndef _GGButton_h_
#include "GGButton.h"
#endif

#ifndef _ClientUI_h_
#include "ClientUI.h"   //include for DrawWindow....for now
#endif

CUI_ModalWnd::CUI_ModalWnd(const std::string& t, int x, int y, int w, int h, Uint32 flags):
    GG::ModalWnd(x,y,w,h,flags),
    m_title(t)
{
    if(flags & CUI_Wnd::MINIMIZABLE)
        m_minimize=true;
    else
        m_minimize=false;
    //create the window and make connections to base buttons
    //get corresponding coordinates that match ClientUI::DrawWindow
    int x1 = 5;
    int y1 = 15;
    int x2 = w - 5;
    int y2 = h - 5;
    
    GG::Button* temp;    
    //first create the close button
    temp = new GG::Button(w-15, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
    GG::Connect(temp->ClickedSignal(), &CUI_ModalWnd::OnCloseClick, this);
    AttachChild(temp);
    //make the button invisible....yet still active
    temp->Hide();
    
    //do the minimize button
    if(m_minimize)
    {
        temp = new GG::Button(w-30, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
        GG::Connect(temp->ClickedSignal(), &CUI_ModalWnd::OnMinimizeClick, this);
        AttachChild(temp);      
        temp->Hide();
    }
    
}//CUI_ModalWnd()

CUI_ModalWnd::CUI_ModalWnd(const GG::XMLElement& elem):
    GG::ModalWnd(elem.Child("GG::ModalWnd"))
{
}

CUI_ModalWnd::~CUI_ModalWnd()
{

}//~CUI_ModalWnd()

int CUI_ModalWnd::Render()
{
    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_title,
         true, m_minimize, Resizable());
    return 0;
}//Render()

void CUI_ModalWnd::OnCloseClick()
{
    //call public member for user defined behavior
    OnClose();
    m_done = true;
}//OnCloseClick()

void CUI_ModalWnd::OnMinimizeClick()
{
    OnMinimize();
    //TODO: Implementation
}//OnMinimizeClick()r

void CUI_ModalWnd::OnResizeClick(int x, int y)
{
    OnResize(x,y);
    //TODO: Implementation
}//OnResize()

//these functions do nothing by default
void CUI_ModalWnd::OnResize(int x, int y){}
void CUI_ModalWnd::OnClose(){}
void CUI_ModalWnd::OnMinimize(){}


////////////////////////////////////////////////////////////////////////////////////////////////
//CUI_Wnd//////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////


CUI_Wnd::CUI_Wnd(const std::string& t, int x, int y, int w, int h, Uint32 flags):
    GG::Wnd(x,y,w,h,flags),
    m_title(t)
{
    if(flags & MINIMIZABLE)
        m_minimize=true;
    else
        m_minimize=false;
    //create the window and make connections to base buttons
    //get corresponding coordinates that match ClientUI::DrawWindow
    int x1 = 5;
    int y1 = 15;
    int x2 = w - 5;
    int y2 = h - 5;
    
    GG::Button* temp;    
    //first create the close button
    temp = new GG::Button(w-15, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
    GG::Connect(temp->ClickedSignal(), &CUI_Wnd::OnCloseClick, this);
    AttachChild(temp);
    //make the button invisible....yet still active
    temp->Hide();
    
    //do the minimize button
    if(m_minimize)
    {
        temp = new GG::Button(w-30, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
        GG::Connect(temp->ClickedSignal(), &CUI_Wnd::OnMinimizeClick, this);
        AttachChild(temp);      
        temp->Hide();
    }
    
}//CUI_Wnd()

CUI_Wnd::CUI_Wnd(const GG::XMLElement& elem):
    GG::Wnd(elem.Child("GG::Wnd"))
{
}

CUI_Wnd::~CUI_Wnd()
{

}//~CUI_Wnd()

int CUI_Wnd::Render()
{
    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_title,
        true, m_minimize, Resizable());
    return 0;
}//Render()

void CUI_Wnd::OnCloseClick()
{
    //call protected member for user-defined behavior:
    OnClose();
    //close the window if it gets clicked
    //first remove from parent
    Hide(true);
    if(Parent() != NULL)
        Parent()->DeleteChild(this);

}//OnCloseClick()

void CUI_Wnd::OnMinimizeClick()
{
    //call protected member for user-defined behavior:
    OnMinimize();
    //TODO: Implementation
}//OnMinimizeClick()

void CUI_Wnd::OnResizeClick(int x, int y)
{
    //call protected member for user-defined behavior:
    OnResize(x,y);
    //TODO: Implementation
}//OnResize()

//these functions do nothing by default
void CUI_Wnd::OnResize(int x, int y){}
void CUI_Wnd::OnClose(){}
void CUI_Wnd::OnMinimize(){}

