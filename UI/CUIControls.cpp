#include "CUIControls.h"

#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "IconTextBrowseWnd.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../universe/Species.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/dialogs/ColorDlg.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/regex.hpp>

#include <limits>


namespace {
    void PlayButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true); }

    struct PlayButtonCheckSound {
        PlayButtonCheckSound(bool play_only_when_checked) : m_play_only_when_checked(play_only_when_checked) {}
        void operator()(bool checked) const {
            if (!m_play_only_when_checked || checked)
                Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
        }
        const bool m_play_only_when_checked;
    };

    void PlayListSelectSound(const GG::ListBox::SelectionSet&)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.list-select"), true); }

    void PlayDropDownListOpenSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.list-pulldown"), true); }

    void PlayItemDropSound(GG::ListBox::iterator)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.item-drop"), true); }

    void PlayTextTypingSound(const std::string&)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.text-typing"), true); }

    const double ARROW_BRIGHTENING_SCALE_FACTOR = 1.5;
    const double STATE_BUTTON_BRIGHTENING_SCALE_FACTOR = 1.25;
    const double TAB_BRIGHTENING_SCALE_FACTOR = 1.25;
}


///////////////////////////////////////
// class CUILabel
///////////////////////////////////////
CUILabel::CUILabel(const std::string& str,
                   GG::Flags<GG::TextFormat> format/* = GG::FORMAT_NONE*/,
                   GG::Flags<GG::WndFlag> flags/* = GG::NO_WND_FLAGS*/) :
    TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, str, ClientUI::GetFont(), ClientUI::TextColor(), format, flags)
{}


///////////////////////////////////////
// class CUIButton
///////////////////////////////////////
namespace {
    const int CUIBUTTON_ANGLE_OFFSET = 5;
}

CUIButton::CUIButton(const std::string& str) :
    Button(str, ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::TextColor(), GG::INTERACTIVE),
    m_border_color(ClientUI::CtrlBorderColor()),
    m_border_thick(1),
    m_checked(false)
{ GG::Connect(LeftClickedSignal, &PlayButtonClickSound, -1); }

CUIButton::CUIButton(const std::string& str, GG::Clr background, GG::Clr border) :
    Button(str, ClientUI::GetFont(), background, ClientUI::TextColor(), GG::INTERACTIVE),
    m_border_color(border),
    m_border_thick(2),
    m_checked(false)
{ GG::Connect(LeftClickedSignal, &PlayButtonClickSound, -1); }

CUIButton::CUIButton(const GG::SubTexture& unpressed, const GG::SubTexture& pressed,
                     const GG::SubTexture& rollover) :
    Button("", ClientUI::GetFont(), GG::CLR_WHITE, GG::CLR_ZERO, GG::INTERACTIVE),
    m_border_color(ClientUI::CtrlBorderColor()),
    m_border_thick(1),
    m_checked(false)
{
    SetColor(GG::CLR_WHITE);
    SetUnpressedGraphic(unpressed);
    SetPressedGraphic  (pressed);
    SetRolloverGraphic (rollover);
    GG::Connect(LeftClickedSignal, &PlayButtonClickSound, -1);
}

bool CUIButton::InWindow(const GG::Pt& pt) const {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    return InAngledCornerRect(pt, ul, lr, CUIBUTTON_ANGLE_OFFSET);
}

void CUIButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        SetState(BN_ROLLOVER);
    }
}

void CUIButton::SetCheck(bool b/* = true*/)
{ m_checked = b; }

GG::Pt CUIButton::MinUsableSize() const {
    GG::Pt result = GG::Button::MinUsableSize();
    const int CUIBUTTON_HPADDING = 10;
    const int CUIBUTTON_VPADDING = 3;

    result.x += CUIBUTTON_HPADDING * 2;
    result.y += CUIBUTTON_VPADDING * 2;

    return result;
}

void CUIButton::RenderPressed() {
    if (PressedGraphic().Empty()) {
        GG::Clr background_clr = Color();
        GG::Clr border_clr     = m_border_color;
        int     border_thick   = m_border_thick;

        if (!m_checked) {
            background_clr = ClientUI::CtrlColor();
            border_clr     = ClientUI::CtrlBorderColor();
            border_thick   = 1;
        }

        AdjustBrightness(background_clr, 25);

        GG::Pt ul = UpperLeft();
        GG::Pt lr = LowerRight();
        AngledCornerRectangle(ul, lr, background_clr, border_clr, CUIBUTTON_ANGLE_OFFSET, border_thick);
        m_label->OffsetMove(GG::Pt(GG::X1, GG::Y1));
        m_label->Render();
        m_label->OffsetMove(GG::Pt(-GG::X1, -GG::Y1));
    } else {
        GG::Button::RenderPressed();
    }
}

void CUIButton::RenderRollover() {
    if (RolloverGraphic().Empty()) {
        GG::Clr background_clr = Color();
        GG::Clr border_clr     = m_border_color;
        int     border_thick   = m_border_thick;

        if (!m_checked) {
            background_clr = ClientUI::CtrlColor();
            border_clr     = ClientUI::CtrlBorderColor();
            border_thick   = 1;
        }

        if (Disabled()) {
            background_clr = DisabledColor(background_clr);
            border_clr     = DisabledColor(border_clr);
        }
        else
            AdjustBrightness(border_clr, 100);

        GG::Pt ul = UpperLeft();
        GG::Pt lr = LowerRight();
        AngledCornerRectangle(ul, lr, background_clr, border_clr, CUIBUTTON_ANGLE_OFFSET, border_thick);
        m_label->Render();
    } else {
        GG::Button::RenderRollover();
    }
}

void CUIButton::RenderUnpressed() {
    if (UnpressedGraphic().Empty()) {
        GG::Clr background_clr = Color();
        GG::Clr border_clr     = m_border_color;
        int     border_thick   = m_border_thick;

        if (!m_checked) {
            background_clr = ClientUI::CtrlColor();
            border_clr     = ClientUI::CtrlBorderColor();
            border_thick   = 1;
        }

        if (Disabled()) {
            background_clr = DisabledColor(background_clr);
            border_clr     = DisabledColor(border_clr);
        }

        GG::Pt ul = UpperLeft();
        GG::Pt lr = LowerRight();
        AngledCornerRectangle(ul, lr, background_clr, border_clr, CUIBUTTON_ANGLE_OFFSET, border_thick);
        m_label->Render();
    } else {
        GG::Button::RenderUnpressed();
    }
}


///////////////////////////////////////
// class SettableInWindowCUIButton
///////////////////////////////////////
SettableInWindowCUIButton::SettableInWindowCUIButton(const GG::SubTexture& unpressed,
                                                     const GG::SubTexture& pressed,
                                                     const GG::SubTexture& rollover,
                                                     boost::function<bool(const SettableInWindowCUIButton*, const GG::Pt&)> in_window_function) :
    CUIButton(unpressed, pressed, rollover)
{ m_in_window_func = in_window_function; }

bool SettableInWindowCUIButton::InWindow(const GG::Pt& pt) const {
    if (m_in_window_func)
        return m_in_window_func(this, pt);
    else
        return CUIButton::InWindow(pt);
}


///////////////////////////////////////
// class CUIArrowButton
///////////////////////////////////////
CUIArrowButton::CUIArrowButton(ShapeOrientation orientation, bool fill_background,
                               GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    Button("", boost::shared_ptr<GG::Font>(), ClientUI::DropDownListArrowColor(), GG::CLR_ZERO, flags),
    m_orientation(orientation),
    m_fill_background_with_wnd_color(fill_background)
{ GG::Connect(LeftClickedSignal, &PlayButtonClickSound, -1); }

bool CUIArrowButton::InWindow(const GG::Pt& pt) const {
    if (m_fill_background_with_wnd_color) {
        return Button::InWindow(pt);
    } else {
        GG::Pt ul = UpperLeft() + GG::Pt(GG::X(3), GG::Y(1)), lr = LowerRight() - GG::Pt(GG::X(2), GG::Y(1));
        return InIsoscelesTriangle(pt, ul, lr, m_orientation);
    }
}

void CUIArrowButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        SetState(BN_ROLLOVER);
    }
}

