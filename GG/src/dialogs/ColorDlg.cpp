/* GG is a GUI for SDL and OpenGL.
   Copyright (C) 2003-2008 T. Zachary Laine

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2.1
   of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
    
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA

   If you do not wish to comply with the terms of the LGPL please
   contact the author as other terms are available for a fee.
    
   Zach Laine
   whatwasthataddress@gmail.com */

#include <GG/dialogs/ColorDlg.h>

#include <GG/DrawUtil.h>
#include <GG/Font.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/GUI.h>
#include <GG/Layout.h>
#include <GG/Slider.h>
#include <GG/StyleFactory.h>
#include <GG/WndEvent.h>


using namespace GG;

namespace {
    const double EPSILON = 0.0001;

    HSVClr Convert(const Clr& color)
    {
        HSVClr retval;
        retval.a = color.a;
        double r = (color.r / 255.0), g = (color.g / 255.0), b = (color.b / 255.0);

        double min_channel = std::min(r, std::min(g, b));
        double max_channel = std::max(r, std::max(g, b));
        double channel_range = max_channel - min_channel;

        retval.v = max_channel;

        if (max_channel < EPSILON) {
            retval.h = 0.0;
            retval.s = 0.0;
        } else {
            retval.s = channel_range / max_channel;

            if (channel_range) {
                double delta_r = (((max_channel - r) / 6.0) + (channel_range / 2.0)) / channel_range;
                double delta_g = (((max_channel - g) / 6.0) + (channel_range / 2.0)) / channel_range;
                double delta_b = (((max_channel - b) / 6.0) + (channel_range / 2.0)) / channel_range;

                if (r == max_channel)
                    retval.h = delta_b - delta_g;
                else if (g == max_channel)
                    retval.h = (1.0 / 3.0) + delta_r - delta_b;
                else if (b == max_channel)
                    retval.h = (2.0 / 3.0) + delta_g - delta_r;

                if (retval.h < 0.0)
                    retval.h += 1.0;
                if (1.0 < retval.h)
                    retval.h -= 1.0;
            } else {
                retval.h = 0.0;
            }
        }
        return retval;
    }

    Clr Convert(const HSVClr& hsv_color)
    {
        Clr retval;
        retval.a = hsv_color.a;

        if (hsv_color.s < EPSILON) {
            retval.r = static_cast<GLubyte>(hsv_color.v * 255);
            retval.g = static_cast<GLubyte>(hsv_color.v * 255);
            retval.b = static_cast<GLubyte>(hsv_color.v * 255);
        } else {
            double tmph = hsv_color.h * 6.0;
            int tmpi = static_cast<int>(tmph);
            double tmp1 = hsv_color.v * (1 - hsv_color.s);
            double tmp2 = hsv_color.v * (1 - hsv_color.s * (tmph - tmpi));
            double tmp3 = hsv_color.v * (1 - hsv_color.s * (1 - (tmph - tmpi)));
            switch (tmpi) {
            case 0:
                retval.r = static_cast<GLubyte>(hsv_color.v * 255);
                retval.g = static_cast<GLubyte>(tmp3 * 255);
                retval.b = static_cast<GLubyte>(tmp1 * 255);
                break;
            case 1:
                retval.r = static_cast<GLubyte>(tmp2 * 255);
                retval.g = static_cast<GLubyte>(hsv_color.v * 255);
                retval.b = static_cast<GLubyte>(tmp1 * 255);
                break;
            case 2:
                retval.r = static_cast<GLubyte>(tmp1 * 255);
                retval.g = static_cast<GLubyte>(hsv_color.v * 255);
                retval.b = static_cast<GLubyte>(tmp3 * 255);
                break;
            case 3:
                retval.r = static_cast<GLubyte>(tmp1 * 255);
                retval.g = static_cast<GLubyte>(tmp2 * 255);
                retval.b = static_cast<GLubyte>(hsv_color.v * 255);
                break;
            case 4:
                retval.r = static_cast<GLubyte>(tmp3 * 255);
                retval.g = static_cast<GLubyte>(tmp1 * 255);
                retval.b = static_cast<GLubyte>(hsv_color.v * 255);
                break;
            default:
                retval.r = static_cast<GLubyte>(hsv_color.v * 255);
                retval.g = static_cast<GLubyte>(tmp1 * 255);
                retval.b = static_cast<GLubyte>(tmp2 * 255);
                break;
            }
        }

        return retval;
    }
}


