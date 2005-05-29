//CUI_Wnd.cpp

#include "CUI_Wnd.h"

#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "GGApp.h"
#include "GGDrawUtil.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

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

namespace {
    bool PlaySounds()
    {
        return GetOptionsDB().Get<bool>("UI.sound.enabled");
    }
    const std::string& SoundDir()
    {
        static std::string retval;
        if (retval == "") {
            retval = GetOptionsDB().Get<std::string>("settings-dir");
            if (!retval.empty() && retval[retval.size() - 1] != '/')
                retval += '/';
            retval += "data/sound/";
        }
        return retval;
    }
    void PlayMinimizeSound()
    {
#ifndef FREEORION_BUILD_UTIL
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-maximize"));
#endif
    }
    void PlayMaximizeSound()
    {
#ifndef FREEORION_BUILD_UTIL
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-minimize"));
#endif
    }
    void PlayCloseSound()
    {
#ifndef FREEORION_BUILD_UTIL
        if (PlaySounds()) HumanClientApp::GetApp()->PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-close"));
#endif
    }

    bool temp_header_bool = RecordHeaderFile(CUI_WndRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

////////////////////////////////////////////////
// CUI_MinRestoreButton
////////////////////////////////////////////////
CUI_MinRestoreButton::CUI_MinRestoreButton(int x, int y) : 
    GG::Button(x, y, 7, 7, "", "", 0, ClientUI::WND_INNER_BORDER_COLOR),
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
   
bool CUI_MinRestoreButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    if (m_mode == MIN_BUTTON) {
        // draw a dash to signify the minimize command
        int middle_y = (lr.y + ul.y) / 2;
        glDisable(GL_TEXTURE_2D);
        glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
        glBegin(GL_LINES);
        glVertex2i(ul.x, middle_y);
        glVertex2i(lr.x, middle_y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    } else {
        // draw a square to signify the restore command
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, ClientUI::WND_INNER_BORDER_COLOR, 1);
    }
    return true;
}

void CUI_MinRestoreButton::Toggle()
{
    if (m_mode == MIN_BUTTON) {
        PlayMinimizeSound();
        m_mode = RESTORE_BUTTON;
    } else {
        PlayMaximizeSound();
        m_mode = MIN_BUTTON;
    }
}


////////////////////////////////////////////////
// CUI_CloseButton
////////////////////////////////////////////////
CUI_CloseButton::CUI_CloseButton(int x, int y) : 
    GG::Button(x, y, 7, 7, "", "", 0, ClientUI::WND_INNER_BORDER_COLOR)
{
    GG::Connect(ClickedSignal(), &PlayCloseSound, -1);
}

CUI_CloseButton::CUI_CloseButton(const GG::XMLElement& elem) : 
    GG::Button(elem.Child("GG::Button"))
{
    if (elem.Tag() != "CUI_CloseButton")
        throw std::invalid_argument("Attempted to construct a CUI_CloseButton from an XMLElement that had a tag other than \"CUI_CloseButton\"");

    GG::Connect(ClickedSignal(), &PlayCloseSound, -1);
}

GG::XMLElement CUI_CloseButton::XMLEncode() const
{
    GG::XMLElement retval("CUI_CloseButton");
    retval.AppendChild(GG::Button::XMLEncode());
    return retval;
}
   
bool CUI_CloseButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    glDisable(GL_TEXTURE_2D);
    glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
    // this is slightly less efficient than using GL_LINES, but the lines are rasterized differently on different 
    // OpengGL implementaions, so we do it this way to produce the "x" we want
    glBegin(GL_POINTS);
    for (int i = 0; i < GG::Wnd::Width(); ++i) {
        glVertex2d(ul.x + i, ul.y + i + 0.5);
    }
    for (int i = 0; i < GG::Wnd::Width(); ++i) {
        if (i != GG::Wnd::Width() / 2)
            glVertex2d(lr.x - i - 1, ul.y + i + 0.5);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    return true;
}


////////////////////////////////////////////////
// CUI_Wnd
////////////////////////////////////////////////

namespace {
const int BUTTON_TOP_OFFSET = 3;
const int BUTTON_RIGHT_OFFSET = 15;
const int MINIMIZED_WND_LENGTH = 150;
const int BORDER_LEFT = 5;
const int BORDER_TOP = 18;
const int BORDER_RIGHT = 5;
const int BORDER_BOTTOM = 5;
const int OUTER_EDGE_ANGLE_OFFSET = 8;
const int INNER_BORDER_ANGLE_OFFSET = 11;
const int RESIZE_HASHMARK1_OFFSET = 7;
const int RESIZE_HASHMARK2_OFFSET = 3;
}

CUI_Wnd::CUI_Wnd(const std::string& t, int x, int y, int w, int h, Uint32 flags) : 
    GG::Wnd(x, y, w, h, flags & ~GG::Wnd::RESIZABLE),
    m_resizable (flags & GG::Wnd::RESIZABLE),
    m_closable(flags & CLOSABLE),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_drag_offset(-1, -1),
    m_close_button(0),
    m_minimize_button(0)
{
    // set window text
    SetText(t);
    // call to CUI_Wnd::MinimizedLength() because MinimizedLength is virtual
    SetMinSize(GG::Pt(CUI_Wnd::MinimizedLength(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 50));
    InitButtons();
    EnableChildClipping(true);
}

CUI_Wnd::CUI_Wnd(const GG::XMLElement& elem) : 
    GG::Wnd(elem.Child("GG::Wnd"))
{
    if (elem.Tag() != "CUI_Wnd")
        throw std::invalid_argument("Attempted to construct a CUI_Wnd from an XMLElement that had a tag other than \"CUI_Wnd\"");

    m_resizable = boost::lexical_cast<bool>(elem.Child("m_resizable").Attribute("value"));
    m_closable = boost::lexical_cast<bool>(elem.Child("m_closable").Attribute("value"));
    m_minimizable = boost::lexical_cast<bool>(elem.Child("m_minimizable").Attribute("value"));
    m_minimized = boost::lexical_cast<bool>(elem.Child("m_minimized").Attribute("value"));

    InitButtons();
}

CUI_Wnd::~CUI_Wnd()
{
}

GG::XMLElement CUI_Wnd::XMLEncode() const
{
    GG::XMLElement retval("CUI_Wnd");
    retval.AppendChild(GG::Wnd::XMLEncode());

    GG::XMLElement temp;

    temp = GG::XMLElement("m_resizable");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_resizable));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_closable");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_closable));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_minimizable");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_minimizable));
    retval.AppendChild(temp);

    temp = GG::XMLElement("m_minimized");
    temp.SetAttribute("value", boost::lexical_cast<std::string>(m_minimized));
    retval.AppendChild(temp);

    return retval;
}

