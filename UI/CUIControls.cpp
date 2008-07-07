//CUIControls.cpp

#include "CUIControls.h"

#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "CUISpin.h"
#include "Sound.h"
#include "../client/human/HumanClientApp.h"
#include "../util/MultiplayerCommon.h"
#include "../util/OptionsDB.h"

#include <GG/GUI.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>
#include <GG/dialogs/ColorDlg.h>

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include <limits>

namespace {
    void PlayButtonClickSound()
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-click"), true); }

    void PlayTurnButtonClickSound()
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.turn-button-click"), true); }

    struct PlayButtonCheckSound
    {
        PlayButtonCheckSound(bool play_only_when_checked) : m_play_only_when_checked(play_only_when_checked) {}
        void operator()(bool checked) const
        {
            if (!m_play_only_when_checked || checked)
                Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-click"), true);
        }
        const bool m_play_only_when_checked;
    };

    void PlayListSelectSound(const GG::ListBox::SelectionSet&)
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.list-select"), true); }

    void PlayDropDownListOpenSound()
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.list-pulldown"), true); }

    void PlayItemDropSound(GG::ListBox::iterator)
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.item-drop"), true); }

    void PlayTextTypingSound(const std::string&)
    { Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.text-typing"), true); }

    boost::shared_ptr<GG::Font> FontOrDefaultFont(const boost::shared_ptr<GG::Font>& font)
    {
        return font ? font : GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts());
    }

    const double ARROW_BRIGHTENING_SCALE_FACTOR = 1.5;
    const double STATE_BUTTON_BRIGHTENING_SCALE_FACTOR = 1.25;
    const double TAB_BRIGHTENING_SCALE_FACTOR = 1.25;
}

#if 0
// TODO: finish implementing these sounds, as recommended by the sound team (the ones commented out are done):
//1) Button Click - includes clicks on regular buttons, check boxes, radio buttons
//2) Button Rollover - includes moving the mouse over regular buttons only
//3) Item Select - selecting/deselecting an item in a drop-down list or listbox
//4) Item Drop - dropping a drag-and-drop item into a listbox
//5) Alert - when a warning or error message box pops up
//6) Minimize window
//7) Maximize window
//8) Close window
//9) Turn Button Click (specialization of Button Click)
//9) Fleet Rollover (specialization of Button Rollover)
//10) Fleet Click (specialization of Button Click)
//11) Planet Click (specialization of Button Click)
//12) Text Typing
13) Planet Background Music (music, instead of sound effect, for the planet detail view)
14) Fleet-Type Sound (plays sound specific to the type of fleet in the fleet detail view)
15) Turn Progressing
#endif

///////////////////////////////////////
// class CUIButton
///////////////////////////////////////
namespace {
    const int CUIBUTTON_ANGLE_OFFSET = 5;
    const int COLOR_SELECTOR_WIDTH = 75;
    const int COLOR_SQUARE_HEIGHT = 10;

    // row type used in the EmpireColorSelector
    struct ColorRow : public GG::ListBox::Row
    {
        struct ColorSquare : GG::Control
        {
            ColorSquare(const GG::Clr& color, int h) : 
                GG::Control(0, 0, COLOR_SELECTOR_WIDTH - 40, h, GG::Flags<GG::WndFlag>())
            {
                SetColor(color);
            }

            virtual void Render()
            {
                GG::Pt ul = UpperLeft(), lr = LowerRight();
                GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, Color(), GG::CLR_ZERO, 0);
            }
        };

        ColorRow(const GG::Clr& color, int h = COLOR_SQUARE_HEIGHT)
        {
            push_back(new ColorSquare(color, h));
        }
    };
}

CUIButton::CUIButton(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                     GG::Clr color/* = ClientUI::ButtonColor()*/,
                     GG::Clr border/* = ClientUI::CtrlBorderColor()*/, int thick/* = 2*/, 
                     GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) :
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
    return InAngledCornerRect(pt, ul.x, ul.y, lr.x, lr.y, CUIBUTTON_ANGLE_OFFSET);
}

void CUIButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
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
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, m_border_color, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    OffsetMove(GG::Pt(1,1));
    TextControl::Render();
    OffsetMove(GG::Pt(-1,-1));
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
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}

void CUIButton::RenderUnpressed()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, m_border_thick);
    TextControl::Render();
}

void CUIButton::MarkNotSelected()
{
    SetColor(ClientUI::ButtonColor());
    SetBorderColor(ClientUI::CtrlBorderColor());
    SetBorderThick(1);
}

void CUIButton::MarkSelectedGray()
{
    GG::Clr colour = ClientUI::CtrlBorderColor();
    AdjustBrightness(colour, 120);
    colour = ClientUI::CtrlColor();
    AdjustBrightness(colour, 50);
    SetColor(colour);
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
// class CUITurnButton
///////////////////////////////////////
CUITurnButton::CUITurnButton(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                             GG::Clr color/* = ClientUI::ButtonColor()*/, 
                             GG::Clr border/* = ClientUI::CtrlBorderColor()*/, int thick/* = 2*/, 
                             GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) : 
    CUIButton(x, y, w, str, FontOrDefaultFont(font), color, border, thick, text_color, flags)
{
    GG::Connect(ClickedSignal, &PlayTurnButtonClickSound, -1);
}


///////////////////////////////////////
// class CUIArrowButton
///////////////////////////////////////
CUIArrowButton::CUIArrowButton(int x, int y, int w, int h, ShapeOrientation orientation, GG::Clr color, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) :
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
        GG::Pt ul = UpperLeft() + GG::Pt(3, 1), lr = LowerRight() - GG::Pt(2, 1);
        return InIsoscelesTriangle(pt, ul.x, ul.y, lr.x, lr.y, m_orientation);
    }
}

