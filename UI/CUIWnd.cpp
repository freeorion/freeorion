//CUIWnd.cpp

#include "CUIWnd.h"

#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>


namespace {
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
    { Sound::GetSound().PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-maximize"), true); }
    void PlayMaximizeSound()
    { Sound::GetSound().PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-minimize"), true); }
    void PlayCloseSound()
    { Sound::GetSound().PlaySound(SoundDir() + GetOptionsDB().Get<std::string>("UI.sound.window-close"), true); }

    const double BUTTON_DIMMING_SCALE_FACTOR = 0.75;
}

////////////////////////////////////////////////
// CUI_MinRestoreButton
////////////////////////////////////////////////
CUI_MinRestoreButton::CUI_MinRestoreButton(GG::X x, GG::Y y) : 
    GG::Button(x, y, GG::X(12), GG::Y(12), "", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor()),
    m_mode(MIN_BUTTON)
{
    GG::Connect(ClickedSignal, &CUI_MinRestoreButton::Toggle, this);
}

void CUI_MinRestoreButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Clr color_to_use = ClientUI::WndInnerBorderColor();
    if (State() != BN_ROLLOVER)
        AdjustBrightness(color_to_use, BUTTON_DIMMING_SCALE_FACTOR);
    if (m_mode == MIN_BUTTON) {
        // draw a dash to signify the minimize command
        GG::Y middle_y = (lr.y + ul.y) / 2;
        glDisable(GL_TEXTURE_2D);
        glColor(color_to_use);
        glBegin(GL_LINES);
        glVertex(ul.x, middle_y);
        glVertex(lr.x, middle_y);
        glEnd();
        glEnable(GL_TEXTURE_2D);
    } else {
        // draw a square to signify the restore command
        GG::FlatRectangle(ul, lr, GG::CLR_ZERO, ClientUI::WndInnerBorderColor(), 1);
    }
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
CUI_CloseButton::CUI_CloseButton(GG::X x, GG::Y y) : 
    GG::Button(x, y, GG::X(12), GG::Y(12), "", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor())
{
    GG::Connect(ClickedSignal, &PlayCloseSound, -1);
}

void CUI_CloseButton::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Clr color_to_use = ClientUI::WndInnerBorderColor();
    if (State() != BN_ROLLOVER)
        AdjustBrightness(color_to_use, BUTTON_DIMMING_SCALE_FACTOR);
    glDisable(GL_TEXTURE_2D);
    glColor(color_to_use);
    // this is slightly less efficient than using GL_LINES, but the lines are rasterized differently on different 
    // OpengGL implementaions, so we do it this way to produce the "x" we want
    glBegin(GL_POINTS);
    for (int i = 0; i < GG::Wnd::Width(); ++i) {
        glVertex(ul.x + i, ul.y + i + 0.5);
    }
    for (int i = 0; i < GG::Wnd::Width(); ++i) {
        if (i != GG::Wnd::Width() / 2)
            glVertex(lr.x - i - 1, ul.y + i + 0.5);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
}


////////////////////////////////////////////////
// CUIWnd
////////////////////////////////////////////////
GG::WndFlag MINIMIZABLE(1 << 10);
GG::WndFlag CLOSABLE(1 << 11);

namespace {
    bool RegisterWndFlags()
    {
        GG::FlagSpec<GG::WndFlag>::instance().insert(MINIMIZABLE, "MINIMIZABLE");
        GG::FlagSpec<GG::WndFlag>::instance().insert(CLOSABLE, "CLOSABLE");
        return true;
    }
    bool dummy = RegisterWndFlags();
}

const GG::Y CUIWnd::BUTTON_TOP_OFFSET(3);
const GG::X CUIWnd::BUTTON_RIGHT_OFFSET(15);
const GG::X CUIWnd::MINIMIZED_WND_WIDTH(150);
const GG::X CUIWnd::BORDER_LEFT(5);
const GG::Y CUIWnd::BORDER_TOP(18);
const GG::X CUIWnd::BORDER_RIGHT(5);
const GG::Y CUIWnd::BORDER_BOTTOM(5);
const int CUIWnd::OUTER_EDGE_ANGLE_OFFSET = 8;
const int CUIWnd::INNER_BORDER_ANGLE_OFFSET = 11;
const int CUIWnd::RESIZE_HASHMARK1_OFFSET = 7;
const int CUIWnd::RESIZE_HASHMARK2_OFFSET = 3;

CUIWnd::CUIWnd(const std::string& t, GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Flags<GG::WndFlag> flags) : 
    GG::Wnd(x, y, w, h, flags & ~GG::RESIZABLE),
    m_resizable(flags & GG::RESIZABLE),
    m_closable(flags & CLOSABLE),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_drag_offset(-GG::X1, -GG::Y1),
    m_close_button(0),
    m_minimize_button(0)
{
    // set window name
    SetName(t);
    // call to CUIWnd::MinimizedWidth() because MinimizedWidth is virtual
    SetMinSize(GG::Pt(CUIWnd::MinimizedWidth(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 50));
    InitButtons();
    EnableChildClipping(true);
}

CUIWnd::~CUIWnd()
{}

void CUIWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();
    if (m_close_button)
        m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
    if (m_minimize_button)
        m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y));
}

