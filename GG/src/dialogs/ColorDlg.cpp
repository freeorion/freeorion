//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/dialogs/ColorDlg.h>
#include <GG/Font.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Slider.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

////////////////////////////////////////////////
// HueSaturationPicker
////////////////////////////////////////////////
HueSaturationPicker::HueSaturationPicker(X x, Y y, X w, Y h) :
    Control(x, y, w, h, INTERACTIVE)
{
    static constexpr std::size_t SAMPLES = 100u;
    static constexpr double INCREMENT = 1.0 / (SAMPLES + 1u);
    static constexpr double VALUE = 1.0;
    m_vertices.resize(SAMPLES, std::vector<std::pair<double, double>>(2 * (SAMPLES + 1)));
    m_colors.resize(SAMPLES, std::vector<Clr>(2 * (SAMPLES + 1)));
    for (std::size_t col = 0u; col < SAMPLES; ++col) {
        for (std::size_t row = 0u; row < SAMPLES + 1; ++row) {
            m_vertices[col][2*row] =     {col * INCREMENT, row * INCREMENT};
            m_vertices[col][2*row + 1] = {(col + 1) * INCREMENT, row * INCREMENT};
            m_colors[col][2*row] =       HSVClr(col * INCREMENT, 1.0 - row * INCREMENT, VALUE);
            m_colors[col][2*row + 1] =   HSVClr((col + 1) * INCREMENT, 1.0 - row * INCREMENT, VALUE);
        }
    }
}

void HueSaturationPicker::Render()
{
    const auto ul = UpperLeft(), lr = LowerRight();
    const auto size = Size();
    glDisable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // grid of quads of varying hue and saturation
    glPushMatrix();
    glTranslated(Value(ul.x), Value(ul.y), 0.0);
    glScaled(Value(size.x), Value(size.y), 1.0);
    for (std::size_t i = 0; i < m_vertices.size(); ++i) {
        glVertexPointer(2, GL_DOUBLE, 0, m_vertices[i].data());
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, m_colors[i].data());
        glDrawArrays(GL_QUAD_STRIP, 0, m_vertices[i].size());
    }
    glPopMatrix();

    glDisableClientState(GL_COLOR_ARRAY);

    // lines to indicate currently selected colour
    glLineWidth(1.5f);
    Pt color_position(ToX(ul.x + size.x * m_hue),
                      ToY(ul.y + size.y * (1.0 - m_saturation)));
    glColor(CLR_SHADOW);

    static constexpr float GAP(3.0f);

    GL2DVertexBuffer lines_verts;
    lines_verts.reserve(16);
    lines_verts.store(Value(color_position.x),      Value(ul.y));
    lines_verts.store(Value(color_position.x),      Value(color_position.y) - GAP);

    lines_verts.store(Value(color_position.x),      Value(lr.y));
    lines_verts.store(Value(color_position.x),      Value(color_position.y) + GAP);

    lines_verts.store(Value(ul.x),                  Value(color_position.y));
    lines_verts.store(Value(color_position.x) - GAP,Value(color_position.y));

    lines_verts.store(Value(lr.x),                  Value(color_position.y));
    lines_verts.store(Value(color_position.x) + GAP,Value(color_position.y));


    lines_verts.store(Value(color_position.x),      Value(color_position.y) - GAP);
    lines_verts.store(Value(color_position.x) - GAP,Value(color_position.y));

    lines_verts.store(Value(color_position.x) - GAP,Value(color_position.y));
    lines_verts.store(Value(color_position.x),      Value(color_position.y) + GAP);

    lines_verts.store(Value(color_position.x),      Value(color_position.y) + GAP);
    lines_verts.store(Value(color_position.x) + GAP,Value(color_position.y));

    lines_verts.store(Value(color_position.x) + GAP,Value(color_position.y));
    lines_verts.store(Value(color_position.x),      Value(color_position.y) - GAP);

    lines_verts.activate();

    glDrawArrays(GL_LINES, 0, lines_verts.size());
    glLineWidth(1.0f);

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void HueSaturationPicker::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{ SetHueSaturationFromPt(pt); }

void HueSaturationPicker::LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{ SetHueSaturationFromPt(pt); }

void HueSaturationPicker::SetHueSaturation(double hue, double saturation)
{
    m_hue = hue;
    m_saturation = saturation;
}

