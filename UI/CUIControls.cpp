#include "CUIControls.h"

#include "CUISpin.h"
#include "ClientUI.h"
#include "CUIDrawUtil.h"
#include "IconTextBrowseWnd.h"
#include "Sound.h"
#include "Hotkeys.h"
#include "../client/human/GGHumanClientApp.h"
#include "../util/i18n.h"
#include "../util/Logger.h"
#include "../util/OptionsDB.h"
#include "../universe/Species.h"
#include "../Empire/Empire.h"
#include "TextBrowseWnd.h"

#include <GG/utf8/checked.h>
#include <GG/dialogs/ColorDlg.h>
#include <GG/GUI.h>
#include <GG/Layout.h>

#include <boost/format.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/clamp.hpp>

#include <limits>
#include <iomanip>


namespace {
    void AddOptions(OptionsDB& db) {
        Hotkey::AddHotkey("ui.autocomplete",    UserStringNop("HOTKEY_AUTOCOMPLETE"),   GG::Key::GGK_TAB);
    }
    bool temp_bool = RegisterOptions(&AddOptions);

    void PlayButtonClickSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.press.sound.path"), true); }

    void PlayButtonRolloverSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.button.rollover.sound.path"), true); }

    void PlayListSelectSound(const GG::ListBox::SelectionSet&)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.listbox.select.sound.path"), true); }

    void PlayDropDownListOpenSound()
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.dropdownlist.select.sound.path"), true); }

    void PlayItemDropSound(GG::ListBox::iterator)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.listbox.drop.sound.path"), true); }

    void PlayTextTypingSound(const std::string&)
    { Sound::GetSound().PlaySound(GetOptionsDB().Get<std::string>("ui.input.keyboard.sound.path"), true); }

    constexpr double ARROW_BRIGHTENING_SCALE_FACTOR = 1.5;
    constexpr double STATE_BUTTON_BRIGHTENING_SCALE_FACTOR = 1.25;
}


///////////////////////////////////////
// class CUILabel
///////////////////////////////////////
CUILabel::CUILabel(std::string str,
                   GG::Flags<GG::TextFormat> format, GG::Flags<GG::WndFlag> flags,
                   std::shared_ptr<GG::Font> font,
                   GG::X x, GG::Y y, GG::X w, GG::Y h) :
    TextControl(x, y, w, h, std::move(str), std::move(font), ClientUI::TextColor(), format, flags)
{ SetName("CUILabel no elements"); }

CUILabel::CUILabel(std::string str,
                   GG::Flags<GG::TextFormat> format, GG::Flags<GG::WndFlag> flags,
                   GG::X x, GG::Y y, GG::X w, GG::Y h) :
    TextControl(x, y, w, h, std::move(str), ClientUI::GetFont(), ClientUI::TextColor(), format, flags)
{ SetName("CUILabel no elements"); }

CUILabel::CUILabel(std::string str, std::vector<GG::Font::TextElement> text_elements,
                   GG::Flags<GG::TextFormat> format, GG::Flags<GG::WndFlag> flags,
                   GG::X x, GG::Y y, GG::X w, GG::Y h) :
    TextControl(x, y, w, h, std::move(str), std::move(text_elements),
                ClientUI::GetFont(), ClientUI::TextColor(), format, flags)
{ SetName("CUILabel from elements"); }

void CUILabel::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto copy_wnd_action = [this]() { GG::GUI::GetGUI()->CopyWndText(this); };
    // create popup menu
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_COPY"), false, false, copy_wnd_action));
    popup->Run();
}


///////////////////////////////////////
// class CUIButton
///////////////////////////////////////
namespace {
    constexpr int CUIBUTTON_ANGLE_OFFSET = 5;
}

CUIButton::CUIButton(std::string str) :
    Button(std::move(str), ClientUI::GetFont(), ClientUI::CtrlColor(),
           ClientUI::TextColor(), GG::INTERACTIVE)
{
    SetName("CUIButton " + this->m_label->Text());
    LeftClickedSignal.connect(-1, &PlayButtonClickSound);
}

CUIButton::CUIButton(GG::SubTexture unpressed, GG::SubTexture pressed,
                     GG::SubTexture rollover) :
    Button("", ClientUI::GetFont(), GG::CLR_WHITE, GG::CLR_ZERO, GG::INTERACTIVE)
{
    SetName("CUIButton SubTextures");
    SetColor(GG::CLR_WHITE);
    SetUnpressedGraphic(std::move(unpressed));
    SetPressedGraphic  (std::move(pressed));
    SetRolloverGraphic (std::move(rollover));
    LeftClickedSignal.connect(-1, &PlayButtonClickSound);
}

bool CUIButton::InWindow(GG::Pt pt) const
{ return InAngledCornerRect(pt, UpperLeft(), LowerRight(), CUIBUTTON_ANGLE_OFFSET); }

void CUIButton::MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    Button::MouseEnter(pt, mod_keys);
    if (!Disabled())
         PlayButtonRolloverSound();
}

GG::Pt CUIButton::MinUsableSize() const {
    GG::Pt result = GG::Button::MinUsableSize();
    static constexpr int CUIBUTTON_HPADDING = 10;
    static constexpr int CUIBUTTON_VPADDING = 3;

    result.x += CUIBUTTON_HPADDING * 2;
    result.y += CUIBUTTON_VPADDING * 2;

    return result;
}

void CUIButton::RenderPressed() {
    if (PressedGraphic().Empty()) {
        GG::Clr background_clr = AdjustBrightness(Color(), 25);
        GG::Clr border_clr     = ClientUI::CtrlBorderColor();
        int     border_thick   = 1;

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
        GG::Clr border_clr     = ClientUI::CtrlBorderColor();
        int     border_thick   = 1;

        if (Disabled()) {
            background_clr = DisabledColor(background_clr);
            border_clr     = DisabledColor(border_clr);
        } else {
            border_clr = AdjustBrightness(border_clr, 100);
        }

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
        GG::Clr border_clr     = ClientUI::CtrlBorderColor();
        int     border_thick   = 1;

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
SettableInWindowCUIButton::SettableInWindowCUIButton(GG::SubTexture unpressed,
                                                     GG::SubTexture pressed,
                                                     GG::SubTexture rollover,
                                                     SettableInWindowCUIButton::TestFuncT in_window_function) :
    CUIButton(std::move(unpressed), std::move(pressed), std::move(rollover))
{
    SetName("SettableInWindowCUIButton SubTextures");
    m_in_window_func = std::move(in_window_function);
}

bool SettableInWindowCUIButton::InWindow(GG::Pt pt) const {
    if (m_in_window_func)
        return m_in_window_func(this, pt);
    else
        return CUIButton::InWindow(pt);
}


///////////////////////////////////////
// class CUIArrowButton
///////////////////////////////////////
CUIArrowButton::CUIArrowButton(ShapeOrientation orientation, bool fill_background,
                               GG::Flags<GG::WndFlag> flags) :
    Button("", nullptr, ClientUI::DropDownListArrowColor(), GG::CLR_ZERO, flags),
    m_orientation(orientation),
    m_fill_background_with_wnd_color(fill_background)
{
    SetName("CUIArrowButton");
    LeftClickedSignal.connect(-1, &PlayButtonClickSound);
}

bool CUIArrowButton::InWindow(GG::Pt pt) const noexcept {
    if (m_fill_background_with_wnd_color) {
        return Button::InWindow(pt);
    } else {
        const GG::Pt ul = UpperLeft() + GG::Pt(GG::X{3}, GG::Y1);
        const GG::Pt lr = LowerRight() - GG::Pt(GG::X{2}, GG::Y1);
        return InIsoscelesTriangle(pt, ul, lr, m_orientation);
    }
}

void CUIArrowButton::MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled()) {
        if (State() != ButtonState::BN_ROLLOVER)
            PlayButtonRolloverSound();
        SetState(ButtonState::BN_ROLLOVER);
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
        color_to_use = AdjustBrightness(color_to_use, ARROW_BRIGHTENING_SCALE_FACTOR);
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

void CUICheckBoxRepresenter::Render(const GG::StateButton& button) const {
    // draw button
    GG::Pt cl_ul = button.ClientUpperLeft();
    GG::Pt bn_ul, bn_lr, tx_ul;

    DoLayout(button, bn_ul, bn_lr, tx_ul);

    bn_ul += cl_ul;
    bn_lr += cl_ul;

    GG::Clr color_to_use = button.Disabled() ? DisabledColor(button.Color()) : button.Color();
    GG::Clr border_color_to_use = button.Disabled() ? DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
    if (!button.Disabled() && !button.Checked() &&
        GG::StateButton::ButtonState::BN_ROLLOVER == button.State())
    {
        color_to_use = AdjustBrightness(color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
        border_color_to_use = AdjustBrightness(border_color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
    }

    static constexpr int MARGIN = 3;
    FlatRectangle(bn_ul, bn_lr, GG::CLR_ZERO, border_color_to_use, 1);
    if (button.Checked()) {
        GG::Clr inside_color = color_to_use;
        GG::Clr outside_color = AdjustBrightness(color_to_use, 50);
        bn_ul += GG::Pt(GG::X(MARGIN), GG::Y(MARGIN));
        bn_lr -= GG::Pt(GG::X(MARGIN), GG::Y(MARGIN));
        const int OFFSET = Value(bn_lr.y - bn_ul.y) / 2;

        GG::GL2DVertexBuffer verts;
        verts.reserve(16);

        verts.store(bn_lr.x, bn_ul.y);
        verts.store(bn_ul.x + OFFSET, bn_ul.y);
        verts.store(bn_ul.x, bn_ul.y + OFFSET);
        verts.store(bn_ul.x, bn_lr.y);
        verts.store(bn_ul.x, bn_lr.y);
        verts.store(bn_lr.x - OFFSET, bn_lr.y);
        verts.store(bn_lr.x, bn_lr.y - OFFSET);

        verts.store(bn_lr.x, bn_ul.y);
        verts.store(bn_ul.x + OFFSET, bn_ul.y);
        verts.store(bn_ul.x, bn_ul.y + OFFSET);
        verts.store(bn_ul.x, bn_lr.y);
        verts.store(bn_ul.x, bn_lr.y);
        verts.store(bn_lr.x - OFFSET, bn_lr.y);
        verts.store(bn_lr.x, bn_lr.y - OFFSET);
        verts.store(bn_lr.x, bn_ul.y);
        verts.store(bn_lr.x, bn_ul.y);

        verts.activate();

        glDisable(GL_TEXTURE_2D);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glColor(inside_color);
        glDrawArrays(GL_QUADS, 0, 8);

        glColor(outside_color);
        glDrawArrays(GL_LINE_STRIP, 8, 8);

        glPopClientAttrib();
        glEnable(GL_TEXTURE_2D);
    } else {
        GG::Clr inside_color = AdjustBrightness(border_color_to_use, -75);
        GG::Clr outside_color = AdjustBrightness(inside_color, 40);
        glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
        glScaled(-1.0, 1.0, 1.0);
        glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
        AngledCornerRectangle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                              GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN),
                              inside_color, outside_color, Value(bn_lr.y - bn_ul.y - 2 * MARGIN) / 2, 1);
        glTranslated((bn_ul.x + bn_lr.x) / 2.0, -(bn_ul.y + bn_lr.y) / 2.0, 0.0);
        glScaled(-1.0, 1.0, 1.0);
        glTranslated(-(bn_ul.x + bn_lr.x) / 2.0, (bn_ul.y + bn_lr.y) / 2.0, 0.0);
    }

    // draw text
    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->TextControl::Render();
    button.GetLabel()->OffsetMove(-tx_ul);
}

void CUICheckBoxRepresenter::OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const
{
    if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State() &&
        GG::StateButton::ButtonState::BN_UNPRESSED == prev_state)
    { PlayButtonRolloverSound(); }
}

void CUICheckBoxRepresenter::OnChecked(bool checked) const {
    if (checked)
        PlayButtonClickSound();
}

void CUIRadioRepresenter::Render(const GG::StateButton& button) const {
    // draw button
    GG::Pt cl_ul = button.ClientUpperLeft();
    GG::Pt bn_ul, bn_lr, tx_ul;

    DoLayout(button, bn_ul, bn_lr, tx_ul);

    bn_ul += cl_ul;
    bn_lr += cl_ul;

    GG::Clr color_to_use = button.Disabled() ? DisabledColor(button.Color()) : button.Color();
    GG::Clr border_color_to_use = button.Disabled() ? DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();
    if (!button.Disabled() && !button.Checked() &&
        GG::StateButton::ButtonState::BN_ROLLOVER == button.State())
    {
        color_to_use = AdjustBrightness(color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
        border_color_to_use = AdjustBrightness(border_color_to_use, STATE_BUTTON_BRIGHTENING_SCALE_FACTOR);
    }

    static constexpr int MARGIN = 2;
    FlatCircle(bn_ul, bn_lr, GG::CLR_ZERO, border_color_to_use, 1);
    if (button.Checked()) {
        GG::Clr inside_color = color_to_use;
        GG::Clr outside_color = AdjustBrightness(color_to_use, 50);
        FlatCircle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                   GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN),
                   GG::CLR_ZERO, outside_color, 1);
        FlatCircle(GG::Pt(bn_ul.x + MARGIN + 1, bn_ul.y + MARGIN + 1),
                   GG::Pt(bn_lr.x - MARGIN - 1, bn_lr.y - MARGIN - 1), 
                   inside_color, outside_color, 1);
    } else {
        GG::Clr inside_color = AdjustBrightness(border_color_to_use, -75);
        GG::Clr outside_color = AdjustBrightness(inside_color, 40);
        FlatCircle(GG::Pt(bn_ul.x + MARGIN, bn_ul.y + MARGIN),
                   GG::Pt(bn_lr.x - MARGIN, bn_lr.y - MARGIN),
                   inside_color, outside_color, 1);
    }

    // draw text
    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->TextControl::Render();
    button.GetLabel()->OffsetMove(-tx_ul);
}