void CUIArrowButton::RenderPressed() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    OffsetMove(GG::Pt(GG::X1, GG::Y1));
    RenderUnpressed();
    OffsetMove(GG::Pt(-GG::X1, -GG::Y1));
}

void CUIArrowButton::RenderRollover() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    if (!Disabled())
        AdjustBrightness(color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
    GG::Pt tri_ul = ul + GG::Pt(GG::X(3), GG::Y1), tri_lr = lr - GG::Pt(GG::X(2), GG::Y1);
    IsoscelesTriangle(tri_ul, tri_lr, m_orientation, color_to_use);
}

void CUIArrowButton::RenderUnpressed() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Pt tri_ul = ul + GG::Pt(GG::X(3), GG::Y1), tri_lr = lr - GG::Pt(GG::X(2), GG::Y1);
    IsoscelesTriangle(tri_ul, tri_lr, m_orientation, color_to_use);
}


///////////////////////////////////////
// class CUIStateButton
///////////////////////////////////////
CUIStateButton::CUIStateButton(const std::string& str, GG::Flags<GG::TextFormat> format,
                               GG::StateButtonStyle style/* = GG::SBSTYLE_3D_CHECKBOX*/) :
    StateButton(str, ClientUI::GetFont(), format,
                ClientUI::StateButtonColor(), ClientUI::TextColor(), GG::CLR_ZERO,
                style),
    m_border_color(ClientUI::CtrlBorderColor()),
    m_mouse_here(false)
{
    if (style == GG::SBSTYLE_3D_TOP_DETACHED_TAB || style == GG::SBSTYLE_3D_TOP_ATTACHED_TAB) {
        SetColor(ClientUI::WndColor());
        GetLabel()->SetTextColor(DarkColor(ClientUI::TextColor()));
    }
    // HACK! radio buttons should only emit sounds when they are checked, and *not* when they are unchecked; currently, there's no 
    // other way to detect the difference between these two kinds of CUIStateButton within the CUIStateButton ctor other than
    // checking the redering style
    GG::Connect(CheckedSignal, PlayButtonCheckSound(style == GG::SBSTYLE_3D_RADIO), -1);
}

GG::Pt CUIStateButton::MinUsableSize() const {
    // same as StateButton::MinUsableSize, but without the forced minimum 25 width
    GG::Pt button_ul = this->ButtonUpperLeft();
    GG::Pt button_lr = this->ButtonLowerRight();
    GG::Pt text_ul = this->TextUpperLeft();
    GG::Pt text_lr = text_ul + GetLabel()->MinUsableSize();
    GG::Pt retval = GG::Pt(std::max(button_lr.x, text_lr.x) - std::min(button_ul.x, text_ul.x),
                           std::max(button_lr.y, text_lr.y) - std::min(button_ul.y, text_ul.y));
    return retval;
}

void CUIStateButton::Render() {
    if (static_cast<int>(Style()) == GG::SBSTYLE_3D_CHECKBOX ||
        static_cast<int>(Style()) == GG::SBSTYLE_3D_RADIO)
    {
        // draw button
        GG::Pt bn_ul = ClientUpperLeft() + ButtonUpperLeft();
        GG::Pt bn_lr = ClientUpperLeft() + ButtonLowerRight();
        GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
        GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
        if (!Disabled() && !Checked() && m_mouse_here) {
            AdjustBrightness(color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
            AdjustBrightness(int_color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
            AdjustBrightness(border_color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
        }

        if (static_cast<int>(Style()) == GG::SBSTYLE_3D_CHECKBOX) {
            const int MARGIN = 3;
            FlatRectangle(bn_ul, bn_lr, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                bn_ul += GG::Pt(GG::X(MARGIN), GG::Y(MARGIN));
                bn_lr -= GG::Pt(GG::X(MARGIN), GG::Y(MARGIN));
                const int OFFSET = Value(bn_lr.y - bn_ul.y) / 2;
                glDisable(GL_TEXTURE_2D);
                glColor(inside_color);
                glBegin(GL_QUADS);
                glVertex(bn_lr.x, bn_ul.y);
                glVertex(bn_ul.x + OFFSET, bn_ul.y);
                glVertex(bn_ul.x, bn_ul.y + OFFSET);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_lr.x - OFFSET, bn_lr.y);
                glVertex(bn_lr.x, bn_lr.y - OFFSET);
                glVertex(bn_lr.x, bn_ul.y);
                glEnd();
                glColor(outside_color);
                glBegin(GL_LINE_STRIP);
                glVertex(bn_lr.x, bn_ul.y);
                glVertex(bn_ul.x + OFFSET, bn_ul.y);
                glVertex(bn_ul.x, bn_ul.y + OFFSET);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_lr.x - OFFSET, bn_lr.y);
                glVertex(bn_lr.x, bn_lr.y - OFFSET);
                glVertex(bn_lr.x, bn_ul.y);
                glEnd();
                glEnable(GL_TEXTURE_2D);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                glTranslated(Value((bn_ul.x + bn_lr.x) / 2.0), Value(-(bn_ul.y + bn_lr.y) / 2.0), 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(Value(-(bn_ul.x + bn_lr.x) / 2.0), Value((bn_ul.y + bn_lr.y) / 2.0), 0.0);
                AngledCornerRectangle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                                      GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN), 
                                      inside_color, outside_color, Value(bn_lr.y - bn_ul.y - 2 * MARGIN) / 2, 1);
                glTranslated(Value((bn_ul.x + bn_lr.x) / 2.0), Value(-(bn_ul.y + bn_lr.y) / 2.0), 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(Value(-(bn_ul.x + bn_lr.x) / 2.0), Value((bn_ul.y + bn_lr.y) / 2.0), 0.0);
            }
        } else if (static_cast<int>(Style()) == GG::SBSTYLE_3D_RADIO) {
            const int MARGIN = 2;
            FlatCircle(bn_ul, bn_lr, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                FlatCircle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                           GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN),
                           GG::CLR_ZERO, outside_color, 1);
                FlatCircle(GG::Pt(bn_ul.x + MARGIN + 1, bn_ul.y + MARGIN + 1),
                           GG::Pt(bn_lr.x - MARGIN - 1, bn_lr.y - MARGIN - 1), 
                           inside_color, outside_color, 1);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                FlatCircle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                           GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN),
                           inside_color, outside_color, 1);
            }
        }
        // draw text
        GetLabel()->OffsetMove(TextUpperLeft());
        GetLabel()->TextControl::Render();
        GetLabel()->OffsetMove(-TextUpperLeft());
    } else if (static_cast<int>(Style()) == GG::SBSTYLE_3D_TOP_DETACHED_TAB) {
        GG::Pt ul = UpperLeft(), lr = LowerRight();
        GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
        if (Checked() || !Disabled() && m_mouse_here)
            AdjustBrightness(border_color_to_use, 100);
        const int UNCHECKED_OFFSET = 4;
        GG::Pt additional_text_offset;
        if (!Checked()) {
            ul.y += UNCHECKED_OFFSET;
            additional_text_offset.y = GG::Y(UNCHECKED_OFFSET / 2);
        }
        AngledCornerRectangle(ul, lr, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, 1, true, false, !Checked());
        GetLabel()->OffsetMove(TextUpperLeft() + additional_text_offset);
        GetLabel()->Render();
        GetLabel()->OffsetMove(-(TextUpperLeft() + additional_text_offset));
    } else {
        StateButton::Render();
    }
}

void CUIStateButton::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ m_mouse_here = true; }

void CUIStateButton::MouseLeave()
{ m_mouse_here = false; }


///////////////////////////////////////
// class CUITabBar
///////////////////////////////////////
CUITabBar::CUITabBar(const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color,
                     GG::TabBarStyle style) :
    GG::TabBar(font, color, text_color, style)
{}