////////////////////////////////////////////////
// HSVClr
////////////////////////////////////////////////
HSVClr::HSVClr() :
    h(0.0),
    s(0.0),
    v(0.0),
    a(0)
{}

HSVClr::HSVClr(double h_, double s_, double v_, GLubyte a_/* = 255*/) :
    h(h_),
    s(s_),
    v(v_),
    a(a_)
{}

////////////////////////////////////////////////
// HueSaturationPicker
////////////////////////////////////////////////
HueSaturationPicker::HueSaturationPicker(X x, Y y, X w, Y h) :
    Control(x, y, w, h, INTERACTIVE),
    m_hue(0.0),
    m_saturation(0.0)
{
    const int SAMPLES = 100;
    const double INCREMENT = 1.0 / (SAMPLES + 1);
    const double VALUE = 1.0;
    m_vertices.resize(SAMPLES, std::vector<std::pair<double, double>>(2 * (SAMPLES + 1)));
    m_colors.resize(SAMPLES, std::vector<Clr>(2 * (SAMPLES + 1)));
    for (int col = 0; col < SAMPLES; ++col) {
        for (int row = 0; row < SAMPLES + 1; ++row) {
            m_vertices[col][2 * row] =      {col * INCREMENT, row * INCREMENT};
            m_vertices[col][2 * row + 1] =  {(col + 1) * INCREMENT, row * INCREMENT};
            m_colors[col][2 * row] =        Convert(HSVClr(col * INCREMENT, 1.0 - row * INCREMENT, VALUE));
            m_colors[col][2 * row + 1] =    Convert(HSVClr((col + 1) * INCREMENT, 1.0 - row * INCREMENT, VALUE));
        }
    }
}

void HueSaturationPicker::Render()
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt size = Size();
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
        glVertexPointer(2, GL_DOUBLE, 0, &m_vertices[i][0]);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, &m_colors[i][0]);
        glDrawArrays(GL_QUAD_STRIP, 0, m_vertices[i].size());
    }
    glPopMatrix();

    glDisableClientState(GL_COLOR_ARRAY);

    // lines to indicate currently selected colour
    glLineWidth(1.5f);
    Pt color_position(static_cast<X>(ul.x + size.x * m_hue),
                      static_cast<Y>(ul.y + size.y * (1.0 - m_saturation)));
    glColor(CLR_SHADOW);

    const float GAP(3.0f);

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

void HueSaturationPicker::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ SetHueSaturationFromPt(pt); }

void HueSaturationPicker::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
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
    Pt size = Size();
    m_hue = Value((pt.x - ul.x) * 1.0 / size.x);
    m_saturation = Value(1.0 - (pt.y - ul.y) * 1.0 / size.y);
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
    colour_buf.store(Convert(HSVClr(m_hue, m_saturation, 1.0)));
    vert_buf.store(Value(eff_ul.x),    Value(eff_ul.y));
    colour_buf.store(Convert(HSVClr(m_hue, m_saturation, 1.0)));

    vert_buf.store(Value(eff_ul.x),    Value(eff_lr.y));
    colour_buf.store(Convert(HSVClr(m_hue, m_saturation, 0.0)));
    vert_buf.store(Value(eff_lr.x),    Value(eff_lr.y));
    colour_buf.store(Convert(HSVClr(m_hue, m_saturation, 0.0)));

    // line indicating currently-selected lightness
    Y color_position(eff_ul.y + h * (1.0 - m_value));
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

void ValuePicker::LButtonDown(const Pt& pt, Flags<ModKey> mod_keys)
{ SetValueFromPt(pt); }

void ValuePicker::LDrag(const Pt& pt, const Pt& move, Flags<ModKey> mod_keys)
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
    m_value = Value(1.0 - (pt.y - ul.y) * 1.0 / h);
    ChangedSignal(m_value);
}


////////////////////////////////////////////////
// ColorDlg
////////////////////////////////////////////////