void CUIRadioRepresenter::OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const
{
    if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State() &&
        GG::StateButton::ButtonState::BN_UNPRESSED == prev_state)
    { PlayButtonRolloverSound(); }
}

void CUIRadioRepresenter::OnChecked(bool checked) const
{ PlayButtonClickSound(); }

void CUITabRepresenter::Render(const GG::StateButton& button) const {
    // draw button
    GG::Pt ul = button.UpperLeft();
    GG::Pt lr = button.LowerRight();
    GG::Pt tx_ul;

    GG::Clr color_to_use = button.Disabled() ? DisabledColor(button.Color()) : button.Color();
    GG::Clr border_color_to_use = button.Disabled() ? DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor();

    if (button.Checked() || (!button.Disabled() &&
                             GG::StateButton::ButtonState::BN_ROLLOVER == button.State()))
    { border_color_to_use = AdjustBrightness(border_color_to_use, 100); }

    static constexpr int UNCHECKED_OFFSET = 4;

    if (!button.Checked()) {
        ul.y += UNCHECKED_OFFSET;
        tx_ul.y = GG::Y(UNCHECKED_OFFSET / 2);
    }

    AngledCornerRectangle(ul, lr, color_to_use, border_color_to_use,
                          CUIBUTTON_ANGLE_OFFSET, 1, true, false, !button.Checked());

    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->Render();
    button.GetLabel()->OffsetMove(-(tx_ul));
}

void CUITabRepresenter::OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const
{
    if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State() &&
        GG::StateButton::ButtonState::BN_UNPRESSED == prev_state)
    { PlayButtonRolloverSound(); }
}

void CUITabRepresenter::OnChecked(bool checked) const
{ PlayButtonClickSound(); }

GG::Pt CUITabRepresenter::MinUsableSize(const GG::StateButton& button) const
{ return button.GetLabel()->MinUsableSize(); }

void CUILabelButtonRepresenter::OnChanged(const GG::StateButton& button, GG::StateButton::ButtonState prev_state) const
{
    if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State() &&
        GG::StateButton::ButtonState::BN_UNPRESSED == prev_state)
    { PlayButtonRolloverSound(); }
}

void CUILabelButtonRepresenter::OnChecked(bool checked) const
{ PlayButtonClickSound(); }

void CUILabelButtonRepresenter::Render(const GG::StateButton& button) const {
    GG::Pt ul = button.UpperLeft();
    GG::Pt lr = button.LowerRight();
    GG::Pt tx_ul;

    GG::Clr background_clr = ClientUI::CtrlColor();
    GG::Clr border_clr     = ClientUI::CtrlBorderColor();
    int     border_thick   = 1;

    if (button.Checked()) {
        background_clr = ClientUI::ButtonHiliteColor();
        border_clr     = ClientUI::ButtonHiliteBorderColor();
        border_thick   = 2;
    }

    if (button.Disabled()) {
        background_clr = DisabledColor(background_clr);
        border_clr     = DisabledColor(border_clr);
    } else {
        if (GG::StateButton::ButtonState::BN_PRESSED == button.State()) {
            background_clr = AdjustBrightness(background_clr, 25);
            tx_ul = GG::Pt(GG::X1, GG::Y1);
        }
        if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State())
            border_clr = AdjustBrightness(border_clr, 100);
    }

    AngledCornerRectangle(ul, lr, background_clr, border_clr, CUIBUTTON_ANGLE_OFFSET, border_thick);

    button.GetLabel()->OffsetMove(tx_ul);
    button.GetLabel()->Render();
    button.GetLabel()->OffsetMove(-(tx_ul));
}

CUIIconButtonRepresenter::CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> icon,
                                                   GG::Clr highlight_clr) :
    m_unchecked_icon(icon),
    m_checked_icon(std::move(icon)),
    m_unchecked_color(highlight_clr),
    m_checked_color(highlight_clr)
{}

CUIIconButtonRepresenter::CUIIconButtonRepresenter(std::shared_ptr<GG::SubTexture> unchecked_icon,
                                                   GG::Clr unchecked_clr,
                                                   std::shared_ptr<GG::SubTexture> checked_icon,
                                                   GG::Clr checked_clr) :
    m_unchecked_icon(std::move(unchecked_icon)),
    m_checked_icon(std::move(checked_icon)),
    m_unchecked_color(unchecked_clr),
    m_checked_color(checked_clr)
{}

void CUIIconButtonRepresenter::OnChecked(bool checked) const
{ PlayButtonClickSound(); }

void CUIIconButtonRepresenter::Render(const GG::StateButton& button) const {
    const bool checked = button.Checked();
    const bool rollover = GG::StateButton::ButtonState::BN_ROLLOVER == button.State();
    const bool render_checked = (checked ^ rollover); // opposite on mouseover

    GG::Clr icon_clr((render_checked && !button.Disabled()) ? m_checked_color : m_unchecked_color);
    GG::Clr bg_clr(ClientUI::CtrlColor());
    GG::Clr border_clr(button.Disabled() ? DisabledColor(ClientUI::CtrlBorderColor()) : ClientUI::CtrlBorderColor());

    static constexpr int BORDER_THICKNESS = 1;
    static constexpr int ICON_SIZE_REDUCE = BORDER_THICKNESS * 2;

    GG::Pt icon_ul = button.UpperLeft();
    GG::Pt icon_lr = button.LowerRight();
    GG::Pt border_ul = icon_ul;
    GG::Pt border_lr = icon_lr;

    // tone color for disabled button, or when unchecked and using a single icon with same color
    if (button.Disabled() || (!render_checked &&
                              (m_checked_color == m_unchecked_color) &&
                              (m_checked_icon == m_unchecked_icon)))
    { icon_clr = GG::BlendClr(DisabledColor(icon_clr), GG::CLR_ZERO, 0.8f); }

    // highlight on mouseover
    if (GG::StateButton::ButtonState::BN_ROLLOVER == button.State()) {
        border_clr = AdjustBrightness(border_clr, 100);
        icon_clr = AdjustBrightness(icon_clr, 1.7);
    }

    // shrink icon for border
    icon_ul.x += ICON_SIZE_REDUCE;
    icon_ul.y += ICON_SIZE_REDUCE;
    icon_lr.x -= ICON_SIZE_REDUCE;
    icon_lr.y -= ICON_SIZE_REDUCE;

    // render border
    AngledCornerRectangle(border_ul, border_lr, bg_clr, border_clr,
                          CUIBUTTON_ANGLE_OFFSET, BORDER_THICKNESS);

    // render icon
    glColor(icon_clr);
    render_checked ?
        m_checked_icon->OrthoBlit(icon_ul, icon_lr) :
        m_unchecked_icon->OrthoBlit(icon_ul, icon_lr);
}


///////////////////////////////////////
// class CUIStateButton
///////////////////////////////////////
CUIStateButton::CUIStateButton(std::string str, GG::Flags<GG::TextFormat> format,
                               std::shared_ptr<GG::StateButtonRepresenter> representer) :
    StateButton(std::move(str), ClientUI::GetFont(), format,
                ClientUI::StateButtonColor(), std::move(representer), ClientUI::TextColor())
{ SetName("CUIStateButton"); }


///////////////////////////////////////
// class CUISpin
///////////////////////////////////////
template<>
void CUISpin<double>::SetEditTextFromValue()
{
    if (!this->m_edit)
        return;
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << this->Value();
    this->m_edit->SetText(ss.str());
}


///////////////////////////////////////
// class CUITabBar
///////////////////////////////////////
CUITabBar::CUITabBar(const std::shared_ptr<GG::Font>& font, GG::Clr color, GG::Clr text_color) :
    GG::TabBar(font, color, text_color)
{ SetName("CUITabBar"); }

void CUITabBar::DistinguishCurrentTab(const std::vector<GG::StateButton*>& tab_buttons) {
    RaiseCurrentTabButton();
    int index = CurrentTabIndex();
    for (int i = 0; i < static_cast<int>(tab_buttons.size()); ++i) {
        GG::StateButton* tab = tab_buttons[i];
        GG::Clr text_color = TextColor();
        if (index == i)
            tab->SetTextColor(text_color);
        else
            tab->SetTextColor(DarkenClr(text_color));
    }
}


