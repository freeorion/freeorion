//CUI_Wnd.cpp

#include "CUI_Wnd.h"

#include "ClientUI.h"   //include for DrawWindow....for now
#include "GGApp.h"
#include "GGDrawUtil.h"

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

////////////////////////////////////////////////
// CUI_MinRestoreButton
////////////////////////////////////////////////
CUI_MinRestoreButton::CUI_MinRestoreButton(int x, int y) : 
    GG::Button(x, y, 7, 7, "", "", 0, ClientUI::INNER_BORDER_COLOR),
    m_mode(MIN_BUTTON)
{
    GG::Connect(ClickedSignal(), &CUI_MinRestoreButton::Toggle, this);
}

CUI_MinRestoreButton::CUI_MinRestoreButton(const GG::XMLElement& elem) : 
    GG::Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUI_MinRestoreButton")
        throw std::invalid_argument("Attempted to construct a CUI_MinRestoreButton from an XMLElement that had a tag other than \"CUI_MinRestoreButton\"");

    m_mode = static_cast<Mode>(boost::lexical_cast<int>(elem.Child("m_mode").Attribute("value")));

    GG::Connect(ClickedSignal(), &CUI_MinRestoreButton::Toggle, this);
}

GG::XMLElement CUI_MinRestoreButton::XMLEncode() const
{
    GG::XMLElement retval("CUI_MinRestoreButton");
    retval.AppendChild(GG::Button::XMLEncode());

    GG::XMLElement temp("m_mode");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_mode));
    retval.AppendChild(temp);

    return retval;
}
   
int CUI_MinRestoreButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    if (m_mode == MIN_BUTTON) {
        // draw a dash to signify the minimize command
        int middle_y = (lr.y + ul.y) / 2;
        glDisable(GL_TEXTURE_2D);
        glColor4ubv(ClientUI::INNER_BORDER_COLOR.v);
        glBegin(GL_LINES);
        glVertex2i(ul.x, middle_y);
        glVertex2i(lr.x, middle_y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    } else {
        // draw a square to signify the restore command
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, ClientUI::INNER_BORDER_COLOR, 1);
    }
    return 1;
}

////////////////////////////////////////////////
// CUI_CloseButton
////////////////////////////////////////////////
CUI_CloseButton::CUI_CloseButton(int x, int y) : 
    GG::Button(x, y, 7, 7, "", "", 0, ClientUI::INNER_BORDER_COLOR)
{
}

CUI_CloseButton::CUI_CloseButton(const GG::XMLElement& elem) : 
    GG::Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUI_CloseButton")
        throw std::invalid_argument("Attempted to construct a CUI_CloseButton from an XMLElement that had a tag other than \"CUI_CloseButton\"");
}

GG::XMLElement CUI_CloseButton::XMLEncode() const
{
    GG::XMLElement retval("CUI_CloseButton");
    retval.AppendChild(GG::Button::XMLEncode());
    return retval;
}
   
int CUI_CloseButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    glDisable(GL_TEXTURE_2D);
    glColor4ubv(ClientUI::INNER_BORDER_COLOR.v);
    glBegin(GL_LINES);
    glVertex2i(ul.x, ul.y);
    glVertex2i(lr.x, lr.y);
    glVertex2i(ul.x, lr.y);
    glVertex2i(lr.x, ul.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
    return 1;
}


////////////////////////////////////////////////
// CUI_Wnd
////////////////////////////////////////////////

// statics
int CUI_Wnd::S_MINIMIZED_WND_LENGTH = 150;

CUI_Wnd::CUI_Wnd(const std::string& t, int x, int y, int w, int h, Uint32 flags) : 
    GG::Wnd(x, y, w, h, flags),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_resize_offset(-1, -1),
    m_close_button(0),
    m_minimize_button(0)
{
    // set window text
    SetText(t);

    InitButtons();
}