void CUITabBar::DistinguishCurrentTab(const std::vector<GG::StateButton*>& tab_buttons) {
    RaiseCurrentTabButton();
    int index = CurrentTabIndex();
    for (int i = 0; i < static_cast<int>(tab_buttons.size()); ++i) {
        GG::StateButton* tab = tab_buttons[i];
        GG::Clr text_color = TextColor();
        if (index == i)
            tab->SetTextColor(text_color);
        else
            tab->SetTextColor(DarkColor(text_color));
    }
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
namespace {
    const int CUISCROLL_ANGLE_OFFSET = 3;
}

///////////////////////////////////////
// class CUIScroll::ScrollTab
///////////////////////////////////////
CUIScroll::ScrollTab::ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color,
                                GG::Clr border_color) : 
    Button("", boost::shared_ptr<GG::Font>(), color),
    m_border_color(border_color),
    m_orientation(orientation),
    m_mouse_here(false),
    m_being_dragged(false)
{
    MoveTo(GG::Pt(GG::X(orientation == GG::VERTICAL ? 0 : 2),
                  GG::Y(orientation == GG::VERTICAL ? 2 : 0)));
    Resize(GG::Pt(GG::X(scroll_width), GG::Y(scroll_width)));
    SetMinSize(GG::Pt(m_orientation == GG::VERTICAL ? MinSize().x : GG::X(10),
                      m_orientation == GG::VERTICAL ? GG::Y(10) : MinSize().y));
}

void CUIScroll::ScrollTab::SetColor(GG::Clr c)
{}  // intentionally ignored

void CUIScroll::ScrollTab::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    if (m_orientation == GG::VERTICAL) {
        ul.x += 3;
        lr.x -= 3;
    } else {
        ul.y += 3;
        lr.y -= 3;
    }

    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    if (!Disabled() && m_mouse_here) {
        AdjustBrightness(color_to_use, TAB_BRIGHTENING_SCALE_FACTOR);
        AdjustBrightness(border_color_to_use, TAB_BRIGHTENING_SCALE_FACTOR);
    }

    // basic shape, no border
    AngledCornerRectangle(ul, lr, color_to_use, GG::CLR_ZERO, CUISCROLL_ANGLE_OFFSET, 0);
    // upper left diagonal stripe
    GG::Clr light_color = Color();
    AdjustBrightness(light_color, 35);
    if (!Disabled() && m_mouse_here)
        AdjustBrightness(light_color, TAB_BRIGHTENING_SCALE_FACTOR);
    glColor(light_color);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    if (m_orientation == GG::VERTICAL) {
        glVertex(lr.x, ul.y);
        glVertex(ul.x + CUISCROLL_ANGLE_OFFSET, ul.y);
        glVertex(ul.x, ul.y + CUISCROLL_ANGLE_OFFSET);
        glVertex(ul.x, ul.y + std::min(Value(lr.x - ul.x), Value(lr.y - ul.y)));
    } else {
        glVertex(ul.x + std::min(Value(lr.x - ul.x), Value(lr.y - ul.y)), ul.y);
        glVertex(ul.x + CUISCROLL_ANGLE_OFFSET, ul.y);
        glVertex(ul.x, lr.y - CUISCROLL_ANGLE_OFFSET);
        glVertex(ul.x, lr.y);
    }
    glEnd();
    // lower right diagonal stripe
    glBegin(GL_POLYGON);
    if (m_orientation == GG::VERTICAL) {
        glVertex(lr.x, lr.y - std::min(Value(lr.x - ul.x), Value(lr.y - ul.y)));
        glVertex(ul.x, lr.y);
        glVertex(lr.x - CUISCROLL_ANGLE_OFFSET, lr.y);
        glVertex(lr.x, lr.y - CUISCROLL_ANGLE_OFFSET);
    } else {
        glVertex(lr.x, ul.y);
        glVertex(lr.x - std::min(Value(lr.x - ul.x), Value(lr.y - ul.y)), lr.y);
        glVertex(lr.x - CUISCROLL_ANGLE_OFFSET, lr.y);
        glVertex(lr.x, lr.y - CUISCROLL_ANGLE_OFFSET);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    // border
    AngledCornerRectangle(ul, lr, GG::CLR_ZERO, border_color_to_use, CUISCROLL_ANGLE_OFFSET, 1);
}

void CUIScroll::ScrollTab::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ m_being_dragged = true; }

void CUIScroll::ScrollTab::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    m_being_dragged = false;
    if (!InWindow(GG::GUI::GetGUI()->MousePosition()))
        m_mouse_here = false;
}

void CUIScroll::ScrollTab::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ m_being_dragged = false; }

void CUIScroll::ScrollTab::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!m_being_dragged && !m_mouse_here) {
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        m_mouse_here = true;
    }
}

void CUIScroll::ScrollTab::MouseLeave() {
    if (!m_being_dragged)
        m_mouse_here = false;
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
CUIScroll::CUIScroll(GG::Orientation orientation) :
    Scroll(orientation, ClientUI::CtrlColor(), ClientUI::CtrlColor()),
    m_border_color(ClientUI::CtrlBorderColor())
{}

void CUIScroll::Render() {
    GG::Clr color_to_use =          Disabled() ? DisabledColor(Color())         :   Color();
    GG::Clr border_color_to_use =   Disabled() ? DisabledColor(m_border_color)  :   m_border_color;
    GG::Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_buffer.activate();
    glColor(color_to_use);
    glDrawArrays(GL_TRIANGLE_FAN,   0, m_buffer.size());
    glColor(border_color_to_use);
    glDrawArrays(GL_LINE_LOOP,      0, m_buffer.size());

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void CUIScroll::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_sz = Size();
    Wnd::SizeMove(ul, lr);

    TabButton()->SizeMove(TabButton()->RelativeUpperLeft(), 
                          (ScrollOrientation() == GG::VERTICAL) ?
                          GG::Pt(Size().x, TabButton()->RelativeLowerRight().y) :
                          GG::Pt(TabButton()->RelativeLowerRight().x, Size().y));

    SizeScroll(ScrollRange().first, ScrollRange().second, LineSize(), PageSize()); // update tab size and position

    if (Size() != old_sz)
        InitBuffer();
}


///////////////////////////////////////
// class CUIListBox
///////////////////////////////////////
CUIListBox::CUIListBox(void):
    ListBox(ClientUI::CtrlBorderColor(), ClientUI::CtrlColor())
{
    RecreateScrolls();
    GG::Connect(SelChangedSignal,   &PlayListSelectSound,   -1);
    GG::Connect(DroppedSignal,      &PlayItemDropSound,     -1);
}

void CUIListBox::Render() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color = Color(); // save color
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    FlatRectangle(ul, lr, InteriorColor(), color_to_use, 1);
    SetColor(GG::CLR_ZERO); // disable the default border by rendering it transparently
    ListBox::Render();
    SetColor(color); // restore color
}


///////////////////////////////////////
// class CUIDropDownList
///////////////////////////////////////
namespace {
    const int CUIDROPDOWNLIST_ANGLE_OFFSET = 5;
}

CUIDropDownList::CUIDropDownList(size_t num_shown_elements) :
    DropDownList(num_shown_elements, ClientUI::CtrlBorderColor()),
    m_render_drop_arrow(true),
    m_mouse_here(false)
{
    SetInteriorColor(ClientUI::CtrlColor());
    SetMinSize(GG::Pt(MinSize().x, CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT));
}