///////////////////////////////////////
// class CUIScroll::ScrollTab
///////////////////////////////////////
CUIScroll::ScrollTab::ScrollTab(GG::Orientation orientation, int scroll_width, GG::Clr color,
                                GG::Clr border_color) :
    Button("ScrollTab", nullptr, color),
    m_border_color(border_color),
    m_orientation(orientation)
{
    SetName("CUIScroll " + this->m_label->Text());
    MoveTo(GG::Pt(GG::X(orientation == GG::Orientation::VERTICAL ? 0 : 2),
                  GG::Y(orientation == GG::Orientation::VERTICAL ? 2 : 0)));
    Resize(GG::Pt(GG::X(scroll_width), GG::Y(scroll_width)));
    SetMinSize(GG::Pt(m_orientation == GG::Orientation::VERTICAL ? MinSize().x : GG::X(10),
                      m_orientation == GG::Orientation::VERTICAL ? GG::Y(10) : MinSize().y));
}

void CUIScroll::ScrollTab::Render() {
    GG::Pt ul = UpperLeft();
    GG::Pt lr = LowerRight();
    if (m_orientation == GG::Orientation::VERTICAL) {
        ul.x += 1;
        lr.x -= 2;
    } else {
        ul.y += 1;
        lr.y -= 2;
    }

    GG::Clr background_color = Disabled() ? DisabledColor(Color()) : Color();
    GG::Clr border_color = Disabled() ? DisabledColor(m_border_color) : m_border_color;
    if (!Disabled() && m_mouse_here) {
        background_color = AdjustBrightness(background_color, 100);
        border_color = AdjustBrightness(border_color, 100);
    }

    static constexpr int CUISCROLL_ANGLE_OFFSET = 3;

    AngledCornerRectangle(ul, lr, background_color, border_color, CUISCROLL_ANGLE_OFFSET, 1);
}

void CUIScroll::ScrollTab::LButtonUp(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    m_being_dragged = false;
    if (!InWindow(GG::GUI::GetGUI()->MousePosition()))
        m_mouse_here = false;
}

void CUIScroll::ScrollTab::MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!m_being_dragged && !m_mouse_here) {
        PlayButtonRolloverSound();
        m_mouse_here = true;
    }
}


///////////////////////////////////////
// class CUIScroll
///////////////////////////////////////
CUIScroll::CUIScroll(GG::Orientation orientation) :
    Scroll(orientation, ClientUI::CtrlColor(), ClientUI::CtrlColor()),
    m_border_color(ClientUI::CtrlBorderColor())
{ SetName("CUIScroll"); }

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

void CUIScroll::SizeMove(GG::Pt ul, GG::Pt lr) {
    const auto old_sz = Size();
    Wnd::SizeMove(ul, lr);

    TabButton()->SizeMove(TabButton()->RelativeUpperLeft(), 
                          (ScrollOrientation() == GG::Orientation::VERTICAL) ?
                          GG::Pt(Size().x, TabButton()->RelativeLowerRight().y) :
                          GG::Pt(TabButton()->RelativeLowerRight().x, Size().y));

    SizeScroll(ScrollRange().first, ScrollRange().second, LineSize(), PageSize()); // update tab size and position

    if (Size() != old_sz)
        InitBuffer();
}


///////////////////////////////////////
// class CUIListBox
///////////////////////////////////////
CUIListBox::CUIListBox() :
    ListBox(ClientUI::CtrlBorderColor(), ClientUI::CtrlColor())
{
    SelRowsChangedSignal.connect(-1,    &PlayListSelectSound);
    DroppedRowSignal.connect(-1,        &PlayItemDropSound);
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
    constexpr int CUIDROPDOWNLIST_ANGLE_OFFSET = 5;
}

CUIDropDownList::CUIDropDownList(std::size_t num_shown_elements) :
    DropDownList(num_shown_elements, ClientUI::CtrlBorderColor()),
    m_render_drop_arrow(true),
    m_mouse_here(false)
{
    SetInteriorColor(ClientUI::CtrlColor());
    SetMinSize(GG::Pt(MinSize().x, CUISimpleDropDownListRow::DEFAULT_ROW_HEIGHT));
    SetChildClippingMode(ChildClippingMode::ClipToClient);
    SetName("CUIDropDownList");
}

void CUIDropDownList::InitBuffer() {
    m_buffer.clear();
    const auto sz = Size();
    BufferStoreAngledCornerRectangleVertices(this->m_buffer, GG::Pt0, sz,
                                             CUIDROPDOWNLIST_ANGLE_OFFSET, false, true, false);

    static constexpr int margin = 3;
    const int triangle_width = Value(sz.y - 4 * margin);
    const int outline_width = triangle_width + 3 * margin;

    const auto triangle_ul = GG::Pt(sz.x - triangle_width - margin * 5 / 2, GG::Y(2 * margin));
    const auto triangle_lr = GG::Pt(sz.x - margin * 5 / 2, sz.y - 2 * margin);
    BufferStoreIsoscelesTriangle(this->m_buffer, triangle_ul, triangle_lr, ShapeOrientation::DOWN);

    const auto btn_ul = GG::Pt(sz.x - outline_width - margin, GG::Y(margin));
    const auto btn_lr = GG::Pt(sz.x - margin, sz.y - margin);

    BufferStoreAngledCornerRectangleVertices(this->m_buffer, btn_ul, btn_lr,
                                             CUIDROPDOWNLIST_ANGLE_OFFSET, false, true, false);

    this->m_buffer.createServerBuffer();
}

void CUIDropDownList::Render() {
    GG::Pt ul = UpperLeft();
    GG::Clr lb_color = LB()->Color();
    GG::Clr border_color = Disabled() ? DisabledColor(lb_color) : lb_color;
    if (GG::GUI::GetGUI()->FocusWnd().get() == this)
        border_color = GG::LightenClr(border_color);
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
            triangle_fill_color = AdjustBrightness(triangle_fill_color, ARROW_BRIGHTENING_SCALE_FACTOR);
        GG::Clr triangle_border_color = AdjustBrightness(triangle_fill_color, 75);

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

GG::Pt CUIDropDownList::ClientLowerRight() const noexcept
{
    const auto sz = Size();
    const int margin = 3;
    const int triangle_width = Value(sz.y - 4 * margin);
    const int outline_width = triangle_width + 4 * margin;
    return DropDownList::ClientLowerRight()- GG::Pt(
        (m_render_drop_arrow ? GG::X(outline_width + 2 * margin) : GG::X0),
        GG::Y0);
}

GG::X CUIDropDownList::DroppedRowWidth() const
{ return (DropDownList::ClientLowerRight().x - DropDownList::ClientUpperLeft().x); }

void CUIDropDownList::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        PlayDropDownListOpenSound();
    DropDownList::LClick(pt, mod_keys);
}

void CUIDropDownList::MouseEnter(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (!Disabled())
        PlayButtonRolloverSound();
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
CUIEdit::CUIEdit(std::string str) :
    Edit(std::move(str), ClientUI::GetFont(), ClientUI::CtrlBorderColor(),
         ClientUI::TextColor(), ClientUI::CtrlColor())
{}

void CUIEdit::CompleteConstruction() {
    EditedSignal.connect(-1, &PlayTextTypingSound);
    SetHiliteColor(ClientUI::EditHiliteColor());

    HotkeyManager& hkm = HotkeyManager::GetManager();
    auto fx = boost::bind(&CUIEdit::AutoComplete, this);
    hkm.Connect(fx, "ui.autocomplete", FocusWindowCondition(this));
    hkm.RebuildShortcuts();

    SetName("CUIEdit");
}

void CUIEdit::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto hotkey_cut_action        = [this]() { GG::GUI::GetGUI()->CutWndText(this); };
    auto hotkey_copy_action       = [this]() { GG::GUI::GetGUI()->CopyWndText(this); };
    auto hotkey_paste_action      = [this]() { GG::GUI::GetGUI()->PasteWndText(this, GG::GUI::GetGUI()->ClipboardText()); };
    auto hotkey_select_all_action = [this]() { GG::GUI::GetGUI()->WndSelectAll(this); };
    auto hotkey_deselect_action   = [this]() { GG::GUI::GetGUI()->WndDeselect(this); };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_CUT"),        this->Disabled(), false, hotkey_cut_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_COPY"),       false,            false, hotkey_copy_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_PASTE"),      this->Disabled(), false, hotkey_paste_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_SELECT_ALL"), this->Disabled(), false, hotkey_select_all_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_DESELECT"),   this->Disabled(), false, hotkey_deselect_action));
    popup->Run();

    // todo: italicize, underline, or colour selected text
}

void CUIEdit::KeyPress(GG::Key key, uint32_t key_code_point,
                       GG::Flags<GG::ModKey> mod_keys)
{
    if (Disabled()) {
        GG::Edit::KeyPress(key, key_code_point, mod_keys);
        return;
    }

    bool shift_down = mod_keys & (GG::MOD_KEY_LSHIFT | GG::MOD_KEY_RSHIFT);
    //bool ctrl_down = mod_keys & (GG::MOD_KEY_CTRL | GG::MOD_KEY_RCTRL);
    //bool numlock_on = mod_keys & GG::MOD_KEY_NUM;

    if (key == GG::Key::GGK_DELETE && shift_down) {
        GG::GUI::GetGUI()->CutWndText(this);

    } else if (key == GG::Key::GGK_INSERT && shift_down) {
        GG::GUI::GetGUI()->PasteWndText(this, GG::GUI::GetGUI()->ClipboardText());

    // todo: italicize, underline selected text with ctrl-i or ctrl-u
    // ... though maybe better with a hotkey?

    } else {
        GG::Edit::KeyPress(key, key_code_point, mod_keys);
    }
}

void CUIEdit::AcceptPastedText(const std::string& text) {
    std::string text_copy;
    std::copy_if(text.begin(), text.end(), std::back_inserter(text_copy),
                 [this](char c){ return m_disallowed_chars.find(c) == std::string::npos; });
    GG::Edit::AcceptPastedText(text_copy);
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
    if (GG::GUI::GetGUI()->FocusWnd().get() == this)
        border_color = GG::LightenClr(border_color);
    GG::Clr int_color_to_use = Disabled() ? DisabledColor(InteriorColor()) : InteriorColor();


    GG::Pt ul = UpperLeft(), lr = LowerRight();
    //GG::Pt client_ul = ClientUpperLeft(), client_lr = ClientLowerRight();

    FlatRectangle(ul, lr, int_color_to_use, border_color, 1);

    SetColor(GG::CLR_ZERO);
    Edit::Render();
    SetColor(color);
}


///////////////////////////////////////
// class CensoredCUIEdit
///////////////////////////////////////
CensoredCUIEdit::CensoredCUIEdit(std::string str, char display_placeholder) :
    CUIEdit(str),
    m_placeholder{display_placeholder},
    m_raw_text(std::move(str))
{
    SetName("CensoredCUIEdit");
    // TODO: allow multi-byte UTF-8 characters as placeholders, stored in a string...?
    if (m_placeholder == 0)
        m_placeholder = ' ';
}

