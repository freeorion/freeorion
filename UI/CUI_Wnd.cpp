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

#ifndef _GGApp_h_
#include "GGApp.h"
#endif

/** \mainpage FreeOrion User Interface

    \section s_overview Overview
    The User Interface module contains all classes pertaining to
    user interactivity.  It consists of the ClientUI class which acts
    as the driver for all of the others.  This module operates as an
    extension to the GG Graphical User Interface Library written by Zach Laine.

    \section s_interface_classes Interface Classes
    <ul>
    <li>ClientUI - the main driver class of the module.
    <li>CUI_Wnd - parent class of all non-modal interface windows.
    <li>CUI_ModalWnd - parent class of all modal interface windows.
    <li>IntroScreen - a combination main menu/splash screen.  The first thing the user sees.
    <li>ServerConnectWnd - a modal window that allows the user to find and choose a game server.
    <li>GalaxySetupWnd - a modal window that allows the user to setup the galaxy size and shape.
    </ul>
    
    \section s_utility_classes Utility Classes
    <ul>
    <li>StringTable - a construct allowing language-independent string storage and retrieval.
    <li>ToolWnd - a GG::Control-derived class that provides balloon-style help
    <li>ToolContainer - a manager construct that drives the functionality of all ToolWnd objects.
    </ul>
    
*/

CUI_ModalWnd::CUI_ModalWnd(const std::string& t, int x, int y, int w, int h, Uint32 flags):
    GG::ModalWnd(x,y,w,h,flags),
    m_title(t),
    m_is_resizing(false),
    m_is_minimized(false),
    m_close_button(NULL),
    m_minimize_button(NULL)
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
    
    //first create the close button
    m_close_button = new GG::Button(w-15, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
    GG::Connect(m_close_button->ClickedSignal(), &CUI_ModalWnd::OnCloseClick, this);
    AttachChild(m_close_button);
    //make the button invisible....yet still active
    m_close_button->Hide();
    
    //do the minimize button
    if(m_minimize)
    {
        m_minimize_button = new GG::Button(w-30, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
        GG::Connect(m_minimize_button->ClickedSignal(), &CUI_ModalWnd::OnMinimizeClick, this);
        AttachChild(m_minimize_button);      
        m_minimize_button->Hide();
    }
    
}//CUI_Wnd()

CUI_ModalWnd::CUI_ModalWnd(const GG::XMLElement& elem):
    GG::ModalWnd(elem.Child("GG::ModalWnd"))
{
}

CUI_ModalWnd::~CUI_ModalWnd()
{

}//~CUI_Wnd()

int CUI_ModalWnd::Render()
{
//this is the only way to get this to work.
//if the lbutton is up, set m_is_resizing unconditionally to false
    if(!GG::App::GetApp()->MouseButtonDown(0))
        m_is_resizing = false;

    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_title,
        true, m_minimize, Resizable(), m_is_minimized);
    return 0;
}//Render()

void CUI_ModalWnd::OnCloseClick()
{
    //call protected member for user-defined behavior:
    OnClose();
    //close the window if it gets clicked
    //first remove from parent
    m_done = true;
}//OnCloseClick()

void CUI_ModalWnd::OnMinimizeClick()
{
    static std::list<GG::Wnd*> s_children;    //this keeps track of the lost children when we minimize
    //this enables us to pass messages down through the window
    
    if(!m_is_minimized)
    {
        //resize the window to only 15 pixels high
        
        //save old lowerright
        m_lower_right = GG::Pt(LowerRight().x, LowerRight().y);
       //move children
        OffsetChildren(GG::Pt(UpperLeft().x + CUI_Wnd::S_MINIMIZED_WND_LENGTH - LowerRight().x, 0));
        //resize the window to keep messages from being sent to it when you click below
        SizeMove(UpperLeft().x, UpperLeft().y, UpperLeft().x + CUI_Wnd::S_MINIMIZED_WND_LENGTH, UpperLeft().y + 15);
       
        
        //do some tricky logic to get the children to stay hidden
//        Hide(true);        //hide everything
//        Show(false);       //then show this window without its children
        m_is_minimized = true;
        //keep the list of children alive
        s_children = Children();
        //now detach all children
        DetachChildren();
        
        //re-add the close and minimize buttons
        AttachChild(m_close_button);
        if(m_minimize_button)
            AttachChild(m_minimize_button);
        
    }
    else
    {
        //restore window size
        SizeMove(UpperLeft(), m_lower_right);
        //restore children
        OffsetChildren(GG::Pt(LowerRight().x - UpperLeft().x - CUI_Wnd::S_MINIMIZED_WND_LENGTH, 0));
//      Show(true);
//      if(m_minimize_button)
//          m_minimize_button->Hide();
//      m_close_button->Hide();
        m_is_minimized = false;
        if(s_children.empty())
            return;                //get out if the list of children is empty
            
        //we need to re-add the children by iterating over the list
        for (std::list<GG::Wnd*>::iterator it = s_children.begin(); it != s_children.end(); ++it) 
        {
            if(*it && (*it != m_close_button && *it != m_minimize_button))    
                AttachChild(*it);
        }
        s_children.clear();        
    }

    //call protected member for user-defined behavior:
    OnMinimize();

}//OnMinimizeClick()

void CUI_ModalWnd::OnResizeClick(int x, int y)
{
    
    //call protected member for user-defined behavior:
    OnResize(x,y);
    //TODO: Implementation
}//OnResize()