void CUIWnd::Render()
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    GG::Pt cl_ul = ClientUpperLeft();
    GG::Pt cl_lr = ClientLowerRight();

    if (!m_minimized) {
        // use GL to draw the lines
        glDisable(GL_TEXTURE_2D);
        GLint initial_modes[2];
        glGetIntegerv(GL_POLYGON_MODE, initial_modes);

        // draw background
        glPolygonMode(GL_BACK, GL_FILL);
        glBegin(GL_POLYGON);
            glColor(ClientUI::WndColor());
            glVertex(ul.x, ul.y);
            glVertex(lr.x, ul.y);
            glVertex(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
            glVertex(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
            glVertex(ul.x, lr.y);
            glVertex(ul.x, ul.y);
        glEnd();

        // draw outer border on pixel inside of the outer edge of the window
        glPolygonMode(GL_BACK, GL_LINE);
        glBegin(GL_POLYGON);
            glColor(ClientUI::WndOuterBorderColor());
            glVertex(ul.x, ul.y);
            glVertex(lr.x, ul.y);
            glVertex(lr.x, lr.y - OUTER_EDGE_ANGLE_OFFSET);
            glVertex(lr.x - OUTER_EDGE_ANGLE_OFFSET, lr.y);
            glVertex(ul.x, lr.y);
            glVertex(ul.x, ul.y);
        glEnd();

        // reset this to whatever it was initially
        glPolygonMode(GL_BACK, initial_modes[1]);

        // draw inner border, including extra resize-tab lines
        glBegin(GL_LINE_STRIP);
            glColor(ClientUI::WndInnerBorderColor());
            glVertex(cl_ul.x, cl_ul.y);
            glVertex(cl_lr.x, cl_ul.y);
            if (m_resizable) {
                glVertex(cl_lr.x, cl_lr.y - INNER_BORDER_ANGLE_OFFSET);
                glVertex(cl_lr.x - INNER_BORDER_ANGLE_OFFSET, cl_lr.y);
            } else {
                glVertex(cl_lr.x, cl_lr.y);
            }
            glVertex(cl_ul.x, cl_lr.y);
            glVertex(cl_ul.x, cl_ul.y);
        glEnd();
        if (m_resizable) {
            glBegin(GL_LINES);
                // draw the extra lines of the resize tab
                glColor(ClientUI::WndInnerBorderColor());

                glVertex(cl_lr.x, cl_lr.y - RESIZE_HASHMARK1_OFFSET);
                glVertex(cl_lr.x - RESIZE_HASHMARK1_OFFSET, cl_lr.y);
                
                glVertex(cl_lr.x, cl_lr.y - RESIZE_HASHMARK2_OFFSET);
                glVertex(cl_lr.x - RESIZE_HASHMARK2_OFFSET, cl_lr.y);
            glEnd();
        }
        glEnable(GL_TEXTURE_2D);
    } else {
        GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
    }

    glColor(ClientUI::TextColor());
    boost::shared_ptr<GG::Font> font = ClientUI::GetTitleFont();
    font->RenderText(GG::Pt(ul.x + BORDER_LEFT, ul.y), Name());
}

void CUIWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_minimized && m_resizable) {
        GG::Pt cl_lr = LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);
        GG::Pt dist_from_lr = cl_lr - pt;
        if (Value(dist_from_lr.x) + Value(dist_from_lr.y) <= INNER_BORDER_ANGLE_OFFSET) {
            m_drag_offset = pt - LowerRight();
        }
    }
}

void CUIWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) { // resize-dragging
        Resize((pt - m_drag_offset) - UpperLeft());
    } else { // normal-dragging
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        GG::Pt final_move(std::max(-ul.x, std::min(move.x, GG::GUI::GetGUI()->AppWidth() - 1 - lr.x)),
                          std::max(-ul.y, std::min(move.y, GG::GUI::GetGUI()->AppHeight() - 1 - lr.y)));
        GG::Wnd::LDrag(pt + final_move - move, final_move, mod_keys);
    }
}

void CUIWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_drag_offset = GG::Pt(-GG::X1, -GG::Y1);
}

GG::Pt CUIWnd::ClientUpperLeft() const
{
    return m_minimized ? UpperLeft() : UpperLeft() + GG::Pt(BORDER_LEFT, BORDER_TOP);
}

GG::Pt CUIWnd::ClientLowerRight() const
{
    return m_minimized ? LowerRight() : LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);
}