void CensoredCUIEdit::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto hotkey_paste_action      = [this]() { GG::GUI::GetGUI()->PasteWndText(this, GG::GUI::GetGUI()->ClipboardText()); };
    auto hotkey_select_all_action = [this]() { GG::GUI::GetGUI()->WndSelectAll(this); };
    auto hotkey_deselect_action   = [this]() { GG::GUI::GetGUI()->WndDeselect(this); };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);

    // omits copy command from base CUIEdit
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_PASTE"),      this->Disabled(), false, hotkey_paste_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_SELECT_ALL"), this->Disabled(), false, hotkey_select_all_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_DESELECT"),   this->Disabled(), false, hotkey_deselect_action));
    popup->Run();
}

void CensoredCUIEdit::SetText(std::string str) {
    m_raw_text = std::move(str);

    const auto& font = GetFont();
    if (!font) {
        ErrorLogger() << "CensoredCUIEdit::SetText couldn't get font!";
        return;
    }
    auto format = this->GetTextFormat();
    auto text_elements = font->ExpensiveParseFromTextToTextElements(m_raw_text, format);
    auto line_data = font->DetermineLines(m_raw_text, format, ClientSize().x, text_elements);

    // generate censored text by appending one placeholder char per char in raw text
    std::string censored_text;
    for (const auto& curr_line : line_data)
        censored_text += std::string(curr_line.char_data.size(), m_placeholder);

    CUIEdit::SetText(std::move(censored_text));
}

void CensoredCUIEdit::AcceptPastedText(const std::string& text) {
    if (!Interactive())
        return;
    if (!utf8::is_valid(text.begin(), text.end()))
        return;

    bool modified_text = false;

    if (MultiSelected()) {
        ClearSelected();
        modified_text = true;
        m_cursor_pos.second = m_cursor_pos.first;   // should be redundant
    }

    if (!text.empty()) {
        const auto [line, pos] = LinePositionOfGlyph(m_cursor_pos.first, GetLineData()); // TODO: should look up code point, not glyph index

        std::string new_raw_text = m_raw_text;
        new_raw_text.insert(Value(StringIndexOfLineAndGlyph(line, pos, GetLineData())), text);

        SetText(std::move(new_raw_text));

        modified_text = true;
    }

    if (modified_text) {
        // moves cursor to end of pasted text
        GG::CPSize text_span{static_cast<std::size_t>(utf8::distance(text.begin(), text.end()))};
        GG::CPSize new_cursor_pos = std::max(GG::CP0, std::min(Length(), m_cursor_pos.second + text_span));
        m_cursor_pos.second = new_cursor_pos;

        // ensure nothing is selected after pasting
        m_cursor_pos.first = m_cursor_pos.second;

        // notify rest of GUI of change to text in this Edit
        EditedSignal(Text());
    }
}

void CensoredCUIEdit::ClearSelected() {
    // get range of indices to remove
    GG::CPSize low = std::min(m_cursor_pos.first, m_cursor_pos.second);
    GG::CPSize high = std::max(m_cursor_pos.first, m_cursor_pos.second);
    // set cursor start/end to start of range to remove
    m_cursor_pos = {low, low};

    auto it = m_raw_text.begin() + Value(low);
    auto end_it = m_raw_text.begin() + Value(high);

    if (it == m_raw_text.end() || it == end_it)
        return;

    m_raw_text.erase(it, end_it);

    SetText(m_raw_text);
}

///////////////////////////////////////
// class CUIMultiEdit
///////////////////////////////////////
CUIMultiEdit::CUIMultiEdit(std::string str, GG::Flags<GG::MultiEditStyle> style) :
    MultiEdit(std::move(str), ClientUI::GetFont(), ClientUI::CtrlBorderColor(), style,
              ClientUI::TextColor(), ClientUI::CtrlColor())
{}

void CUIMultiEdit::CompleteConstruction() {
    GG::MultiEdit::CompleteConstruction();

    RecreateScrolls();
    SetHiliteColor(ClientUI::EditHiliteColor());
    SetName("CUIMultiEdit");
}

void CUIMultiEdit::Render() {
    GG::Clr color = Color();
    GG::Clr border_color =      Disabled()  ?   DisabledColor(color)            :   color;
    if (GG::GUI::GetGUI()->FocusWnd().get() == this)
        border_color = GG::LightenClr(border_color);
    GG::Clr int_color_to_use =  Disabled()  ?   DisabledColor(InteriorColor())  :   InteriorColor();

    GG::Pt ul = UpperLeft(), lr = LowerRight();

    FlatRectangle(ul, lr, int_color_to_use, border_color, 1);

    SetColor(GG::CLR_ZERO);
    MultiEdit::Render();
    SetColor(color);
}

void CUIMultiEdit::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto hotkey_cut_action        = [this]() { GG::GUI::GetGUI()->CutWndText(this); };
    auto hotkey_copy_action       = [this]() { GG::GUI::GetGUI()->CopyWndText(this); };
    auto hotkey_paste_action      = [this]() { GG::GUI::GetGUI()->PasteWndText(this, GG::GUI::GetGUI()->ClipboardText()); };
    auto hotkey_select_all_action = [this]() { GG::GUI::GetGUI()->WndSelectAll(this); };
    auto hotkey_deselect_action   = [this]() { GG::GUI::GetGUI()->WndDeselect(this); };

    // create popup menu
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (!(this->Style() & GG::MULTI_READ_ONLY))
        popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_CUT"),    this->Disabled(), false, hotkey_cut_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_COPY"),       false,            false, hotkey_copy_action));
    if (!(this->Style() & GG::MULTI_READ_ONLY))
        popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_PASTE"),  this->Disabled(), false, hotkey_paste_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_SELECT_ALL"), this->Disabled(), false, hotkey_select_all_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_DESELECT"),   this->Disabled(), false, hotkey_deselect_action));
    popup->Run();
    // todo: italicize, underline, or colour selected text
}


///////////////////////////////////////
// class CUILinkTextMultiEdit
///////////////////////////////////////
CUILinkTextMultiEdit::CUILinkTextMultiEdit(std::string str,
                                           GG::Flags<GG::MultiEditStyle> style) :
    CUIMultiEdit(str, style),
    TextLinker(),
    m_raw_text(std::move(str))
{}

void CUILinkTextMultiEdit::CompleteConstruction() {
    // Prevent double wrapping or setting the raw_text equal to an already
    // wrapped raw text.  CUIMultiEdit::CompleteConstruction calls
    // SetText when adjusting the scroll bars.
    m_already_setting_text_so_dont_link = true;
    CUIMultiEdit::CompleteConstruction();
    m_already_setting_text_so_dont_link = false;

    FindLinks();
    MarkLinks();

    SetName("CUILinkTextMultiEdit: " + m_raw_text.substr(0, 16));
}

GG::Pt CUILinkTextMultiEdit::TextUpperLeft() const
{ return CUIMultiEdit::TextUpperLeft() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5)); }

GG::Pt CUILinkTextMultiEdit::TextLowerRight() const
{ return CUIMultiEdit::TextLowerRight() - ScrollPosition() + GG::Pt(GG::X(5), GG::Y(5)); }

void CUILinkTextMultiEdit::Render() {
    CUIMultiEdit::Render();
    TextLinker::Render_();
}

void CUILinkTextMultiEdit::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::LClick(pt, mod_keys);
    TextLinker::LClick_(pt, mod_keys);
}

void CUILinkTextMultiEdit::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto rclick_action = [this, pt, mod_keys]() { TextLinker::RClick_(pt, mod_keys); };
    auto hotkey_cut_action        = [this]() { GG::GUI::GetGUI()->CutWndText(this); };
    auto hotkey_copy_action       = [this]() { GG::GUI::GetGUI()->CopyWndText(this); };
    auto hotkey_paste_action      = [this]() { GG::GUI::GetGUI()->PasteWndText(this, GG::GUI::GetGUI()->ClipboardText()); };
    auto hotkey_select_all_action = [this]() { GG::GUI::GetGUI()->WndSelectAll(this); };
    auto hotkey_deselect_action   = [this]() { GG::GUI::GetGUI()->WndDeselect(this); };

    // create popup menu
    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    if (GetLinkUnderPt(pt) != -1) {
        popup->AddMenuItem(GG::MenuItem(UserString("OPEN"),          false,            false, rclick_action));
        popup->AddMenuItem(GG::MenuItem(true)); // separator
    }
    if (!(this->Style() & GG::MULTI_READ_ONLY))
        popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_CUT"),    this->Disabled(), false, hotkey_cut_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_COPY"),       false,            false, hotkey_copy_action));
    if (!(this->Style() & GG::MULTI_READ_ONLY))
        popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_PASTE"),  this->Disabled(), false, hotkey_paste_action));
    popup->AddMenuItem(GG::MenuItem(true)); // separator
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_SELECT_ALL"), this->Disabled(), false, hotkey_select_all_action));
    popup->AddMenuItem(GG::MenuItem(UserString("HOTKEY_DESELECT"),   this->Disabled(), false, hotkey_deselect_action));
    popup->Run();

    // todo: italicize, underline, or colour selected text
}

void CUILinkTextMultiEdit::MouseHere(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    CUIMultiEdit::MouseHere(pt, mod_keys);
    TextLinker::MouseHere_(pt, mod_keys);
}

void CUILinkTextMultiEdit::MouseLeave() {
    CUIMultiEdit::MouseLeave();
    TextLinker::MouseLeave_();
}

void CUILinkTextMultiEdit::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt lower_right = lr;
    if (Style() & GG::MULTI_INTEGRAL_HEIGHT)
        lower_right.y -= Value((lr.y - ul.y) - (2 * PIXEL_MARGIN)) % Value(GetFont()->Lineskip());
    const bool resized = lower_right - ul != Size();

    // need to restore scroll position after SetText call below, so that
    // resizing this control doesn't reset the scroll position to the top.
    // just calling PreserveTextPositionOnNextSetText() before the SetText
    // call doesn't work as that leaves the scrollbars unadjusted for the resize
    const GG::Pt initial_scroll_pos = ScrollPosition();

    GG::Edit::SizeMove(ul, lower_right);

    if (resized) {
        SetText(RawText());                     // this line is the primary difference between this and MultiEdit::SizeMove
        SetScrollPosition(initial_scroll_pos);  // restores scroll position
    }
}

void CUILinkTextMultiEdit::SetText(std::string str) {
    // MultiEdit have scrollbars that are adjusted every time the text is set.
    // Adjusting scrollbars also requires setting text, because the space for
    // the text is added or removed when scrollbars are shown or hidden. Since
    // highlighting links on rollover also involves setting text, there are a
    // lot of potentially unnecessary calls to SetText and FindLinks.  This
    // check for whether text is already being set eliminates many of those
    // calls when they aren't necessary, since the results will be overridden
    // later anyway by the outermost (or lowest on stack, or first) call to
    // SetText
    if (!m_already_setting_text_so_dont_link) {
        m_already_setting_text_so_dont_link = true;
        m_raw_text = std::move(str);
        CUIMultiEdit::SetText(m_raw_text);  // so that line data is updated for use in FindLinks. intentionally not moving
        FindLinks();
        MarkLinks();
        m_already_setting_text_so_dont_link = false;
        // The scrollbar shenanigans apparently also confuse the link locations
        // so we refresh them here.
        LocateLinks();

    } else {
        CUIMultiEdit::SetText(std::move(str));
    }
}