void CUI_Wnd::SizeMove(int x1, int y1, int x2, int y2)
{
    Wnd::SizeMove(x1, y1, x2, y2);
    if (Width() < MinSize().x)
        Resize(MinSize().x, Height());
    if (MaxSize().x < Width())
        Resize(MaxSize().x, Height());

    if (Height() < MinSize().y)
        Resize(Width(), MinSize().y);
    if (MaxSize().y < Height())
        Resize(Width(), MaxSize().y);

    if (m_close_button)
        m_close_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
    if (m_minimize_button)
        m_minimize_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET * (m_close_button ? 2 : 1), BUTTON_TOP_OFFSET);
}

bool CUI_Wnd::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ul + GG::Pt(BORDER_LEFT, BORDER_TOP);
    GG::Pt cl_lr = lr - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);

    if (!m_minimized) {
        // use GL to draw the lines
        glDisable(GL_TEXTURE_2D);
        GLint initial_modes[2];
        glGetIntegerv(GL_POLYGON_MODE, initial_modes);

        // draw background
        glPolygonMode(GL_BACK, GL_FILL);
        glBegin(GL_POLYGON);
            glColor4ubv(ClientUI::WND_COLOR.v);
            glVertex2i(ul.x, ul.y);
            glVertex2i(lr.x, ul.y);
            glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
            glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
            glVertex2i(ul.x, lr.y);
            glVertex2i(ul.x, ul.y);
        glEnd();

        // draw outer border on pixel inside of the outer edge of the window
        glPolygonMode(GL_BACK, GL_LINE);
        glBegin(GL_POLYGON);
            glColor4ubv(ClientUI::WND_OUTER_BORDER_COLOR.v);
            glVertex2i(ul.x, ul.y);
            glVertex2i(lr.x, ul.y);
            glVertex2i(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
            glVertex2i(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
            glVertex2i(ul.x, lr.y);
            glVertex2i(ul.x, ul.y);
        glEnd();

        // reset this to whatever it was initially
        glPolygonMode(GL_BACK, initial_modes[1]);

        // draw inner border, including extra resize-tab lines
        glBegin(GL_LINE_STRIP);
            glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
            glVertex2i(cl_ul.x, cl_ul.y);
            glVertex2i(cl_lr.x, cl_ul.y);
            glVertex2i(cl_lr.x, cl_lr.y - INNER_BORDER_ANGLE_OFFSET);
            glVertex2i(cl_lr.x - INNER_BORDER_ANGLE_OFFSET, cl_lr.y);
            glVertex2i(cl_ul.x, cl_lr.y);
            glVertex2i(cl_ul.x, cl_ul.y);
        glEnd();
        glBegin(GL_LINES);
            // draw the extra lines of the resize tab
            if (m_resizable) {
                glColor4ubv(ClientUI::WND_INNER_BORDER_COLOR.v);
            } else {
                glColor4ubv(GG::DisabledColor(ClientUI::WND_INNER_BORDER_COLOR).v);
            }
            glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK1_OFFSET);
            glVertex2i(cl_lr.x - RESIZE_HASHMARK1_OFFSET, cl_lr.y);
            
            glVertex2i(cl_lr.x, cl_lr.y - RESIZE_HASHMARK2_OFFSET);
            glVertex2i(cl_lr.x - RESIZE_HASHMARK2_OFFSET, cl_lr.y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    } else {
        GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WND_COLOR, ClientUI::WND_OUTER_BORDER_COLOR, 1);
    }

    glColor4ubv(ClientUI::TEXT_COLOR.v);
    boost::shared_ptr<GG::Font> font = GG::App::GetApp()->GetFont(ClientUI::TITLE_FONT, ClientUI::TITLE_PTS);
    font->RenderText(ul.x + BORDER_LEFT, ul.y, WindowText());

    return true;
}