bool CUIArrowButton::FillBackgroundWithWndColor() const
{ return m_fill_background_with_wnd_color; }

void CUIArrowButton::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    if (!Disabled()) {
        if (State() != BN_ROLLOVER)
            Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
        SetState(BN_ROLLOVER);
    }
}

void CUIArrowButton::FillBackgroundWithWndColor(bool fill)
{ m_fill_background_with_wnd_color = fill; }

void CUIArrowButton::RenderPressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    OffsetMove(GG::Pt(1, 1));
    RenderUnpressed();
    OffsetMove(GG::Pt(-1, -1));
}

void CUIArrowButton::RenderRollover()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    if (!Disabled())
        AdjustBrightness(color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
    GG::Pt tri_ul = ul + GG::Pt(3, 1), tri_lr = lr - GG::Pt(2, 1);
    IsoscelesTriangle(tri_ul.x, tri_ul.y, tri_lr.x, tri_lr.y, m_orientation, color_to_use);
}

void CUIArrowButton::RenderUnpressed()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    if (m_fill_background_with_wnd_color)
        FlatRectangle(ul.x, ul.y, lr.x, lr.y, ClientUI::WndColor(), GG::CLR_ZERO, 0);
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Pt tri_ul = ul + GG::Pt(3, 1), tri_lr = lr - GG::Pt(2, 1);
    IsoscelesTriangle(tri_ul.x, tri_ul.y, tri_lr.x, tri_lr.y, m_orientation, color_to_use);
}


///////////////////////////////////////
// class CUIStateButton
///////////////////////////////////////
CUIStateButton::CUIStateButton(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::TextFormat> format, GG::StateButtonStyle style/* = GG::SBSTYLE_3D_CHECKBOX*/,
                               GG::Clr color/* = ClientUI::StateButtonColor()*/, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                               GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Clr interior/* = GG::CLR_ZERO*/,
                               GG::Clr border/* = ClientUI::CtrlBorderColor()*/, GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) :
    StateButton(x, y, w, h, str, FontOrDefaultFont(font), format, color, text_color, interior, style, flags),
    m_border_color(border),
    m_mouse_here(false)
{
    if (style == GG::SBSTYLE_3D_TOP_DETACHED_TAB || style == GG::SBSTYLE_3D_TOP_ATTACHED_TAB)
        SetColor(ClientUI::WndColor());
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
            FlatRectangle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                bn_ul += GG::Pt(MARGIN, MARGIN);
                bn_lr -= GG::Pt(MARGIN, MARGIN);
                const int OFFSET = (bn_lr.y - bn_ul.y) / 2;
                glDisable(GL_TEXTURE_2D);
                glColor(inside_color);
                glBegin(GL_QUADS);
                glVertex2i(bn_lr.x, bn_ul.y);
                glVertex2i(bn_ul.x + OFFSET, bn_ul.y);
                glVertex2i(bn_ul.x, bn_ul.y + OFFSET);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_lr.x - OFFSET, bn_lr.y);
                glVertex2i(bn_lr.x, bn_lr.y - OFFSET);
                glVertex2i(bn_lr.x, bn_ul.y);
                glEnd();
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glColor(outside_color);
                glBegin(GL_POLYGON);
                glVertex2i(bn_lr.x, bn_ul.y);
                glVertex2i(bn_ul.x + OFFSET, bn_ul.y);
                glVertex2i(bn_ul.x, bn_ul.y + OFFSET);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_ul.x, bn_lr.y);
                glVertex2i(bn_lr.x - OFFSET, bn_lr.y);
                glVertex2i(bn_lr.x, bn_lr.y - OFFSET);
                glVertex2i(bn_lr.x, bn_ul.y);
                glEnd();
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                glEnable(GL_TEXTURE_2D);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
                AngledCornerRectangle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, 
                                      inside_color, outside_color, (bn_lr.y - bn_ul.y - 2 * MARGIN) / 2, 1);
                glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
                glScaled(-1.0, 1.0, 1.0);
                glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
            }
        } else if (static_cast<int>(Style()) == GG::SBSTYLE_3D_RADIO) {
            const int MARGIN = 2;
            FlatCircle(bn_ul.x, bn_ul.y, bn_lr.x, bn_lr.y, int_color_to_use, border_color_to_use, 1);
            if (Checked()) {
                GG::Clr inside_color = color_to_use;
                GG::Clr outside_color = color_to_use;
                AdjustBrightness(outside_color, 50);
                FlatCircle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, GG::CLR_ZERO, 
                           outside_color, 1);
                FlatCircle(bn_ul.x + MARGIN + 1, bn_ul.y + MARGIN + 1, bn_lr.x - MARGIN - 1, bn_lr.y - MARGIN - 1, 
                           inside_color, outside_color, 1);
            } else {
                GG::Clr inside_color = border_color_to_use;
                AdjustBrightness(inside_color, -75);
                GG::Clr outside_color = inside_color;
                AdjustBrightness(outside_color, 40);
                FlatCircle(bn_ul.x + MARGIN, bn_ul.y + MARGIN, bn_lr.x - MARGIN, bn_lr.y - MARGIN, inside_color, 
                           outside_color, 1);
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
            additional_text_offset.y = UNCHECKED_OFFSET / 2;
        }
        AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, CUIBUTTON_ANGLE_OFFSET, 1, true, false, !Checked());
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
// class CUIScroll
///////////////////////////////////////
namespace {
    const int CUISCROLL_ANGLE_OFFSET = 3;
}