void CUILinkTextMultiEdit::SetLinkedText(std::string str) {
    MultiEdit::PreserveTextPositionOnNextSetText();
    CUIMultiEdit::SetText(std::move(str));
}


///////////////////////////////////////
// class CUISimpleDropDownListRow
///////////////////////////////////////
CUISimpleDropDownListRow::CUISimpleDropDownListRow(std::string row_text, GG::Y row_height) :
    GG::ListBox::Row(GG::X1, row_height),
    m_row_label(GG::Wnd::Create<CUILabel>(std::move(row_text), GG::FORMAT_LEFT | GG::FORMAT_NOWRAP))
{}

void CUISimpleDropDownListRow::CompleteConstruction() {
    GG::ListBox::Row::CompleteConstruction();
    push_back(m_row_label);
    SetName("CUILinkTextMultiEdit");
}


///////////////////////////////////////
// class StatisticIcon
///////////////////////////////////////
namespace {
    constexpr int STAT_ICON_PAD = 2;    // horizontal or vertical space between icon and label
}

StatisticIcon::StatisticIcon(std::shared_ptr<GG::Texture> texture, GG::X w, GG::Y h) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE)
{ m_icon = GG::Wnd::Create<GG::StaticGraphic>(std::move(texture), GG::GRAPHIC_FITGRAPHIC); }

StatisticIcon::StatisticIcon(std::shared_ptr<GG::Texture> texture,
                             double value, int digits, bool showsign,
                             GG::X w, GG::Y h) :
    GG::Control(GG::X0, GG::Y0, w, h, GG::INTERACTIVE),
    m_values({std::tuple<double, int, bool>{value, digits, showsign}, {0.0, 0, false}})
{ m_icon = GG::Wnd::Create<GG::StaticGraphic>(std::move(texture), GG::GRAPHIC_FITGRAPHIC); }

void StatisticIcon::CompleteConstruction() {
    GG::Control::CompleteConstruction();

    SetName("StatisticIcon");
    SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));

    AttachChild(m_icon);    // created in constructor to forward texture

    const auto format = (Value(Width()) >= Value(Height())) ? GG::FORMAT_LEFT : GG::FORMAT_BOTTOM;
    m_text = GG::Wnd::Create<CUILabel>("    ", format, GG::NO_WND_FLAGS);
    AttachChild(m_text);

    RequirePreRender();
}

void StatisticIcon::PreRender() {
    GG::Wnd::PreRender();
    DoLayout();
}

double StatisticIcon::GetValue(std::size_t index) const {
    if (index < 0u || (!m_have_two && index > 0) || (m_have_two && index > 1)) {
        ErrorLogger() << "StatisticIcon::GetValue passed index out of range index:" << index;
        return 0.0;
    }
    return std::get<0>(m_values[index]);
}

void StatisticIcon::SetValue(double value, std::size_t index) {
    if (index > 1u) {
        ErrorLogger() << "StatisticIcon::SetValue passed index out of range index:" << index;
        return;
    }
    const auto& font = ClientUI::GetFont();
    if (!font) {
        ErrorLogger() << "StatisticIcon::SetValue couldn't get a font";
        return;
    }

    auto& [value0, precision0, show_sign0] = m_values[0];
    auto& [value1, precision1, show_sign1] = m_values[1];

    auto& valuei = index == 0 ? value0 : value1;
    if (value != valuei) {
        RequirePreRender();
        valuei = value;
    }

    const bool had_two = m_have_two;
    if (index == 1)
        m_have_two = true;
    if (had_two != m_have_two)
        RequirePreRender();

    // Compute text elements
    GG::Font::TextAndElementsAssembler text_elements(*font, 120, 8); // usually 63 chars enough, but might have bigger numbers and don't want to guess from precision...

    text_elements
        .AddOpenTag(ClientUI::TextColor())
        .AddText(DoubleToString(value0, precision0, show_sign0))
        .AddCloseTag("rgba");

    if (m_have_two) {
        const auto effective_sign = EffectiveSign(value1);
        const auto clr = (effective_sign == -1) ? ClientUI::StatDecrColor() :
            (effective_sign == 1) ? ClientUI::StatIncrColor() :
            ClientUI::TextColor();

        text_elements
            .AddText(" ")
            .AddOpenTag(clr);

        if (effective_sign != -1)
            text_elements.AddText("+");

        text_elements
            .AddText(DoubleToString(value1, precision1, show_sign1))
            .AddCloseTag("rgba");
    }

    auto [text, elements] = text_elements.Extract();
    m_text->SetText(std::move(text), std::move(elements));

    DoLayout();
}

void StatisticIcon::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    GG::Wnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        RequirePreRender();
}

void StatisticIcon::LButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::RButtonDown(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (Disabled())
        return;
    LeftClickedSignal(pt);
}

void StatisticIcon::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    if (Disabled())
        return;
    RightClickedSignal(pt);
}

void StatisticIcon::MouseWheel(GG::Pt pt, int move, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::AcceptDrops(GG::Pt pt, std::vector<std::shared_ptr<GG::Wnd>> wnds, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::DragDropEnter(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                                  GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::DragDropHere(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable,
                                 GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::CheckDrops(GG::Pt pt, std::map<const GG::Wnd*, bool>& drop_wnds_acceptable, GG::Flags<GG::ModKey> mod_keys)
{ ForwardEventToParent(); }

void StatisticIcon::DragDropLeave()
{ ForwardEventToParent(); }

void StatisticIcon::DoLayout() {
    // arrange child controls horizontally if icon is wider than it is high, or vertically otherwise
    int icon_dim = std::min(Value(Height()), Value(Width()));
    m_icon->SizeMove(GG::Pt0,
                     GG::Pt(GG::X(icon_dim), GG::Y(icon_dim)));

    if (m_values.empty())
        return;

    GG::Pt text_ul;
    if (Value(Width()) >= Value(Height())) {
        text_ul.x = GG::X(icon_dim + STAT_ICON_PAD);
    } else {
        text_ul.y = GG::Y(icon_dim + STAT_ICON_PAD);
    }

    // Adjust text and place label.
    m_text->SizeMove(text_ul, {Width(), Height()});
}

GG::Pt StatisticIcon::MinUsableSize() const {
    if (!m_icon)
        return GG::Pt(GG::X1, GG::Y1);

    if (m_values.empty() || !m_text)
        return m_icon->Size();

    if (Value(Width()) >= Value(Height()))
        return GG::Pt(m_text->RelativeUpperLeft().x + m_text->Width(),
                      std::max(m_icon->RelativeLowerRight().y, m_text->Height()));
    else
        return GG::Pt(std::max(m_icon->RelativeLowerRight().x, m_text->Width()),
                      m_icon->RelativeLowerRight().y + m_text->Height());
}

///////////////////////////////////////
// class CUIToolBar
///////////////////////////////////////
CUIToolBar::CUIToolBar() :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::ONTOP | GG::INTERACTIVE)
{}

bool CUIToolBar::InWindow(GG::Pt pt) const {
    for (auto& wnd : Children())
        if (wnd->InWindow(pt))
            return true;
    return GG::Wnd::InWindow(pt);
}

void CUIToolBar::Render() {
    GG::Pt ul(UpperLeft() - GG::Pt(GG::X1, GG::Y1));
    GG::Pt lr(LowerRight() + GG::Pt(GG::X1, GG::Y0));
    GG::FlatRectangle(ul, lr, ClientUI::WndColor(), ClientUI::WndOuterBorderColor(), 1);
}


///////////////////////////////////////
// class SpeciesSelector
///////////////////////////////////////
namespace {
#if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 12) || (__GNUC__ == 12 && __GNUC_MINOR__ >= 2))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934))) && ((!defined(__clang_major__) || (__clang_major__ >= 17)))
    constexpr std::string EMPTY_STRING;
#else
    const std::string EMPTY_STRING;
#endif

    // row type used in the SpeciesSelector
    struct SpeciesRow : public GG::ListBox::Row {
        SpeciesRow(const Species& species, GG::X w, GG::Y h) :
            GG::ListBox::Row(w, h)
        {
            SetMargin(0);
            const std::string& species_name = species.Name();
            GG::Wnd::SetName(species_name);
            Init(species_name, UserString(species_name), species.GameplayDescription(), w, h,
                 ClientUI::SpeciesIcon(species_name));
        };

        SpeciesRow(std::string species_name, std::string localized_name, std::string species_desc,
                   GG::X w, GG::Y h, std::shared_ptr<GG::Texture> species_icon) :
            GG::ListBox::Row(w, h)
        {
            SetMargin(0);
            Init(std::move(species_name), std::move(localized_name),
                 std::move(species_desc), w, h, std::move(species_icon));
        };

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();

            push_back(m_icon);
            push_back(m_species_label);
            GG::X first_col_width{Value(Height())};
            SetColWidth(0, first_col_width);
            SetColWidth(1, Width() - first_col_width);
            GetLayout()->SetColumnStretch(0, 0.0);
            GetLayout()->SetColumnStretch(1, 1.0);
        }
    private:
        void Init(std::string species_name, std::string localized_name, std::string species_desc,
                  GG::X width, GG::Y height, std::shared_ptr<GG::Texture> species_icon)
        {
            GG::Wnd::SetName(std::move(species_name));
            m_icon = GG::Wnd::Create<GG::StaticGraphic>(
                species_icon, GG::GRAPHIC_FITGRAPHIC| GG::GRAPHIC_PROPSCALE);
            m_icon->Resize(GG::Pt(GG::X(Value(height - 5)), height - 5));
            m_species_label = GG::Wnd::Create<CUILabel>(
                localized_name, GG::FORMAT_LEFT | GG::FORMAT_VCENTER);
            if (!species_desc.empty()) {
                SetBrowseModeTime(GetOptionsDB().Get<int>("ui.tooltip.delay"));
                SetBrowseInfoWnd(GG::Wnd::Create<IconTextBrowseWnd>(
                    std::move(species_icon), std::move(localized_name), std::move(species_desc)));
            }
        }

        std::shared_ptr<GG::StaticGraphic> m_icon;
        std::shared_ptr<CUILabel> m_species_label;
    };
}

SpeciesSelector::SpeciesSelector(const std::string& preselect_species, GG::X w, GG::Y h) :
    CUIDropDownList(6)
{
    ManuallyManageColProps();
    NormalizeRowsOnInsert(false);
    SetNumCols(2);
    SetChildClippingMode(ChildClippingMode::ClipToClient);

    Resize(GG::Pt(w, h - 8));

    SelChangedSignal.connect(
        [this](GG::DropDownList::iterator it) {
            SpeciesChangedSignal((it == this->end() || !(*it)) ? EMPTY_STRING : (*it)->Name()); });

    const SpeciesManager& sm = GGHumanClientApp::GetApp()->GetSpeciesManager();
    for (auto it = sm.playable_begin(); it != sm.playable_end(); ++it) {
        auto row_it = Insert(GG::Wnd::Create<SpeciesRow>(it->second, w, h - 4));
        if (it->first == preselect_species)
            Select(row_it);
    }

    if (!this->Empty()) {
        // Add an option for random selection
        auto rand_species_it = Insert(GG::Wnd::Create<SpeciesRow>(
            "RANDOM", UserString("GSETUP_RANDOM"), UserString("GSETUP_SPECIES_RANDOM_DESC"), w, h - 4,
            ClientUI::GetTexture(ClientUI::ArtDir() / "icons/unknown.png")));

        if (preselect_species == "RANDOM")
            Select(rand_species_it);
    }

    if (!Empty() && CurrentItem() == end()) {
        ErrorLogger() << "SpeciesSelector::SpeciesSelector was unable to find a species in the list with name " << preselect_species;
        Select(begin());
    }
}

const std::string& SpeciesSelector::CurrentSpeciesName() const {
    auto row_it = this->CurrentItem();
    if (row_it == this->end())
        return EMPTY_STRING;
    const auto& row = *row_it;
    if (!row) {
        ErrorLogger() << "SpeciesSelector::CurrentSpeciesName couldn't get current item due to invalid Row pointer";
        return EMPTY_STRING;
    }
    return row->Name();
}


///////////////////////////////////////
// class EmpireColorSelector
///////////////////////////////////////
namespace {
    constexpr GG::X COLOR_SELECTOR_WIDTH{75};

    // row type used in the EmpireColorSelector
    struct ColorRow : public GG::ListBox::Row {
        struct ColorSquare : GG::Control {
            ColorSquare(GG::Clr color, GG::Y h) :
                GG::Control(GG::X0, GG::Y0, COLOR_SELECTOR_WIDTH - 40, h, GG::NO_WND_FLAGS)
            {
                SetColor(color);
                SetMinSize(GG::Pt(COLOR_SELECTOR_WIDTH - 40, GG::Y1));
            }

            void Render() override
            { GG::FlatRectangle(UpperLeft(), LowerRight(), Color(), GG::CLR_ZERO, 0); }
        };

        ColorRow(GG::Clr color, GG::Y h) :
            GG::ListBox::Row(GG::X(Value(h)), h),
            m_color_square(GG::Wnd::Create<ColorSquare>(color, h))
        {}

        void CompleteConstruction() override {
            GG::ListBox::Row::CompleteConstruction();
            push_back(m_color_square);
        }

        void SizeMove(GG::Pt ul, GG::Pt lr) override {
            // Prevent the width from changing
            GG::Control::SizeMove(ul, GG::Pt(ul.x + Width(), lr.y));
        }
    private:
        std::shared_ptr<ColorSquare> m_color_square;

    };
}

EmpireColorSelector::EmpireColorSelector(GG::Y h) :
    CUIDropDownList(6)
{
    Resize(GG::Pt(COLOR_SELECTOR_WIDTH, h - 8));

    for (auto& color : EmpireColors())
        Insert(GG::Wnd::Create<ColorRow>(color, h - 4));

    SelChangedSignal.connect(
        [this](GG::DropDownList::iterator it) {
            ColorChangedSignal(!(it == end() || !*it || (*it)->empty()) ? (*it)->at(0)->Color() : GG::CLR_RED); });
}

GG::Clr EmpireColorSelector::CurrentColor() const
{ return !(**CurrentItem()).empty() ? (**CurrentItem()).at(0)->Color() : GG::CLR_RED; }

void EmpireColorSelector::SelectColor(GG::Clr clr) {
    for (iterator list_it = begin(); list_it != end(); ++list_it) {
        const auto& row = *list_it;
        if (row && !row->empty() && row->at(0)->Color() == clr) {
            Select(list_it);
            return;
        }
    }
    ErrorLogger() << "EmpireColorSelector::SelectColor was unable to find a requested color!";
}


///////////////////////////////////////
// class ColorSelector
///////////////////////////////////////
ColorSelector::ColorSelector(GG::Clr color, GG::Clr default_color) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1),
    m_default_color(default_color)
{ SetColor(color); }

void ColorSelector::InitBuffer() {
    const auto sz = Size();
    m_border_buffer.clear();
    m_border_buffer.store(0.0f,                             0.0f);
    m_border_buffer.store(static_cast<float>(Value(sz.x)),  0.0f);
    m_border_buffer.store(static_cast<float>(Value(sz.x)),  static_cast<float>(Value(sz.y)));
    m_border_buffer.store(0.0f,                             static_cast<float>(Value(sz.y)));
    m_border_buffer.createServerBuffer();
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

void ColorSelector::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Control::Size();

    GG::Control::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        InitBuffer();
}

void ColorSelector::LClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    GG::X x = std::min(pt.x, GG::GUI::GetGUI()->AppWidth() - 315);    // 315 is width of ColorDlg from GG::ColorDlg:::ColorDlg
    GG::Y y = std::min(pt.y, GG::GUI::GetGUI()->AppHeight() - 300);   // 300 is height of ColorDlg from GG::ColorDlg:::ColorDlg
    auto dlg = GG::Wnd::Create<GG::ColorDlg>(x, y, Color(), ClientUI::GetFont(),
                                             ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(),
                                             ClientUI::TextColor());
    dlg->Run();
    if (dlg->ColorWasSelected()) {
        GG::Clr clr = dlg->Result();
        SetColor(clr);
        ColorChangedSignal(clr);
    }
}

