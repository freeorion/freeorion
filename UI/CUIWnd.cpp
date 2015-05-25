//CUIWnd.cpp

#include "CUIWnd.h"

#include "../client/human/HumanClientApp.h"
#include "ClientUI.h"
#include "CUIControls.h"
#include "Sound.h"
#include "../util/i18n.h"
#include "../util/OptionsDB.h"
#include "../util/Directories.h"
#include "../util/Logger.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>


namespace {
    void PlayMinimizeSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-maximize"), true); }
    void PlayMaximizeSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-minimize"), true); }
    void PlayCloseSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.window-close"), true); }

    const double BUTTON_DIMMING_SCALE_FACTOR = 0.75;
}

////////////////////////////////////////////////
// CUI_MinRestoreButton
////////////////////////////////////////////////
CUI_MinRestoreButton::CUI_MinRestoreButton() :
    GG::Button("", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor()),
    m_mode(MIN_BUTTON)
{ GG::Connect(LeftClickedSignal, &CUI_MinRestoreButton::Toggle, this); }

void CUI_MinRestoreButton::Render() {
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

void CUI_MinRestoreButton::Toggle() {
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
CUI_CloseButton::CUI_CloseButton() :
    GG::Button("", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor())
{
    GG::Connect(LeftClickedSignal, &PlayCloseSound, -1);
    SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "close.png"   )));
    SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "close_clicked.png"  )));
    SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "close_mouseover.png")));
}

////////////////////////////////////////////////
// CUI_PinButton
////////////////////////////////////////////////
CUI_PinButton::CUI_PinButton() :
    GG::Button("", boost::shared_ptr<GG::Font>(), ClientUI::WndInnerBorderColor())
{
    GG::Connect(LeftClickedSignal, &PlayCloseSound, -1);
    SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png"   )));
    SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png"  )));
    SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin_mouseover.png")));
}