///////////////////////////////////////
// class CUIScroll::ScrollTab
CUIScroll::ScrollTab::ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color, 
                                GG::Clr border_color) : 
    Button(orientation == GG::VERTICAL ? 0 : 2,
           orientation == GG::VERTICAL ? 2 : 0,
           scroll_width, scroll_width, "", boost::shared_ptr<GG::Font>(), color),
    m_border_color(border_color),
    m_orientation(orientation),
    m_mouse_here(false),
    m_being_dragged(false)
{
    SetMinSize(GG::Pt(m_orientation == GG::VERTICAL ? MinSize().x : 10,
                      m_orientation == GG::VERTICAL ? 10 : MinSize().y));
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
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, GG::CLR_ZERO, CUISCROLL_ANGLE_OFFSET, 0);
    // upper left diagonal stripe
    GG::Clr light_color = Color();
    AdjustBrightness(light_color, 35);
    if (!Disabled() && m_mouse_here)
        AdjustBrightness(light_color, TAB_BRIGHTENING_SCALE_FACTOR);
    glColor(light_color);
    glDisable(GL_TEXTURE_2D);
    glBegin(GL_POLYGON);
    if (m_orientation == GG::VERTICAL) {
        glVertex2i(lr.x, ul.y);
        glVertex2i(ul.x + CUISCROLL_ANGLE_OFFSET, ul.y);
        glVertex2i(ul.x, ul.y + CUISCROLL_ANGLE_OFFSET);
        glVertex2i(ul.x, ul.y + std::min(lr.x - ul.x, lr.y - ul.y));
    } else {
        glVertex2i(ul.x + std::min(lr.x - ul.x, lr.y - ul.y), ul.y);
        glVertex2i(ul.x + CUISCROLL_ANGLE_OFFSET, ul.y);
        glVertex2i(ul.x, lr.y - CUISCROLL_ANGLE_OFFSET);
        glVertex2i(ul.x, lr.y);
    }
    glEnd();
    // lower right diagonal stripe
    glBegin(GL_POLYGON);
    if (m_orientation == GG::VERTICAL) {
        glVertex2i(lr.x, lr.y - std::min(lr.x - ul.x, lr.y - ul.y));
        glVertex2i(ul.x, lr.y);
        glVertex2i(lr.x - CUISCROLL_ANGLE_OFFSET, lr.y);
        glVertex2i(lr.x, lr.y - CUISCROLL_ANGLE_OFFSET);
    } else {
        glVertex2i(lr.x, ul.y);
        glVertex2i(lr.x - std::min(lr.x - ul.x, lr.y - ul.y), lr.y);
        glVertex2i(lr.x - CUISCROLL_ANGLE_OFFSET, lr.y);
        glVertex2i(lr.x, lr.y - CUISCROLL_ANGLE_OFFSET);
    }
    glEnd();
    glEnable(GL_TEXTURE_2D);
    // border
    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, border_color_to_use, CUISCROLL_ANGLE_OFFSET, 1);
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
        Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
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
CUIScroll::CUIScroll(int x, int y, int w, int h, GG::Orientation orientation, GG::Clr color/* = GG::CLR_ZERO*/, 
                     GG::Clr border/* = ClientUI::CtrlBorderColor()*/, GG::Clr interior/* = GG::CLR_ZERO*/, 
                     GG::Flags<GG::WndFlag> flags/* = CLICKABLE | REPEAT_BUTTON_DOWN*/) :
    Scroll(x, y, w, h, orientation, color, interior, flags),
    m_border_color(border)
{}

void CUIScroll::Render()
{
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color_to_use = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, color_to_use, border_color_to_use, 1);
}

void CUIScroll::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    int bn_width = (ScrollOrientation() == GG::VERTICAL) ? Size().x : Size().y;
    TabButton()->SizeMove(TabButton()->RelativeUpperLeft(), 
                          (ScrollOrientation() == GG::VERTICAL) ? GG::Pt(bn_width, TabButton()->RelativeLowerRight().y) :
                          GG::Pt(TabButton()->RelativeLowerRight().x, bn_width));
    SizeScroll(ScrollRange().first, ScrollRange().second, LineSize(), PageSize()); // update tab size and position
}