void CUI_Wnd::LButtonDown(const GG::Pt& pt, Uint32 keys)
{
    if (!m_minimized && m_resizable) {
        GG::Pt cl_lr = LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);
        GG::Pt dist_from_lr = cl_lr - pt;
        if (dist_from_lr.x + dist_from_lr.y <= INNER_BORDER_ANGLE_OFFSET) {
            m_drag_offset = pt - LowerRight();
        }
    }
}

void CUI_Wnd::LDrag(const GG::Pt& pt, const GG::Pt& move, Uint32 keys)
{
    if (m_drag_offset != GG::Pt(-1, -1)) { // resize-dragging
        Resize((pt - m_drag_offset) - UpperLeft());
    } else { // normal-dragging
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        if ((0 <= ul.x + move.x) && (lr.x + move.x < GG::App::GetApp()->AppWidth()) &&
            (0 <= ul.y + move.y) && (lr.y + move.y < GG::App::GetApp()->AppHeight()))
            GG::Wnd::LDrag(pt, move, keys);
    }
}

void CUI_Wnd::LButtonUp(const GG::Pt& pt, Uint32 keys)
{
    m_drag_offset = GG::Pt(-1, -1);
}

bool CUI_Wnd::InWindow(const GG::Pt& pt) const
{
    GG::Pt lr = LowerRight();
    GG::Pt dist_from_lr = lr - pt;
    bool inside_lower_right_corner = OUTER_EDGE_ANGLE_OFFSET < dist_from_lr.x + dist_from_lr.y;
    return (UpperLeft() <= pt && pt < LowerRight() && inside_lower_right_corner);
}