void CUI_PinButton::Toggle(bool pinned) {
    if (!pinned) {
        SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png")));
        SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin.png")));
        SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pin_mouseover.png")));
    } else {
        SetUnpressedGraphic(GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned.png")));
        SetPressedGraphic  (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned.png")));
        SetRolloverGraphic (GG::SubTexture(ClientUI::GetTexture( ClientUI::ArtDir() / "icons" / "buttons" / "pinned_mouseover.png")));
    }
}

////////////////////////////////////////////////
// CUIWnd
////////////////////////////////////////////////
GG::WndFlag MINIMIZABLE(1 << 10);
GG::WndFlag CLOSABLE(1 << 11);
GG::WndFlag PINABLE(1 << 12);

namespace {
    bool RegisterWndFlags() {
        GG::FlagSpec<GG::WndFlag>::instance().insert(MINIMIZABLE, "MINIMIZABLE");
        GG::FlagSpec<GG::WndFlag>::instance().insert(CLOSABLE, "CLOSABLE");
        GG::FlagSpec<GG::WndFlag>::instance().insert(PINABLE, "PINABLE");
        return true;
    }
    bool dummy = RegisterWndFlags();
}

const GG::Y CUIWnd::BUTTON_TOP_OFFSET(3);
const GG::X CUIWnd::BUTTON_RIGHT_OFFSET(15);
const GG::X CUIWnd::MINIMIZED_WND_WIDTH(150);
const GG::X CUIWnd::BORDER_LEFT(5);
const GG::Y CUIWnd::BORDER_TOP(21);
const GG::X CUIWnd::BORDER_RIGHT(5);
const GG::Y CUIWnd::BORDER_BOTTOM(5);
const int CUIWnd::OUTER_EDGE_ANGLE_OFFSET = 8;
const int CUIWnd::INNER_BORDER_ANGLE_OFFSET = 11;
const int CUIWnd::TITLE_OFFSET = 3;
const int CUIWnd::RESIZE_HASHMARK1_OFFSET = 7;
const int CUIWnd::RESIZE_HASHMARK2_OFFSET = 3;

CUIWnd::CUIWnd(const std::string& t, GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Flags<GG::WndFlag> flags) :
    GG::Wnd(x, y, w, h, flags & ~GG::RESIZABLE),
    m_resizable(flags & GG::RESIZABLE),
    m_closable(flags & CLOSABLE),
    m_minimizable(flags & MINIMIZABLE),
    m_minimized(false),
    m_pinable(flags & PINABLE),
    m_pinned(false),
    m_drag_offset(-GG::X1, -GG::Y1),
    m_mouse_in_resize_tab(false),
    m_close_button(0),
    m_minimize_button(0),
    m_pin_button(0)
{
    // set window name
    SetName(t);
    // call to CUIWnd::MinimizedWidth() because MinimizedWidth is virtual
    SetMinSize(GG::Pt(CUIWnd::MinimizedSize().x, BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 50));
    InitButtons();
    SetChildClippingMode(ClipToClientAndWindowSeparately);
    ValidatePosition();
    InitBuffers();
}

CUIWnd::~CUIWnd() {
    m_minimized_buffer.clear();
    m_outer_border_buffer.clear();
    m_inner_border_buffer.clear();
    m_resize_corner_lines_buffer.clear();
}

void CUIWnd::ValidatePosition()
{ SizeMove(UpperLeft(), LowerRight()); }

void CUIWnd::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_sz = Size();
    GG::Pt available_size;

    if (const GG::Wnd* parent = Parent()) {
        // Keep this CUIWnd entirely inside its parent.
        available_size = parent->ClientSize();
    } else if (const HumanClientApp* app = HumanClientApp::GetApp()) {
        // Keep this CUIWnd entirely inside the application window.
        available_size = GG::Pt( app->AppWidth(), app->AppHeight() );
    } else {
        ErrorLogger() << "CUIWnd::SizeMove() could not get app instance!";
        return;
    }

    // Limit window size to be no larger than the containing window.
    GG::Pt new_size( std::min(lr.x - ul.x, available_size.x),
                     std::min(lr.y - ul.y, available_size.y) );

    // Clamp position of this window to keep its entire area visible in the
    // containing window.
    GG::Pt new_ul( std::min( available_size.x - new_size.x,
                             std::max(GG::X0, ul.x) ),
                   std::min( available_size.y - new_size.y,
                             std::max(GG::Y0, ul.y) ));

    Wnd::SizeMove(new_ul, new_ul + new_size);
    if (Size() != old_sz) {
        PositionButtons();
        InitBuffers();
    }
}

void CUIWnd::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);

    if (m_minimized) {
        m_minimized_buffer.activate();
        glColor(ClientUI::WndColor());
        glDrawArrays(GL_TRIANGLE_FAN,   0, m_minimized_buffer.size() - 1);
        glColor(ClientUI::WndOuterBorderColor());
        glDrawArrays(GL_LINE_STRIP,     0, m_minimized_buffer.size());

    } else {
        m_outer_border_buffer.activate();
        glColor(ClientUI::WndColor());
        glDrawArrays(GL_TRIANGLE_FAN,   0, m_outer_border_buffer.size() - 1);
        glColor(ClientUI::WndOuterBorderColor());
        glDrawArrays(GL_LINE_STRIP,     0, m_outer_border_buffer.size());

        m_inner_border_buffer.activate();
        glColor(ClientUI::WndInnerBorderColor());
        glDrawArrays(GL_LINE_STRIP,     0, m_inner_border_buffer.size());

        if (m_resizable) {
            m_resize_corner_lines_buffer.activate();
            GG::Clr tab_lines_colour = m_mouse_in_resize_tab ? ClientUI::WndInnerBorderColor() : ClientUI::WndOuterBorderColor();
            glColor(tab_lines_colour);
            glDrawArrays(GL_LINES,          0, m_resize_corner_lines_buffer.size());
        }
    }

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    GG::BeginScissorClipping(ul, lr);
    glColor(ClientUI::TextColor());
    boost::shared_ptr<GG::Font> font = ClientUI::GetTitleFont();
    font->RenderText(GG::Pt(ul.x + BORDER_LEFT, ul.y + TITLE_OFFSET), Name());
    GG::EndScissorClipping();
}

void CUIWnd::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!InResizeTab(pt))
        return;
    m_drag_offset = pt - LowerRight();
}

bool CUIWnd::InResizeTab(const GG::Pt& pt) const {
    if (!m_resizable || m_minimized)
        return false;

    GG::Pt cl_lr = LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM);
    GG::Pt dist_from_lr = cl_lr - pt;
    if (Value(dist_from_lr.x) + Value(dist_from_lr.y) <= INNER_BORDER_ANGLE_OFFSET)
        return true;

    return false;
}