void HueSaturationPicker::SetHueSaturationFromPt(Pt pt)
{
    Pt ul = UpperLeft(), lr = LowerRight();
    if (pt.x < ul.x)
        pt.x = ul.x;
    if (pt.y < ul.y)
        pt.y = ul.y;
    if (lr.x < pt.x)
        pt.x = lr.x;
    if (lr.y < pt.y)
        pt.y = lr.y;
    const auto size = Size();
    m_hue = Value((pt.x - ul.x)) * 1.0 / Value(size.x);
    m_saturation = 1.0 - (pt.y - ul.y)*1.0/Value(size.y);
    ChangedSignal(m_hue, m_saturation);
}


////////////////////////////////////////////////
// ValuePicker
////////////////////////////////////////////////
ValuePicker::ValuePicker(X x, Y y, X w, Y h, Clr arrow_color) :
    Control(x, y, w, h, INTERACTIVE),
    m_hue(0.0),
    m_saturation(0.0),
    m_value(0.0),
    m_arrow_color(arrow_color)
{}

void ValuePicker::Render()
{
    Pt eff_ul = UpperLeft() + Pt(X(3), Y0), eff_lr = LowerRight() - Pt(X(4), Y0);
    Y h = Height();
    glDisable(GL_TEXTURE_2D);

    GL2DVertexBuffer vert_buf;
    vert_buf.reserve(12);
    GLRGBAColorBuffer colour_buf;   // need to give each vertex in lightness bar its own colour so can't just use a glColor call
    colour_buf.reserve(12);

    // bar for picking lightness
    vert_buf.store(Value(eff_lr.x),    Value(eff_ul.y));
    colour_buf.store(HSVClr(m_hue, m_saturation, 1.0));
    vert_buf.store(Value(eff_ul.x),    Value(eff_ul.y));
    colour_buf.store(HSVClr(m_hue, m_saturation, 1.0));

    vert_buf.store(Value(eff_ul.x),    Value(eff_lr.y));
    colour_buf.store(HSVClr(m_hue, m_saturation, 0.0));
    vert_buf.store(Value(eff_lr.x),    Value(eff_lr.y));
    colour_buf.store(HSVClr(m_hue, m_saturation, 0.0));

    // line indicating currently-selected lightness
    Y color_position(ToY(eff_ul.y + h * (1.0 - m_value)));
    vert_buf.store(Value(eff_ul.x),    Value(color_position));
    colour_buf.store(CLR_SHADOW);
    vert_buf.store(Value(eff_lr.x),    Value(color_position));
    colour_buf.store(CLR_SHADOW);

    // arrows marking lightness position
    vert_buf.store(Value(eff_lr.x + 5),Value(color_position - 4));
    colour_buf.store(m_arrow_color);
    vert_buf.store(Value(eff_lr.x + 1),Value(color_position));
    colour_buf.store(m_arrow_color);
    vert_buf.store(Value(eff_lr.x + 5),Value(color_position + 4));
    colour_buf.store(m_arrow_color);
    vert_buf.store(Value(eff_ul.x - 5),Value(color_position - 4));
    colour_buf.store(m_arrow_color);
    vert_buf.store(Value(eff_ul.x),    Value(color_position));
    colour_buf.store(m_arrow_color);
    vert_buf.store(Value(eff_ul.x - 5),Value(color_position + 4));
    colour_buf.store(m_arrow_color);


    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();
    colour_buf.activate();
    glDrawArrays(GL_QUADS, 0, 4);
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 4, 2);
    glLineWidth(1.0f);
    glDrawArrays(GL_TRIANGLES, 6, 6);

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void ValuePicker::LButtonDown(Pt pt, Flags<ModKey> mod_keys)
{ SetValueFromPt(pt); }

void ValuePicker::LDrag(Pt pt, Pt move, Flags<ModKey> mod_keys)
{ SetValueFromPt(pt); }

void ValuePicker::SetHueSaturation(double hue, double saturation)
{
    m_hue = hue;
    m_saturation = saturation;
}

void ValuePicker::SetValue(double value)
{ m_value = value; }

void ValuePicker::SetValueFromPt(Pt pt)
{
    Pt ul = UpperLeft(), lr = LowerRight();
    if (pt.x < ul.x)
        pt.x = ul.x;
    if (pt.y < ul.y)
        pt.y = ul.y;
    if (lr.x < pt.x)
        pt.x = lr.x;
    if (lr.y < pt.y)
        pt.y = lr.y;
    Y h = Height();
    m_value = 1.0 - (pt.y - ul.y) * 1.0 / Value(h);
    ChangedSignal(m_value);
}