///////////////////////////////////////
// class CUIListBox
///////////////////////////////////////
CUIListBox::CUIListBox(int x, int y, int w, int h, GG::Clr color/* = ClientUI::CtrlBorderColor()*/, 
                       GG::Clr interior/* = GG::CLR_ZERO*/, GG::Flags<GG::WndFlag> flags/* = CLICKABLE*/) : 
    ListBox(x, y, w, h, color, interior, flags)
{
    RecreateScrolls();
    GG::Connect(SelChangedSignal, &PlayListSelectSound, -1);
    GG::Connect(DroppedSignal, &PlayItemDropSound, -1);
}

void CUIListBox::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr color = Color(); // save color
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    FlatRectangle(ul.x, ul.y, lr.x, lr.y, InteriorColor(), color_to_use, 1);
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

CUIDropDownList::CUIDropDownList(int x, int y, int w, int h, int drop_ht, GG::Clr color/* = ClientUI::CtrlBorderColor()*/,
                                 GG::Clr interior/* = ClientUI::DropDownListIntColor()*/, GG::Flags<GG::WndFlag> flags/* = CLICKABLE*/) : 
    DropDownList(x, y, w, h, drop_ht, color),
    m_render_drop_arrow(true),
    m_mouse_here(false)
{
    SetInteriorColor(interior);
}