void CUIWnd::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys) {
    if (m_pinned)
        return;

    if (m_drag_offset != GG::Pt(-GG::X1, -GG::Y1)) { // resize-dragging
        // drag offset: position of cursor relative to lower-right of window when left button was pressed
        // pt: position of cursor relative to upper-left of screen
        GG::Pt requested_lr = pt - m_drag_offset;

        GG::Pt max_lr;
        if (const GG::Wnd* parent = Parent()) {
            max_lr = parent->ClientLowerRight();
        } else {
            max_lr.x = GG::GUI::GetGUI()->AppWidth();
            max_lr.y = GG::GUI::GetGUI()->AppHeight();
        }

        GG::X new_x = std::min(max_lr.x, requested_lr.x);
        GG::Y new_y = std::min(max_lr.y, requested_lr.y);
        GG::Pt new_lr(new_x, new_y);

        Resize(new_lr - UpperLeft());

    } else { // normal-dragging
        GG::Wnd::LDrag(pt, move, mod_keys);
    }
}

void CUIWnd::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ m_drag_offset = GG::Pt(-GG::X1, -GG::Y1); }

void CUIWnd::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_mouse_in_resize_tab = InResizeTab(pt);
    Wnd::MouseEnter(pt, mod_keys);
}

void CUIWnd::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_mouse_in_resize_tab = InResizeTab(pt);
    Wnd::MouseHere(pt, mod_keys);
}

void CUIWnd::MouseLeave() {
    m_mouse_in_resize_tab = false;
    Wnd::MouseLeave();
}

GG::Pt CUIWnd::ClientUpperLeft() const
{ return m_minimized ? UpperLeft() : UpperLeft() + GG::Pt(BORDER_LEFT, BORDER_TOP); }

GG::Pt CUIWnd::ClientLowerRight() const
{ return m_minimized ? LowerRight() : LowerRight() - GG::Pt(BORDER_RIGHT, BORDER_BOTTOM); }

bool CUIWnd::InWindow(const GG::Pt& pt) const {
    GG::Pt lr = LowerRight();
    if (m_resizable) {
        return UpperLeft() <= pt && pt < lr;
    } else {
        GG::Pt dist_from_lr = lr - pt;
        bool inside_lower_right_corner = OUTER_EDGE_ANGLE_OFFSET < Value(dist_from_lr.x) + Value(dist_from_lr.y);
        return (UpperLeft() <= pt && pt < lr && inside_lower_right_corner);
    }
}

void CUIWnd::PositionButtons() {
    // The buttons are to be positioned based on the presence of other buttons
    GG::Pt button_ul = GG::Pt(Width() - BUTTON_RIGHT_OFFSET, BUTTON_TOP_OFFSET) + UpperLeft() - ClientUpperLeft();

    if (m_close_button) {
        m_close_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        button_ul -= GG::Pt(m_close_button->Width(), GG::Y0) + GG::Pt(GG::X(4), GG::Y0);
    }

    if (m_minimize_button) {
        m_minimize_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
        button_ul -= GG::Pt(m_minimize_button->Width(), GG::Y0) + GG::Pt(GG::X(4), GG::Y0);
    }

    if (m_pin_button)
        m_pin_button->MoveTo(GG::Pt(button_ul.x, button_ul.y));
}

void CUIWnd::InitButtons() {
    // create the close button
    if (m_closable) {
        m_close_button = new CUI_CloseButton();
        m_close_button->Resize(GG::Pt(GG::X(12), GG::Y(12)));
        GG::Connect(m_close_button->LeftClickedSignal, &CUIWnd::CloseClicked, this);
        AttachChild(m_close_button);
        m_close_button->NonClientChild(true);
    }

    // create the minimize button
    if (m_minimizable) {
        m_minimize_button = new CUI_MinRestoreButton();
        m_minimize_button->Resize(GG::Pt(GG::X(12), GG::Y(12)));
        GG::Connect(m_minimize_button->LeftClickedSignal, &CUIWnd::MinimizeClicked, this);
        AttachChild(m_minimize_button);
        m_minimize_button->NonClientChild(true);
    }

    // create the pin button
    if (m_pinable) {
        m_pin_button = new CUI_PinButton();
        m_pin_button->Resize(GG::Pt(GG::X(12), GG::Y(12)));
        GG::Connect(m_pin_button->LeftClickedSignal, &CUIWnd::PinClicked, this);
        AttachChild(m_pin_button);
        m_pin_button->NonClientChild(true);
    }

    // All buttons were created at the same spot, position them to the correct spot
    PositionButtons();
}