////////////////////////////////////////////////
// ColorDlg
////////////////////////////////////////////////

ColorDlg::ColorButton::ColorButton(Clr color) :
    Button("", nullptr, color),
    m_represented_color(CLR_BLACK)
{}

void ColorDlg::ColorButton::RenderUnpressed()
{
    Button::RenderUnpressed();
    Pt ul = UpperLeft() + Pt(X(3), Y(3)), lr = LowerRight() - Pt(X(3), Y(3));
    FlatRectangle(ul, lr, m_represented_color, CLR_ZERO, 0);
}

void ColorDlg::ColorButton::RenderPressed()
{
    Button::RenderPressed();
    Pt ul = UpperLeft() + Pt(X(4), Y(4)), lr = LowerRight() - Pt(X(2), Y(2));
    FlatRectangle(ul, lr, m_represented_color, CLR_ZERO, 0);
}

void ColorDlg::ColorButton::RenderRollover()
{ RenderUnpressed(); }


// ColorDlg::ColorDisplay
ColorDlg::ColorDisplay::ColorDisplay(Clr color) :
    Control(X0, Y0, X1, Y1, NO_WND_FLAGS)
{ SetColor(color); }

void ColorDlg::ColorDisplay::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    static constexpr int SQUARE_SIZE = 7;

    GL2DVertexBuffer    vert_buf;
    GLRGBAColorBuffer   colour_buf;

    // background checkerboard for curent colour display (to see through transparent areas)
    int i = 0, j = 0;
    for (Y y = lr.y; y > ul.y; y -= SQUARE_SIZE, ++j) {
        Y y0 = y - SQUARE_SIZE;
        if (y0 < ul.y)
            y0 = ul.y;
        i = 0;
        for (X x = lr.x; x > ul.x; x -= SQUARE_SIZE, ++i) {
            X x0 = x - SQUARE_SIZE;
            if (x0 < ul.x)
                x0 = ul.x;
            Clr vert_clr = ((i + j) % 2) ? CLR_WHITE : CLR_BLACK;
            colour_buf.store(vert_clr);
            vert_buf.store(Value(x), Value(y0));
            colour_buf.store(vert_clr);
            vert_buf.store(Value(x0), Value(y0));
            colour_buf.store(vert_clr);
            vert_buf.store(Value(x0), Value(y));
            colour_buf.store(vert_clr);
            vert_buf.store(Value(x), Value(y));
        }
    }


    Clr full_alpha_color = Color();
    full_alpha_color.a = 255;


    GLfloat verts[12] = {};
    // upper left: full alpha colour
    verts[0] = Value(lr.x); verts[1] = Value(ul.y);
    verts[2] = Value(ul.x); verts[3] = Value(ul.y);
    verts[4] = Value(ul.x); verts[5] = Value(lr.y);
    // bottom right: actual alpha colour
    verts[6] = Value(ul.x); verts[7] = Value(lr.y);
    verts[8] = Value(lr.x); verts[9] = Value(lr.y);
    verts[10]= Value(lr.x); verts[11]= Value(ul.y);


    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();
    colour_buf.activate();
    glDrawArrays(GL_QUADS, 0, vert_buf.size());

    glDisableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, verts);
    glColor(full_alpha_color);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glColor(Color());
    glDrawArrays(GL_TRIANGLES, 3, 3);

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}


namespace {
    enum : uint8_t {R, G, B, A, H, S, V};

    constexpr std::size_t COLOR_BUTTON_ROWS = 4;
    constexpr std::size_t COLOR_BUTTON_COLS = 5;

    std::vector<Clr> custom_colors = {
        GG::CLR_WHITE,      GG::CLR_LIGHT_GRAY, GG::CLR_GRAY,       GG::CLR_DARK_GRAY,  GG::CLR_BLACK,
        GG::CLR_PINK,       GG::CLR_RED,        GG::CLR_DARK_RED,   GG::CLR_MAGENTA,    GG::CLR_PURPLE,
        GG::CLR_BLUE,       GG::CLR_DARK_BLUE,  GG::CLR_TEAL,       GG::CLR_CYAN,       GG::CLR_GREEN,
        GG::CLR_DARK_GREEN, GG::CLR_OLIVE,      GG::CLR_YELLOW,     GG::CLR_ORANGE,     GG::CLR_GRAY};
}