void CUIDropDownList::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr lb_color = LB()->Color();
    GG::Clr lb_interior_color = LB()->InteriorColor();
    GG::Clr color_to_use = Disabled() ? DisabledColor(lb_color) : lb_color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, GG::CLR_ZERO, CUIDROPDOWNLIST_ANGLE_OFFSET, 3, false);

    LB()->SetColor(GG::CLR_ZERO);
    LB()->SetInteriorColor(GG::CLR_ZERO);
    DropDownList::Render();
    LB()->SetInteriorColor(lb_interior_color);
    LB()->SetColor(lb_color);

    AngledCornerRectangle(ul.x, ul.y, lr.x, lr.y, GG::CLR_ZERO, color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);

    int margin = 3;
    int triangle_width = lr.y - ul.y - 4 * margin;
    int outline_width = triangle_width + 3 * margin;

    if (m_render_drop_arrow) {
        GG::Clr triangle_color_to_use = ClientUI::DropDownListArrowColor();
        if (m_mouse_here && !Disabled())
            AdjustBrightness(triangle_color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
        IsoscelesTriangle(lr.x - triangle_width - margin * 5 / 2, ul.y + 2 * margin, lr.x - margin * 5 / 2, lr.y - 2 * margin, 
                          SHAPE_DOWN, triangle_color_to_use);
        AngledCornerRectangle(lr.x - outline_width - margin, ul.y + margin, lr.x - margin, lr.y - margin, GG::CLR_ZERO, 
                              color_to_use, CUIDROPDOWNLIST_ANGLE_OFFSET, 1, false);
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
    Sound::GetSound().PlaySound(ClientUI::SoundDir() / GetOptionsDB().Get<std::string>("UI.sound.button-rollover"), true);
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
CUIEdit::CUIEdit(int x, int y, int w, const std::string& str, const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                 GG::Clr color/* = ClientUI::CtrlBorderColor()*/, 
                 GG::Clr text_color/* = ClientUI::TextColor()*/, GG::Clr interior/* = ClientUI::EditIntColor()*/, 
                 GG::Flags<GG::WndFlag> flags/* = CLICKABLE*/) : 
    Edit(x, y, w, str, FontOrDefaultFont(font), color, text_color, interior, flags)
{
    GG::Connect(EditedSignal, &PlayTextTypingSound, -1);
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIEdit::Render()
{
    GG::Clr color = Color();
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);

    SetColor(GG::CLR_ZERO);
    Edit::Render();
    SetColor(color);
}

///////////////////////////////////////
// class CUIMultiEdit
///////////////////////////////////////
CUIMultiEdit::CUIMultiEdit(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::MultiEditStyle> style/* = MULTI_LINEWRAP*/, 
                           const boost::shared_ptr<GG::Font>& font/* = boost::shared_ptr<GG::Font>()*/,
                           GG::Clr color/* = ClientUI::CtrlBorderColor()*/, GG::Clr text_color/* = ClientUI::TextColor()*/, 
                           GG::Clr interior/* = ClientUI::MultieditIntColor()*/, GG::Flags<GG::WndFlag> flags/* = CLICKABLE*/) : 
    MultiEdit(x, y, w, h, str, FontOrDefaultFont(font), color, style, text_color, interior, flags)
{
    RecreateScrolls();
    SetHiliteColor(ClientUI::EditHiliteColor());
}

void CUIMultiEdit::Render()
{
    GG::Clr color = Color();
    GG::Clr color_to_use = Disabled() ? DisabledColor(color) : color;
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();

    FlatRectangle(ul.x, ul.y, lr.x, lr.y, int_color_to_use, color_to_use, 1);

    SetColor(GG::CLR_ZERO);
    MultiEdit::Render();
    SetColor(color);
}

///////////////////////////////////////
// class CUILinkTextMultiEdit
///////////////////////////////////////
CUILinkTextMultiEdit::CUILinkTextMultiEdit(int x, int y, int w, int h, const std::string& str, GG::Flags<GG::MultiEditStyle> style,
                                           const boost::shared_ptr<GG::Font>& font,
                                           GG::Clr color, GG::Clr text_color, 
                                           GG::Clr interior, GG::Flags<GG::WndFlag> flags) :
    CUIMultiEdit(x, y, w, h, str, style, font, color, text_color, interior, flags),
    TextLinker(),
    m_already_setting_text_so_dont_link(false)
{}

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
    return CUIMultiEdit::TextUpperLeft();
}

GG::Pt CUILinkTextMultiEdit::TextLowerRight() const
{
    return CUIMultiEdit::TextLowerRight();
}

const std::string& CUILinkTextMultiEdit::WindowText() const
{
    return CUIMultiEdit::WindowText();
}

void CUILinkTextMultiEdit::Render()
{
    CUIMultiEdit::Render();
    TextLinker::Render_();
}

void CUILinkTextMultiEdit::LButtonDown(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::LButtonDown(pt, mod_keys);
    TextLinker::LButtonDown_(pt, mod_keys);
}

void CUILinkTextMultiEdit::LButtonUp(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::LButtonUp(pt, mod_keys);
    TextLinker::LButtonUp_(pt, mod_keys);
}

void CUILinkTextMultiEdit::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::LClick(pt, mod_keys);
    TextLinker::LClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseHere(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    CUIMultiEdit::MouseHere(pt, mod_keys);
    TextLinker::MouseHere_(pt + ScrollPosition(), mod_keys);
}

void CUILinkTextMultiEdit::MouseLeave()
{
    CUIMultiEdit::MouseLeave();
    TextLinker::MouseLeave_();
}

void CUILinkTextMultiEdit::SetText(const std::string& str)
{
    // MultiEdit have scrollbars that are adjusted every time the text is set.  Adjusting scrollbars also requires
    // setting text, because the space for the text is added or removed when scrollbars are shown or hidden.
    // Since highlighting links on rollover also involves setting text, there are a lot of potentially unnecessary
    // calls to SetText and FindLinks.  This check for whether text is already being set eliminates many of those
    // calls when they aren't necessary, since the results will be overridden later anyway by the outermost (or
    // lowest on stack, or first ) call to SetText
    if (!m_already_setting_text_so_dont_link) {
        m_already_setting_text_so_dont_link = true;
        CUIMultiEdit::SetText(str);
        FindLinks();
        m_already_setting_text_so_dont_link = false;
        return;
    } else {
        CUIMultiEdit::SetText(str);
    }
    //CUIMultiEdit::SetText(str);
    //FindLinks();
}

void CUILinkTextMultiEdit::SetLinkedText(const std::string& str)
{
    MultiEdit::PreserveTextPositionOnNextSetText();
    CUIMultiEdit::SetText(str);
}

///////////////////////////////////////
// class CUISlider
///////////////////////////////////////
CUISlider::CUISlider(int x, int y, int w, int h, int min, int max, GG::Orientation orientation, GG::Flags<GG::WndFlag> flags/* = CLICKABLE*/) :
    Slider(x, y, w, h, min, max, orientation, GG::FLAT, ClientUI::CtrlColor(), orientation == GG::VERTICAL ? w : h, 5, flags)
{}

void CUISlider::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::Clr border_color_to_use = Disabled() ? GG::DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
    int tab_width = GetOrientation() == GG::VERTICAL ? Tab()->Height() : Tab()->Width();
    int x_start, x_end, y_start, y_end;
    if (GetOrientation() == GG::VERTICAL) {
        x_start = ((lr.x + ul.x) - LineWidth()) / 2;
        x_end   = x_start + LineWidth();
        y_start = ul.y + tab_width / 2;
        y_end   = lr.y - tab_width / 2;
    } else {
        x_start = ul.x + tab_width / 2;
        x_end   = lr.x - tab_width / 2;
        y_start = ((lr.y + ul.y) - LineWidth()) / 2;
        y_end   = y_start + LineWidth();
    }
    GG::FlatRectangle(x_start, y_start, x_end, y_end, GG::CLR_ZERO, border_color_to_use, 1);
    Tab()->OffsetMove(UpperLeft());
    Tab()->Render();
    Tab()->OffsetMove(-UpperLeft());
}

void CUISlider::SizeMove(const GG::Pt& ul, const GG::Pt& lr)
{
    Wnd::SizeMove(ul, lr);
    if (GetOrientation() == GG::VERTICAL) {
        Tab()->Resize(GG::Pt(TabWidth(), TabWidth()));
        Tab()->MoveTo(GG::Pt((Width() - Tab()->Width()) / 2, Tab()->RelativeUpperLeft().y));
        Tab()->SetMinSize(GG::Pt(Tab()->MinSize().x, 10));
    } else {
        Tab()->SizeMove(GG::Pt(2, 0), GG::Pt(TabWidth(), TabWidth()));
        Tab()->MoveTo(GG::Pt(Tab()->RelativeUpperLeft().x, (Height() - Tab()->Height()) / 2));
        Tab()->SetMinSize(GG::Pt(10, Tab()->MinSize().y));
    }
    MoveTabToPosn();
}


///////////////////////////////////////
// class CUISimpleDropDownListRow
///////////////////////////////////////
// static(s)
const int CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT = 22;