GG::Pt CUIWnd::MinimizedSize() const
{ return GG::Pt(MINIMIZED_WND_WIDTH, BORDER_TOP); }

GG::X CUIWnd::LeftBorder() const
{ return BORDER_LEFT; }

GG::Y CUIWnd::TopBorder() const
{ return BORDER_TOP; }

GG::X CUIWnd::RightBorder() const
{ return BORDER_RIGHT; }

GG::Y CUIWnd::BottomBorder() const
{ return BORDER_BOTTOM; }

int CUIWnd::InnerBorderAngleOffset() const
{ return INNER_BORDER_ANGLE_OFFSET; }

void CUIWnd::CloseClicked() {
    m_done = true;
    if (Parent())
        Parent()->DetachChild(this);
    else
        GG::GUI::GetGUI()->Remove(this);

    //m_minimized_buffer.clear();
    //m_outer_border_buffer.clear();
    //m_inner_border_buffer.clear();
    //m_resize_corner_lines_buffer.clear();
}

void CUIWnd::PinClicked() {
    m_pinned = !m_pinned;
    m_resizable = !m_pinned;
    m_pin_button->Toggle(m_pinned); // Change the icon on the pin button
    InitBuffers();
}

void CUIWnd::MinimizeClicked() {
    if (!m_minimized) {
        m_minimized = true;
        m_original_size = Size();
        SetMinSize(MinimizedSize());
        Resize(MinimizedSize());

        // hide all children, re-showing only position/size controls
        Hide();
        Show(false);
        if (m_close_button)
            m_close_button->Show();
        if (m_minimize_button)
            m_minimize_button->Show();
        if (m_pin_button)
            m_pin_button->Show();

    } else {
        m_minimized = false;
        SetMinSize(GG::Pt(MinimizedSize().x, BORDER_TOP + INNER_BORDER_ANGLE_OFFSET + BORDER_BOTTOM + 10));
        Resize(GG::Pt(m_original_size));
        Show();
    }
}

void CUIWnd::InitBuffers() {
    // for when minimized... m_minimized_buffer
    GG::Pt m_sz = MinimizedSize();

    m_minimized_buffer.clear();
    m_minimized_buffer.store(0.0f,          0.0f);
    m_minimized_buffer.store(Value(m_sz.x), 0.0f);
    m_minimized_buffer.store(Value(m_sz.x), Value(m_sz.y));
    m_minimized_buffer.store(0.0f,          Value(m_sz.y));
    m_minimized_buffer.store(0.0f,          0.0f);
    m_minimized_buffer.createServerBuffer();

    GG::Pt sz = Size();
    GG::Pt cl_ul = ClientUpperLeft() - UpperLeft();
    GG::Pt cl_lr = ClientLowerRight() - UpperLeft();

    // outer border, with optional corner cutout
    m_outer_border_buffer.clear();
    m_outer_border_buffer.store(0.0f,           0.0f);
    m_outer_border_buffer.store(Value(sz.x),    0.0f);
    if (!m_resizable) {
        m_outer_border_buffer.store(Value(sz.x),                            Value(sz.y) - OUTER_EDGE_ANGLE_OFFSET);
        m_outer_border_buffer.store(Value(sz.x) - OUTER_EDGE_ANGLE_OFFSET,  Value(sz.y));
    } else {
        m_outer_border_buffer.store(Value(sz.x),Value(sz.y));
    }
    m_outer_border_buffer.store(0.0f,           Value(sz.y));
    m_outer_border_buffer.store(0.0f,           0.0f);

    // inner border, with optional corner cutout
    m_inner_border_buffer.clear();
    m_inner_border_buffer.store(Value(cl_ul.x), Value(cl_ul.y));
    m_inner_border_buffer.store(Value(cl_lr.x), Value(cl_ul.y));
    if (m_resizable) {
        m_inner_border_buffer.store(Value(cl_lr.x),                             Value(cl_lr.y) - INNER_BORDER_ANGLE_OFFSET);
        m_inner_border_buffer.store(Value(cl_lr.x) - INNER_BORDER_ANGLE_OFFSET, Value(cl_lr.y));
    } else {
        m_inner_border_buffer.store(Value(cl_lr.x),Value(cl_lr.y));
    }
    m_inner_border_buffer.store(Value(cl_ul.x), Value(cl_lr.y));
    m_inner_border_buffer.store(Value(cl_ul.x), Value(cl_ul.y));

    // resize hash marks
    m_resize_corner_lines_buffer.clear();
    m_resize_corner_lines_buffer.store(Value(cl_lr.x),                          Value(cl_lr.y) - RESIZE_HASHMARK1_OFFSET);
    m_resize_corner_lines_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK1_OFFSET,Value(cl_lr.y));
    m_resize_corner_lines_buffer.store(Value(cl_lr.x),                          Value(cl_lr.y) - RESIZE_HASHMARK2_OFFSET);
    m_resize_corner_lines_buffer.store(Value(cl_lr.x) - RESIZE_HASHMARK2_OFFSET,Value(cl_lr.y));
}

