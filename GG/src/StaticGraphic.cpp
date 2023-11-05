//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/StaticGraphic.h>


using namespace GG;

///////////////////////////////////////
// GraphicStyle
///////////////////////////////////////
GG_FLAGSPEC_IMPL(GraphicStyle);

namespace {

bool RegisterGraphicStyles()
{
    FlagSpec<GraphicStyle>& spec = FlagSpec<GraphicStyle>::instance();
    spec.insert(GRAPHIC_NONE,       "GRAPHIC_NONE");
    spec.insert(GRAPHIC_VCENTER,    "GRAPHIC_VCENTER");
    spec.insert(GRAPHIC_TOP,        "GRAPHIC_TOP");
    spec.insert(GRAPHIC_BOTTOM,     "GRAPHIC_BOTTOM");
    spec.insert(GRAPHIC_CENTER,     "GRAPHIC_CENTER");
    spec.insert(GRAPHIC_LEFT,       "GRAPHIC_LEFT");
    spec.insert(GRAPHIC_RIGHT,      "GRAPHIC_RIGHT");
    spec.insert(GRAPHIC_FITGRAPHIC, "GRAPHIC_FITGRAPHIC");
    spec.insert(GRAPHIC_SHRINKFIT,  "GRAPHIC_SHRINKFIT");
    spec.insert(GRAPHIC_PROPSCALE,  "GRAPHIC_PROPSCALE");
    return true;
}
bool dummy = RegisterGraphicStyles();

}


////////////////////////////////////////////////
// GG::StaticGraphic
////////////////////////////////////////////////
StaticGraphic::StaticGraphic(std::shared_ptr<Texture> texture,
                             Flags<GraphicStyle> style,
                             Flags<WndFlag> flags) :
    Control(X0, Y0, X1, Y1, flags),
    m_style(style)
{
    auto w = texture->DefaultWidth();
    auto h = texture->DefaultHeight();
    m_graphic = SubTexture(std::move(texture), X0, Y0, w, h);

    ValidateStyle();  // correct any disagreements in the style flags
    SetColor(CLR_WHITE);
}

StaticGraphic::StaticGraphic(SubTexture subtexture,
                             Flags<GraphicStyle> style,
                             Flags<WndFlag> flags) :
    Control(X0, Y0, X1, Y1, flags),
    m_graphic(std::move(subtexture)),
    m_style(style)
{
    ValidateStyle();  // correct any disagreements in the style flags
    SetColor(CLR_WHITE);
}

Rect StaticGraphic::RenderedArea() const
{
    Pt ul = UpperLeft(), lr = LowerRight();
    Pt window_sz(lr - ul);

    Pt graphic_sz;
    if (m_graphic.GetTexture())
        graphic_sz = {m_graphic.Width(), m_graphic.Height()};

    Pt pt1, pt2(graphic_sz); // (unscaled) default graphic size
    if (m_style & GRAPHIC_FITGRAPHIC) {
        if (m_style & GRAPHIC_PROPSCALE) {
            double scale_x = Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x));
            double scale_y = Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y));
            double scale = std::min(scale_x, scale_y);
            pt2.x = ToX(graphic_sz.x * scale);
            pt2.y = ToY(graphic_sz.y * scale);
        } else {
            pt2 = window_sz;
        }
    } else if (m_style & GRAPHIC_SHRINKFIT) {
        if (m_style & GRAPHIC_PROPSCALE) {
            double scale_x = (graphic_sz.x > window_sz.x) ? Value(window_sz.x) / static_cast<double>(Value(graphic_sz.x)) : 1.0;
            double scale_y = (graphic_sz.y > window_sz.y) ? Value(window_sz.y) / static_cast<double>(Value(graphic_sz.y)) : 1.0;
            double scale = std::min(scale_x, scale_y);
            pt2.x = ToX(graphic_sz.x * scale);
            pt2.y = ToY(graphic_sz.y * scale);
        } else {
            pt2 = window_sz;
        }
    }

    X x_shift(X0);
    if (m_style & GRAPHIC_LEFT) {
        x_shift = ul.x;
    } else if (m_style & GRAPHIC_CENTER) {
        x_shift = ul.x + (window_sz.x - (pt2.x - pt1.x)) / 2;
    } else { // m_style & GRAPHIC_RIGHT
        x_shift = lr.x - (pt2.x - pt1.x);
    }
    pt1.x += x_shift;
    pt2.x += x_shift;

    Y y_shift(Y0);
    if (m_style & GRAPHIC_TOP) {
        y_shift = ul.y;
    } else if (m_style & GRAPHIC_VCENTER) {
        y_shift = ul.y + (window_sz.y - (pt2.y - pt1.y)) / 2;
    } else { // m_style & GRAPHIC_BOTTOM
        y_shift = lr.y - (pt2.y - pt1.y);
    }
    pt1.y += y_shift;
    pt2.y += y_shift;

    return Rect(pt1, pt2);
}

const boost::filesystem::path& StaticGraphic::GetTexturePath() const
{
    static const boost::filesystem::path EMPTY_PATH;

    if (const Texture* texture = m_graphic.GetTexture())
        return texture->Path();

    return EMPTY_PATH;
}

void StaticGraphic::Render()
{
    const Clr color_to_use = Disabled() ? DisabledColor(Color()) : Color();
    glColor(color_to_use);
    Rect rendered_area = RenderedArea();

    if (m_graphic.GetTexture())
        m_graphic.OrthoBlit(rendered_area.ul, rendered_area.lr);
}

void StaticGraphic::SetStyle(Flags<GraphicStyle> style)
{
    m_style = style;
    ValidateStyle();
}

void StaticGraphic::SetTexture(std::shared_ptr<Texture> texture)
{
    const auto w = texture->DefaultWidth();
    const auto h = texture->DefaultHeight();
    SetTexture(SubTexture(std::move(texture), X0, Y0, w, h));
}

void StaticGraphic::SetTexture(SubTexture subtexture)
{ m_graphic = std::move(subtexture); }

void StaticGraphic::ValidateStyle()
{
    int dup_ct = 0;   // duplication count
    if (m_style & GRAPHIC_LEFT) ++dup_ct;
    if (m_style & GRAPHIC_RIGHT) ++dup_ct;
    if (m_style & GRAPHIC_CENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_CENTER by default
        m_style &= ~(GRAPHIC_RIGHT | GRAPHIC_LEFT);
        m_style |= GRAPHIC_CENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_TOP) ++dup_ct;
    if (m_style & GRAPHIC_BOTTOM) ++dup_ct;
    if (m_style & GRAPHIC_VCENTER) ++dup_ct;
    if (dup_ct != 1) {   // exactly one must be picked; when none or multiples are picked, use GRAPHIC_VCENTER by default
        m_style &= ~(GRAPHIC_TOP | GRAPHIC_BOTTOM);
        m_style |= GRAPHIC_VCENTER;
    }
    dup_ct = 0;
    if (m_style & GRAPHIC_FITGRAPHIC) ++dup_ct;
    if (m_style & GRAPHIC_SHRINKFIT) ++dup_ct;
    if (dup_ct > 1) {   // mo more than one may be picked; when both are picked, use GRAPHIC_SHRINKFIT by default
        m_style &= ~GRAPHIC_FITGRAPHIC;
        m_style |= GRAPHIC_SHRINKFIT;
    }
}