void CUIDropDownList::InitBuffer() {
    m_buffer.clear();
    GG::Pt sz = Size();
    BufferStoreAngledCornerRectangleVertices(this->m_buffer, GG::Pt(GG::X0, GG::Y0), sz,
                                             CUIDROPDOWNLIST_ANGLE_OFFSET, false, true, false);

    int margin = 3;
    int triangle_width = Value(sz.y - 4 * margin);
    int outline_width = triangle_width + 3 * margin;

    GG::Pt triangle_ul = GG::Pt(sz.x - triangle_width - margin * 5 / 2, GG::Y(2 * margin));
    GG::Pt triangle_lr = GG::Pt(sz.x - margin * 5 / 2, sz.y - 2 * margin);
    BufferStoreIsoscelesTriangle(this->m_buffer, triangle_ul, triangle_lr, SHAPE_DOWN);

    GG::Pt btn_ul = GG::Pt(sz.x - outline_width - margin, GG::Y(margin));
    GG::Pt btn_lr = GG::Pt(sz.x - margin, sz.y - margin);

    BufferStoreAngledCornerRectangleVertices(this->m_buffer, btn_ul, btn_lr,
                                             CUIDROPDOWNLIST_ANGLE_OFFSET, false, true, false);
}

void CUIDropDownList::Render() {
    GG::Pt ul = UpperLeft();
    GG::Clr lb_color = LB()->Color();
    GG::Clr lb_interior_color = LB()->InteriorColor();
    GG::Clr border_color = Disabled() ? DisabledColor(lb_color) : lb_color;
    GG::Clr interior_color = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);


    m_buffer.activate();

    // interior
    glColor(interior_color);
    glDrawArrays(GL_TRIANGLE_FAN,   0, 5);
    // border
    glLineWidth(1.0f);
    glColor(border_color);
    glDrawArrays(GL_LINE_LOOP,      0, 5);

    if (m_render_drop_arrow) {
        GG::Clr triangle_fill_color = ClientUI::DropDownListArrowColor();
        if (m_mouse_here && !Disabled())
            AdjustBrightness(triangle_fill_color, ARROW_BRIGHTENING_SCALE_FACTOR);
        GG::Clr triangle_border_color = triangle_fill_color;
        AdjustBrightness(triangle_border_color, 75);

        // triangle interior
        glColor(triangle_fill_color);
        glDrawArrays(GL_TRIANGLE_FAN,   5, 3);
        // triangle border
        glColor(triangle_border_color);
        glDrawArrays(GL_LINE_LOOP,      5, 3);

        // button border around triangle
        glColor(border_color);
        glDrawArrays(GL_LINE_LOOP,      8, 5);
    }

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    // Draw the ListBox::Row of currently displayed item, if any.
    RenderDisplayedRow();
}

void CUIDropDownList::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        PlayDropDownListOpenSound();
    DropDownList::LClick(pt, mod_keys);
}

void CUIDropDownList::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
    m_mouse_here = true;
}

void CUIDropDownList::MouseLeave()
{ m_mouse_here = false; }

void CUIDropDownList::DisableDropArrow()
{ m_render_drop_arrow = false; }

void CUIDropDownList::EnableDropArrow()
{ m_render_drop_arrow = true; }


///////////////////////////////////////
// class CUIEdit
///////////////////////////////////////
CUIEdit::CUIEdit(const std::string& str) :
    Edit(str, ClientUI::GetFont(), ClientUI::CtrlBorderColor(), ClientUI::TextColor(), ClientUI::CtrlColor())
{
    GG::Connect(EditedSignal, &PlayTextTypingSound, -1);
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIEdit::GainingFocus() {
    GG::Edit::GainingFocus();
    GainingFocusSignal();
}

void CUIEdit::LosingFocus() {
    GG::Edit::LosingFocus();
    LosingFocusSignal();
}

void CUIEdit::Render() {
    GG::Clr color = Color();
    GG::Clr border_color = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    //GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul, lr, int_color_to_use, border_color, 1);

    SetColor(GG::CLR_ZERO);
    Edit::Render();
    SetColor(color);
}


///////////////////////////////////////
// class CUIMultiEdit
///////////////////////////////////////
CUIMultiEdit::CUIMultiEdit(const std::string& str, GG::Flags<GG::MultiEditStyle> style/* = MULTI_LINEWRAP*/) :
    MultiEdit(str, ClientUI::GetFont(), ClientUI::CtrlBorderColor(), style, ClientUI::TextColor(), ClientUI::CtrlColor())
{
    RecreateScrolls();
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIMultiEdit::Render() {
    GG::Clr color = Color();
    GG::Clr border_color =      Disabled()  ?   DisabledColor(color)            :   color;
    GG::Clr int_color_to_use =  Disabled()  ?   DisabledColor(InteriorColor())  :   InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();

    FlatRectangle(ul, lr, int_color_to_use, border_color, 1);

    SetColor(GG::CLR_ZERO);
    MultiEdit::Render();
    SetColor(color);
}

///////////////////////////////////////
// class CUILinkTextMultiEdit
///////////////////////////////////////
CUILinkTextMultiEdit::CUILinkTextMultiEdit(const std::string& str, GG::Flags<GG::MultiEditStyle> style) :
    CUIMultiEdit(str, style),
    TextLinker(),
    m_already_setting_text_so_dont_link(false),
    m_raw_text(str)
{
    FindLinks();
    MarkLinks();
}

const std::vector<GG::Font::LineData>& CUILinkTextMultiEdit::GetLineData() const
{ return CUIMultiEdit::GetLineData(); }

const boost::shared_ptr<GG::Font>& CUILinkTextMultiEdit::GetFont() const
{ return CUIMultiEdit::GetFont(); }

GG::Pt CUILinkTextMultiEdit::TextUpperLeft() const
{ return CUIMultiEdit::TextUpperLeft() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5)); }

GG::Pt CUILinkTextMultiEdit::TextLowerRight() const
{ return CUIMultiEdit::TextLowerRight() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5)); }

const std::string& CUILinkTextMultiEdit::RawText() const
{ return m_raw_text; }

void CUILinkTextMultiEdit::Render() {
    CUIMultiEdit::Render();
    TextLinker::Render_();
}

void CUILinkTextMultiEdit::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::LClick(pt, mod_keys);
    TextLinker::LClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::LDoubleClick(pt, mod_keys);
    TextLinker::LDoubleClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::RClick(pt, mod_keys);
    TextLinker::RClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::MouseHere(pt, mod_keys);
    TextLinker::MouseHere_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseLeave() {
    CUIMultiEdit::MouseLeave();
    TextLinker::MouseLeave_();
}

void CUILinkTextMultiEdit::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt lower_right = lr;
    if (Style() & GG::MULTI_INTEGRAL_HEIGHT)
        lower_right.y -= ((lr.y - ul.y) - (2 * PIXEL_MARGIN)) % GetFont()->Lineskip();
    bool resized = lower_right - ul != Size();

    // need to restore scroll position after SetText call below, so that
    // resizing this control doesn't reset the scroll position to the top.
    // just calling PreserveTextPositionOnNextSetText() before the SetText
    // call doesn't work as that leaves the scrollbars unadjusted for the resize
    GG::Pt initial_scroll_pos = ScrollPosition();

    GG::Edit::SizeMove(ul, lower_right);

    if (resized) {
        SetText(RawText());                     // this line is the primary difference between this and MultiEdit::SizeMove
        SetScrollPosition(initial_scroll_pos);  // restores scroll position
    }
}

void CUILinkTextMultiEdit::SetText(const std::string& str) {
    // MultiEdit have scrollbars that are adjusted every time the text is set.  Adjusting scrollbars also requires
    // setting text, because the space for the text is added or removed when scrollbars are shown or hidden.
    // Since highlighting links on rollover also involves setting text, there are a lot of potentially unnecessary
    // calls to SetText and FindLinks.  This check for whether text is already being set eliminates many of those
    // calls when they aren't necessary, since the results will be overridden later anyway by the outermost (or
    // lowest on stack, or first) call to SetText
    if (!m_already_setting_text_so_dont_link) {
        m_already_setting_text_so_dont_link = true;
        m_raw_text = str;
        CUIMultiEdit::SetText(m_raw_text);  // so that line data is updated for use in FindLinks
        FindLinks();
        MarkLinks();
        m_already_setting_text_so_dont_link = false;
        // The scrollbar shenanigans apparently also confuse the link locations
        // so we refresh them here.
        LocateLinks();
        return;
    } else {
        CUIMultiEdit::SetText(str);
    }
}