///////////////////////////////////////
// class CUIEditWnd
///////////////////////////////////////
const GG::X CUIEditWnd::BUTTON_WIDTH(75);
const int CUIEditWnd::CONTROL_MARGIN = 5;

CUIEditWnd::CUIEditWnd(GG::X w, const std::string& prompt_text, const std::string& edit_text, GG::Flags<GG::WndFlag> flags/* = Wnd::MODAL*/) :
    CUIWnd(prompt_text, GG::X0, GG::Y0, w, GG::Y1, flags)
{
    m_edit = new CUIEdit(edit_text);
    m_ok_bn = new CUIButton(UserString("OK"));
    m_cancel_bn = new CUIButton(UserString("CANCEL"));

    m_edit->MoveTo(GG::Pt(LeftBorder() + 3, TopBorder() + 3));
    m_edit->Resize(GG::Pt(ClientWidth() - 2 * BUTTON_WIDTH - 2 * CONTROL_MARGIN - 6 - LeftBorder() - RightBorder(), m_edit->MinUsableSize().y));

    m_ok_bn->MoveTo(GG::Pt(m_edit->Right() + CONTROL_MARGIN, TopBorder() + 3));
    m_ok_bn->Resize(GG::Pt(BUTTON_WIDTH, m_ok_bn->MinUsableSize().y));
    m_ok_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));

    m_cancel_bn->MoveTo(GG::Pt(m_ok_bn->Right() + CONTROL_MARGIN, TopBorder() + 3));
    m_cancel_bn->Resize(GG::Pt(BUTTON_WIDTH, m_cancel_bn->MinUsableSize().y));
    m_cancel_bn->OffsetMove(GG::Pt(GG::X0, (m_edit->Height() - m_ok_bn->Height()) / 2));

    Resize(GG::Pt(w, std::max(m_edit->Bottom(), m_cancel_bn->Bottom()) + BottomBorder() + 3));
    MoveTo(GG::Pt((GG::GUI::GetGUI()->AppWidth() - w) / 2, (GG::GUI::GetGUI()->AppHeight() - Height()) / 2));

    AttachChild(m_edit);
    AttachChild(m_ok_bn);
    AttachChild(m_cancel_bn);

    GG::Connect(m_ok_bn->LeftClickedSignal,     &CUIEditWnd::OkClicked, this);
    GG::Connect(m_cancel_bn->LeftClickedSignal, &CUIWnd::CloseClicked, static_cast<CUIWnd*>(this));

    m_edit->SelectAll();
}

void CUIEditWnd::ModalInit()
{ GG::GUI::GetGUI()->SetFocusWnd(m_edit); }

void CUIEditWnd::KeyPress(GG::Key key, boost::uint32_t key_code_point, GG::Flags<GG::ModKey> mod_keys) {
    switch (key) {
    case GG::GGK_RETURN: if (!m_ok_bn->Disabled()) OkClicked(); break;
    case GG::GGK_ESCAPE: CloseClicked(); break;
    default: break;
    }
}

const std::string& CUIEditWnd::Result() const
{ return m_result; }

void CUIEditWnd::OkClicked() {
    m_result = m_edit->Text();
    CloseClicked();
}