CUISimpleDropDownListRow::CUISimpleDropDownListRow(const std::string& row_text, int row_height/* = DEFAULT_ROW_HEIGHT*/) :
    GG::ListBox::Row(1, row_height, "")
{
    push_back(new GG::TextControl(0, 0, row_text, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT));
}


///////////////////////////////////////
// class StatisticIcon
///////////////////////////////////////
StatisticIcon::StatisticIcon(int x, int y, int w, int h, const boost::shared_ptr<GG::Texture> texture,
                             double value, int digits, bool integerize, bool showsign,
                             GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) :
    GG::Control(x, y, w, h, flags),
    m_num_values(1),
    m_values(std::vector<double>(1, value)), m_digits(std::vector<int>(1, digits)),
    m_integerize(std::vector<bool>(1, integerize)), m_show_signs(std::vector<bool>(1, showsign)),
    m_icon(0), m_text(0)
{
    int font_space = ClientUI::Pts()*3/2;
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    if (w >= h) {
        m_icon = new GG::StaticGraphic(0, 0, h, h, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(h, 0, w - h, std::max(font_space, h), "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    } else {
        // need vertical space for text, but don't want icon to be larger than available horizintal space
        int icon_height = std::min(w, std::max(h - font_space, 1));
        int icon_left = (w - icon_height)/2;
        m_icon = new GG::StaticGraphic(icon_left, 0, icon_height, icon_height, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(0, icon_height, w, font_space, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_TOP);
    }
    
    AttachChild(m_icon);
    AttachChild(m_text);
    Refresh();
}

StatisticIcon::StatisticIcon(int x, int y, int w, int h, const boost::shared_ptr<GG::Texture> texture,
                             double value0, double value1, int digits0, int digits1,
                             bool integerize0, bool integerize1, bool showsign0, bool showsign1,
                             GG::Flags<GG::WndFlag> flags/* = GG::CLICKABLE*/) :
    GG::Control(x, y, w, h, flags),
    m_num_values(2),
    m_values(std::vector<double>(2, 0.0)), m_digits(std::vector<int>(2, 2)),
    m_integerize(std::vector<bool>(2, false)), m_show_signs(std::vector<bool>(2, false)),
    m_icon(0), m_text(0)
{
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    if (w >= h) {
        m_icon = new GG::StaticGraphic(0, 0, h, h, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(h, 0, w - h, std::max(ClientUI::Pts()*3/2, h), "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
    } else {
        m_icon = new GG::StaticGraphic(0, 0, w, w, texture, GG::GRAPHIC_FITGRAPHIC);
        m_text = new GG::TextControl(0, w, w, ClientUI::Pts()*3/2, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_CENTER | GG::FORMAT_BOTTOM);
    }

    m_values[0] = value0;
    m_values[1] = value1;
    m_digits[0] = digits0;
    m_digits[1] = digits1;
    m_integerize[0] = integerize0;
    m_integerize[1] = integerize1;
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
        m_integerize.resize(m_num_values, m_integerize[0]);
        m_digits.resize(m_num_values, m_digits[0]);
    }
    m_values[index] = value;
    Refresh();
}

void StatisticIcon::Refresh()
{
    std::string text = "";
    
    // first value: always present
    std::string clr_tag = GG::RgbaTag(ValueColor(0));
    text += clr_tag + DoubleToString(m_values[0], m_digits[0], m_integerize[0], m_show_signs[0]) + "</rgba>";
    
    // second value: may or may not be present
    if (m_num_values > 1)
    {
        clr_tag = GG::RgbaTag(ValueColor(1));
        text += " (" + clr_tag + DoubleToString(m_values[1], m_digits[1], m_integerize[1], m_show_signs[1]) + "</rgba>)";
    }

    m_text->SetText(text);
}

GG::Clr StatisticIcon::ValueColor(int index) const
{
    int effectiveSign = EffectiveSign(m_values.at(index), m_integerize.at(index));
     
    if (index == 0) return ClientUI::TextColor();

    if (effectiveSign == -1) return ClientUI::StatDecrColor();
    if (effectiveSign == 1) return ClientUI::StatIncrColor();

    return ClientUI::TextColor();
}

///////////////////////////////////////
// class CUIToolBar
///////////////////////////////////////
CUIToolBar::CUIToolBar(int x, int y, int w, int h) :
    GG::Control(x, y, w, h, GG::ONTOP | GG::CLICKABLE)
{}

void CUIToolBar::Render()
{
    GG::Pt ul(UpperLeft()),lr(LowerRight());
    GG::FlatRectangle(ul.x,ul.y,lr.x,lr.y,GG::FloatClr(0.0f,0.0f,0.0f,0.8f),GG::CLR_ZERO,0);
}

///////////////////////////////////////
// class EmpireColorSelector
///////////////////////////////////////
EmpireColorSelector::EmpireColorSelector(int h) : 
    CUIDropDownList(0, 0, COLOR_SELECTOR_WIDTH, h, 5 * h)
{
    const std::vector<GG::Clr>& colors = EmpireColors();
    for (unsigned int i = 0; i < colors.size(); ++i) {
        Insert(new ColorRow(colors[i], h - 2));
    }
    GG::Connect(SelChangedSignal, &EmpireColorSelector::SelectionChanged, this);
}

GG::Clr EmpireColorSelector::CurrentColor() const
{
    return (**CurrentItem())[0]->Color();
}

void EmpireColorSelector::SelectColor(const GG::Clr& clr)
{
    Select(begin());
    const std::vector<GG::Clr>& colors = EmpireColors();
    iterator it = begin();
    for (unsigned int i = 0; i < colors.size(); ++i, ++it) {
        if (colors[i] == clr) {
            Select(it);
            break;
        } else if (i == colors.size() - 1) {
            assert(!"EmpireColorSelector::SelectColor() : No such color!");
        }
    }
}

void EmpireColorSelector::SelectionChanged(GG::DropDownList::iterator it)
{
    const std::vector<GG::Clr>& colors = EmpireColors();
    ColorChangedSignal(colors[std::distance(begin(), it)]);
}

///////////////////////////////////////
// class ColorSelector
///////////////////////////////////////
ColorSelector::ColorSelector(int x, int y, int w, int h, GG::Clr color) :
    Control(x, y, w, h)
{
    SetColor(color);
}

void ColorSelector::Render()
{
    GG::Pt ul = UpperLeft(), lr = LowerRight();
    GG::FlatRectangle(ul.x, ul.y, lr.x, lr.y, Color(), GG::CLR_WHITE, 1);
}

void ColorSelector::LClick(const GG::Pt& pt, GG::Flags<GG::ModKey> mod_keys)
{
    int x = std::min(pt.x, GG::GUI::GetGUI()->AppWidth() - 315);    // 315 is width of ColorDlg from GG::ColorDlg:::ColorDlg
    int y = std::min(pt.y, GG::GUI::GetGUI()->AppHeight() - 300);   // 300 is height of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::ColorDlg dlg(x, y, Color(), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::TextColor());
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

///////////////////////////////////////
// class FileDlg
///////////////////////////////////////
FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 const std::vector<std::pair<std::string, std::string> >& types) :
    GG::FileDlg(directory, filename, save, multi, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()),
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
const int ProductionInfoPanel::VERTICAL_SECTION_GAP = 4;

ProductionInfoPanel::ProductionInfoPanel(int w, int h, const std::string& title, const std::string& points_str,
                                         double border_thickness, const GG::Clr& color, const GG::Clr& text_and_border_color) :
    GG::Wnd(0, 0, w, h, GG::Flags<GG::WndFlag>()),
    m_border_thickness(border_thickness),
    m_color(color),
    m_text_and_border_color(text_and_border_color)
{
    const int RESEARCH_TITLE_PTS = ClientUI::Pts() + 10;
    const int STAT_TEXT_PTS = ClientUI::Pts();
    const int CENTERLINE_GAP = 4;
    const int LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 2 / 3;
    const int VALUE_TEXT_WIDTH = Width() - 4 - CENTERLINE_GAP - LABEL_TEXT_WIDTH;
    const int LEFT_TEXT_X = 0;
    const int RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const int P_LABEL_X = RIGHT_TEXT_X + 40;
    const int P_LABEL_WIDTH = Width() - 2 - 5 - P_LABEL_X;
    const GG::Clr TEXT_COLOR = ClientUI::KnownTechTextAndBorderColor();
    m_center_gap = std::make_pair(LABEL_TEXT_WIDTH + 2, LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP);

    m_title = new GG::TextControl(2, 4, Width() - 4, RESEARCH_TITLE_PTS + 4, title, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), RESEARCH_TITLE_PTS), TEXT_COLOR);
    m_total_points_label = new GG::TextControl(LEFT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("PRODUCTION_INFO_TOTAL_PS_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_total_points = new GG::TextControl(RIGHT_TEXT_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_total_points_P_label = new GG::TextControl(P_LABEL_X, m_title->LowerRight().y + VERTICAL_SECTION_GAP + 4, P_LABEL_WIDTH, STAT_TEXT_PTS + 4, points_str, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_wasted_points_label = new GG::TextControl(LEFT_TEXT_X, m_total_points_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("PRODUCTION_INFO_WASTED_PS_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_wasted_points = new GG::TextControl(RIGHT_TEXT_X, m_total_points_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_wasted_points_P_label = new GG::TextControl(P_LABEL_X, m_total_points_label->LowerRight().y, P_LABEL_WIDTH, STAT_TEXT_PTS + 4, points_str, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_projects_in_progress_label = new GG::TextControl(LEFT_TEXT_X, m_wasted_points_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("PRODUCTION_INFO_PROJECTS_IN_PROGRESS_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_projects_in_progress = new GG::TextControl(RIGHT_TEXT_X, m_wasted_points_label->LowerRight().y + VERTICAL_SECTION_GAP + 4, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_points_to_underfunded_projects_label = new GG::TextControl(LEFT_TEXT_X, m_projects_in_progress_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("PRODUCTION_INFO_PS_TO_UNDERFUNDED_PROJECTS_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_points_to_underfunded_projects = new GG::TextControl(RIGHT_TEXT_X, m_projects_in_progress_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_points_to_underfunded_projects_P_label = new GG::TextControl(P_LABEL_X, m_projects_in_progress_label->LowerRight().y, P_LABEL_WIDTH, STAT_TEXT_PTS + 4, points_str, GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);
    m_projects_in_queue_label = new GG::TextControl(LEFT_TEXT_X, m_points_to_underfunded_projects_label->LowerRight().y, LABEL_TEXT_WIDTH, STAT_TEXT_PTS + 4, UserString("PRODUCTION_INFO_PROJECTS_IN_QUEUE_LABEL"), GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_RIGHT);
    m_projects_in_queue = new GG::TextControl(RIGHT_TEXT_X, m_points_to_underfunded_projects_label->LowerRight().y, VALUE_TEXT_WIDTH, STAT_TEXT_PTS + 4, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), STAT_TEXT_PTS), TEXT_COLOR, GG::FORMAT_LEFT);

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
    GG::Pt ul = UpperLeft() + GG::Pt(3, 3), lr = LowerRight() - GG::Pt(3, 3);
    glColor(clr);
    PartlyRoundedRect(ul, GG::Pt(lr.x, m_title->LowerRight().y + 2),
                      CORNER_RADIUS, true, true, false, false, fill);
    std::pair<int, int> gap_to_use(m_center_gap.first + ul.x, m_center_gap.second + ul.x);
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
MultiTurnProgressBar::MultiTurnProgressBar(int w, int h, int total_turns, int turns_completed, double partially_complete_turn,
                                           const GG::Clr& bar_color, const GG::Clr& background, const GG::Clr& outline_color) :
    Control(0, 0, w, h, GG::Flags<GG::WndFlag>()),
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
    int h = Height();
    const double TURN_SEGMENT_WIDTH = Width() / static_cast<double>(m_total_turns);
    glDisable(GL_TEXTURE_2D);
    glColor(m_background);
    if (m_partially_complete_turn && m_turns_completed == m_total_turns - 1) {
        GG::BeginScissorClipping(static_cast<int>(lr.x - TURN_SEGMENT_WIDTH), ul.y,
                                 lr.x, static_cast<int>(lr.y - m_partially_complete_turn * h));
        glBegin(GL_POLYGON);
        RightEndVertices(lr.x - TURN_SEGMENT_WIDTH, ul.y, lr.x, lr.y);
        glEnd();
        GG::EndScissorClipping();
        GG::BeginScissorClipping(static_cast<int>(lr.x - TURN_SEGMENT_WIDTH),
                                 static_cast<int>(lr.y - m_partially_complete_turn * h),
                                 lr.x, lr.y);
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
        glVertex2d(lr.x - TURN_SEGMENT_WIDTH, ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y);
        glVertex2d(lr.x - TURN_SEGMENT_WIDTH, lr.y);
    }
    if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y - h * m_partially_complete_turn);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y - h * m_partially_complete_turn);
    }
    glColor(m_bar_color);
    if (0 < m_turns_completed && m_turns_completed < m_total_turns - 1) {
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y - h * m_partially_complete_turn);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y - h * m_partially_complete_turn);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * (m_turns_completed + 1), lr.y);
    }
    if (m_turns_completed) {
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH, ul.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glVertex2d(ul.x + TURN_SEGMENT_WIDTH * m_turns_completed, lr.y);
    }
    glEnd();
    if (m_partially_complete_turn && !m_turns_completed) {
        GG::BeginScissorClipping(ul.x, static_cast<int>(lr.y - m_partially_complete_turn * h),
                                 static_cast<int>(ul.x + TURN_SEGMENT_WIDTH), lr.y);
        glBegin(GL_POLYGON);
        LeftEndVertices(ul.x, ul.y, ul.x + TURN_SEGMENT_WIDTH, lr.y);
        glEnd();
        GG::EndScissorClipping();
        GG::BeginScissorClipping(ul.x, ul.y,
                                 static_cast<int>(ul.x + TURN_SEGMENT_WIDTH),
                                 static_cast<int>(lr.y - m_partially_complete_turn * h));
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
    for (double x = ul.x + TURN_SEGMENT_WIDTH; x < lr.x - 1.0e-5; x += TURN_SEGMENT_WIDTH) {
        glVertex2d(x, ul.y);
        glVertex2d(x, lr.y);
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

void MultiTurnProgressBar::LeftEndVertices(double x1, double y1, double x2, double y2)
{
    glVertex2d(x2, y1);
    glVertex2d(x1 + 5, y1);
    glVertex2d(x1, y1 + 4);
    glVertex2d(x1, y2 - 4);
    glVertex2d(x1 + 5, y2);
    glVertex2d(x2, y2);
}

void MultiTurnProgressBar::RightEndVertices(double x1, double y1, double x2, double y2)
{
    glVertex2d(x1, y2);
    glVertex2d(x2 - 5, y2);
    glVertex2d(x2, y2 - 4);
    glVertex2d(x2, y1 + 4);
    glVertex2d(x2 - 5, y1);
    glVertex2d(x1, y1);
}

//////////////////////////////////////////////////
// FPSIndicator
//////////////////////////////////////////////////
FPSIndicator::FPSIndicator(int x, int y) :
    GG::TextControl(x, y, "", GG::GUI::GetGUI()->GetFont(ClientUI::Font(), ClientUI::Pts()), ClientUI::TextColor(), GG::FORMAT_NONE, GG::ONTOP)
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
