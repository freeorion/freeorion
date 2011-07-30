#include "CUIControls.h"

#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUISpin.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"
#include "../universe/Species.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/dialogs/ColorDlg.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <limits>


namespace {
    void PlayButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-click"), true); }

    void PlayTurnButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.turn-button-click"), true); }

    struct PlayButtonCheckSound
    {
        PlayButtonCheckSound(bool play_only_when_checked) : m_play_only_when_checked(play_only_when_checked) {}
        void operator()(bool checked) const
        {
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

    boost::shared_ptr<GG::Font> FontOrDefaultFont(const boost::shared_ptr<GG::Font>& font)
    {
        return font ? font : ClientUI::GetFont();
    }

    const double ARROW_BRIGHTENING_SCALE_FACTOR = 1.5;
    const double STATE_BUTTON_BRIGHTENING_SCALE_FACTOR = 1.25;
    const double TAB_BRIGHTENING_SCALE_FACTOR = 1.25;
}


///////////////////////////////////////
// class CUIButton
///////////////////////////////////////
namespace {
    const int CUIBUTTON_ANGLE_OFFSET = 5;
}

CUIButton::CUIButton(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                     GG::Clr color/* = ClientUI::CtrlColor()*/,
                     GG::Clr border/* = ClientUI::CtrlBorderColor()*/, int thick/* = 2*/, 
                     GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    Button(x, y, w, FontOrDefaultFont(font)->Lineskip() + 6, str, FontOrDefaultFont(font), color, text_color, flags),
    m_border_color(border),
    m_border_thick(thick)
{
    GG::Connect(ClickedSignal, &PlayButtonClickSound, -1);
}

bool CUIButton::InWindow(const GG::Pt& pt) const
{
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    return InAngledCornerRect(pt, ul, lr, CUIBUTTON_ANGLE_OFFSET);
}

void CUIButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        SetState(BN_ROLLOVER);
    }
}

void CUIButton::SetBorderColor(GG::Clr clr)
{
    m_border_color = clr;
}

void CUIButton::SetBorderThick(int thick)
{
    m_border_thick = std::max(thick, 0);    // don't allow negative thickness borders
}

void CUIButton::RenderPressed()
{
    GG::Clr color_to_use = Color();
    AdjustBrightness(color_to_use, 25);
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul, lr, color_to_use, m_border_color, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    OffsetMove(GG::Pt(GG::X1, GG::Y1));
    TextControl::Render();
    OffsetMove(GG::Pt(-GG::X1, -GG::Y1));
}

void CUIButton::RenderRollover()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = m_border_color;
    AdjustBrightness(border_color_to_use, 100);
    if (Disabled())
        border_color_to_use = DisabledColor(m_border_color);
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul, lr, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}

void CUIButton::RenderUnpressed()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul, lr, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}

void CUIButton::MarkNotSelected()
{
    SetColor(ClientUI::CtrlColor());
    SetBorderColor(ClientUI::CtrlBorderColor());
    SetBorderThick(1);
}

void CUIButton::MarkSelectedGray()
{
    GG::Clr colour = ClientUI::CtrlColor();
    AdjustBrightness(colour, 50);
    SetColor(colour);

    colour = ClientUI::CtrlBorderColor();
    AdjustBrightness(colour, 50);
    SetBorderColor(colour);

    SetBorderThick(2);
}

void CUIButton::MarkSelectedTechCategoryColor(std::string category)
{
    GG::Clr cat_colour = ClientUI::CategoryColor(category);
    SetBorderColor(cat_colour);
    AdjustBrightness(cat_colour, -50);
    SetColor(cat_colour);
    SetBorderThick(2);
}


///////////////////////////////////////
// class SettableInWindowCUIButton
///////////////////////////////////////
SettableInWindowCUIButton::SettableInWindowCUIButton(GG::X x, GG::Y y, GG::X w, const std::string& str,
                                                     const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                                                     GG::Clr color/* = ClientUI::CtrlColor()*/,
                                                     GG::Clr border/* = ClientUI::CtrlBorderColor()*/,
                                                     int thick/* = 2*/, 
                                                     GG::Clr text_color/* = ClientUI::TextColor()*/,
                                                     GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    CUIButton(x, y, w, str, font, color, border, thick, text_color, flags)
{}

bool SettableInWindowCUIButton::InWindow(const GG::Pt& pt) const
{
    if (m_in_window_func)
        return m_in_window_func(pt);
    else
        return CUIButton::InWindow(pt);
}

void SettableInWindowCUIButton::SetInWindow(boost::function<bool(const GG::Pt&)> in_window_function)
{
    m_in_window_func = in_window_function;
}


///////////////////////////////////////
// class CUITurnButton
///////////////////////////////////////
CUITurnButton::CUITurnButton(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                             GG::Clr color/* = ClientUI::WndColor()*/, 
                             GG::Clr border/* = ClientUI::CtrlBorderColor()*/, int thick/* = 2*/, 
                             GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) : 
    CUIButton(x, y, w, str, FontOrDefaultFont(font), color, border, thick, text_color, flags)
{
    GG::Connect(ClickedSignal, &PlayTurnButtonClickSound, -1);
}


///////////////////////////////////////
// class CUIArrowButton
///////////////////////////////////////
CUIArrowButton::CUIArrowButton(GG::X x, GG::Y y, GG::X w, GG::Y h, ShapeOrientation orientation, GG::Clr color, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    Button(x, y, w, h, "", boost::shared_ptr<GG::Font>(), color, GG::CLR_ZERO, flags),
    m_orientation(orientation),
    m_fill_background_with_wnd_color (false)
{
    GG::Connect(ClickedSignal, &PlayButtonClickSound, -1);
}

bool CUIArrowButton::InWindow(const GG::Pt& pt) const
{
    if (m_fill_background_with_wnd_color) {
        return Button::InWindow(pt);
    } else {
        GG::Pt ul = UpperLeft() + GG::Pt(GG::X(3), GG::Y(1)), lr = LowerRight() - GG::Pt(GG::X(2), GG::Y(1));
        return InIsoscelesTriangle(pt, ul, lr, m_orientation);
    }
}

bool CUIArrowButton::FillBackgroundWithWndColor() const
{ return m_fill_background_with_wnd_color; }

void CUIArrowButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        SetState(BN_ROLLOVER);
    }
}

void CUIArrowButton::FillBackgroundWithWndColor(bool fill)
{ m_fill_background_with_wnd_color = fill; }

void CUIArrowButton::RenderPressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    OffsetMove(GG::Pt(GG::X1, GG::Y1));
    RenderUnpressed();
    OffsetMove(GG::Pt(-GG::X1, -GG::Y1));
}