void ColorSelector::RClick(GG::Pt pt, GG::Flags<GG::ModKey> mod_keys) {
    auto reset_color_action = [this]() {
        SetColor(m_default_color);
        ColorChangedSignal(m_default_color);
    };

    auto popup = GG::Wnd::Create<CUIPopupMenu>(pt.x, pt.y);
    popup->AddMenuItem(GG::MenuItem(UserString("RESET"), false, false, reset_color_action));
    popup->Run();
}


///////////////////////////////////////
// class FileDlg
///////////////////////////////////////
FileDlg::FileDlg(const std::string& directory, const std::string& filename, bool save, bool multi,
                 std::vector<std::pair<std::string, std::string>> types) :
    GG::FileDlg(directory, filename, save, multi, ClientUI::GetFont(),
                ClientUI::CtrlColor(), ClientUI::CtrlBorderColor(), ClientUI::TextColor()),
    m_init_file_filters(std::move(types))
{}

void FileDlg::CompleteConstruction() {
    GG::FileDlg::CompleteConstruction();

    SetFileFilters(m_init_file_filters);
    AppendMissingSaveExtension(true);
}


//////////////////////////////////////////////////
// ResourceInfoPanel
//////////////////////////////////////////////////
ResourceInfoPanel::ResourceInfoPanel(std::string title, std::string point_units_str,
                                     const GG::X x, const GG::Y y, const GG::X w, const GG::Y h,
                                     std::string_view config_name) :
    CUIWnd(title, x, y, w, h,
           GG::INTERACTIVE | GG::RESIZABLE | GG::DRAGABLE | GG::ONTOP | PINABLE,
           config_name, false),
    m_units_str(std::move(point_units_str)),
    m_title_str(std::move(title)),
    m_empire_id(ALL_EMPIRES),
    m_empire_column_label(GG::Wnd::Create<CUILabel>(UserString("EMPIRE"), GG::FORMAT_LEFT)),
    m_local_column_label(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_total_points_label(GG::Wnd::Create<CUILabel>(UserString("PRODUCTION_INFO_TOTAL_PS_LABEL"), GG::FORMAT_RIGHT)),
    m_total_points(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_total_points_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_stockpile_points_label(GG::Wnd::Create<CUILabel>(UserString("PRODUCTION_INFO_STOCKPILE_PS_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_points(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_stockpile_points_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_stockpile_use_label(GG::Wnd::Create<CUILabel>(UserString("PRODUCTION_INFO_STOCKPILE_USE_PS_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_use(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_stockpile_use_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_local_stockpile_use(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_local_stockpile_use_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_stockpile_max_use_label(GG::Wnd::Create<CUILabel>(UserString("PRODUCTION_INFO_STOCKPILE_USE_MAX_LABEL"), GG::FORMAT_RIGHT)),
    m_stockpile_max_use(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_stockpile_max_use_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_wasted_points_label(GG::Wnd::Create<CUILabel>(UserString("PRODUCTION_INFO_WASTED_PS_LABEL"), GG::FORMAT_RIGHT)),
    m_wasted_points(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_wasted_points_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_local_points(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_local_points_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT)),
    m_local_wasted_points(GG::Wnd::Create<CUILabel>("", GG::FORMAT_LEFT)),
    m_local_wasted_points_P_label(GG::Wnd::Create<CUILabel>(m_units_str, GG::FORMAT_LEFT))
{}

void ResourceInfoPanel::CompleteConstruction() {
    CUIWnd::CompleteConstruction();

    AttachChild(m_empire_column_label);
    AttachChild(m_local_column_label);

    AttachChild(m_total_points_label);
    AttachChild(m_total_points);
    AttachChild(m_total_points_P_label);

    AttachChild(m_wasted_points_label);
    AttachChild(m_wasted_points);
    AttachChild(m_wasted_points_P_label);

    DoLayout();
}

GG::Pt ResourceInfoPanel::MinUsableSize() const {
    GG::X min_x = this->LeftBorder() + this->RightBorder() + 20*ClientUI::Pts();
    GG::Y min_y = this->TopBorder() + this->BottomBorder() + 4*ClientUI::Pts();
    return GG::Pt(min_x, min_y);
}

void ResourceInfoPanel::SizeMove(GG::Pt ul, GG::Pt lr) {
    GG::Pt old_size = GG::Wnd::Size();

    CUIWnd::SizeMove(ul, lr);

    if (old_size != GG::Wnd::Size())
        DoLayout();
}

void ResourceInfoPanel::SetTotalPointsCost(float total_points, float total_cost,
                                           const ScriptingContext& context)
{
    AttachChild(m_total_points_label);
    AttachChild(m_total_points);
    AttachChild(m_total_points_P_label);

    AttachChild(m_wasted_points_label);
    AttachChild(m_wasted_points);
    AttachChild(m_wasted_points_P_label);

    float wasted_points = std::max(0.0f, total_points - total_cost);
    *m_total_points << DoubleToString(total_points, 3, false);
    *m_wasted_points << DoubleToString(wasted_points, 3, false);
    if (wasted_points > 0.01f)
        m_wasted_points->SetTextColor(ClientUI::StatDecrColor());
    else
        m_wasted_points->SetTextColor(ClientUI::TextColor());

    auto empire = context.GetEmpire(m_empire_id);
    const auto& empire_name{empire ? empire->Name() : EMPTY_STRING};
    SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_INFO_EMPIRE")) % m_title_str % empire_name));
}

void ResourceInfoPanel::SetStockpileCost(float stockpile, float stockpile_use,
                                         float stockpile_use_max)
{
    AttachChild(m_stockpile_points_label);
    AttachChild(m_stockpile_points);
    AttachChild(m_stockpile_points_P_label);

    AttachChild(m_stockpile_use_label);
    AttachChild(m_stockpile_use);
    AttachChild(m_stockpile_use_P_label);

    AttachChild(m_stockpile_max_use_label);
    AttachChild(m_stockpile_max_use);
    AttachChild(m_stockpile_max_use_P_label);

    TraceLogger() << "SetStockpileCost:  update values";
    *m_stockpile_points << DoubleToString(stockpile, 3, false);

    *m_stockpile_use << DoubleToString(stockpile_use, 3, false);

    *m_stockpile_max_use << DoubleToString(stockpile_use_max, 3, false);
}

void ResourceInfoPanel::SetLocalPointsCost(
    float local_points, float local_cost, float local_stockpile_use,
    float local_stockpile_use_max, const std::string& location_name,
    const ScriptingContext& context)
{
    AttachChild(m_local_points);
    AttachChild(m_local_points_P_label);

    AttachChild(m_local_wasted_points);
    AttachChild(m_local_wasted_points_P_label);

    AttachChild(m_local_stockpile_use);
    AttachChild(m_local_stockpile_use_P_label);

    float wasted_points = std::max(0.0f, local_points - local_cost);
    *m_local_points << DoubleToString(local_points, 3, false);
    *m_local_wasted_points << DoubleToString(wasted_points, 3, false);
    *m_local_stockpile_use << DoubleToString(local_stockpile_use, 3, false);
    if (wasted_points > 0.01f)
        m_local_wasted_points->SetTextColor(ClientUI::StatDecrColor());
    else
        m_local_wasted_points->SetTextColor(ClientUI::TextColor());

    auto empire = context.GetEmpire(m_empire_id);
    const auto& empire_name{empire ? empire->Name() : EMPTY_STRING};
    SetName(boost::io::str(FlexibleFormat(
        UserString("PRODUCTION_INFO_AT_LOCATION_TITLE")) % m_title_str % location_name % empire_name));

    m_local_column_label->SetText(boost::io::str(FlexibleFormat(
        UserString("PRODUCTION_QUEUE_ITEM_LOCATION")) % location_name));
}

void ResourceInfoPanel::SetEmpireID(int empire_id) {
    int old_empire_id = m_empire_id;
    m_empire_id = empire_id;
    if (old_empire_id != m_empire_id) {
        const ScriptingContext& context = IApp::GetApp()->GetContext();
        auto empire = context.GetEmpire(m_empire_id);
        const auto& empire_name{empire ? empire->Name() : EMPTY_STRING};
        // let a subsequent SetLocalPointsCost call re-set the title to include location info if necessary
        SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_INFO_EMPIRE")) % m_title_str % empire_name));
    }
}

void ResourceInfoPanel::ClearLocalInfo() {
    DetachChild(m_local_points);
    DetachChild(m_local_points_P_label);
    DetachChild(m_local_wasted_points);
    DetachChild(m_local_wasted_points_P_label);
    DetachChild(m_local_stockpile_use);
    DetachChild(m_local_stockpile_use_P_label);

    const ScriptingContext& context = IApp::GetApp()->GetContext();
    auto empire = context.GetEmpire(m_empire_id);
    const auto& empire_name{empire ? empire->Name() : EMPTY_STRING};
    SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_INFO_EMPIRE")) % m_title_str % empire_name));

    m_local_column_label->SetText("");
}

void ResourceInfoPanel::Clear() {
    DetachChild(m_total_points_label);
    DetachChild(m_total_points);
    DetachChild(m_total_points_P_label);
    DetachChild(m_stockpile_points_label);
    DetachChild(m_stockpile_points);
    DetachChild(m_stockpile_points_P_label);
    DetachChild(m_stockpile_use_label);
    DetachChild(m_stockpile_use);
    DetachChild(m_stockpile_use_P_label);
    DetachChild(m_stockpile_max_use_label);
    DetachChild(m_stockpile_max_use);
    DetachChild(m_stockpile_max_use_P_label);
    DetachChild(m_wasted_points_label);
    DetachChild(m_wasted_points);
    DetachChild(m_wasted_points_P_label);
    m_empire_id = ALL_EMPIRES;

    ClearLocalInfo();
    SetName(boost::io::str(FlexibleFormat(UserString("PRODUCTION_INFO_EMPIRE")) % m_title_str % ""));
}

void ResourceInfoPanel::DoLayout() {
    const int STAT_TEXT_PTS = ClientUI::Pts();
    static constexpr int CENTERLINE_GAP = 4;
    const GG::X LABEL_TEXT_WIDTH = (Width() - 4 - CENTERLINE_GAP) * 7 / 16 ;
    const GG::X VALUE_TEXT_WIDTH = ((Width() - 4 - CENTERLINE_GAP) - LABEL_TEXT_WIDTH) / 2;

    static constexpr GG::X LEFT_TEXT_X{0};
    const GG::X RIGHT_TEXT_X = LEFT_TEXT_X + LABEL_TEXT_WIDTH + 8 + CENTERLINE_GAP;
    const GG::X P_LABEL_X = RIGHT_TEXT_X + 40;
    const GG::X DOUBLE_LEFT_TEXT_X = P_LABEL_X + 30 + 4;
    const GG::X DOUBLE_P_LABEL_X = DOUBLE_LEFT_TEXT_X + 40;

    std::pair<int, int> m_center_gap(Value(LABEL_TEXT_WIDTH + 2), Value(LABEL_TEXT_WIDTH + 2 + CENTERLINE_GAP));

    const GG::Pt LABEL_TEXT_SIZE(LABEL_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt VALUE_TEXT_SIZE(VALUE_TEXT_WIDTH, GG::Y(STAT_TEXT_PTS + 4));
    const GG::Pt P_LABEL_SIZE(Width() - 7 - DOUBLE_P_LABEL_X, GG::Y(STAT_TEXT_PTS + 4));

    GG::Y row_offset{4};

    // first row: column labels
    m_empire_column_label->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_empire_column_label->Resize(VALUE_TEXT_SIZE);
    m_local_column_label->MoveTo(GG::Pt(DOUBLE_LEFT_TEXT_X, row_offset));
    m_local_column_label->Resize(VALUE_TEXT_SIZE);

    row_offset += m_empire_column_label->Height();


    // second row: total / local points
    // total points to left
    m_total_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
    m_total_points_label->Resize(LABEL_TEXT_SIZE);
    m_total_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_total_points->Resize(VALUE_TEXT_SIZE);
    m_total_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
    m_total_points_P_label->Resize(P_LABEL_SIZE);

    // local points to right of total points
    m_local_points->MoveTo(GG::Pt(DOUBLE_LEFT_TEXT_X, row_offset));
    m_local_points->Resize(VALUE_TEXT_SIZE);
    m_local_points_P_label->MoveTo(GG::Pt(DOUBLE_P_LABEL_X, row_offset));
    m_local_points_P_label->Resize(P_LABEL_SIZE);

    row_offset += m_total_points_label->Height();


    // third row: wasted points
    m_wasted_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
    m_wasted_points_label->Resize(LABEL_TEXT_SIZE);
    m_wasted_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_wasted_points->Resize(VALUE_TEXT_SIZE);
    m_wasted_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
    m_wasted_points_P_label->Resize(P_LABEL_SIZE);

    m_local_wasted_points->MoveTo(GG::Pt(DOUBLE_LEFT_TEXT_X, row_offset));
    m_local_wasted_points->Resize(VALUE_TEXT_SIZE);
    m_local_wasted_points_P_label->MoveTo(GG::Pt(DOUBLE_P_LABEL_X, row_offset));
    m_local_wasted_points_P_label->Resize(P_LABEL_SIZE);

    row_offset += m_wasted_points_label->Height();


    // fourth row: stockpile points contents
    m_stockpile_points_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
    m_stockpile_points_label->Resize(LABEL_TEXT_SIZE);
    m_stockpile_points->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_stockpile_points->Resize(VALUE_TEXT_SIZE);
    m_stockpile_points_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
    m_stockpile_points_P_label->Resize(P_LABEL_SIZE);

    row_offset += m_stockpile_points_label->Height();


    // fifth row: stockpile max use
    m_stockpile_max_use_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
    m_stockpile_max_use_label->Resize(LABEL_TEXT_SIZE);
    m_stockpile_max_use->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_stockpile_max_use->Resize(VALUE_TEXT_SIZE);
    m_stockpile_max_use_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
    m_stockpile_max_use_P_label->Resize(P_LABEL_SIZE);

    row_offset += m_stockpile_points_label->Height();


    // sixth row: stockpile use
    m_stockpile_use_label->MoveTo(GG::Pt(LEFT_TEXT_X, row_offset));
    m_stockpile_use_label->Resize(LABEL_TEXT_SIZE);
    m_stockpile_use->MoveTo(GG::Pt(RIGHT_TEXT_X, row_offset));
    m_stockpile_use->Resize(VALUE_TEXT_SIZE);
    m_stockpile_use_P_label->MoveTo(GG::Pt(P_LABEL_X, row_offset));
    m_stockpile_use_P_label->Resize(P_LABEL_SIZE);

    m_local_stockpile_use->MoveTo(GG::Pt(DOUBLE_LEFT_TEXT_X, row_offset));
    m_local_stockpile_use->Resize(VALUE_TEXT_SIZE);
    m_local_stockpile_use_P_label->MoveTo(GG::Pt(DOUBLE_P_LABEL_X, row_offset));
    m_local_stockpile_use_P_label->Resize(P_LABEL_SIZE);
}


//////////////////////////////////////////////////
// MultiTurnProgressBar
//////////////////////////////////////////////////
MultiTurnProgressBar::MultiTurnProgressBar(int num_segments, float percent_completed, float percent_predicted,
                                           GG::Clr bar_color, GG::Clr bg_color, GG::Clr outline_color) :
    Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_num_segments(std::min<int>(2048, std::max<int>(1, num_segments))),
    m_perc_completed(percent_completed),
    m_perc_predicted(percent_predicted),
    m_clr_bar(bar_color),
    m_clr_bg(bg_color),
    m_clr_outline(outline_color)
{
    if (m_num_segments > 1000)
        WarnLogger() << "Very many segments in MultiTurnProgressBar!";

    // validate percentage values
    if (m_perc_completed < 0.0f || m_perc_predicted < 0.0f ||
        (m_perc_completed + m_perc_predicted) > 1.0f)
    {
        WarnLogger() << "Values not within percent range, clamping: "
                     << m_perc_completed << ", " << m_perc_predicted;
        using boost::algorithm::clamp;
        m_perc_predicted = clamp(m_perc_predicted, 0.0f, 1.0f);
        // total of predicted + completed should not exceed 1.0 (100%)
        // cap completed percentage to remainder from predicted percentage
        m_perc_completed = clamp(m_perc_completed, 0.0f, 1.0f - m_perc_predicted);
    }
}

void MultiTurnProgressBar::Render() {
    const GG::Pt ul(UpperLeft());
    const GG::Pt lr(LowerRight());
    const GG::Y bottom(lr.y);
    const GG::Rect border_rect({GG::X1, GG::Y1}, {GG::X1, GG::Y1});

    // background
    GG::GL2DVertexBuffer bg_verts;
    BufferStoreRectangle(bg_verts, {ul, lr}, border_rect);

    // define completed and predicted bar sizes
    const GG::X comp_width(GG::ToX(Width() * m_perc_completed));
    const GG::X pred_width(GG::ToX(Width() * m_perc_predicted));
    // maximum lower right points to lower right of control
    const GG::Rect comp_rect(ul, {std::min(lr.x, ul.x + comp_width), bottom});
    const GG::Rect pred_rect({std::max(ul.x, comp_rect.lr.x - 1), ul.y},
                             {std::min(lr.x, comp_rect.lr.x + pred_width), bottom});

    // predicted progress
    GG::GL2DVertexBuffer pred_verts;
    if (m_perc_predicted > 0.0f)
        BufferStoreRectangle(pred_verts, pred_rect, border_rect);

    // completed progress
    GG::GL2DVertexBuffer comp_verts;
    if (m_perc_completed > 0.0f)
        BufferStoreRectangle(comp_verts, comp_rect, border_rect);

    // segment lines
    GG::GL2DVertexBuffer segment_verts;
    GG::GLRGBAColorBuffer segment_colors;
    if (m_num_segments > 1u && std::cmp_less(m_num_segments, Value(Width()))) {
        try {
            segment_verts.reserve(std::size_t{2u} * m_num_segments);
            segment_colors.reserve(std::size_t{2u} * m_num_segments);

            GG::Clr current_colour(GG::DarkenClr(m_clr_bar));

            for (int n = 1; n < static_cast<int>(m_num_segments); ++n) {
                GG::X separator_x(ul.x + Width() * n / static_cast<int>(m_num_segments));
                if (separator_x > comp_rect.lr.x)
                    current_colour = GG::LightenClr(m_clr_bg);
                segment_verts.store(separator_x, ul.y);
                segment_verts.store(separator_x, lr.y);
                segment_colors.store(current_colour);
                segment_colors.store(current_colour);
            }
        } catch (...) {}
    }

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // draw background
    if (!bg_verts.empty()) {
        bg_verts.activate();
        glColor(m_clr_outline);
        glDrawArrays(GL_QUAD_STRIP, 0, 10);
        glColor(m_clr_bg);
        glDrawArrays(GL_QUADS, 10, 4);
    }

    // draw predicated progress
    if (!pred_verts.empty()) {
        pred_verts.activate();
        glColor(m_clr_outline);
        glDrawArrays(GL_QUAD_STRIP, 0, 10);
        glColor(GG::LightenClr(m_clr_bar));
        glDrawArrays(GL_QUADS, 10, 4);
    }

    // draw completed progress
    if (!comp_verts.empty()) {
        comp_verts.activate();
        glColor(m_clr_outline);
        glDrawArrays(GL_QUAD_STRIP, 0, 10);
        glColor(m_clr_bar);
        glDrawArrays(GL_QUADS, 10, 4);
    }

    // draw segment lines
    glEnableClientState(GL_COLOR_ARRAY);
    if (!segment_verts.empty() && !segment_colors.empty()) {
        segment_verts.activate();
        segment_colors.activate();
        glDrawArrays(GL_LINES, 0, segment_verts.size());
    }

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);

}


//////////////////////////////////////////////////
// FPSIndicator
//////////////////////////////////////////////////
FPSIndicator::FPSIndicator() :
    GG::TextControl(GG::X0, GG::Y0, GG::X1, GG::Y1, "",
                    ClientUI::GetFont(), ClientUI::TextColor(), GG::FORMAT_NOWRAP, GG::ONTOP)
{
    SetResetMinSize(true);
    GetOptionsDB().OptionChangedSignal("video.fps.shown").connect(
        boost::bind(&FPSIndicator::UpdateEnabled, this));
    UpdateEnabled();
    RequirePreRender();
}

void FPSIndicator::PreRender() {
    GG::Wnd::PreRender();
    m_displayed_FPS = static_cast<int>(GG::GUI::GetGUI()->FPS());
    if (m_enabled)
        SetText(boost::io::str(FlexibleFormat(UserString("MAP_INDICATOR_FPS")) % m_displayed_FPS));
}

void FPSIndicator::Render() {
    if (m_enabled) {
        int new_FPS = static_cast<int>(GG::GUI::GetGUI()->FPS());
        if (m_displayed_FPS != new_FPS) {
            m_displayed_FPS = new_FPS;
            // Keep the ss width uniform (2) to prevent re-layout when size changes cause ChildSizeOrMinSizeOrMaxSizeChanged()
            std::stringstream ss;
            ss << std::setw(2) << std::right << std::to_string(m_displayed_FPS);
            ChangeTemplatedText(ss.str(), 0);
        }
        TextControl::Render();
    }
}

void FPSIndicator::UpdateEnabled() {
    m_enabled = GetOptionsDB().Get<bool>("video.fps.shown");
    if (!m_enabled)
        Clear();
}


//////////////////////////////////////////////////
// MultiTextureStaticGraphic
//////////////////////////////////////////////////
MultiTextureStaticGraphic::MultiTextureStaticGraphic(std::vector<std::shared_ptr<GG::Texture>> textures,
                                                     std::vector<GG::Flags<GG::GraphicStyle>> styles) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_styles(std::move(styles))
{
    for (auto& texture : textures) {
        auto w = texture->DefaultWidth();
        auto h = texture->DefaultHeight();
        m_graphics.emplace_back(std::move(texture), GG::X0, GG::Y0, w, h);
    }
    Init();
}


MultiTextureStaticGraphic::MultiTextureStaticGraphic(std::vector<GG::SubTexture> subtextures,
                                                     std::vector<GG::Flags<GG::GraphicStyle>> styles) :
    GG::Control(GG::X0, GG::Y0, GG::X1, GG::Y1, GG::NO_WND_FLAGS),
    m_graphics(std::move(subtextures)),
    m_styles(std::move(styles))
{ Init(); }

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
            pt2.x = GG::ToX(graphic_sz.x * scale);
            pt2.y = GG::ToY(graphic_sz.y * scale);
        } else {
            pt2 = window_sz;
        }
    } else if (style & GG::GRAPHIC_SHRINKFIT) {
        if (style & GG::GRAPHIC_PROPSCALE) {
            double scale_x = (graphic_sz.x > window_sz.x) ? Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x)) : 1.0;
            double scale_y = (graphic_sz.y > window_sz.y) ? Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y)) : 1.0;
            double scale = std::min(scale_x, scale_y);
            pt2.x = GG::ToX(graphic_sz.x * scale);
            pt2.y = GG::ToY(graphic_sz.y * scale);
        } else {
            pt2 = window_sz;
        }
    }

    GG::X x_shift(GG::X0);
    if (style & GG::GRAPHIC_LEFT) {
        x_shift = ul.x;
    } else if (style & GG::GRAPHIC_CENTER) {
        x_shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
    } else { // style & GG::GRAPHIC_RIGHT
        x_shift = lr.x - (pt2.x - pt1.x);
    }
    pt1.x += x_shift;
    pt2.x += x_shift;

    GG::Y y_shift(GG::Y0);
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
    const GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);
    for (std::size_t i = 0; i < m_graphics.size(); ++i) {
        const GG::Rect rendered_area = RenderedArea(m_graphics[i], m_styles[i]);
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

    for (auto& style : m_styles) {
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


////////////////////////////////////////////////
// RotatingGraphic
////////////////////////////////////////////////
RotatingGraphic::RotatingGraphic(std::shared_ptr<GG::Texture> texture,
                                 GG::Flags<GG::GraphicStyle> style,
                                 GG::Flags<GG::WndFlag> flags) :
    GG::StaticGraphic(std::move(texture), style, flags)
{}

void RotatingGraphic::Render() {
    GG::Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);

    const GG::Texture* texture = GetTexture().GetTexture();
    if (!texture)
        return;
    glBindTexture(GL_TEXTURE_2D, texture->OpenGLId());


    const int ticks = GG::GUI::GetGUI()->Ticks();     // in ms
    const float minutes = ticks / 1000.0f / 60.0f;

    // rotate around centre of rendered area
    const GG::Rect rendered_area = RenderedArea();
    const float angle = 360 * minutes * m_rpm + m_phase_offset;   // in degrees


    glPushMatrix();

    glTranslatef(static_cast<float>(Value(rendered_area.MidX())), static_cast<float>(Value(rendered_area.MidY())), 0.0f);   // tx back into position
    glRotatef(angle, 0.0f, 0.0f, 1.0f); // rotate about centre
    glTranslatef(static_cast<float>(-Value(rendered_area.MidX())), static_cast<float>(-Value(rendered_area.MidY())), 0.0f); // tx to be centred on 0, 0

    if (rendered_area != last_rendered_area) {
        last_rendered_area = rendered_area;
        // set up vertices for translated scaled quad corners
        verts.clear();
        verts.store(rendered_area.UpperLeft());                     // upper left
        verts.store(rendered_area.Right(), rendered_area.Top());    // upper right
        verts.store(rendered_area.Left(),  rendered_area.Bottom()); // lower left
        verts.store(rendered_area.LowerRight());                    // lower right
    }

    // set up texture coordinates for vertices
    const auto tc = texture->DefaultTexCoords();
    GLfloat texture_coordinate_data[8] = {tc[0], tc[1], tc[2], tc[1], tc[0], tc[3], tc[2], tc[3]};


    //// debug
    //std::cout << "rendered area: " << rendered_area << "  ul: " << UpperLeft() << "  sz: " << Size() << std::endl;
    //std::cout << "tex coords: " << tc[0] << ", " << tc[1] << ";  " << tc[2] << ", " << tc[3] << std::endl;
    //// end debug

    // render textured quad
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    verts.activate();
    glTexCoordPointer(2, GL_FLOAT, 0, texture_coordinate_data);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, verts.size());

    //// debug: triangle lines rendering
    //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    //glBindTexture(GL_TEXTURE_2D, 0);
    //glColor(GG::CLR_WHITE);
    //glDrawArrays(GL_LINE_LOOP, 0, verts.size());
    //// end debug

    glPopClientAttrib();
    glPopMatrix();
}

////////////////////////////////////////////////
// CUIPopupMenu
////////////////////////////////////////////////

CUIPopupMenu::CUIPopupMenu(GG::X x, GG::Y y) :
    GG::PopupMenu(x, y, ClientUI::GetFont(), ClientUI::TextColor(), ClientUI::WndOuterBorderColor(),
                  ClientUI::WndColor(), ClientUI::EditHiliteColor())
{}

////////////////////////////////////////////////
// ScanlineControl
////////////////////////////////////////////////

namespace {
    ScanlineRenderer scanline_shader;
}

void ScanlineControl::Render() {
    scanline_shader.SetColor(m_color);
    if (m_square)
        scanline_shader.RenderRectangle(UpperLeft(), LowerRight());
    else
        scanline_shader.RenderCircle(UpperLeft(), LowerRight());
}