ColorDlg::ColorDlg(X x, Y y, Clr original_color, const std::shared_ptr<Font>& font,
                   Clr dialog_color, Clr border_color, Clr text_color) :
    Wnd(x, y, X(315), Y(300), INTERACTIVE | DRAGABLE | MODAL),
    m_original_color(original_color),
    m_current_color_button(INVALID_COLOR_BUTTON),
    m_color(dialog_color),
    m_border_color(border_color),
    m_text_color(text_color)
{
    m_current_color = m_original_color_specified ? m_original_color : CLR_BLACK;
    Clr color = m_current_color;

    const auto& style = GetStyleFactory();

    m_hue_saturation_picker = Wnd::Create<HueSaturationPicker>(X(10), Y(10), X(300), Y(300));
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker = Wnd::Create<ValuePicker>(X(320), Y(10), X(25), Y(300), m_text_color);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    static constexpr int HUE_SATURATION_PICKER_SIZE = 200;
    m_pickers_layout = Wnd::Create<Layout>(X0, Y0, X(HUE_SATURATION_PICKER_SIZE + 30),
                                           Y(HUE_SATURATION_PICKER_SIZE),
                                           1, 2, 0, 5);
    m_pickers_layout->SetColumnStretch(0, 1);
    m_pickers_layout->SetMinimumColumnWidth(1, X(24));
    m_pickers_layout->Add(m_hue_saturation_picker, 0, 0);
    m_pickers_layout->Add(m_value_picker, 0, 1);

    m_color_squares_layout = Wnd::Create<Layout>(X0, m_pickers_layout->Bottom() + 5,
                                                 m_pickers_layout->Width(), Y(40),
                                                 1, 1, 0, 4);
    m_new_color_square = Wnd::Create<ColorDisplay>(color);
    if (m_original_color_specified) {
        m_new_color_square_text = style.NewTextControl(style.Translate("New"), font, m_text_color, FORMAT_RIGHT);
        m_color_squares_layout->Add(m_new_color_square_text, 0, 0);
        m_color_squares_layout->Add(m_new_color_square, 0, 1);
        m_old_color_square_text = style.NewTextControl(style.Translate("Old"), font, m_text_color, FORMAT_RIGHT);
        m_color_squares_layout->Add(m_old_color_square_text, 1, 0);
        m_old_color_square = Wnd::Create<ColorDisplay>(m_original_color);
        m_color_squares_layout->Add(m_old_color_square, 1, 1);
        m_color_squares_layout->SetMinimumColumnWidth(0, X(30));
        m_color_squares_layout->SetColumnStretch(1, 1);
    } else {
        m_color_squares_layout->Add(m_new_color_square, 0, 0);
    }

    m_color_buttons_layout = Wnd::Create<Layout>(X0, m_color_squares_layout->Bottom() + 5,
                                                 m_pickers_layout->Width(), Y(80),
                                                 COLOR_BUTTON_ROWS, COLOR_BUTTON_COLS, 0, 4);
    for (std::size_t i = 0u; i < COLOR_BUTTON_ROWS; ++i) {
        for (std::size_t j = 0u; j < COLOR_BUTTON_COLS; ++j) {
            m_color_buttons.push_back(Wnd::Create<ColorButton>(m_color));
            m_color_buttons.back()->SetRepresentedColor(custom_colors[i * COLOR_BUTTON_COLS + j]);
            m_color_buttons_layout->Add(m_color_buttons.back(), i, j);
        }
    }

    m_sliders_ok_cancel_layout = Wnd::Create<Layout>(m_pickers_layout->Right() + 5, Y0,
                                                     X(150), Y((25 + 5) * 8 - 5),
                                                     9, 3, 0, 5);
    m_sliders_ok_cancel_layout->SetMinimumColumnWidth(0, X(15));
    m_sliders_ok_cancel_layout->SetMinimumColumnWidth(1, X(30));
    m_sliders_ok_cancel_layout->SetColumnStretch(2, 1);

    int row = 0;

    for (auto& [color_value, color_min, color_max, color_label]  : {
            std::make_tuple(static_cast<int>(color.r), 0, 255, "R:"),
            std::make_tuple(static_cast<int>(color.g), 0, 255, "G:"),
            std::make_tuple(static_cast<int>(color.b), 0, 255, "B:"),
            std::make_tuple(static_cast<int>(color.a), 0, 255, "A:"),
            std::make_tuple(static_cast<int>(m_current_color.h * 359), 0, 359, "H:"),
            std::make_tuple(static_cast<int>(m_current_color.s * 255), 0, 255, "S:"),
            std::make_tuple(static_cast<int>(m_current_color.v * 255), 0, 255, "V:")
        })
    {
        m_slider_labels.push_back(style.NewTextControl(style.Translate(color_label), font, m_text_color, FORMAT_RIGHT));
        m_sliders_ok_cancel_layout->Add(m_slider_labels.back(), row, 0);
        m_slider_values.push_back(style.NewTextControl(std::to_string(color_value), font, m_text_color, FORMAT_LEFT));
        m_sliders_ok_cancel_layout->Add(m_slider_values.back(), row, 1);
        m_sliders.push_back(style.NewIntSlider(color_min, color_max, Orientation::HORIZONTAL, m_color, 10));
        m_sliders.back()->SlideTo(color_value);
        m_sliders_ok_cancel_layout->Add(m_sliders.back(), row, 2);

        ++row;
    }

    m_ok = style.NewButton(style.Translate("Ok"), font, m_color, m_text_color);
    m_sliders_ok_cancel_layout->Add(m_ok, 7, 0, 1, 3);
    m_cancel = style.NewButton(style.Translate("Cancel"), font, m_color, m_text_color);
    m_sliders_ok_cancel_layout->Add(m_cancel, 8, 0, 1, 3);
}