void CUIArrowButton::RenderRollover()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul, lr, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    if (!Disabled())
        AdjustBrightness(color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
    GG::Pt tri_ul = ul + GG::Pt(GG::X(3), GG::Y1), tri_lr = lr - GG::Pt(GG::X(2), GG::Y1);
    IsoscelesTriangle(tri_ul, tri_lr, m_orientation, color_to_use);
}

void CUIArrowButton::RenderUnpressed()
{
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
CUIStateButton::CUIStateButton(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::TextFormat> format, GG::StateButtonStyle style/* = GG::SBSTYLE_3D_CHECKBOX*/,
                               GG::Clr color/* = ClientUI::CtrlColor()*/, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                               GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                               GG::Clr border/* = ClientUI::CtrlBorderColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    StateButton(x, y, w, h, str, FontOrDefaultFont(font), format, color, text_color, interior, style, flags),
    m_border_color(border),
    m_mouse_here(false)
{
    if (style == GG::SBSTYLE_3D_TOP_DETACHED_TAB || style == GG::SBSTYLE_3D_TOP_ATTACHED_TAB) {
        SetColor(ClientUI::WndColor());
        SetTextColor(DarkColor(text_color));
    }
    // HACK! radio buttons should only emit sounds when they are checked, and *not* when they are unchecked; currently, there's no 
    // other way to detect the difference between these two kinds of CUIStateButton within the CUIStateButton ctor other than
    // checking the redering style
    GG::Connect(CheckedSignal, PlayButtonCheckSound(style == GG::SBSTYLE_3D_RADIO), -1);
}

GG::Pt CUIStateButton::MinUsableSize() const
{
    // HACK! This code assumes that the text_format flag GG::FORMAT_VCENTER is in effect.  This is currently the case for
    // all of CUIStateButton in FO.
    GG::Pt retval = StateButton::MinUsableSize();
    retval.y = TextControl::MinUsableSize().y;
    return retval;
}

void CUIStateButton::Render()
{
    if (static_cast<int>(Style()) == GG::SBSTYLE_3D_CHECKBOX || 
        static_cast<int>(Style()) == GG::SBSTYLE_3D_RADIO) {
        // draw button
        GG::Pt bn_ul = ClientUpperLeft() + ButtonUpperLeft();
        GG::Pt bn_lr = ClientUpperLeft() + ButtonLowerRight();
        GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
        GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();
        GG::Clr border_color_to_use = Disabled() ? DisabledColor(BorderColor()) : BorderColor();
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
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor(outside_color);
                glBegin(GL_POLYGON);
                glVertex(bn_lr.x, bn_ul.y);
                glVertex(bn_ul.x + OFFSET, bn_ul.y);
                glVertex(bn_ul.x, bn_ul.y + OFFSET);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_ul.x, bn_lr.y);
                glVertex(bn_lr.x - OFFSET, bn_lr.y);
                glVertex(bn_lr.x, bn_lr.y - OFFSET);
                glVertex(bn_lr.x, bn_ul.y);
                glEnd();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
        OffsetMove(TextUpperLeft());
        TextControl::Render();
        OffsetMove(-TextUpperLeft());
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
        OffsetMove(TextUpperLeft() + additional_text_offset);
        TextControl::Render();
        OffsetMove(-(TextUpperLeft() + additional_text_offset));
    } else {
        StateButton::Render();
    }
}

void CUIStateButton::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_mouse_here = true;
}

void CUIStateButton::MouseLeave()
{
    m_mouse_here = false;
}


///////////////////////////////////////
// class CUITabBar
///////////////////////////////////////
CUITabBar::CUITabBar(GG::X x, GG::Y y, GG::X w, const boost::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color,
                     GG::TabBarStyle style, GG::Flags<GG::WndFlag> flags) :
    GG::TabBar(x, y, w, font, color, text_color, style, flags)
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
    Button(GG::X(orientation == GG::VERTICAL ? 0 : 2),
           GG::Y(orientation == GG::VERTICAL ? 2 : 0),
           GG::X(scroll_width), GG::Y(scroll_width),
           "", boost::shared_ptr<GG::Font>(), color),
    m_border_color(border_color),
    m_orientation(orientation),
    m_mouse_here(false),
    m_being_dragged(false)
{
    SetMinSize(GG::Pt(m_orientation == GG::VERTICAL ? MinSize().x : GG::X(10),
                      m_orientation == GG::VERTICAL ? GG::Y(10) : MinSize().y));
}

void CUIScroll::ScrollTab::SetColor(GG::Clr c)
{
    // ignore
}

void CUIScroll::ScrollTab::Render()
{
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
{
    m_being_dragged = true;
}

void CUIScroll::ScrollTab::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_being_dragged = false;
    if (!InWindow(GG::GUI::GetGUI()->MousePosition()))
        m_mouse_here = false;
}

void CUIScroll::ScrollTab::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    m_being_dragged = false;
}

void CUIScroll::ScrollTab::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!m_being_dragged && !m_mouse_here) {
        Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        m_mouse_here = true;
    }
}

void CUIScroll::ScrollTab::MouseLeave()
{
    if (!m_being_dragged)
        m_mouse_here = false;
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
CUIScroll::CUIScroll(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Orientation orientation,
                     GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/, GG::Clr interior_color/* = ClientUI::CtrlColor()*/, 
                     GG::Flags<GG::WndFlag> flags/* = INTERACTIVE | REPEAT_BUTTON_DOWN*/) :
    Scroll(x, y, w, h, orientation, interior_color, interior_color, flags),
    m_border_color(border_color)
{}

void CUIScroll::Render()
{
    GG::Clr color_to_use =          Disabled() ? DisabledColor(Color())         :   Color();
    GG::Clr border_color_to_use =   Disabled() ? DisabledColor(m_border_color)  :   m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    FlatRectangle(ul, lr, color_to_use, border_color_to_use, 1);
}

void CUIScroll::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    TabButton()->SizeMove(TabButton()->RelativeUpperLeft(), 
                          (ScrollOrientation() == GG::VERTICAL) ?
                          GG::Pt(Size().x, TabButton()->RelativeLowerRight().y) :
                          GG::Pt(TabButton()->RelativeLowerRight().x, Size().y));
    SizeScroll(ScrollRange().first, ScrollRange().second, LineSize(), PageSize()); // update tab size and position
}


///////////////////////////////////////
// class CUIListBox
///////////////////////////////////////
CUIListBox::CUIListBox(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/, 
                       GG::Clr interior_color/* = ClientUI::CtrlColor()*/, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) : 
    ListBox(x, y, w, h, border_color, interior_color, flags)
{
    RecreateScrolls();
    GG::Connect(SelChangedSignal,   &PlayListSelectSound,   -1);
    GG::Connect(DroppedSignal,      &PlayItemDropSound,     -1);
}