int CUI_ModalWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    using namespace std;
    //define a rect where the resize area is
    GG::Rect rect(LowerRight().x-10, LowerRight().y-10, LowerRight().x, LowerRight().y);
    //if drag started in lower left and this is a resizable window
    if(!m_is_minimized && Resizable() && (m_is_resizing || rect.Contains(pt)))
    {
        SizeMove(UpperLeft(), LowerRight() + move);
        //move the close and resize windows
        OffsetChildren(move);
        //boolean value to move
        m_is_resizing = true;

        return 1;
    }
    
    m_is_resizing = false;
    
    if(m_is_minimized)
    {
        m_lower_right += move;
    }
    return GG::ModalWnd::LDrag(pt,move,keys);

}//LDrag()

void CUI_ModalWnd::OffsetChildren(const GG::Pt& move)
{
    m_close_button->OffsetMove(move.x, 0);
    if(m_minimize_button)
        m_minimize_button->OffsetMove(move.x, 0);  
}//OffsetChildren
void CUI_ModalWnd::OnResize(int x, int y){}
void CUI_ModalWnd::OnClose(){}
void CUI_ModalWnd::OnMinimize(){}


////////////////////////////////////////////////////////////////////////////////////////////////
//CUI_Wnd//////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

//statics

int CUI_Wnd::S_MINIMIZED_WND_LENGTH = 150;

CUI_Wnd::CUI_Wnd(const std::string& t, int x, int y, int w, int h, Uint32 flags):
    GG::Wnd(x,y,w,h,flags),
    m_title(t),
    m_is_resizing(false),
    m_is_minimized(false),
    m_close_button(NULL),
    m_minimize_button(NULL)
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
    
    //first create the close button
    m_close_button = new GG::Button(w-15, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
    GG::Connect(m_close_button->ClickedSignal(), &CUI_Wnd::OnCloseClick, this);
    AttachChild(m_close_button);
    //make the button invisible....yet still active
    m_close_button->Hide();
    
    //do the minimize button
    if(m_minimize)
    {
        m_minimize_button = new GG::Button(w-30, 3, 7, 7, "", ClientUI::FONT, ClientUI::PTS, ClientUI::CTRL_COLOR);
        GG::Connect(m_minimize_button->ClickedSignal(), &CUI_Wnd::OnMinimizeClick, this);
        AttachChild(m_minimize_button);      
        m_minimize_button->Hide();
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
//this is the only way to get this to work.
//if the lbutton is up, set m_is_resizing unconditionally to false
    if(!GG::App::GetApp()->MouseButtonDown(0))
        m_is_resizing = false;

    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, m_title,
        true, m_minimize, Resizable(), m_is_minimized);
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
    static std::list<GG::Wnd*> s_children;    //this keeps track of the lost children when we minimize
    //this enables us to pass messages down through the window
    
    if(!m_is_minimized)
    {
        //resize the window to only 15 pixels high
        
        //save old lowerright
        m_lower_right = GG::Pt(LowerRight().x, LowerRight().y);
       //move children
        OffsetChildren(GG::Pt(UpperLeft().x + S_MINIMIZED_WND_LENGTH - LowerRight().x, 0));
        //resize the window to keep messages from being sent to it when you click below
        SizeMove(UpperLeft().x, UpperLeft().y, UpperLeft().x + S_MINIMIZED_WND_LENGTH, UpperLeft().y + 15);
       
        
        //do some tricky logic to get the children to stay hidden
//        Hide(true);        //hide everything
//        Show(false);       //then show this window without its children
        m_is_minimized = true;
        //keep the list of children alive
        s_children = Children();
        //now detach all children
        DetachChildren();
        
        //re-add the close and minimize buttons
        AttachChild(m_close_button);
        if(m_minimize_button)
            AttachChild(m_minimize_button);
        
    }
    else
    {
        //restore window size
        SizeMove(UpperLeft(), m_lower_right);
        //restore children
        OffsetChildren(GG::Pt(LowerRight().x - UpperLeft().x - S_MINIMIZED_WND_LENGTH, 0));
//      Show(true);
//      if(m_minimize_button)
//          m_minimize_button->Hide();
//      m_close_button->Hide();
        m_is_minimized = false;
        if(s_children.empty())
            return;                //get out if the list of children is empty
            
        //we need to re-add the children by iterating over the list
        for (std::list<GG::Wnd*>::iterator it = s_children.begin(); it != s_children.end(); ++it) 
        {
            if(*it && (*it != m_close_button && *it != m_minimize_button))    
                AttachChild(*it);
        }
        s_children.clear();        
    }

    //call protected member for user-defined behavior:
    OnMinimize();

}//OnMinimizeClick()

void CUI_Wnd::OnResizeClick(int x, int y)
{
    
    //call protected member for user-defined behavior:
    OnResize(x,y);
    //TODO: Implementation
}//OnResize()

int CUI_Wnd::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    using namespace std;
    //define a rect where the resize area is
    GG::Rect rect(LowerRight().x-10, LowerRight().y-10, LowerRight().x, LowerRight().y);
    //if drag started in lower left and this is a resizable window
    if(!m_is_minimized && Resizable() && (m_is_resizing || rect.Contains(pt)))
    {
        SizeMove(UpperLeft(), LowerRight() + move);
        //move the close and resize windows
        OffsetChildren(move);
        //boolean value to move
        m_is_resizing = true;

        return 1;
    }
    
    m_is_resizing = false;
    
    if(m_is_minimized)
    {
        m_lower_right += move;
    }
    return GG::Wnd::LDrag(pt,move,keys);

}//LDrag()

void CUI_Wnd::OffsetChildren(const GG::Pt& move)
{
    m_close_button->OffsetMove(move.x, 0);
    if(m_minimize_button)
        m_minimize_button->OffsetMove(move.x, 0);  
}//OffsetChildren

//these functions do nothing by default
void CUI_Wnd::OnResize(int x, int y){}
void CUI_Wnd::OnClose(){}
void CUI_Wnd::OnMinimize(){}