void CUILinkTextMultiEdit::SetLinkedText(const std::string& str) {
    MultiEdit::PreserveTextPositionOnNextSetText();
    CUIMultiEdit::SetText(str);
}


///////////////////////////////////////
// class CUISimpleDropDownListRow
///////////////////////////////////////
// static(s)
const GG::Y CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT(22);

CUISimpleDropDownListRow::CUISimpleDropDownListRow(const std::string& row_text, GG::Y row_height/* = DEFAULT_ROW_HEIGHT*/) :
    GG::ListBox::Row(GG::X1, row_height, "")
{
    push_back(new CUILabel(row_text, GG::FORMAT_LEFT | GG::FORMAT_NOWRAP));
}


///////////////////////////////////////
// class StatisticIcon
///////////////////////////////////////
namespace {
    const int STAT_ICON_PAD = 2;    // horizontal or vertical space between icon and label
}

StatisticIcon::StatisticIcon(const boost::shared_ptr<GG::Texture> texture) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_num_values(0),
    m_values(),
    m_digits(),
    m_show_signs(),
    m_icon(0),
    m_text(0)
{
    m_icon = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC);

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    AttachChild(m_icon);

    DoLayout();
    Refresh();
}

StatisticIcon::StatisticIcon(const boost::shared_ptr<GG::Texture> texture,
                             double value, int digits, bool showsign) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_num_values(1),
    m_values(std::vector<double>(1, value)),
    m_digits(std::vector<int>(1, digits)),
    m_show_signs(std::vector<bool>(1, showsign)),
    m_icon(0),
    m_text(0)
{
    m_icon = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC);
    m_text = new CUILabel("");

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    AttachChild(m_icon);
    AttachChild(m_text);

    DoLayout();
    Refresh();
}

StatisticIcon::StatisticIcon(const boost::shared_ptr<GG::Texture> texture,
                             double value0, double value1, int digits0, int digits1,
                             bool showsign0, bool showsign1) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::INTERACTIVE),
    m_num_values(2),
    m_values(std::vector<double>(2, 0.0)),
    m_digits(std::vector<int>(2, 2)),
    m_show_signs(std::vector<bool>(2, false)),
    m_icon(0),
    m_text(0)
{
    SetName("StatisticIcon");
    m_icon = new GG::StaticGraphic(texture, GG::GRAPHIC_FITGRAPHIC);
    m_text = new CUILabel("");

    m_values[0] = value0;
    m_values[1] = value1;
    m_digits[0] = digits0;
    m_digits[1] = digits1;
    m_show_signs[0] = showsign0;
    m_show_signs[1] = showsign1;

    SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));

    AttachChild(m_icon);
    AttachChild(m_text);

    DoLayout();
    Refresh();
}

double StatisticIcon::GetValue(int index) const {
    if (index < 0) throw std::invalid_argument("negative index passed to StatisticIcon::Value");
    if (index > 1) throw std::invalid_argument("index greater than 1 passed to StatisticIcon::Value.  Only 1 or 2 values, with indices 0 or 1, supported.");
    if (index >= m_num_values) throw std::invalid_argument("index greater than largest index available passed to StatisticIcon::Value");
    return m_values[index];
}

void StatisticIcon::SetValue(double value, int index) {
    if (index < 0) throw std::invalid_argument("negative index passed to StatisticIcon::SetValue");
    if (index > 1) throw std::invalid_argument("index greater than 1 passed to StatisticIcon::SetValue.  Only 1 or 2 values, with indices 0 or 1, supported.");
    if (index + 1 > m_num_values) {
        m_num_values = index + 1;
        m_values.resize(m_num_values, 0.0);
        m_show_signs.resize(m_num_values, m_show_signs[0]);
        m_digits.resize(m_num_values, m_digits[0]);
    }
    m_values[index] = value;
    Refresh();
}

void StatisticIcon::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void StatisticIcon::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        LeftClickedSignal();
}

void StatisticIcon::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        RightClickedSignal();
}

void StatisticIcon::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::DoLayout() {
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    int icon_dim = std::min(Value(Height()), Value(Width()));
    m_icon->SizeMove(GG::Pt(GG::X0, GG::Y0),
                     GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

    if (!m_text)
        return;

    GG::Pt text_ul;
    GG::Pt text_lr(Width(), Height());

    if (Width() >= Value(Height())) {
        m_text->SetTextFormat(GG::FORMAT_LEFT);
        text_ul.x = GG::X(icon_dim + STAT_ICON_PAD);
    } else {
        m_text->SetTextFormat(GG::FORMAT_BOTTOM);
        text_ul.y = GG::Y(icon_dim + STAT_ICON_PAD);
    }

    m_text->SizeMove(text_ul, text_lr);
}

void StatisticIcon::Refresh() {
    if (!m_text)
        return;

    std::string text = "";

    // first value: always present
    std::string clr_tag = GG::RgbaTag(ValueColor(0));
    text += clr_tag + DoubleToString(m_values[0], m_digits[0], m_show_signs[0]) + "</rgba>";

    // second value: may or may not be present
    if (m_num_values > 1) {
        clr_tag = GG::RgbaTag(ValueColor(1));
        text += " (" + clr_tag + DoubleToString(m_values[1], m_digits[1], m_show_signs[1]) + "</rgba>)";
    }

    m_text->SetText(text);
}

GG::Clr StatisticIcon::ValueColor(int index) const {
    if (m_values.empty() || index >= m_num_values || index < 0)
        return ClientUI::TextColor();

    int effectiveSign = EffectiveSign(m_values.at(index));

    if (index == 0) return ClientUI::TextColor();

    if (effectiveSign == -1) return ClientUI::StatDecrColor();
    if (effectiveSign == 1) return ClientUI::StatIncrColor();

    return ClientUI::TextColor();
}


///////////////////////////////////////
// class CUIToolBar
///////////////////////////////////////
CUIToolBar::CUIToolBar() :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::ONTOP | GG::INTERACTIVE)
{}

bool CUIToolBar::InWindow(const GG::Pt& pt) const {
    const std::list<GG::Wnd*>& children = Children();
    for (std::list<GG::Wnd*>::const_iterator it = children.begin(); it != children.end(); ++it)
        if ((*it)->InWindow(pt))
            return true;
    return GG::Wnd::InWindow(pt);
}

void CUIToolBar::Render() {
    GG::Pt ul(UpperLeft() - GG::Pt(GG::X1, GG::Y1));
    GG::Pt lr(LowerRight() + GG::Pt(GG::X(1), GG::Y0));
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}


///////////////////////////////////////
// class SpeciesSelector
///////////////////////////////////////
namespace {
    static const std::string EMPTY_STRING("");

    // row type used in the SpeciesSelector
    struct SpeciesRow : public GG::ListBox::Row {
        SpeciesRow(const Species* species, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h, "", GG::ALIGN_VCENTER, 0)
        {
            if (!species)
                return;
            GG::Wnd::SetName(species->Name());
            GG::StaticGraphic* icon = new GG::StaticGraphic(ClientUI::SpeciesIcon(species->Name()), GG::GRAPHIC_FITGRAPHIC);
            icon->Resize(GG::Pt(GG::X(Value(h) - 6), h - 6));
            push_back(icon);
            GG::Label* species_name = new CUILabel(UserString(species->Name()), GG::FORMAT_LEFT);
            species_name->Resize(GG::Pt(Width() - GG::X(Value(h)), h));
            push_back(species_name);
            GG::X first_col_width(Value(h) - 10);
            SetColWidth(0, first_col_width);
            SetColWidth(1, w - first_col_width);
            SetBrowseModeTime(GetOptionsDB().Get<int>("UI.tooltip-delay"));
            SetBrowseInfoWnd(boost::shared_ptr<GG::BrowseInfoWnd>(
                new IconTextBrowseWnd(ClientUI::SpeciesIcon(species->Name()),
                                      UserString(species->Name()),
                                      species->GameplayDescription())
                                     )
                            );
        }
    };
}