void CUIListBox::Render()
{
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

CUIDropDownList::CUIDropDownList(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Y drop_ht, GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/,
                                 GG::Clr interior/* = ClientUI::WndColor()*/, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) : 
    DropDownList(x, y, w, h, drop_ht, border_color),
    m_render_drop_arrow(true),
    m_mouse_here(false)
{
    SetInteriorColor(interior);
    SetMinSize(GG::Pt(MinSize().x, CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT));
}

void CUIDropDownList::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr lb_color = LB()->Color();
    GG::Clr lb_interior_color = LB()->InteriorColor();
    GG::Clr color_to_use = Disabled() ? DisabledColor(lb_color) : lb_color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    AngledCornerRectangle(ul, lr, int_color_to_use, GG::CLR_ZERO, CUIDROPDOWNLIST_ANGLE_OFFSET, 3, false);

    LB()->SetColor(GG::CLR_ZERO);
    LB()->SetInteriorColor(GG::CLR_ZERO);
    DropDownList::Render();
    LB()->SetInteriorColor(lb_interior_color);
    LB()->SetColor(lb_color);

    AngledCornerRectangle(ul, lr, GG::CLR_ZERO, color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);

    int margin = 3;
    int triangle_width = Value(lr.y - ul.y - 4 * margin);
    int outline_width = triangle_width + 3 * margin;

    if (m_render_drop_arrow) {
        GG::Clr triangle_color_to_use = ClientUI::DropDownListArrowColor();
        if (m_mouse_here && !Disabled())
            AdjustBrightness(triangle_color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
        IsoscelesTriangle(GG::Pt(lr.x - triangle_width - margin * 5 / 2, ul.y + 2 * margin),
                          GG::Pt(lr.x - margin * 5 / 2, lr.y - 2 * margin), 
                          SHAPE_DOWN, triangle_color_to_use);
        AngledCornerRectangle(GG::Pt(lr.x - outline_width - margin, ul.y + margin),
                              GG::Pt(lr.x - margin, lr.y - margin),
                              GG::CLR_ZERO, color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);
    }
}

void CUIDropDownList::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled())
        PlayDropDownListOpenSound();
    DropDownList::LClick(pt, mod_keys);
}

void CUIDropDownList::MouseEnter(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
    m_mouse_here = true;
}

void CUIDropDownList::MouseLeave()
{
    m_mouse_here = false;
}

void CUIDropDownList::DisableDropArrow()
{
    m_render_drop_arrow = false;
}

void CUIDropDownList::EnableDropArrow()
{
    m_render_drop_arrow = true;
}

///////////////////////////////////////
// class CUIEdit
///////////////////////////////////////
CUIEdit::CUIEdit(GG::X x, GG::Y y, GG::X w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                 GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/, 
                 GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Clr interior/* = ClientUI::CtrlColor()*/, 
                 GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) : 
    Edit(x, y, w, str, FontOrDefaultFont(font), border_color, text_color, interior, flags)
{
    GG::Connect(EditedSignal, &PlayTextTypingSound, -1);
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIEdit::Render()
{
    GG::Clr color = Color();
    GG::Clr border_color = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul, lr, int_color_to_use, border_color, 1);

    SetColor(GG::CLR_ZERO);
    Edit::Render();
    SetColor(color);
}

///////////////////////////////////////
// class CUIMultiEdit
///////////////////////////////////////
CUIMultiEdit::CUIMultiEdit(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::MultiEditStyle> style/* = MULTI_LINEWRAP*/,
                           const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                           GG::Clr border_color/* = ClientUI::CtrlBorderColor()*/, GG::Clr text_color/* = ClientUI::TextColor()*/,
                           GG::Clr interior/* = ClientUI::CtrlColor()*/, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) :
    MultiEdit(x, y, w, h, str, FontOrDefaultFont(font), border_color, style, text_color, interior, flags)
{
    RecreateScrolls();
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIMultiEdit::Render()
{
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
CUILinkTextMultiEdit::CUILinkTextMultiEdit(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str, GG::Flags<GG::MultiEditStyle> style,
                                           const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr border_color, GG::Clr text_color, 
                                           GG::Clr interior, GG::Flags<GG::WndFlag> flags) :
    CUIMultiEdit(x, y, w, h, str, style, font, border_color, text_color, interior, flags),
    TextLinker(),
    m_already_setting_text_so_dont_link(false),
    m_raw_text(str)
{
    FindLinks();
    MarkLinks();
}

const std::vector<GG::Font::LineData>& CUILinkTextMultiEdit::GetLineData() const
{
    return CUIMultiEdit::GetLineData();
}

const boost::shared_ptr<GG::Font>& CUILinkTextMultiEdit::GetFont() const
{
    return CUIMultiEdit::GetFont();
}

GG::Pt CUILinkTextMultiEdit::TextUpperLeft() const
{
    return CUIMultiEdit::TextUpperLeft() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5));
}

GG::Pt CUILinkTextMultiEdit::TextLowerRight() const
{
    return CUIMultiEdit::TextLowerRight() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5));
}

const std::string& CUILinkTextMultiEdit::RawText() const
{
    return m_raw_text;
}

void CUILinkTextMultiEdit::Render()
{
    CUIMultiEdit::Render();
    TextLinker::Render_();
}

void CUILinkTextMultiEdit::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::LClick(pt, mod_keys);
    TextLinker::LClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::LDoubleClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::LDoubleClick(pt, mod_keys);
    TextLinker::LDoubleClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::RClick(pt, mod_keys);
    TextLinker::RClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::MouseHere(pt, mod_keys);
    TextLinker::MouseHere_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseLeave()
{
    CUIMultiEdit::MouseLeave();
    TextLinker::MouseLeave_();
}

void CUILinkTextMultiEdit::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    GG::Pt lower_right = lr;
    if (Style() & GG::MULTI_INTEGRAL_HEIGHT)
        lower_right.y -= ((lr.y - ul.y) - (2 * PIXEL_MARGIN)) % GetFont()->Lineskip();
    bool resized = lower_right - ul != Size();
    GG::Edit::SizeMove(ul, lower_right);
    if (resized)
        SetText(RawText());
}

void CUILinkTextMultiEdit::SetText(const std::string& str)
{
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
        return;
    } else {
        CUIMultiEdit::SetText(str);
    }
}

void CUILinkTextMultiEdit::SetLinkedText(const std::string& str)
{
    MultiEdit::PreserveTextPositionOnNextSetText();
    CUIMultiEdit::SetText(str);
}