void ColorDlg::CompleteConstruction()
{
    Wnd::CompleteConstruction();

    auto master_layout = Wnd::Create<Layout>(X0, Y0, ClientWidth(), ClientHeight(), 3, 2, 5, 5);
    master_layout->SetColumnStretch(0, 1.25);
    master_layout->SetColumnStretch(1, 1);
    master_layout->SetRowStretch(0, 1.25);
    master_layout->SetMinimumRowHeight(1, Y(40));
    master_layout->SetRowStretch(2, 1);
    master_layout->Add(m_pickers_layout, 0, 0);
    master_layout->Add(m_color_squares_layout, 1, 0);
    master_layout->Add(m_color_buttons_layout, 2, 0);
    master_layout->Add(m_sliders_ok_cancel_layout, 0, 1, 3, 1);
    SetLayout(master_layout);

    for (std::size_t i = 0; i < m_color_buttons.size(); ++i) {
        m_color_buttons[i]->LeftClickedSignal.connect(
            [this, i](){ this->ColorButtonClicked(i); });
    }

    m_sliders[R]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { RedSliderChanged(pos, ub, lb); });
    m_sliders[G]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { GreenSliderChanged(pos, ub, lb); });
    m_sliders[B]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { BlueSliderChanged(pos, ub, lb); });
    m_sliders[A]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { AlphaSliderChanged(pos, ub, lb); });
    m_sliders[H]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { HueSliderChanged(pos, ub, lb); });
    m_sliders[S]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { SaturationSliderChanged(pos, ub, lb); });
    m_sliders[V]->SlidSignal.connect([this](auto pos, auto ub, auto lb) { ValueSliderChanged(pos, ub, lb); });
    m_ok->LeftClickedSignal.connect([this]() { OkClicked(); });
    m_cancel->LeftClickedSignal.connect([this]() { CancelClicked(); });
    m_hue_saturation_picker->ChangedSignal.connect([this](auto hue, auto sat) {
        m_value_picker->SetHueSaturation(hue, sat);
        HueSaturationPickerChanged(hue, sat);
    });
    m_value_picker->ChangedSignal.connect([this](auto value) { ValuePickerChanged(value); });
}

bool ColorDlg::ColorWasSelected() const
{ return m_color_was_picked; }

Clr ColorDlg::Result() const
{ return m_current_color; }

void ColorDlg::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    FlatRectangle(ul, lr, m_color, m_border_color, 1);
    if (m_current_color_button != INVALID_COLOR_BUTTON) {
        Pt button_ul = m_color_buttons[m_current_color_button]->UpperLeft() - Pt(X(2), Y(2));
        Pt button_lr = m_color_buttons[m_current_color_button]->LowerRight() + Pt(X(2), Y(2));
        FlatRectangle(button_ul, button_lr, CLR_ZERO, m_text_color, 2);
    }
}