SpeciesSelector::SpeciesSelector(GG::X w, GG::Y h) :
    CUIDropDownList(6)
{
    Resize(GG::Pt(w, h - 8));
    const SpeciesManager& sm = GetSpeciesManager();
    for (SpeciesManager::playable_iterator it = sm.playable_begin(); it != sm.playable_end(); ++it)
        Insert(new SpeciesRow(it->second, w, h - 4));
    if (!this->Empty())
        Select(this->begin());
    GG::Connect(SelChangedSignal, &SpeciesSelector::SelectionChanged, this);
}

SpeciesSelector::SpeciesSelector(GG::X w, GG::Y h, const std::vector<std::string>& species_names) :
    CUIDropDownList(6)
{
    Resize(GG::Pt(w, h - 8));
    SetSpecies(species_names);
    GG::Connect(SelChangedSignal, &SpeciesSelector::SelectionChanged, this);
}

const std::string& SpeciesSelector::CurrentSpeciesName() const {
    CUIDropDownList::iterator row_it = this->CurrentItem();
    if (row_it == this->end())
        return EMPTY_STRING;
    const CUIDropDownList::Row* row = *row_it;
    if (!row) {
        ErrorLogger() << "SpeciesSelector::CurrentSpeciesName couldn't get current item due to invalid Row pointer";
        return EMPTY_STRING;
    }
    return row->Name();
}

std::vector<std::string> SpeciesSelector::AvailableSpeciesNames() const {
    std::vector<std::string> retval;
    for (CUIDropDownList::const_iterator row_it = this->begin(); row_it != this->end(); ++row_it)
        if (const SpeciesRow* species_row = dynamic_cast<const SpeciesRow*>(*row_it))
            retval.push_back(species_row->Name());
    return retval;
}

void SpeciesSelector::SelectSpecies(const std::string& species_name) {
    for (CUIDropDownList::iterator row_it = this->begin(); row_it != this->end(); ++row_it) {
        CUIDropDownList::Row* row = *row_it;
        if (const SpeciesRow* species_row = dynamic_cast<const SpeciesRow*>(row)) {
            if (species_row->Name() == species_name) {
                Select(row_it);
                return;
            }
        }
    }
    ErrorLogger() << "SpeciesSelector::SelectSpecies was unable to find a species in the list with name " << species_name;
}

void SpeciesSelector::SetSpecies(const std::vector<std::string>& species_names) {
    const std::string& previous_selection = CurrentSpeciesName();
    bool selection_changed = (previous_selection == "");

    Clear();

    for (std::vector<std::string>::const_iterator it = species_names.begin(); it != species_names.end(); ++it) {
        const std::string& species_name = *it;
        if (const Species* species = GetSpecies(species_name)) {
            CUIDropDownList::iterator it = Insert(new SpeciesRow(species, this->Width(), this->Height() - 4));
            if (species_name == previous_selection) {
                Select(it);
                selection_changed = false;
            }
        } else {
            ErrorLogger() << "SpeciesSelector::SpeciesSelector couldn't find species with name: " << species_name;
        }
    }
    if (selection_changed)
        SelectionChanged(this->CurrentItem());
}

void SpeciesSelector::SelectionChanged(GG::DropDownList::iterator it) {
    const GG::ListBox::Row* row = 0;
    if (it != this->end())
        row = *it;
    if (row)
        SpeciesChangedSignal(row->Name());
    else
        SpeciesChangedSignal(EMPTY_STRING);
}


///////////////////////////////////////
// class EmpireColorSelector
///////////////////////////////////////
namespace {
    const GG::X COLOR_SELECTOR_WIDTH(75);

    // row type used in the EmpireColorSelector
    struct ColorRow : public GG::ListBox::Row {
        struct ColorSquare : GG::Control {
            ColorSquare(const GG::Clr& color, GG::Y h) :
                GG::Control(GG::X0, GG::Y0, COLOR_SELECTOR_WIDTH - 40, h, GG::NO_WND_FLAGS)
            {
                SetColor(color);
            }
            virtual void Render() {
                GG::FlatRectangle(UpperLeft(), LowerRight(), Color(), GG::CLR_ZERO, 0);
            }
        };
        ColorRow(const GG::Clr& color, GG::Y h) :
            GG::ListBox::Row(GG::X(Value(h)), h, "")
        {
            push_back(new ColorSquare(color, h));
        }
    };
}

EmpireColorSelector::EmpireColorSelector(GG::Y h) :
    CUIDropDownList(6)
{
    Resize(GG::Pt(COLOR_SELECTOR_WIDTH, h - 8));
    const std::vector<GG::Clr>& colors = EmpireColors();
    for (unsigned int i = 0; i < colors.size(); ++i) {
        Insert(new ColorRow(colors[i], h - 4));
    }
    GG::Connect(SelChangedSignal, &EmpireColorSelector::SelectionChanged, this);
}

GG::Clr EmpireColorSelector::CurrentColor() const
{ return (**CurrentItem())[0]->Color(); }

void EmpireColorSelector::SelectColor(const GG::Clr& clr) {
    for (iterator list_it = begin(); list_it != end(); ++list_it) {
        const GG::ListBox::Row* row = *list_it;
        if (row && !row->empty() && (*row)[0]->Color() == clr) {
            Select(list_it);
            return;
        }
    }
    ErrorLogger() << "EmpireColorSelector::SelectColor was unable to find a requested color!";
}

void EmpireColorSelector::SelectionChanged(GG::DropDownList::iterator it) {
    const GG::ListBox::Row* row = *it;
    if (row && !row->empty())
        ColorChangedSignal((*row)[0]->Color());
    else
        ErrorLogger() << "EmpireColorSelector::SelectionChanged had trouble getting colour from row!";
}


///////////////////////////////////////
// class ColorSelector
///////////////////////////////////////
ColorSelector::ColorSelector(GG::Clr color, GG::Clr default_color) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1),
    m_default_color(default_color)
{ SetColor(color); }

ColorSelector::~ColorSelector()
{ m_border_buffer.clear(); }

void ColorSelector::InitBuffer() {
    GG::Pt sz = Size();
    m_border_buffer.clear();
    m_border_buffer.store(0.0f,        0.0f);
    m_border_buffer.store(Value(sz.x), 0.0f);
    m_border_buffer.store(Value(sz.x), Value(sz.y));
    m_border_buffer.store(0.0f,        Value(sz.y));
}

void ColorSelector::Render() {
    GG::Pt ul = UpperLeft();

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(static_cast<GLfloat>(Value(ul.x)), static_cast<GLfloat>(Value(ul.y)), 0.0f);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(1.0f);
    glEnableClientState(GL_VERTEX_ARRAY);

    m_border_buffer.activate();
    glColor(Color());
    glDrawArrays(GL_TRIANGLE_FAN,   0, m_border_buffer.size());
    glColor(GG::CLR_WHITE);
    glDrawArrays(GL_LINE_LOOP,      0, m_border_buffer.size());

    glEnable(GL_TEXTURE_2D);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
}

void ColorSelector::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Control::Size();

    GG::Control::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        InitBuffer();
}

void ColorSelector::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    GG::X x = std::min(pt.x, GG::GUI::GetGUI()->AppWidth() - 315);    // 315 is width of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::Y y = std::min(pt.y, GG::GUI::GetGUI()->AppHeight() - 300);   // 300 is height of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::ColorDlg dlg(x, y, Color(), ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::TextColor());
    dlg.Run();
    if (dlg.ColorWasSelected()) {
        GG::Clr clr = dlg.Result();
        SetColor(clr);
        ColorChangedSignal(clr);
    }
}

void ColorSelector::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys) {
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RESET"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents, ClientUI::TextColor(),
                        ClientUI::WndOuterBorderColor(), ClientUI::WndColor(), ClientUI::EditHiliteColor());

    if (popup.Run()) {
        switch (popup.MenuID()) {
        case 1: // reset colour option to default value
            SetColor(m_default_color);
            ColorChangedSignal(m_default_color);
        default:
            break;
        }
    }
}