///////////////////////////////////////
// class CUISlider
///////////////////////////////////////
CUISlider::CUISlider(GG::X x, GG::Y y, GG::X w, GG::Y h, int min, int max, GG::Orientation orientation, GG::Flags<GG::WndFlag> flags/* = INTERACTIVE*/) :
    Slider(x, y, w, h, min, max, orientation, GG::FLAT, ClientUI::CtrlColor(), orientation == GG::VERTICAL ? Value(w) : Value(h), 5, flags)
{}

void CUISlider::Render()
{
    const GG::Pt UL = UpperLeft();
    const GG::Pt LR = LowerRight();
    GG::Clr border_color_to_use = Disabled() ? GG::DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
    int tab_width = GetOrientation() == GG::VERTICAL ? Value(Tab()->Height()) : Value(Tab()->Width());
    GG::Pt ul, lr;
    if (GetOrientation() == GG::VERTICAL) {
        ul.x = ((LR.x + UL.x) - static_cast<int>(LineWidth())) / 2;
        lr.x   = ul.x + static_cast<int>(LineWidth());
        ul.y = UL.y + tab_width / 2;
        lr.y   = LR.y - tab_width / 2;
    } else {
        ul.x = UL.x + tab_width / 2;
        lr.x   = LR.x - tab_width / 2;
        ul.y = ((LR.y + UL.y) - static_cast<int>(LineWidth())) / 2;
        lr.y   = ul.y + static_cast<int>(LineWidth());
    }
    GG::FlatRectangle(ul, lr, GG::CLR_ZERO, border_color_to_use, 1);
}

void CUISlider::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (GetOrientation() == GG::VERTICAL) {
        Tab()->Resize(GG::Pt(GG::X(TabWidth()), GG::Y(TabWidth())));
        Tab()->MoveTo(GG::Pt((Width() - Tab()->Width()) / 2, Tab()->RelativeUpperLeft().y));
        Tab()->SetMinSize(GG::Pt(Tab()->MinSize().x, GG::Y(10)));
    } else {
        Tab()->SizeMove(GG::Pt(GG::X(2), GG::Y0), GG::Pt(GG::X(TabWidth()), GG::Y(TabWidth())));
        Tab()->MoveTo(GG::Pt(Tab()->RelativeUpperLeft().x, (Height() - Tab()->Height()) / 2));
        Tab()->SetMinSize(GG::Pt(GG::X(10), Tab()->MinSize().y));
    }
    MoveTabToPosn();
}


///////////////////////////////////////
// class CUISimpleDropDownListRow
///////////////////////////////////////
// static(s)
const GG::Y CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT(22);

CUISimpleDropDownListRow::CUISimpleDropDownListRow(const std::string& row_text, GG::Y row_height/* = DEFAULT_ROW_HEIGHT*/) :
    GG::ListBox::Row(GG::X1, row_height, "")
{
    push_back(new GG::TextControl(GG::X0, GG::Y0, row_text, ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT));
}


///////////////////////////////////////
// class StatisticIcon
///////////////////////////////////////
namespace {
    const int STAT_ICON_PAD = 2;    // horizontal or vertical space between icon and label
}