bool CUIWnd::InWindow(const GG::Pt& pt) const
{
    GG::Pt lr = LowerRight();
    GG::Pt dist_from_lr = lr - pt;
    bool inside_lower_right_corner = OUTER_EDGE_ANGLE_OFFSET < Value(dist_from_lr.x) + Value(dist_from_lr.y);
    return (UpperLeft() <= pt && pt < LowerRight() && inside_lower_right_corner);
}

void CUIWnd::InitButtons()
{
    // create the close button
    GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();
    if (m_closable) {
        m_close_button = new CUI_CloseButton(button_ul.x, button_ul.y);
        GG::Connect(m_close_button->ClickedSignal, &CUIWnd::CloseClicked, this);
        AttachChild(m_close_button);
    }

    // create the minimize button
    if (m_minimizable) {
        m_minimize_button = new CUI_MinRestoreButton(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y);
        GG::Connect(m_minimize_button->ClickedSignal, &CUIWnd::MinimizeClicked, this);
        AttachChild(m_minimize_button);      
    }    
}

GG::X CUIWnd::MinimizedWidth() const
{
    return MINIMIZED_WND_WIDTH;
}

GG::X CUIWnd::LeftBorder() const
{
    return BORDER_LEFT;
}

GG::Y CUIWnd::TopBorder() const
{
    return BORDER_TOP;
}

GG::X CUIWnd::RightBorder() const
{
    return BORDER_RIGHT;
}

GG::Y CUIWnd::BottomBorder() const
{
    return BORDER_BOTTOM;
}

int CUIWnd::InnerBorderAngleOffset() const
{
    return INNER_BORDER_ANGLE_OFFSET;
}

void CUIWnd::CloseClicked()
{
    m_done = true;
    if (Parent())
        Parent()->DetachChild(this);
    else
        GG::GUI::GetGUI()->Remove(this);
}

void CUIWnd::MinimizeClicked()
{
    if (!m_minimized) {
        m_minimized = true;
        m_original_size = Size();
        SetMinSize(GG::Pt(MinimizedWidth(), BORDER_TOP));
        Resize(GG::Pt(MINIMIZED_WND_WIDTH, BORDER_TOP));
        GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET);
        if (m_close_button)
            m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        if (m_minimize_button)
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y));
        Hide();
        Show(false);
        if (m_close_button)
            m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
    } else {
        m_minimized = false;
        SetMinSize(GG::Pt(MinimizedWidth(), BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 10));
        Resize(GG::Pt(m_original_size));
        GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();
        if (m_close_button)
            m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        if (m_minimize_button)
            m_minimize_button->MoveTo(GG::Pt(button_ul.x - (m_close_button ? BUTTON_RIGHT_OFFSET : GG::X0), button_ul.y));
        Show();
    }
}

///////////////////////////////////////
// class CUIEditWnd
///////////////////////////////////////
const GG::X CUIEditWnd::BUTTON_WIDTH(75);
const int CUIEditWnd::CONTROL_MARGIN = 5;

CUIEditWnd::CUIEditWnd(GG::X w, const std::string& prompt_text, const std::string& edit_text, GG::Flags<GG::WndFlag> flags/* = Wnd::MODAL*/) : 
    CUIWnd(prompt_text, GG::X0, GG::Y0, w, GG::Y1, flags)
{
    m_edit = new CUIEdit(LeftBorder() + 3, TopBorder() + 3, ClientWidth() - 2 * BUTTON_WIDTH - 2 * CONTROL_MARGIN - 6 - LeftBorder() - RightBorder(), edit_text);
    m_ok_bn = new CUIButton(m_edit->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, UserString("OK"));
    m_cancel_bn = new CUIButton(m_ok_bn->LowerRight().x + CONTROL_MARGIN, TopBorder() + 3, BUTTON_WIDTH, UserString("CANCEL"));
    m_ok_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));
    m_cancel_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));

    Resize(GG::Pt(w, std::max(m_edit->LowerRight().y, m_cancel_bn->LowerRight().y) + BottomBorder() + 3));
    MoveTo(GG::Pt((GG::GUI::GetGUI()->AppWidth() - w) / 2, (GG::GUI::GetGUI()->AppHeight() - Height()) / 2));

    AttachChild(m_edit);
    AttachChild(m_ok_bn);
    AttachChild(m_cancel_bn);

    GG::Connect(m_ok_bn->ClickedSignal, &CUIEditWnd::OkClicked, this);
    GG::Connect(m_cancel_bn->ClickedSignal, &CUIWnd::CloseClicked, static_cast<CUIWnd*>(this));

    m_edit->SelectAll();
}

void CUIEditWnd::ModalInit()
{
    GG::GUI::GetGUI()->SetFocusWnd(m_edit);
}

void CUIEditWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys)
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
    m_result = m_edit->Text();
    CloseClicked();
}