ColorDlg::ColorButton::ColorButton(const Clr& color) :
    Button("", nullptr, color),
    m_represented_color(CLR_BLACK)
{}

Clr ColorDlg::ColorButton::RepresentedColor() const
{ return m_represented_color; }

void ColorDlg::ColorButton::SetRepresentedColor(const Clr& color)
{ m_represented_color = color; }

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
    const int SQUARE_SIZE = 7;

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


    GLfloat verts[12];
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


// ColorDlg

// static(s)
std::vector<Clr> ColorDlg::s_custom_colors;
const std::size_t ColorDlg::INVALID_COLOR_BUTTON = std::numeric_limits<std::size_t>::max();

ColorDlg::ColorDlg(X x, Y y, Clr original_color, const std::shared_ptr<Font>& font,
                   Clr dialog_color, Clr border_color, Clr text_color/* = CLR_BLACK*/) :
    Wnd(x, y, X(315), Y(300), INTERACTIVE | DRAGABLE | MODAL),
    m_original_color(original_color),
    m_current_color_button(INVALID_COLOR_BUTTON),
    m_color(dialog_color),
    m_border_color(border_color),
    m_text_color(text_color)
{
    m_current_color = m_original_color_specified ? Convert(m_original_color) : Convert(CLR_BLACK);
    Clr color = Convert(m_current_color);

    const auto& style = GetStyleFactory();

    const int COLOR_BUTTON_ROWS = 4;
    const int COLOR_BUTTON_COLS = 5;
    if (s_custom_colors.empty()) {
        s_custom_colors = { GG::CLR_WHITE,      GG::CLR_LIGHT_GRAY, GG::CLR_GRAY,       GG::CLR_DARK_GRAY,  GG::CLR_BLACK,
                            GG::CLR_PINK,       GG::CLR_RED,        GG::CLR_DARK_RED,   GG::CLR_MAGENTA,    GG::CLR_PURPLE,
                            GG::CLR_BLUE,       GG::CLR_DARK_BLUE,  GG::CLR_TEAL,       GG::CLR_CYAN,       GG::CLR_GREEN,
                            GG::CLR_DARK_GREEN, GG::CLR_OLIVE,      GG::CLR_YELLOW,     GG::CLR_ORANGE};

        for (unsigned int i = s_custom_colors.size(); i < COLOR_BUTTON_ROWS * COLOR_BUTTON_COLS; ++i) {
            s_custom_colors.push_back(CLR_GRAY);
        }
    }

    m_hue_saturation_picker = Wnd::Create<HueSaturationPicker>(X(10), Y(10), X(300), Y(300));
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker = Wnd::Create<ValuePicker>(X(320), Y(10), X(25), Y(300), m_text_color);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    const int HUE_SATURATION_PICKER_SIZE = 200;
    m_pickers_layout = Wnd::Create<Layout>(X0, Y0, X(HUE_SATURATION_PICKER_SIZE + 30), Y(HUE_SATURATION_PICKER_SIZE),
                                           1, 2, 0, 5);
    m_pickers_layout->SetColumnStretch(0, 1);
    m_pickers_layout->SetMinimumColumnWidth(1, X(24));
    m_pickers_layout->Add(m_hue_saturation_picker, 0, 0);
    m_pickers_layout->Add(m_value_picker, 0, 1);

    m_color_squares_layout = Wnd::Create<Layout>(X0, m_pickers_layout->Bottom() + 5, m_pickers_layout->Width(), Y(40),
                                                 1, 1, 0, 4);
    m_new_color_square = Wnd::Create<ColorDisplay>(color);
    if (m_original_color_specified) {
        m_new_color_square_text = style->NewTextControl(style->Translate("New"), font, m_text_color, FORMAT_RIGHT);
        m_color_squares_layout->Add(m_new_color_square_text, 0, 0);
        m_color_squares_layout->Add(m_new_color_square, 0, 1);
        m_old_color_square_text = style->NewTextControl(style->Translate("Old"), font, m_text_color, FORMAT_RIGHT);
        m_color_squares_layout->Add(m_old_color_square_text, 1, 0);
        m_old_color_square = Wnd::Create<ColorDisplay>(m_original_color);
        m_color_squares_layout->Add(m_old_color_square, 1, 1);
        m_color_squares_layout->SetMinimumColumnWidth(0, X(30));
        m_color_squares_layout->SetColumnStretch(1, 1);
    } else {
        m_color_squares_layout->Add(m_new_color_square, 0, 0);
    }

    m_color_buttons_layout = Wnd::Create<Layout>(X0, m_color_squares_layout->Bottom() + 5, m_pickers_layout->Width(), Y(80),
                                                 COLOR_BUTTON_ROWS, COLOR_BUTTON_COLS, 0, 4);
    for (int i = 0; i < COLOR_BUTTON_ROWS; ++i) {
        for (int j = 0; j < COLOR_BUTTON_COLS; ++j) {
            m_color_buttons.push_back(Wnd::Create<ColorButton>(m_color));
            m_color_buttons.back()->SetRepresentedColor(s_custom_colors[i * COLOR_BUTTON_COLS + j]);
            m_color_buttons_layout->Add(m_color_buttons.back(), i, j);
        }
    }

    m_sliders_ok_cancel_layout = Wnd::Create<Layout>(m_pickers_layout->Right() + 5, Y0, X(150), Y((25 + 5) * 8 - 5),
                                                     9, 3, 0, 5);
    m_sliders_ok_cancel_layout->SetMinimumColumnWidth(0, X(15));
    m_sliders_ok_cancel_layout->SetMinimumColumnWidth(1, X(30));
    m_sliders_ok_cancel_layout->SetColumnStretch(2, 1);

    int row = 0;

    for (auto entry : {
            std::make_tuple(static_cast<int>(color.r), 0, 255, "R:"),
            std::make_tuple(static_cast<int>(color.g), 0, 255, "G:"),
            std::make_tuple(static_cast<int>(color.b), 0, 255, "B:"),
            std::make_tuple(static_cast<int>(color.a), 0, 255, "A:"),
            std::make_tuple(static_cast<int>(m_current_color.h * 359), 0, 359, "H:"),
            std::make_tuple(static_cast<int>(m_current_color.s * 255), 0, 255, "S:"),
            std::make_tuple(static_cast<int>(m_current_color.v * 255), 0, 255, "V:")
        })
    {
        auto color_value = std::get<0>(entry);
        auto color_min   = std::get<1>(entry);
        auto color_max   = std::get<2>(entry);
        auto color_label = std::get<3>(entry);

        m_slider_labels.push_back(style->NewTextControl(style->Translate(color_label), font, m_text_color, FORMAT_RIGHT));
        m_sliders_ok_cancel_layout->Add(m_slider_labels.back(), row, 0);
        m_slider_values.push_back(style->NewTextControl(std::to_string(color_value),
                                                        font, m_text_color, FORMAT_LEFT));
        m_sliders_ok_cancel_layout->Add(m_slider_values.back(), row, 1);
        m_sliders.push_back(style->NewIntSlider(color_min, color_max, HORIZONTAL, m_color, 10));
        m_sliders.back()->SlideTo(color_value);
        m_sliders_ok_cancel_layout->Add(m_sliders.back(), row, 2);

        ++row;
    }

    m_ok = style->NewButton(style->Translate("Ok"), font, m_color, m_text_color);
    m_sliders_ok_cancel_layout->Add(m_ok, 7, 0, 1, 3);
    m_cancel = style->NewButton(style->Translate("Cancel"), font, m_color, m_text_color);
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
    m_sliders[R]->SlidSignal.connect(
        boost::bind(&ColorDlg::RedSliderChanged, this, _1, _2, _3));
    m_sliders[G]->SlidSignal.connect(
        boost::bind(&ColorDlg::GreenSliderChanged, this, _1, _2, _3));
    m_sliders[B]->SlidSignal.connect(
        boost::bind(&ColorDlg::BlueSliderChanged, this, _1, _2, _3));
    m_sliders[A]->SlidSignal.connect(
        boost::bind(&ColorDlg::AlphaSliderChanged, this, _1, _2, _3));
    m_sliders[H]->SlidSignal.connect(
        boost::bind(&ColorDlg::HueSliderChanged, this, _1, _2, _3));
    m_sliders[S]->SlidSignal.connect(
        boost::bind(&ColorDlg::SaturationSliderChanged, this, _1, _2, _3));
    m_sliders[V]->SlidSignal.connect(
        boost::bind(&ColorDlg::ValueSliderChanged, this, _1, _2, _3));
    m_ok->LeftClickedSignal.connect(
        boost::bind(&ColorDlg::OkClicked, this));
    m_cancel->LeftClickedSignal.connect(
        boost::bind(&ColorDlg::CancelClicked, this));
    m_hue_saturation_picker->ChangedSignal.connect(
        boost::bind(&ValuePicker::SetHueSaturation, m_value_picker, _1, _2));
    m_hue_saturation_picker->ChangedSignal.connect(
        boost::bind(&ColorDlg::HueSaturationPickerChanged, this, _1, _2));
    m_value_picker->ChangedSignal.connect(
        boost::bind(&ColorDlg::ValuePickerChanged, this, _1));
}