StatisticIcon::StatisticIcon(GG::X x, GG::Y y, GG::X w, GG::Y h, const boost::shared_ptr<GG::Texture> texture,
                             double value, int digits, bool showsign,
                             GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    GG::Control(x, y, w, h, flags),
    m_num_values(1),
    m_values(std::vector<double>(1, value)), m_digits(std::vector<int>(1, digits)),
    m_show_signs(std::vector<bool>(1, showsign)),
    m_icon(0), m_text(0)
{
    int font_space = ClientUI::Pts()*3/2;
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    if (w >= Value(h)) {
        m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(Value(h)), h, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(GG::X(Value(h) + STAT_ICON_PAD), GG::Y0, w - Value(h) - STAT_ICON_PAD, GG::Y(std::max(font_space, Value(h))), "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    } else {
        // need vertical space for text, but don't want icon to be larger than available horizontal space
        int icon_height = std::min(Value(w), std::max(Value(h - font_space - STAT_ICON_PAD), 1));
        int icon_left = Value(w - icon_height)/2;
        m_icon = new GG::StaticGraphic(GG::X(icon_left), GG::Y0, GG::X(icon_height), GG::Y(icon_height), texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(GG::X0, GG::Y(icon_height + STAT_ICON_PAD), w, GG::Y(font_space), "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_TOP);
    }

    AttachChild(m_icon);
    AttachChild(m_text);
    Refresh();
}

StatisticIcon::StatisticIcon(GG::X x, GG::Y y, GG::X w, GG::Y h, const boost::shared_ptr<GG::Texture> texture,
                             double value0, double value1, int digits0, int digits1,
                             bool showsign0, bool showsign1,
                             GG::Flags<GG::WndFlag> flags/* = GG::INTERACTIVE*/) :
    GG::Control(x, y, w, h, flags),
    m_num_values(2),
    m_values(std::vector<double>(2, 0.0)), m_digits(std::vector<int>(2, 2)),
    m_show_signs(std::vector<bool>(2, false)),
    m_icon(0), m_text(0)
{
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    if (w >= Value(h)) {
        m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(Value(h)), h, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(GG::X(Value(h) + STAT_ICON_PAD), GG::Y0, w - Value(h) - STAT_ICON_PAD, std::max(GG::Y(ClientUI::Pts()*3/2), h), "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    } else {
        m_icon = new GG::StaticGraphic(GG::X0, GG::Y0, w, GG::Y(Value(w)), texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(GG::X0, GG::Y(Value(w) + STAT_ICON_PAD), w, GG::Y(ClientUI::Pts()*3/2), "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_BOTTOM);
    }

    m_values[0] = value0;
    m_values[1] = value1;
    m_digits[0] = digits0;
    m_digits[1] = digits1;
    m_show_signs[0] = showsign0;
    m_show_signs[1] = showsign1;
    AttachChild(m_icon);
    AttachChild(m_text);
    Refresh();
}

void StatisticIcon::SetValue(double value, int index)
{
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

void StatisticIcon::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::RButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::MouseWheel(const GG::Pt& pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::Refresh()
{
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

GG::Clr StatisticIcon::ValueColor(int index) const
{
    int effectiveSign = EffectiveSign(m_values.at(index));

    if (index == 0) return ClientUI::TextColor();

    if (effectiveSign == -1) return ClientUI::StatDecrColor();
    if (effectiveSign == 1) return ClientUI::StatIncrColor();

    return ClientUI::TextColor();
}

///////////////////////////////////////
// class CUIToolBar
///////////////////////////////////////
CUIToolBar::CUIToolBar(GG::X x, GG::Y y, GG::X w, GG::Y h) :
    GG::Control(x, y, w, h, GG::ONTOP | GG::INTERACTIVE | GG::DRAGABLE)
{}

bool CUIToolBar::InWindow(const GG::Pt& pt) const
{
    const std::list<GG::Wnd*>& children = Children();
    for (std::list<GG::Wnd*>::const_iterator it = children.begin(); it != children.end(); ++it)
        if ((*it)->InWindow(pt))
            return true;
    return GG::Wnd::InWindow(pt);
}

void CUIToolBar::LDrag(const GG::Pt& pt, const GG::Pt& move, GG::Flags<GG::ModKey> mod_keys)
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt final_move(std::max(-ul.x, std::min(move.x, GG::GUI::GetGUI()->AppWidth() - 1 - lr.x)),
                      std::max(-ul.y, std::min(move.y, GG::GUI::GetGUI()->AppHeight() - 1 - lr.y)));
    GG::Wnd::LDrag(pt + final_move - move, final_move, mod_keys);
}

void CUIToolBar::Render()
{
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
            GG::ListBox::Row(w, h, "SpeciesRow", GG::ALIGN_VCENTER, 0)
        {
            if (!species)
                return;
            GG::Wnd::SetName(species->Name());
            push_back(new GG::StaticGraphic(GG::X0, GG::Y0, GG::X(Value(h) - 6), h - 6, ClientUI::SpeciesIcon(species->Name()), GG::GRAPHIC_FITGRAPHIC));
            push_back(new GG::TextControl(GG::X0, GG::Y0, Width() - GG::X(Value(h)), h, UserString(species->Name()),
                                          ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER));
            GG::X first_col_width(Value(h) - 10);
            SetColWidth(0, first_col_width);
            SetColWidth(1, w - first_col_width);
        }
    };
}

SpeciesSelector::SpeciesSelector(GG::X w, GG::Y h) :
    CUIDropDownList(GG::X0, GG::Y0, w, h - 8, 6 * h)
{
    const SpeciesManager& sm = GetSpeciesManager();
    for (SpeciesManager::playable_iterator it = sm.playable_begin(); it != sm.playable_end(); ++it)
        Insert(new SpeciesRow(it->second, w, h - 4));
    if (!this->Empty())
        Select(this->begin());
    GG::Connect(SelChangedSignal, &SpeciesSelector::SelectionChanged, this);
}

SpeciesSelector::SpeciesSelector(GG::X w, GG::Y h, const std::vector<std::string>& species_names) :
    CUIDropDownList(GG::X0, GG::Y0, w, h - 8, 6 * h)
{
    SetSpecies(species_names);
    GG::Connect(SelChangedSignal, &SpeciesSelector::SelectionChanged, this);
}

const std::string& SpeciesSelector::CurrentSpeciesName() const
{
    CUIDropDownList::iterator row_it = this->CurrentItem();
    if (row_it == this->end())
        return EMPTY_STRING;
    const CUIDropDownList::Row* row = *row_it;
    if (!row) {
        Logger().errorStream() << "SpeciesSelector::CurrentSpeciesName couldn't get current item due to invalid Row pointer";
        return EMPTY_STRING;
    }
    return row->Name();
}

void SpeciesSelector::SelectSpecies(const std::string& species_name)
{
    for (CUIDropDownList::iterator row_it = this->begin(); row_it != this->end(); ++row_it) {
        CUIDropDownList::Row* row = *row_it;
        if (const SpeciesRow* species_row = dynamic_cast<const SpeciesRow*>(row)) {
            if (species_row->Name() == species_name) {
                Select(row_it);
                return;
            }
        }
    }
    Logger().errorStream() << "SpeciesSelector::SelectSpecies was unable to find a species in the list with name " << species_name;
}

void SpeciesSelector::SetSpecies(const std::vector<std::string>& species_names)
{
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
            Logger().errorStream() << "SpeciesSelector::SpeciesSelector couldn't find species with name: " << species_name;
        }
    }
    if (selection_changed)
        SelectionChanged(this->CurrentItem());
}

void SpeciesSelector::SelectionChanged(GG::DropDownList::iterator it)
{
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
                GG::Control(GG::X0, GG::Y0, COLOR_SELECTOR_WIDTH - 40, h, GG::Flags<GG::WndFlag>())
            {
                SetColor(color);
            }
            virtual void Render() {
                GG::FlatRectangle(UpperLeft(), LowerRight(), Color(), GG::CLR_ZERO, 0);
            }
        };
        ColorRow(const GG::Clr& color, GG::Y h) :
            GG::ListBox::Row(GG::X(Value(h)), h, "ColorRow")
        {
            push_back(new ColorSquare(color, h));
        }
    };
}
EmpireColorSelector::EmpireColorSelector(GG::Y h) : 
    CUIDropDownList(GG::X0, GG::Y0, COLOR_SELECTOR_WIDTH, h - 8, 12 * h)
{
    const std::vector<GG::Clr>& colors = EmpireColors();
    for (unsigned int i = 0; i < colors.size(); ++i) {
        Insert(new ColorRow(colors[i], h - 4));
    }
    GG::Connect(SelChangedSignal, &EmpireColorSelector::SelectionChanged, this);
}

GG::Clr EmpireColorSelector::CurrentColor() const
{
    return (**CurrentItem())[0]->Color();
}

void EmpireColorSelector::SelectColor(const GG::Clr& clr)
{
    for (iterator list_it = begin(); list_it != end(); ++list_it) {
        const GG::ListBox::Row* row = *list_it;
        if (row && !row->empty() && (*row)[0]->Color() == clr) {
            Select(list_it);
            return;
        }
    }
    Logger().errorStream() << "EmpireColorSelector::SelectColor was unable to find a requested color!";
}

void EmpireColorSelector::SelectionChanged(GG::DropDownList::iterator it)
{
    const GG::ListBox::Row* row = *it;
    if (row && !row->empty())
        ColorChangedSignal((*row)[0]->Color());
    else
        Logger().errorStream() << "EmpireColorSelector::SelectionChanged had trouble getting colour from row!";
}

///////////////////////////////////////
// class ColorSelector
///////////////////////////////////////
ColorSelector::ColorSelector(GG::X x, GG::Y y, GG::X w, GG::Y h, GG::Clr color, GG::Clr default_color) :
    Control(x, y, w, h),
    m_default_color(default_color)
{
    SetColor(color);
}

void ColorSelector::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::FlatRectangle(ul, lr, Color(), GG::CLR_WHITE, 1);
}

void ColorSelector::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    GG::X x = std::min(pt.x, GG::GUI::GetGUI()->AppWidth() - 315);    // 315 is width of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::Y y = std::min(pt.y, GG::GUI::GetGUI()->AppHeight() - 300);   // 300 is height of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::ColorDlg dlg(x, y, Color(), ClientUI::GetFont(), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::TextColor());
    dlg.SetNewString(UserString("COLOR_DLG_NEW"));
    dlg.SetOldString(UserString("COLOR_DLG_OLD"));
    dlg.SetRedString(UserString("COLOR_DLG_RED"));
    dlg.SetGreenString(UserString("COLOR_DLG_GREEN"));
    dlg.SetBlueString(UserString("COLOR_DLG_BLUE"));
    dlg.SetHueString(UserString("COLOR_DLG_HUE"));
    dlg.SetSaturationString(UserString("COLOR_DLG_SATURATION"));
    dlg.SetValueString(UserString("COLOR_DLG_VALUE"));
    dlg.SetAlphaString(UserString("COLOR_DLG_ALPHA"));
    dlg.SetOkString(UserString("OK"));
    dlg.SetCancelString(UserString("CANCEL"));
    dlg.Run();
    if (dlg.ColorWasSelected()) {
        GG::Clr clr = dlg.Result();
        SetColor(clr);
        ColorChangedSignal(clr);
    }
}

void ColorSelector::RClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    GG::MenuItem menu_contents;
    menu_contents.next_level.push_back(GG::MenuItem(UserString("RESET"), 1, false, false));

    GG::PopupMenu popup(pt.x, pt.y, ClientUI::GetFont(), menu_contents,
                        ClientUI::TextColor(), ClientUI::WndOuterBorderColor(), m_default_color);

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

    SetFilesString(UserString("FILE_DLG_FILES"));
    SetFileTypesString(UserString("FILE_DLG_FILE_TYPES"));
    SetSaveString(UserString("SAVE"));
    SetOpenString(UserString("OPEN"));
    SetCancelString(UserString("CANCEL"));
    SetMalformedFilenameString(UserString("FILE_DLG_MALFORMED_FILENAME"));
    SetOverwritePromptString(UserString("FILE_DLG_OVERWRITE_PROMPT"));
    SetInvalidFilenameString(UserString("FILE_DLG_INVALID_FILENAME"));
    SetFilenameIsADirectoryString(UserString("FILE_DLG_FILENAME_IS_A_DIRECTORY"));
    SetFileDoesNotExistString(UserString("FILE_DLG_FILE_DOES_NOT_EXIST"));
    SetDeviceIsNotReadyString(UserString("FILE_DLG_DEVICE_IS_NOT_READY"));
    SetThreeButtonDlgOKString(UserString("OK"));
    SetThreeButtonDlgCancelString(UserString("CANCEL"));
}

//////////////////////////////////////////////////
// ProductionInfoPanel
//////////////////////////////////////////////////
// static(s)
const int ProductionInfoPanel::CORNER_RADIUS = 9;
const GG::Y ProductionInfoPanel::VERTICAL_SECTION_GAP(4);

ProductionInfoPanel::ProductionInfoPanel(GG::X w, GG::Y h, const std::string& title, const std::string& points_str,
                                         float border_thickness, const GG::Clr& color, const GG::Clr& text_and_border_color) :
    GG::Wnd(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
    m_border_thickness(border_thickness),
    m_color(color),
    m_text_and_border_color(text_and_border_color)
{
    const int RESEARCH_TITLE_PTS = ClientUI::Pts() + 10;
    const int STAT_TEXT_PTS = ClientUI::Pts();
    const int CENTERLINE_GAP = 4;
    const GG::X LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 2 / 3;
    const GG::X VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const GG::X LEFT_TEXT_X(0);
    const GG::X RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const GG::X P_LABEL_X = RIGHT_TEXT_X + 40;
    const GG::X P_LABEL_WIDTH = Width() - 2 - 5 - P_LABEL_X;
    const GG::Clr TEXT_COLOR = ClientUI::KnownTechTextAndBorderColor();
    m_center_gap = std::make_pair(Value(LABEL_TEXT_WIDTH + 2), Value(LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP));

    m_title = new GG::TextControl(GG::X(2), GG::Y(4), Width() - 4, GG::Y(RESEARCH_TITLE_PTS + 4), title, ClientUI::GetFont(RESEARCH_TITLE_PTS), TEXT_COLOR);
    m_total_points_label = new GG::TextControl(LEFT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), UserString("PRODUCTION_INFO_TOTAL_PS_LABEL"), ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_total_points = new GG::TextControl(RIGHT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), "", ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_total_points_P_label = new GG::TextControl(P_LABEL_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, P_LABEL_WIDTH, GG::Y(STAT_TEXT_PTS + 4), points_str, ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_wasted_points_label = new GG::TextControl(LEFT_TEXT_X, m_total_points_label->LowerRight().y, LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), UserString("PRODUCTION_INFO_WASTED_PS_LABEL"), ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_wasted_points = new GG::TextControl(RIGHT_TEXT_X, m_total_points_label->LowerRight().y, VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), "", ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_wasted_points_P_label = new GG::TextControl(P_LABEL_X, m_total_points_label->LowerRight().y, P_LABEL_WIDTH, GG::Y(STAT_TEXT_PTS + 4), points_str, ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_projects_in_progress_label = new GG::TextControl(LEFT_TEXT_X, m_wasted_points_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), UserString("PRODUCTION_INFO_PROJECTS_IN_PROGRESS_LABEL"), ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_projects_in_progress = new GG::TextControl(RIGHT_TEXT_X, m_wasted_points_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), "", ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_points_to_underfunded_projects_label = new GG::TextControl(LEFT_TEXT_X, m_projects_in_progress_label->LowerRight().y, LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), UserString("PRODUCTION_INFO_PS_TO_UNDERFUNDED_PROJECTS_LABEL"), ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_points_to_underfunded_projects = new GG::TextControl(RIGHT_TEXT_X, m_projects_in_progress_label->LowerRight().y, VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), "", ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_points_to_underfunded_projects_P_label = new GG::TextControl(P_LABEL_X, m_projects_in_progress_label->LowerRight().y, P_LABEL_WIDTH, GG::Y(STAT_TEXT_PTS + 4), points_str, ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_projects_in_queue_label = new GG::TextControl(LEFT_TEXT_X, m_points_to_underfunded_projects_label->LowerRight().y, LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), UserString("PRODUCTION_INFO_PROJECTS_IN_QUEUE_LABEL"), ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_projects_in_queue = new GG::TextControl(RIGHT_TEXT_X, m_points_to_underfunded_projects_label->LowerRight().y, VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4), "", ClientUI::GetFont(STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);

    Resize(GG::Pt(Width(), m_projects_in_queue_label->LowerRight().y + 5));

    AttachChild(m_title);
    AttachChild(m_total_points_label);
    AttachChild(m_total_points);
    AttachChild(m_total_points_P_label);
    AttachChild(m_wasted_points_label);
    AttachChild(m_wasted_points);
    AttachChild(m_wasted_points_P_label);
    AttachChild(m_projects_in_progress_label);
    AttachChild(m_projects_in_progress);
    AttachChild(m_points_to_underfunded_projects_label);
    AttachChild(m_points_to_underfunded_projects);
    AttachChild(m_points_to_underfunded_projects_P_label);
    AttachChild(m_projects_in_queue_label);
    AttachChild(m_projects_in_queue);
}

void ProductionInfoPanel::Render()
{
    glDisable(GL_TEXTURE_2D);
    Draw(ClientUI::KnownTechFillColor(), true);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(m_border_thickness);
    Draw(GG::Clr(ClientUI::KnownTechTextAndBorderColor().r, ClientUI::KnownTechTextAndBorderColor().g, ClientUI::KnownTechTextAndBorderColor().b, 127), false);
    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    Draw(GG::Clr(ClientUI::KnownTechTextAndBorderColor().r, ClientUI::KnownTechTextAndBorderColor().g, ClientUI::KnownTechTextAndBorderColor().b, 255), false);
    glEnable(GL_TEXTURE_2D);
}

void ProductionInfoPanel::Reset(double total_points, double total_queue_cost, int projects_in_progress, double points_to_underfunded_projects, int queue_size)
{
    double wasted_points = total_queue_cost < total_points ? total_points - total_queue_cost : 0.0;
    *m_total_points << static_cast<int>(total_points);
    *m_wasted_points << static_cast<int>(wasted_points);
    *m_projects_in_progress << projects_in_progress;
    *m_points_to_underfunded_projects << static_cast<int>(points_to_underfunded_projects);
    *m_projects_in_queue << queue_size;
}

void ProductionInfoPanel::Draw(GG::Clr clr, bool fill)
{
    GG::Pt square_3(GG::X(3), GG::Y(3));
    GG::Pt ul = UpperLeft() + square_3, lr = LowerRight() - square_3;
    glColor(clr);
    PartlyRoundedRect(ul, GG::Pt(lr.x, m_title->LowerRight().y + 2),
                      CORNER_RADIUS, true, true, false, false, fill);
    std::pair<GG::X, GG::X> gap_to_use(m_center_gap.first + ul.x, m_center_gap.second + ul.x);
    PartlyRoundedRect(GG::Pt(ul.x, m_total_points_label->UpperLeft().y - 2), GG::Pt(gap_to_use.first, m_wasted_points_label->LowerRight().y + 2),
                      CORNER_RADIUS, false, false, false, false, fill);
    PartlyRoundedRect(GG::Pt(gap_to_use.second, m_total_points_label->UpperLeft().y - 2), GG::Pt(lr.x, m_wasted_points_label->LowerRight().y + 2),
                      CORNER_RADIUS, false, false, false, false, fill);
    PartlyRoundedRect(GG::Pt(ul.x, m_projects_in_progress_label->UpperLeft().y - 2), GG::Pt(gap_to_use.first, m_projects_in_queue_label->LowerRight().y + 2),
                      CORNER_RADIUS, false, false, true, false, fill);
    PartlyRoundedRect(GG::Pt(gap_to_use.second, m_projects_in_progress_label->UpperLeft().y - 2), GG::Pt(lr.x, m_projects_in_queue_label->LowerRight().y + 2),
                      CORNER_RADIUS, false, false, false, true, fill);
}

//////////////////////////////////////////////////
// MultiTurnProgressBar
//////////////////////////////////////////////////
MultiTurnProgressBar::MultiTurnProgressBar(GG::X w, GG::Y h, int total_turns, int turns_completed, double partially_complete_turn,
                                           const GG::Clr& bar_color, const GG::Clr& background, const GG::Clr& outline_color) :
    Control(GG::X0, GG::Y0, w, h, GG::Flags<GG::WndFlag>()),
    m_total_turns(total_turns),
    m_turns_completed(turns_completed),
    m_partially_complete_turn(partially_complete_turn),
    m_bar_color(bar_color),
    m_background(background),
    m_outline_color(outline_color)
{}

void MultiTurnProgressBar::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Y h = Height();
    const GG::X_d TURN_SEGMENT_WIDTH = Width() / static_cast<double>(m_total_turns);
    glDisable(GL_TEXTURE_2D);
    glColor(m_background);
    if (m_partially_complete_turn && m_turns_completed == m_total_turns - 1) {
        GG::BeginScissorClipping(GG::Pt(lr.x - TURN_SEGMENT_WIDTH, ul.y),
                                 GG::Pt(lr.x, lr.y - m_partially_complete_turn * h));
        glBegin(GL_POLYGON);
        RightEndVertices(lr.x - TURN_SEGMENT_WIDTH, ul.y, lr.x, lr.y);
        glEnd();
        GG::EndScissorClipping();
        GG::BeginScissorClipping(GG::Pt(lr.x - TURN_SEGMENT_WIDTH,
                                        lr.y - m_partially_complete_turn * h),
                                 lr);
        glColor(m_bar_color);
        glBegin(GL_POLYGON);
        RightEndVertices(lr.x - TURN_SEGMENT_WIDTH, ul.y, lr.x, lr.y);
        glEnd();
        GG::EndScissorClipping();
        glColor(m_background);
    } else {
        glBegin(GL_POLYGON);
        RightEndVertices(lr.x - TURN_SEGMENT_WIDTH, ul.y, lr.x, lr.y);
        glEnd();
    }
    glBegin(GL_QUADS);
    if (m_turns_completed != m_total_turns - 1) {
        glVertex(lr.x - TURN_SEGMENT_WIDTH, ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y);
        glVertex(lr.x - TURN_SEGMENT_WIDTH, lr.y);
    }
    if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y - h * m_partially_complete_turn);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y - h * m_partially_complete_turn);
    }
    glColor(m_bar_color);
    if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y - h * m_partially_complete_turn);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y - h * m_partially_complete_turn);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y);
    }
    if (m_turns_completed) {
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH, ul.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glVertex(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y);
    }
    glEnd();
    if (m_partially_complete_turn && !m_turns_completed) {
        GG::BeginScissorClipping(GG::Pt(ul.x, lr.y - m_partially_complete_turn * h),
                                 GG::Pt(ul.x + TURN_SEGMENT_WIDTH, lr.y));
        glBegin(GL_POLYGON);
        LeftEndVertices(ul.x, ul.y, ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glEnd();
        GG::EndScissorClipping();
        GG::BeginScissorClipping(ul,
                                 GG::Pt(ul.x + TURN_SEGMENT_WIDTH,
                                        lr.y - m_partially_complete_turn * h));
        glColor(m_background);
        glBegin(GL_POLYGON);
        LeftEndVertices(ul.x, ul.y, ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glEnd();
        GG::EndScissorClipping();
    } else {
        if (!m_turns_completed)
            glColor(m_background);
        glBegin(GL_POLYGON);
        LeftEndVertices(ul.x, ul.y, ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glEnd();
    }
    glColor(m_outline_color);
    glBegin(GL_LINES);
    for (GG::X_d x = ul.x + TURN_SEGMENT_WIDTH; x < lr.x - 1.0e-5; x += TURN_SEGMENT_WIDTH) {
        glVertex(x, ul.y);
        glVertex(x, lr.y);
    }
    glEnd();
    glEnable(GL_LINE_SMOOTH);
    glBegin(GL_LINE_LOOP);
    LeftEndVertices(ul.x, ul.y, ul.x + TURN_SEGMENT_WIDTH, lr.y);
    RightEndVertices(lr.x - TURN_SEGMENT_WIDTH, ul.y, lr.x, lr.y);
    glEnd();
    glDisable(GL_LINE_SMOOTH);

    glEnable(GL_TEXTURE_2D);
}

void MultiTurnProgressBar::LeftEndVertices(GG::X x1, GG::Y y1, GG::X_d x2, GG::Y y2)
{
    glVertex(x2, y1);
    glVertex(x1 + 5, y1);
    glVertex(x1, y1 + 4);
    glVertex(x1, y2 - 4);
    glVertex(x1 + 5, y2);
    glVertex(x2, y2);
}

void MultiTurnProgressBar::RightEndVertices(GG::X_d x1, GG::Y y1, GG::X x2, GG::Y y2)
{
    glVertex(x1, y2);
    glVertex(x2 - 5, y2);
    glVertex(x2, y2 - 4);
    glVertex(x2, y1 + 4);
    glVertex(x2 - 5, y1);
    glVertex(x1, y1);
}

//////////////////////////////////////////////////
// FPSIndicator
//////////////////////////////////////////////////
FPSIndicator::FPSIndicator(GG::X x, GG::Y y) :
    GG::TextControl(x, y, "", ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_NONE, GG::ONTOP)
{
    GG::Connect(GetOptionsDB().OptionChangedSignal("show-fps"), &FPSIndicator::UpdateEnabled, this);
    UpdateEnabled();
}

void FPSIndicator::Render()
{
    if (m_enabled) {
        SetText(boost::io::str(FlexibleFormat(UserString("MAP_INDICATOR_FPS")) % static_cast<int>(GG::GUI::GetGUI()->FPS())));
        TextControl::Render();
    }
}

void FPSIndicator::UpdateEnabled()
{
    m_enabled = GetOptionsDB().Get<bool>("show-fps");
}

//////////////////////////////////////////////////
// ShadowedTextControl
//////////////////////////////////////////////////
ShadowedTextControl::ShadowedTextControl(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::string& str,
                                         const boost::shared_ptr<GG::Font>& font,
                                         GG::Clr color, GG::Flags<GG::TextFormat> format,
                                         GG::Flags<GG::WndFlag> flags) :
    GG::TextControl(x, y, w, h, str, font, color, format, flags)
{}

ShadowedTextControl::ShadowedTextControl(GG::X x, GG::Y y, const std::string& str,
                                         const boost::shared_ptr<GG::Font>& font,
                                         GG::Clr color, GG::Flags<GG::TextFormat> format,
                                         GG::Flags<GG::WndFlag> flags) :
    GG::TextControl(x, y, str, font, color, format, flags)
{}

void ShadowedTextControl::Render()
{
    GG::Clr text_colour = TextColor();          // save original colour

    SetTextColor(GG::CLR_BLACK);                // render shadows in opaque black

    OffsetMove(GG::Pt(-GG::X1, GG::Y(0)));      // shadow to left
    TextControl::Render();

    OffsetMove(GG::Pt(GG::X1, GG::Y1));         // up
    TextControl::Render();

    OffsetMove(GG::Pt(GG::X1, -GG::Y1));        // right
    TextControl::Render();

    OffsetMove(GG::Pt(-GG::X1, -GG::Y1));       // down
    TextControl::Render();

    SetTextColor(text_colour);                  // restore original colour

    OffsetMove(GG::Pt(GG::X(0), GG::Y1));       // render main coloured text
    TextControl::Render();
}


//////////////////////////////////////////////////
// MultiTextureStaticGraphic
//////////////////////////////////////////////////

/** creates a MultiTextureStaticGraphic from multiple pre-existing Textures which are rendered back-to-front in the
      * order they are specified in \a textures with GraphicStyles specified in the same-indexed value of \a styles.
      * if \a styles is not specified or contains fewer entres than \a textures, entries in \a textures without 
      * associated styles use the style GRAPHIC_NONE. */
MultiTextureStaticGraphic::MultiTextureStaticGraphic(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::vector<boost::shared_ptr<GG::Texture> >& textures,
                                                     const std::vector<GG::Flags<GG::GraphicStyle> >& styles, GG::Flags<GG::WndFlag> flags) :
    GG::Control(x, y, w, h, flags),
    m_graphics(),
    m_styles(styles)
{
    for (std::vector<boost::shared_ptr<GG::Texture> >::const_iterator it = textures.begin(); it != textures.end(); ++it)
        m_graphics.push_back(GG::SubTexture(*it, GG::X0, GG::Y0, (*it)->DefaultWidth(), (*it)->DefaultHeight()));
    Init();
}

MultiTextureStaticGraphic::MultiTextureStaticGraphic(GG::X x, GG::Y y, GG::X w, GG::Y h, const std::vector<GG::SubTexture>& subtextures,
                                                     const std::vector<GG::Flags<GG::GraphicStyle> >& styles, GG::Flags<GG::WndFlag> flags) :
    GG::Control(x, y, w, h, flags),
    m_graphics(subtextures),
    m_styles(styles)
{
    Init();
}

MultiTextureStaticGraphic::MultiTextureStaticGraphic() :
    m_graphics(),
    m_styles()
{}

GG::Rect MultiTextureStaticGraphic::RenderedArea(const GG::SubTexture& subtexture, GG::Flags<GG::GraphicStyle> style) const
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

void MultiTextureStaticGraphic::Render()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);
    for (std::vector<GG::SubTexture>::size_type i = 0; i < m_graphics.size(); ++i) {
        GG::Rect rendered_area = RenderedArea(m_graphics[i], m_styles[i]);
        m_graphics[i].OrthoBlit(rendered_area.ul, rendered_area.lr);
    }
}

void MultiTextureStaticGraphic::Init()
{
    ValidateStyles();
    SetColor(GG::CLR_WHITE);
}

void MultiTextureStaticGraphic::ValidateStyles()
{
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