void CUI_Wnd::InitButtons()
{
    // create the close button
    if (m_closable) {
        m_close_button = new CUI_CloseButton(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
        GG::Connect(m_close_button->ClickedSignal(), &CUI_Wnd::CloseClicked, this);
        AttachChild(m_close_button);
    }

    // create the minimize button
    if (m_minimizable) {
        m_minimize_button = new CUI_MinRestoreButton(Width() - BUTTON_RIGHT_OFFSET * (m_close_button ? 2 : 1), BUTTON_TOP_OFFSET);
        GG::Connect(m_minimize_button->ClickedSignal(), &CUI_Wnd::MinimizeClicked, this);
        AttachChild(m_minimize_button);      
    }    
}

int CUI_Wnd::MinimizedLength() const 
{
    return MINIMIZED_WND_LENGTH;
}

int CUI_Wnd::LeftBorder() const
{
    return BORDER_LEFT;
}

int CUI_Wnd::TopBorder() const
{
    return BORDER_TOP;
}

int CUI_Wnd::RightBorder() const
{
    return BORDER_RIGHT;
}

int CUI_Wnd::BottomBorder() const
{
    return BORDER_BOTTOM;
}

int CUI_Wnd::InnerBorderAngleOffset() const
{
    return INNER_BORDER_ANGLE_OFFSET;
}

void CUI_Wnd::CloseClicked()
{
    m_done = true;
    if (Parent()) {
        Parent()->DeleteChild(this);
    } else {
        GG::App::GetApp()->Remove(this);
    }
}

void CUI_Wnd::MinimizeClicked()
{
    if (!m_minimized) {
        m_original_size = Size();
        SetMinSize(GG::Pt(MinimizedLength(), BORDER_TOP));
        Resize(MINIMIZED_WND_LENGTH, BORDER_TOP);
        if (m_close_button)
            m_close_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
        if (m_minimize_button)
            m_minimize_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET * (m_close_button ? 2 : 1), BUTTON_TOP_OFFSET);
        Hide();
        Show(false);
        if (m_close_button)
            m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
        m_minimized = true;
    } else {
        SetMinSize(GG::Pt(MinimizedLength(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM));
        Resize(m_original_size);
        if (m_close_button)
            m_close_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
        if (m_minimize_button)
            m_minimize_button->MoveTo(Width() - BUTTON_RIGHT_OFFSET * (m_close_button ? 2 : 1), BUTTON_TOP_OFFSET);
        Show();
        m_minimized = false;
    }
}

///////////////////////////////////////
// class CUIEditWnd
///////////////////////////////////////

CUIEditWnd::CUIEditWnd(int w, const std::string& prompt_text, const std::string& edit_text, Uint32 flags/* = Wnd::MODAL*/) : 
    CUI_Wnd(prompt_text, 0, 0, 1, 1, flags)
{
    m_edit = new CUIEdit(LeftBorder() + 3, TopBorder() + 3, w - 2 * BUTTON_WIDTH - 2 * CONTROL_MARGIN - 6 - LeftBorder() - RightBorder(), ClientUI::PTS + 10, edit_text);
    m_ok_bn = new CUIButton(m_edit->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, UserString("OK"));
    m_cancel_bn = new CUIButton(m_ok_bn->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, UserString("CANCEL"));

    Resize(w, std::max(m_edit->LowerRight().y, m_cancel_bn->LowerRight().y) + BottomBorder() + 3);
    MoveTo((GG::App::GetApp()->AppWidth() - w) / 2, (GG::App::GetApp()->AppHeight() - Height()) / 2);

    AttachChild(m_edit);
    AttachChild(m_ok_bn);
    AttachChild(m_cancel_bn);

    GG::Connect(m_ok_bn->ClickedSignal(), &CUIEditWnd::OkClicked, this);
    GG::Connect(m_cancel_bn->ClickedSignal(), &CUI_Wnd::CloseClicked, static_cast<CUI_Wnd*>(this));

    m_edit->SelectAll();
}

void CUIEditWnd::ModalInit()
{
    GG::App::GetApp()->SetFocusWnd(m_edit);
}

void CUIEditWnd::Keypress(GG::Key key, Uint32 key_mods)
{
    switch (key) {
    case GG::GGK_RETURN: if (!m_ok_bn->Disabled()) OkClicked(); break;
    case GG::GGK_ESCAPE: CloseClicked(); break;
    default: break;
    }
}

const std::string& CUIEditWnd::Result() const 
{
    return m_result;
}

void CUIEditWnd::OkClicked() 
{
    m_result = m_edit->WindowText(); CloseClicked();
}