bool ColorDlg::ColorWasSelected() const
{ return m_color_was_picked; }

Clr ColorDlg::Result() const
{ return Convert(m_current_color); }

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

void ColorDlg::KeyPress(Key key, std::uint32_t key_code_point, Flags<ModKey> mod_keys)
{
    if (key == GGK_RETURN || key == GGK_KP_ENTER)
        OkClicked();
    else if (key == GGK_ESCAPE)
        CancelClicked();
}

void ColorDlg::ColorChanged(HSVClr color)
{
    m_current_color = color;
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    Clr rgb_color = Convert(m_current_color);
    m_new_color_square->SetColor(rgb_color);
    if (m_current_color_button != INVALID_COLOR_BUTTON) {
        m_color_buttons[m_current_color_button]->SetRepresentedColor(rgb_color);
        s_custom_colors[m_current_color_button] = rgb_color;
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
    Clr color = Convert(m_current_color);
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
    Clr color = Convert(m_current_color);
    m_hue_saturation_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetHueSaturation(m_current_color.h, m_current_color.s);
    m_value_picker->SetValue(m_current_color.v);
    m_new_color_square->SetColor(color);
    if (m_current_color_button != INVALID_COLOR_BUTTON) {
        m_color_buttons[m_current_color_button]->SetRepresentedColor(color);
        s_custom_colors[m_current_color_button] = color;
    }
    UpdateHSVSliders();
}

void ColorDlg::ColorButtonClicked(std::size_t i)
{
    m_current_color_button = i;
    m_current_color = Convert(m_color_buttons[m_current_color_button]->RepresentedColor());
    ColorChanged(m_current_color);
}

void ColorDlg::RedSliderChanged(int value, int low, int high)
{
    Clr color = Convert(m_current_color);
    color.r = value;
    m_current_color = Convert(color);
    ColorChangeFromRGBSlider();
    *m_slider_values[R] << value;
}

void ColorDlg::GreenSliderChanged(int value, int low, int high)
{
    Clr color = Convert(m_current_color);
    color.g = value;
    m_current_color = Convert(color);
    ColorChangeFromRGBSlider();
    *m_slider_values[G] << value;
}

void ColorDlg::BlueSliderChanged(int value, int low, int high)
{
    Clr color = Convert(m_current_color);
    color.b = value;
    m_current_color = Convert(color);
    ColorChangeFromRGBSlider();
    *m_slider_values[B] << value;
}

void ColorDlg::AlphaSliderChanged(int value, int low, int high)
{
    Clr color = Convert(m_current_color);
    color.a = value;
    m_current_color = Convert(color);
    ColorChangeFromRGBSlider();
    *m_slider_values[A] << value;
}

void ColorDlg::HueSliderChanged(int value, int low, int high)
{
    m_current_color.h = value / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::SaturationSliderChanged(int value, int low, int high)
{
    m_current_color.s = value / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::ValueSliderChanged(int value, int low, int high)
{
    m_current_color.v = value / static_cast<double>(high - low);
    ColorChanged(m_current_color);
}

void ColorDlg::OkClicked()
{
    m_color_was_picked = true;
    m_done = true;
}

void ColorDlg::CancelClicked()
{
    m_current_color = Convert(m_original_color);
    m_done = true;
}