///////////////////////////////////////
// class FileDlg
///////////////////////////////////////
FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 const std::vector<std::pair<std::string, std::string> >& types) :
    GG::FileDlg(directory, filename, save, multi, ClientUI::GetFont(),
                ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::TextColor())
{
    SetFileFilters(types);
    AppendMissingSaveExtension(true);
}


//////////////////////////////////////////////////
// ProductionInfoPanel
//////////////////////////////////////////////////
namespace {
    GG::Y VERTICAL_SECTION_GAP(4);
}

ProductionInfoPanel::ProductionInfoPanel(const std::string& title, const std::string& point_units_str,
                                         GG::X w, GG::Y h) :
    CUIWnd(title, GG::X0, GG::Y0, GG::X(120), GG::Y(120),
           GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | PINABLE),
    m_units_str(point_units_str),
    m_title_str(title),
    m_total_points_label(0),
    m_total_points(0),
    m_total_points_P_label(0),
    m_wasted_points_label(0),
    m_wasted_points(0),
    m_wasted_points_P_label(0),
    m_local_points_label(0),
    m_local_points(0),
    m_local_points_P_label(0),
    m_local_wasted_points_label(0),
    m_local_wasted_points(0),
    m_local_wasted_points_P_label(0)
{}

GG::Pt ProductionInfoPanel::MinUsableSize() const {
    GG::X min_x = this->LeftBorder() + this->RightBorder() + 20*ClientUI::Pts();
    GG::Y min_y = this->TopBorder() + this->BottomBorder() + 6*ClientUI::Pts();
    return GG::Pt(min_x, min_y);
}

void ProductionInfoPanel::SizeMove(const GG::Pt& ul, const GG::Pt& lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ProductionInfoPanel::SetTotalPointsCost(float total_points, float total_cost) {
    if (!m_total_points_label) {
        m_total_points_label = new CUILabel(UserString("PRODUCTION_INFO_TOTAL_PS_LABEL"), GG::FORMAT_RIGHT);
        m_total_points = new CUILabel("", GG::FORMAT_LEFT);
        m_total_points_P_label = new CUILabel(m_units_str, GG::FORMAT_LEFT);

        m_wasted_points_label = new CUILabel(UserString("PRODUCTION_INFO_WASTED_PS_LABEL"), GG::FORMAT_RIGHT);
        m_wasted_points = new CUILabel("", GG::FORMAT_LEFT);
        m_wasted_points_P_label = new CUILabel(m_units_str, GG::FORMAT_LEFT);

        AttachChild(m_total_points_label);
        AttachChild(m_total_points);
        AttachChild(m_total_points_P_label);

        AttachChild(m_wasted_points_label);
        AttachChild(m_wasted_points);
        AttachChild(m_wasted_points_P_label);

        DoLayout();
    }

    float wasted_points = std::max(0.0f, total_points - total_cost);
    *m_total_points << DoubleToString(total_points, 3, false);
    *m_wasted_points << DoubleToString(wasted_points, 3, false);
    if (wasted_points > 0.01f)
        m_wasted_points->SetTextColor(ClientUI::StatDecrColor());
    else
        m_wasted_points->SetTextColor(ClientUI::TextColor());
}

void ProductionInfoPanel::SetLocalPointsCost(float local_points, float local_cost, const std::string& location_name) {
    if (!m_local_points_label) {
        m_local_points_label = new CUILabel(UserString("PRODUCTION_INFO_LOCAL_PS_LABEL"), GG::FORMAT_RIGHT);
        m_local_points = new CUILabel("", GG::FORMAT_LEFT);
        m_local_points_P_label = new CUILabel(m_units_str, GG::FORMAT_LEFT);

        m_local_wasted_points_label = new CUILabel(UserString("PRODUCTION_INFO_WASTED_PS_LABEL"), GG::FORMAT_RIGHT);
        m_local_wasted_points = new CUILabel("", GG::FORMAT_LEFT);
        m_local_wasted_points_P_label = new CUILabel(m_units_str, GG::FORMAT_LEFT);

        AttachChild(m_local_points_label);
        AttachChild(m_local_points);
        AttachChild(m_local_points_P_label);

        AttachChild(m_local_wasted_points_label);
        AttachChild(m_local_wasted_points);
        AttachChild(m_local_wasted_points_P_label);

        DoLayout();
    }

    float wasted_points = std::max(0.0f, local_points - local_cost);
    *m_local_points << DoubleToString(local_points, 3, false);
    *m_local_wasted_points << DoubleToString(wasted_points, 3, false);
    if (wasted_points > 0.01f)
        m_local_wasted_points->SetTextColor(ClientUI::StatDecrColor());
    else
        m_local_wasted_points->SetTextColor(ClientUI::TextColor());
    SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_INFO_AT_LOCATION_TITLE")) % m_title_str % location_name));
}

void ProductionInfoPanel::ClearLocalInfo() {
    delete m_local_points_label;            m_local_points_label = 0;
    delete m_local_points;                  m_local_points = 0;
    delete m_local_points_P_label;          m_local_points_P_label = 0;
    delete m_local_wasted_points_label;     m_local_wasted_points_label = 0;
    delete m_local_wasted_points;           m_local_wasted_points = 0;
    delete m_local_wasted_points_P_label;   m_local_wasted_points_P_label = 0;

    SetName(m_title_str);
}

void ProductionInfoPanel::DoLayout() {
    const int STAT_TEXT_PTS = ClientUI::Pts();
    const int CENTERLINE_GAP = 4;
    const GG::X LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 2 / 3;
    const GG::X VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const GG::X LEFT_TEXT_X(0);
    const GG::X RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const GG::X P_LABEL_X = RIGHT_TEXT_X + 40;

    std::pair<int, int> m_center_gap(Value(LABEL_TEXT_WIDTH + 2), Value(LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP));

    const GG::Pt LABEL_TEXT_SIZE(LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt VALUE_TEXT_SIZE(VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt P_LABEL_SIZE(Width() - 2 - 5 - P_LABEL_X, GG::Y(STAT_TEXT_PTS + 4));

    GG::Y row_offset(4);

    if (m_total_points_label) {
        m_total_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
        m_total_points_label->Resize(LABEL_TEXT_SIZE);
        m_total_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
        m_total_points->Resize(VALUE_TEXT_SIZE);
        m_total_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
        m_total_points_P_label->Resize(P_LABEL_SIZE);

        row_offset += m_total_points_label->Height();
    }

    if (m_wasted_points_label) {
        m_wasted_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
        m_wasted_points_label->Resize(LABEL_TEXT_SIZE);
        m_wasted_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
        m_wasted_points->Resize(VALUE_TEXT_SIZE);
        m_wasted_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
        m_wasted_points_P_label->Resize(P_LABEL_SIZE);

        row_offset += m_wasted_points_label->Height();
    }

    if (m_local_points_label) {
        m_local_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
        m_local_points_label->Resize(LABEL_TEXT_SIZE);
        m_local_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
        m_local_points->Resize(VALUE_TEXT_SIZE);
        m_local_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
        m_local_points_P_label->Resize(P_LABEL_SIZE);

        row_offset += m_local_points_label->Height();
    }

    if (m_local_wasted_points_label) {
        m_local_wasted_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
        m_local_wasted_points_label->Resize(LABEL_TEXT_SIZE);
        m_local_wasted_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
        m_local_wasted_points->Resize(VALUE_TEXT_SIZE);
        m_local_wasted_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
        m_local_wasted_points_P_label->Resize(P_LABEL_SIZE);
    }
}


//////////////////////////////////////////////////
// MultiTurnProgressBar
//////////////////////////////////////////////////
MultiTurnProgressBar::MultiTurnProgressBar(int total_turns, double turns_completed,
                                           const GG::Clr& bar_color, const GG::Clr& background,
                                           const GG::Clr& outline_color) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_total_turns(std::max(1, total_turns)),
    m_turns_completed(std::max(0.0, std::min<double>(turns_completed, m_total_turns))),
    m_bar_color(bar_color),
    m_background(background),
    m_outline_color(outline_color)
{}

void MultiTurnProgressBar::Render() {
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    bool segmented = true;
    if (Width() / m_total_turns < 3)
        segmented = false;

    // draw background over whole area
    FlatRectangle(ul, lr, m_background, m_outline_color, 1);
    // draw completed portion bar
    GG::X completed_bar_width(std::max(1.0, Value(Width() * m_turns_completed / m_total_turns)));
    if (completed_bar_width > 3) {
        GG::Pt bar_lr(ul.x + completed_bar_width, lr.y);
        FlatRectangle(ul, bar_lr, m_bar_color, m_outline_color, 1);
    }
    // draw segment separators
    if (segmented) {
        glColor(GG::DarkColor(m_bar_color));
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
        for (int n = 1; n < m_total_turns; ++n) {
            GG::X separator_x = ul.x + Width() * n / m_total_turns;
            if (separator_x > ul.x + completed_bar_width)
                glColor(GG::LightColor(m_background));            
            glVertex(separator_x, ul.y);
            glVertex(separator_x, lr.y);
        }
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
}


//////////////////////////////////////////////////
// FPSIndicator
//////////////////////////////////////////////////
FPSIndicator::FPSIndicator(void) :
    GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_NOWRAP, GG::ONTOP)
{
    GG::Connect(GetOptionsDB().OptionChangedSignal("show-fps"), &FPSIndicator::UpdateEnabled, this);
    UpdateEnabled();
}

void FPSIndicator::Render() {
    if (m_enabled) {
        SetText(boost::io::str(FlexibleFormat(UserString("MAP_INDICATOR_FPS")) % static_cast<int>(GG::GUI::GetGUI()->FPS())));
        TextControl::Render();
    }
}

void FPSIndicator::UpdateEnabled()
{ m_enabled = GetOptionsDB().Get<bool>("show-fps"); }


//////////////////////////////////////////////////
// MultiTextureStaticGraphic
//////////////////////////////////////////////////
/** creates a MultiTextureStaticGraphic from multiple pre-existing Textures which are rendered back-to-front in the
  * order they are specified in \a textures with GraphicStyles specified in the same-indexed value of \a styles.
  * if \a styles is not specified or contains fewer entres than \a textures, entries in \a textures without 
  * associated styles use the style GRAPHIC_NONE. */
MultiTextureStaticGraphic::MultiTextureStaticGraphic(const std::vector<boost::shared_ptr<GG::Texture> >& textures,
                                                     const std::vector<GG::Flags<GG::GraphicStyle> >& styles) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_graphics(),
    m_styles(styles)
{
    for (std::vector<boost::shared_ptr<GG::Texture> >::const_iterator it = textures.begin(); it != textures.end(); ++it)
        m_graphics.push_back(GG::SubTexture(*it, GG::X0, GG::Y0, (*it)->DefaultWidth(), (*it)->DefaultHeight()));
    Init();
}

MultiTextureStaticGraphic::MultiTextureStaticGraphic(const std::vector<GG::SubTexture>& subtextures,
                                                     const std::vector<GG::Flags<GG::GraphicStyle> >& styles) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_graphics(subtextures),
    m_styles(styles)
{ Init(); }

MultiTextureStaticGraphic::MultiTextureStaticGraphic() :
    m_graphics(),
    m_styles()
{}

GG::Rect MultiTextureStaticGraphic::RenderedArea(const GG::SubTexture& subtexture,
                                                 GG::Flags<GG::GraphicStyle> style) const
{
    // copied from GG::StaticGraphic
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt window_sz(lr - ul);
    GG::Pt graphic_sz(subtexture.Width(), subtexture.Height());
    GG::Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
    if (style & GG::GRAPHIC_FITGRAPHIC) {
        if (style & GG::GRAPHIC_PROPSCALE) {
            double scale_x = Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x));
            double scale_y = Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y));
            double scale = std::min(scale_x, scale_y);
            pt2.x = graphic_sz.x * scale;
            pt2.y = graphic_sz.y * scale;
        } else {
            pt2 = window_sz;
        }
    } else if (style & GG::GRAPHIC_SHRINKFIT) {
        if (style & GG::GRAPHIC_PROPSCALE) {
            double scale_x = (graphic_sz.x > window_sz.x) ? Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x)) : 1.0;
            double scale_y = (graphic_sz.y > window_sz.y) ? Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y)) : 1.0;
            double scale = std::min(scale_x, scale_y);
            pt2.x = graphic_sz.x * scale;
            pt2.y = graphic_sz.y * scale;
        } else {
            pt2 = window_sz;
        }
    }

    GG::X x_shift(0);
    if (style & GG::GRAPHIC_LEFT) {
        x_shift = ul.x;
    } else if (style & GG::GRAPHIC_CENTER) {
        x_shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
    } else { // style & GG::GRAPHIC_RIGHT
        x_shift = lr.x - (pt2.x - pt1.x);
    }
    pt1.x += x_shift;
    pt2.x += x_shift;

    GG::Y y_shift(0);
    if (style & GG::GRAPHIC_TOP) {
        y_shift = ul.y;
    } else if (style & GG::GRAPHIC_VCENTER) {
        y_shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
    } else { // style & GRAPHIC_BOTTOM
        y_shift = lr.y - (pt2.y - pt1.y);
    }
    pt1.y += y_shift;
    pt2.y += y_shift;

    return GG::Rect(pt1, pt2);
}