void ColorDlg::KeyPress(Key key, uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (key == Key::GGK_RETURN || key == Key::GGK_KP_ENTER)
        OkClicked();
    else if (key == Key::GGK_ESCAPE)
        CancelClicked();
}

void ColorDlg::ColorChanged(HSVClr color)
{
    m_current_color = color;
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    Clr rgb_color = m_current_color;
    m_new_color_square->SetColor(rgb_color);
    if (m_current_color_button != INVALID_COLOR_BUTTON) {
        m_color_buttons[m_current_color_button]->SetRepresentedColor(rgb_color);
        custom_colors[m_current_color_button] = rgb_color;
    }
    UpdateRGBSliders();
    UpdateHSVSliders();
}

void ColorDlg::HueSaturationPickerChanged(double hue, double saturation)
{
    m_current_color.h = hue;
    m_current_color.s = saturation;
    ColorChanged(m_current_color);
}

void ColorDlg::ValuePickerChanged(double value)
{
    m_current_color.v = value;
    ColorChanged(m_current_color);
}

void ColorDlg::UpdateRGBSliders()
{
    Clr color = m_current_color;
    *m_slider_values[R] << static_cast<int>(color.r);
    *m_slider_values[G] << static_cast<int>(color.g);
    *m_slider_values[B] << static_cast<int>(color.b);
    *m_slider_values[A] << static_cast<int>(color.a);
    m_sliders[R]->SlideTo(color.r);
    m_sliders[G]->SlideTo(color.g);
    m_sliders[B]->SlideTo(color.b);
    m_sliders[A]->SlideTo(color.a);
}

void ColorDlg::UpdateHSVSliders()
{
    *m_slider_values[H] << static_cast<int>(m_current_color.h * 359);
    *m_slider_values[S] << static_cast<int>(m_current_color.s * 255);
    *m_slider_values[V] << static_cast<int>(m_current_color.v * 255);
    m_sliders[H]->SlideTo(static_cast<int>(m_current_color.h * 359));
    m_sliders[S]->SlideTo(static_cast<int>(m_current_color.s * 255));
    m_sliders[V]->SlideTo(static_cast<int>(m_current_color.v * 255));
}

void ColorDlg::ColorChangeFromRGBSlider()
{
    Clr color = m_current_color;
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    m_new_color_square->SetColor(color);
    if (m_current_color_button != INVALID_COLOR_BUTTON) {
        m_color_buttons[m_current_color_button]->SetRepresentedColor(color);
        custom_colors[m_current_color_button] = color;
    }
    UpdateHSVSliders();
}

void ColorDlg::ColorButtonClicked(std::size_t i)
{
    m_current_color_button = i;
    m_current_color = m_color_buttons[m_current_color_button]->RepresentedColor();
    ColorChanged(m_current_color);
}

void ColorDlg::RedSliderChanged(int value, int low, int high)
{
    Clr color = m_current_color;
    color.r = value;
    m_current_color = color;
    ColorChangeFromRGBSlider();
    *m_slider_values[R] << value;
}

void ColorDlg::GreenSliderChanged(int value, int low, int high)
{
    Clr color = m_current_color;
    color.g = value;
    m_current_color = color;
    ColorChangeFromRGBSlider();
    *m_slider_values[G] << value;
}

void ColorDlg::BlueSliderChanged(int value, int low, int high)
{
    Clr color = m_current_color;
    color.b = value;
    m_current_color = color;
    ColorChangeFromRGBSlider();
    *m_slider_values[B] << value;
}

void ColorDlg::AlphaSliderChanged(int value, int low, int high)
{
    Clr color = m_current_color;
    color.a = value;
    m_current_color = color;
    ColorChangeFromRGBSlider();
    *m_slider_values[A] << value;
}

void ColorDlg::HueSliderChanged(int value, int low, int high)
{
    m_current_color.h = static_cast<double>(value) / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::SaturationSliderChanged(int value, int low, int high)
{
    m_current_color.s = static_cast<double>(value) / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::ValueSliderChanged(int value, int low, int high)
{
    m_current_color.v = static_cast<double>(value) / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::OkClicked()
{
    m_color_was_picked = true;
    m_modal_done.store(true);
}

void ColorDlg::CancelClicked()
{
    m_current_color = m_original_color;
    m_modal_done.store(true);
}