CUI_Wnd::CUI_Wnd(const GG::XMLElement& elem) : 
    GG::Wnd(elem.Child("GG::Wnd"))
{
    if (elem.Tag() != "CUI_Wnd")
        throw std::invalid_argument("Attempted to construct a CUI_Wnd from an XMLElement that had a tag other than \"CUI_Wnd\"");

    m_minimizable = boost::lexical_cast<bool>(elem.Child("m_minimizable").Attribute("value"));
    m_minimized = boost::lexical_cast<bool>(elem.Child("m_minimized").Attribute("value"));

    InitButtons();
}

CUI_Wnd::~CUI_Wnd()
{
}

GG::XMLElement CUI_Wnd::XMLEncode() const
{
    GG::XMLElement retval;
    retval.AppendChild(GG::Wnd::XMLEncode());

    GG::XMLElement temp;

    temp = GG::XMLElement("m_minimizable");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_minimizable));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_minimized");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_minimized));
    retval.AppendChild(temp);

    return retval;
}

int CUI_Wnd::Render()
{
    ClientUI::DrawWindow(UpperLeft().x, UpperLeft().y, LowerRight().x, LowerRight().y, WindowText(), 
        Resizable() && !m_minimized);
    return 1;
}

int CUI_Wnd::LButtonDown(const GG::Pt& pt, Uint32 keys)
{
    if (!m_minimized && Resizable()) {
        GG::Rect resize_rect(LowerRight() - GG::Pt(10, 10), LowerRight());
        if (resize_rect.Contains(pt))
            m_resize_offset = resize_rect.ul - pt;
    }
    return 1;
}

int CUI_Wnd::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    // if we're resize-dragging
    if (m_resize_offset != GG::Pt(-1, -1)) {
        SizeMove(UpperLeft(), pt + m_resize_offset);
        if (Width() < S_MINIMIZED_WND_LENGTH)
            Resize(S_MINIMIZED_WND_LENGTH, Height());
        if (Height() < 35)
            Resize(Width(), 35);
        m_close_button->MoveTo(Width() - 15, 3);
        if (m_minimize_button)
            m_minimize_button->MoveTo(Width() - 30, 3);
        return 1;
    } else { // if we're normal-dragging
      return GG::Wnd::LDrag(pt, move, keys);
    }
}

int CUI_Wnd::LButtonUp(const GG::Pt& pt, Uint32 keys)
{
    m_resize_offset = GG::Pt(-1, -1);
    return 1;
}

void CUI_Wnd::InitButtons()
{
    // create the close button
    m_close_button = new CUI_CloseButton(Width() - 15, 3);
    GG::Connect(m_close_button->ClickedSignal(), &CUI_Wnd::CloseClicked, this);
    AttachChild(m_close_button);

    // create the minimize button
    if (m_minimizable) {
        m_minimize_button = new CUI_MinRestoreButton(Width() - 30, 3);
        GG::Connect(m_minimize_button->ClickedSignal(), &CUI_Wnd::MinimizeClicked, this);
        AttachChild(m_minimize_button);      
    }    
}

void CUI_Wnd::CloseClicked()
{
    //call protected member for user-defined behavior:
    OnClose();

    if (Parent())
        Parent()->DeleteChild(this);
}

void CUI_Wnd::MinimizeClicked()
{
    if (!m_minimized) {
        m_original_size = WindowDimensions();
        Resize(S_MINIMIZED_WND_LENGTH, 15);
        m_close_button->MoveTo(Width() - 15, 3);
        if (m_minimize_button)
            m_minimize_button->MoveTo(Width() - 30, 3);
        Hide();
        Show(false);
        m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
        m_minimized = true;
    } else {
        Resize(m_original_size);
        m_close_button->MoveTo(Width() - 15, 3);
        if (m_minimize_button)
            m_minimize_button->MoveTo(Width() - 30, 3);
        Show();
        m_minimized = false;
    }

    //call protected member for user-defined behavior:
    OnMinimize();
}

void CUI_Wnd::ResizeClicked(int x, int y)
{
    //call protected member for user-defined behavior:
    OnResize(x,y);
    //TODO: Implementation
}

//these functions do nothing by default
void CUI_Wnd::OnResize(int x, int y){}
void CUI_Wnd::OnClose(){}
void CUI_Wnd::OnMinimize(){}