void MultiTextureStaticGraphic::Render() {
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);
    for (std::vector<GG::SubTexture>::size_type i = 0; i < m_graphics.size(); ++i) {
        GG::Rect rendered_area = RenderedArea(m_graphics[i], m_styles[i]);
        m_graphics[i].OrthoBlit(rendered_area.ul, rendered_area.lr);
    }
}

void MultiTextureStaticGraphic::Init() {
    ValidateStyles();
    SetColor(GG::CLR_WHITE);
}

void MultiTextureStaticGraphic::ValidateStyles() {
    // ensure enough styles for graphics
    unsigned int num_graphics = m_graphics.size();
    m_styles.resize(num_graphics, GG::GRAPHIC_CENTER);

    for (std::vector<GG::Flags<GG::GraphicStyle> >::iterator it = m_styles.begin(); it != m_styles.end(); ++it) {
        GG::Flags<GG::GraphicStyle>& style = *it;

        int dup_ct = 0;   // duplication count
        if (style & GG::GRAPHIC_LEFT) ++dup_ct;
        if (style & GG::GRAPHIC_RIGHT) ++dup_ct;
        if (style & GG::GRAPHIC_CENTER) ++dup_ct;
        if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GG::GRAPHIC_CENTER by default
            style &= ~(GG::GRAPHIC_RIGHT | GG::GRAPHIC_LEFT);
            style |= GG::GRAPHIC_CENTER;
        }
        dup_ct = 0;
        if (style & GG::GRAPHIC_TOP) ++dup_ct;
        if (style & GG::GRAPHIC_BOTTOM) ++dup_ct;
        if (style & GG::GRAPHIC_VCENTER) ++dup_ct;
        if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GG::GRAPHIC_VCENTER by default
            style &= ~(GG::GRAPHIC_TOP | GG::GRAPHIC_BOTTOM);
            style |= GG::GRAPHIC_VCENTER;
        }
        dup_ct = 0;
        if (style & GG::GRAPHIC_FITGRAPHIC) ++dup_ct;
        if (style & GG::GRAPHIC_SHRINKFIT) ++dup_ct;
        if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use GG::GRAPHIC_SHRINKFIT by default
            style &= ~GG::GRAPHIC_FITGRAPHIC;
            style |= GG::GRAPHIC_SHRINKFIT;
        }
    }
}
