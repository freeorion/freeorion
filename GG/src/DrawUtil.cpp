//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

#include <utility>
#include <GG/ClrConstants.h>
#include <GG/DrawUtil.h>
#include <GG/GLClientAndServerBuffer.h>
#include <GG/GUI.h>


#if !defined(__cpp_lib_integer_comparison_functions)
namespace {
constexpr bool cmp_less_equal(std::size_t l, int r) noexcept
{ return (r < 0) ? false : (l <= static_cast<std::size_t>(r)); }
static_assert(cmp_less_equal(0u, 0));
static_assert(cmp_less_equal(0u, 1));
static_assert(cmp_less_equal(1u, 1));
static_assert(!cmp_less_equal(1u, 0));
static_assert(cmp_less_equal(1u, 2));
static_assert(cmp_less_equal(0u, INT_MAX));
static_assert(cmp_less_equal(static_cast<std::size_t>(INT_MAX), INT_MAX));
static_assert(!cmp_less_equal(static_cast<std::size_t>(INT_MAX), 0));
static_assert(!cmp_less_equal(0u, -1));
static_assert(!cmp_less_equal(0u, INT_MIN));
static_assert(!cmp_less_equal(static_cast<std::size_t>(INT_MAX), INT_MIN));

constexpr bool cmp_greater(std::size_t l, int r) noexcept
{ return (r < 0) ? true : (l > static_cast<std::size_t>(r)); }
static_assert(!cmp_greater(0u, 0));
static_assert(!cmp_greater(0u, 1));
static_assert(!cmp_greater(1u, 1));
static_assert(cmp_greater(1u, 0));
static_assert(!cmp_greater(1u, 2));
static_assert(!cmp_greater(0u, INT_MAX));
static_assert(!cmp_greater(static_cast<std::size_t>(INT_MAX), INT_MAX));
static_assert(cmp_greater(static_cast<std::size_t>(INT_MAX), 0));
static_assert(cmp_greater(0u, -1));
static_assert(cmp_greater(0u, INT_MIN));
static_assert(cmp_greater(static_cast<std::size_t>(INT_MAX), INT_MIN));
}
#else
using std::cmp_less_equal;
using std::cmp_greater;
#endif

using namespace GG;

namespace {
    static_assert(uint32_t{Clr{0,0,0,1}} == 1u);
    static_assert(uint32_t{Clr{0,0,2,3}} == 2*256u + 3u);
    static_assert(uint32_t{Clr{255,1,0,0}} == 256*256*256*255u + 256*256*1u);

    static_assert(Clr::HexClr("A0FF01") == Clr{160, 255, 1, 255});
    static_assert(Clr::HexClr("12345678") == Clr{16*1+2, 16*3+4, 16*5+6, 16*7+8});

    // workaround for operator==(array, array) not being constexpr in C++17
    template <std::size_t N>
    constexpr bool ArrEq(std::array<std::string::value_type, N> l,
                         const std::string::value_type* r) noexcept
    {
        for (std::size_t idx = 0; idx < l.size(); ++idx)
            if (l[idx] != r[idx])
                return false;
        return true;
    }
    static_assert(ArrEq(Clr::HexClr("A0FF01BB").Hex(), "A0FF01BB"));

    static_assert(Clr::HexCharToUint8('0') == 0u);
    static_assert(Clr::HexCharToUint8('9') == 9u);
    static_assert(Clr::HexCharToUint8('A') == 10u);
    static_assert(Clr::HexCharToUint8('F') == 15u);
    static_assert(Clr::HexCharToUint8('G') == 16u);

    static_assert(Clr::HexCharsToUInt8("") == 0u);
    static_assert(Clr::HexCharsToUInt8("A") == 10u);
    static_assert(Clr::HexCharsToUInt8("01") == 1u);
    static_assert(Clr::HexCharsToUInt8("FF") == 255u);
    static_assert(Clr::HexCharsToUInt8("A0") == 160u);
    static_assert(Clr::HexCharsToUInt8("!.") == 14u);

    static_assert(ArrEq(Clr::ToHexChars(0), "00"));
    static_assert(ArrEq(Clr::ToHexChars(1), "01"));
    static_assert(ArrEq(Clr::ToHexChars(9), "09"));
    static_assert(ArrEq(Clr::ToHexChars(10), "0A"));
    static_assert(ArrEq(Clr::ToHexChars(15), "0F"));
    static_assert(ArrEq(Clr::ToHexChars(16), "10"));
    static_assert(ArrEq(Clr::ToHexChars(255), "FF"));

    static_assert(ArrEq(Clr::ToHexChars(Clr::HexCharsToUInt8("00")), "00"));
    static_assert(ArrEq(Clr::ToHexChars(Clr::HexCharsToUInt8("09")), "09"));
    static_assert(ArrEq(Clr::ToHexChars(Clr::HexCharsToUInt8("2C")), "2C"));
    static_assert(ArrEq(Clr::ToHexChars(Clr::HexCharsToUInt8("EF")), "EF"));

    using sva4 = std::array<std::string::value_type, 4>;
    constexpr bool TestUint8ToCharArray(uint8_t num, sva4 expected_result) noexcept
    { return ArrEq(Clr::UInt8ToCharArray(num), expected_result.data()); }

    static_assert(TestUint8ToCharArray(0, sva4{"0\0\0"}));
    static_assert(TestUint8ToCharArray(1, sva4{"1\0\0"}));
    static_assert(TestUint8ToCharArray(20, sva4{"20\0"}));
    static_assert(TestUint8ToCharArray(21, sva4{"21\0"}));
    static_assert(TestUint8ToCharArray(200, sva4{"200"}));
    static_assert(TestUint8ToCharArray(210, sva4{"210"}));
    static_assert(TestUint8ToCharArray(201, sva4{"201"}));
    static_assert(TestUint8ToCharArray(255, sva4{"255"}));

    static_assert(Clr::CharsToUInt8("") == 0);
    static_assert(Clr::CharsToUInt8("abcdefgh") == 0);
    static_assert(Clr::CharsToUInt8("0") == 0);
    static_assert(Clr::CharsToUInt8("\0") == 0);
    static_assert(Clr::CharsToUInt8("-25") == 0);
    static_assert(Clr::CharsToUInt8("25") == 25);
    static_assert(Clr::CharsToUInt8("00001") == 1);
    static_assert(Clr::CharsToUInt8("888") == 888-3*256);
    static_assert(Clr::CharsToUInt8("109") == 109);
    static_assert(Clr::CharsToUInt8("30") == 30);

    static_assert(Clr::RGBAClr("", "0", "000001") == Clr{0,0,1,255});
    static_assert(Clr::RGBAClr("1", "-2", "", "") == Clr{1,0,0,255});

    using namespace std::literals;

    static_assert("  345 "sv.find_first_not_of(' ') == 2u);
    static_assert("  345 "sv.find_first_of(' ') == 0u);
    static_assert(""sv.find_first_of(' ') == std::string_view::npos);
    static_assert(""sv.find_first_not_of(' ') == std::string_view::npos);
    static_assert("   "sv.find_first_not_of(' ') == std::string_view::npos);
    static_assert("345 "sv.substr(0u, std::string_view::npos) == "345 ");
    static_assert("345"sv.substr(3u).empty());

    using svpair = std::pair<std::string_view, std::string_view>;
    static_assert(Clr::NextSpaceDelimChunkAndRest("") == svpair{});
    static_assert(Clr::NextSpaceDelimChunkAndRest("   ") == svpair{});
    static_assert(Clr::NextSpaceDelimChunkAndRest("1  ") == svpair{"1", "  "});
    static_assert(Clr::NextSpaceDelimChunkAndRest("  1") == svpair{"1", ""});
    static_assert(Clr::NextSpaceDelimChunkAndRest(" 1   2 ") == svpair{"1", "   2 "});

    static_assert(Clr::RGBAClr("1 2 3 4 5 6 a b c \0") == Clr{1,2,3,4});
    static_assert(Clr::RGBAClr(" 001  255 3 ") == Clr{1,255,3});

    static_assert(LightenClr(CLR_DARK_GRAY, 3.0f) == CLR_LIGHT_GRAY);
    static_assert(LightenClr(CLR_DARK_GRAY, 100.0f) == CLR_WHITE);
    static_assert(DarkenClr(CLR_DARK_GRAY, 1000.0f) == CLR_BLACK);
    static_assert(DarkenClr(CLR_DARK_GRAY, 0.00001f) == CLR_WHITE);
    static_assert(BlendClr(CLR_DARK_GRAY, CLR_LIGHT_GRAY) == InvertClr(CLR_GRAY));
    static_assert(BlendClr(CLR_WHITE, CLR_ZERO, 0.8f) == Clr(255*4/5, 255*4/5, 255*4/5, 255*4/5));
}

namespace {

constexpr double PI = 3.141594; // intentionally slightly more than Pi
constexpr double twoPI = 2*PI;
constexpr float SQRT2OVER2 = 0.70710678118654757274f; //std::sqrt(2.0) / 2.0;

/// a stack of the currently-active clipping rects, in GG coordinates, not OpenGL scissor coordinates
std::vector<Rect> g_scissor_clipping_rects;

/// the index of the next stencil bit to use for stencil clipping
unsigned int g_stencil_bit = 0;

void Rectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2,
               unsigned int bevel_thick, bool bevel_left, bool bevel_top,
               bool bevel_right, bool bevel_bottom)
{
    X inner_x1 = ul.x + (bevel_left ? static_cast<int>(bevel_thick) : 0);
    Y inner_y1 = ul.y + (bevel_top ? static_cast<int>(bevel_thick) : 0);
    X inner_x2 = lr.x - (bevel_right ? static_cast<int>(bevel_thick) : 0);
    Y inner_y2 = lr.y - (bevel_bottom ? static_cast<int>(bevel_thick) : 0);

    GL2DVertexBuffer verts;
    verts.reserve(14);
    verts.store(inner_x2,   inner_y1);
    verts.store(lr.x,       ul.y);
    verts.store(inner_x1,   inner_y1);
    verts.store(ul.x,       ul.y);
    verts.store(inner_x1,   inner_y2);
    verts.store(ul.x,       lr.y);
    verts.store(inner_x2,   inner_y2);
    verts.store(lr.x,       lr.y);
    verts.store(inner_x2,   inner_y1);
    verts.store(lr.x,       ul.y);

    verts.store(inner_x2,   inner_y1);
    verts.store(inner_x1,   inner_y1);
    verts.store(inner_x1,   inner_y2);
    verts.store(inner_x2,   inner_y2);

    verts.activate();

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // draw beveled edges
    if (bevel_thick && (border_color1 != CLR_ZERO || border_color2 != CLR_ZERO)) {
        glColor(border_color1);
        if (border_color1 == border_color2) {
            glDrawArrays(GL_QUAD_STRIP, 0, 10);
        } else {
            glDrawArrays(GL_QUAD_STRIP, 0, 6);
            glColor(border_color2);
            glDrawArrays(GL_QUAD_STRIP, 4, 6);
        }
    }

    // draw interior of rectangle
    if (color != CLR_ZERO) {
        glColor(color);
        glDrawArrays(GL_QUADS, 10, 4);
    }

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void Check(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3)
{
    const GLfloat wdhalf = Value(lr.x - ul.x) / 2.0f;
    const GLfloat hthalf = Value(lr.y - ul.y) / 2.0f;
    const GLfloat x = Value(ul.x);
    const GLfloat y = Value(ul.y);

    // all vertices
    static constexpr std::array<std::array<GLfloat, 2>, 9> verts =
        {{{-0.2f,  0.2f}, {-0.6f, -0.2f}, {-0.6f,  0.0f},
          {-0.2f,  0.4f}, {-0.8f,  0.0f}, {-0.2f,  0.6f},
          { 0.8f, -0.4f}, { 0.6f, -0.4f}, { 0.8f, -0.8f}}};

    glPushMatrix();
    static constexpr float sf = 1.25f;              // scale factor to make the check look right
    glTranslatef(x + wdhalf, y + hthalf*sf, 0.0f);  // move origin to the center of the rectangle
    glScalef(     wdhalf*sf,     hthalf*sf, 1.0f);  // map the range [-1,1] to the rectangle in both directions

    static constexpr std::array<std::size_t, 22> indices =
      { 1,  4,  2,
        8,  0,  3,  7,
        2,  4,  5,  3,  7,  3,  5,  6,
        8,  7,  6,
        0,  1,  2,  3};

    GL2DVertexBuffer vert_buf;
    vert_buf.reserve(indices.size());
    for (const auto index : indices)
        vert_buf.store(verts[index][0], verts[index][1]);

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();

    glColor(color3);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_QUADS, 3, 4);

    glColor(color2);
    glDrawArrays(GL_QUADS, 7, 8);

    glColor(color1);
    glDrawArrays(GL_TRIANGLES, 15, 3);
    glDrawArrays(GL_QUADS, 18, 4);

    glPopClientAttrib();

    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void XMark(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3)
{
    const GLfloat wdhalf = Value(lr.x - ul.x) / 2.0f;
    const GLfloat hthalf = Value(lr.y - ul.y) / 2.0f;

    // all vertices
    static constexpr std::array<std::array<GLfloat, 2>, 17> verts =
        {{{-0.4f, -0.6f}, {-0.6f, -0.4f}, {-0.4f, -0.4f}, {-0.2f,  0.0f}, {-0.6f,  0.4f},
         {-0.4f,  0.6f}, {-0.4f,  0.4f}, { 0.0f,  0.2f}, { 0.4f,  0.6f}, { 0.6f,  0.4f},
         { 0.4f,  0.4f}, { 0.2f,  0.0f}, { 0.6f, -0.4f}, { 0.4f, -0.6f}, { 0.4f, -0.4f},
         { 0.0f, -0.2f}, { 0.0f,  0.0f}}};

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    static constexpr GLfloat sf = 1.75f;                            // scale factor; the check wasn't the right size as drawn originally
    glTranslatef(Value(ul.x) + wdhalf, Value(ul.y) + hthalf, 0.0f); // move origin to the center of the rectangle
    glScalef(wdhalf*sf, hthalf*sf, 1.0f);                           // map the range [-1,1] to the rectangle in both directions

    static constexpr std::array<std::size_t, 44> indices =
        {12, 13, 14,
         15,  0,  2, 16,  9, 11, 16, 10,
         0,  1,  2,
         13, 15, 16, 14,  3,  4,  6, 16,
         4,  5,  6,  8,  9, 10,
         14, 16, 11, 12,  2,  1,  3, 16, 16,  6,  5,  7, 16,  7,  8, 10};

    GL2DVertexBuffer vert_buf;
    vert_buf.reserve(indices.size());
    for (std::size_t idx : indices)
        vert_buf.store(verts[idx][0], verts[idx][1]);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();

    glColor(color1);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDrawArrays(GL_QUADS, 3, 8);

    glColor(color2);
    glDrawArrays(GL_TRIANGLES, 11, 3);
    glDrawArrays(GL_QUADS, 14, 8);

    glColor(color3);
    glDrawArrays(GL_TRIANGLES, 22, 6);
    glDrawArrays(GL_QUADS, 28, 16);

    glPopClientAttrib();

    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

namespace {
    constexpr auto CorrectAngle(double theta) {
        // correct to range [0, 2pi)
        if (theta < 0)
            return theta + (static_cast<int64_t>(-theta / twoPI) + 1) * twoPI;
        else if (theta >= twoPI)
            return theta - static_cast<int64_t>(theta / twoPI) * twoPI;
        else
            return theta;
    }

    constexpr std::size_t SLICES = 36u; // how much to tesselate the circle coordinates

    // this doesn't serve as a cache, but does allow us to prevent numerous
    // constructions and destructions of Clr vectors.
#if defined(__cpp_constinit)
    constinit
#endif
    auto colors = []() {
        std::array<GG::Clr, SLICES+1u> colors{};
        colors.fill(GG::CLR_ZERO);
        return colors;
    }();

    constexpr double HORZ_THETA = twoPI / SLICES;

    // cache of sin() and cos() calls
    const auto unit_vertices = []() {
        std::array<double, 2u*(SLICES+1u)> verts{};

        const double HORZ_THETA = twoPI / SLICES;

        // calculate x,y values for each point on a unit circle divided into SLICES arcs
        double theta = 0.0f;
        for (std::size_t j = 0; j <= SLICES; theta += HORZ_THETA, ++j) {
            verts[j*2] = cos(-theta);
            verts[j*2+1] = sin(-theta);
        }
        return verts;
    }();
}

void CircleArc(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2,
               unsigned int bevel_thick, double theta1, double theta2)
{
    //std::cout << "GG::CircleArc ul: " << ul << "  lr: " << lr << " bevel thick: " << bevel_thick << "  theta1: " << theta1 << "  theta2: " << theta2 << std::endl;
    const GLfloat wd = Value(lr.x - ul.x);
    const GLfloat ht = Value(lr.y - ul.y);
    glDisable(GL_TEXTURE_2D);

    theta1 = CorrectAngle(theta1);
    theta2 = CorrectAngle(theta2);

    const double theta1_x = cos(-theta1);
    const double theta1_y = sin(-theta1);
    const double theta2_x = cos(-theta2);
    const double theta2_y = sin(-theta2);

    const int first_slice_idx = int(theta1 / HORZ_THETA + 1);
    int last_slice_idx = int(theta2 / HORZ_THETA - 1);
    if (theta1 >= theta2)
        last_slice_idx += SLICES;

    // calculate the color for each needed point
    for (std::size_t j = first_slice_idx; cmp_less_equal(j, last_slice_idx); ++j) {
        std::size_t X = (j > SLICES ? (j - SLICES) : j) * 2;
        std::size_t Y = X + 1;
        // this is essentially the dot product of (x,y) with (sqrt2over2,sqrt2over2),
        // the direction of the light source, scaled to the range [0,1]
        const double color_scale_factor = (SQRT2OVER2 * (unit_vertices[X] + unit_vertices[Y]) + 1) / 2;
        colors[j] = BlendClr(border_color1, border_color2, color_scale_factor);
    }


    glPushMatrix();
    glTranslatef(Value(ul.x) + wd/2.0f, Value(ul.y) + ht/2.0f, 0.0f);// move origin to the center of the rectangle
    glScalef(wd/2.0f, ht/2.0f, 1.0f);                                // map the range [-1,1] to the rectangle in both (x- and y-) directions

    const double inner_radius = (std::min(wd, ht) - 2.0*bevel_thick) / std::min(wd, ht);

    GL2DVertexBuffer fan_vert_buf;
    fan_vert_buf.reserve(3u + last_slice_idx - first_slice_idx);
    fan_vert_buf.store(X0, Y0);
    fan_vert_buf.store(theta1_x * inner_radius, theta1_y * inner_radius);
    // angles in between theta1 and theta2, if any
    for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
        int X = 2 * (cmp_greater(i, SLICES) ? (i - SLICES) : i);
        int Y = X + 1;
        fan_vert_buf.store(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
    }
    fan_vert_buf.store(theta2_x * inner_radius, theta2_y * inner_radius);

    glColor(color);

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    fan_vert_buf.activate();
    glDrawArrays(GL_TRIANGLE_FAN, 0, fan_vert_buf.size());



    GL2DVertexBuffer quads_vert_buf;
    quads_vert_buf.reserve(4u + 2u*(last_slice_idx - first_slice_idx));

    GLRGBAColorBuffer quads_colour_buf;
    quads_colour_buf.reserve(quads_vert_buf.size());


    // point on circle at angle theta1
    const auto color_scale_factor1 = (SQRT2OVER2 * (theta1_x + theta1_y) + 1) / 2;
    const auto clr1 = BlendClr(border_color1, border_color2, color_scale_factor1);
    quads_colour_buf.store(clr1);
    quads_vert_buf.store(theta1_x, theta1_y);
    quads_colour_buf.store(clr1);
    quads_vert_buf.store(theta1_x * inner_radius, theta1_y * inner_radius);
    // angles in between theta1 and theta2, if any
    for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
        const int X = 2 * (cmp_greater(i, SLICES) ? (i - SLICES) : i);
        const int Y = X + 1;
        quads_colour_buf.store(colors[i]);
        quads_vert_buf.store(unit_vertices[X], unit_vertices[Y]);
        quads_colour_buf.store(colors[i]);
        quads_vert_buf.store(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
    }
    // theta2
    const auto color_scale_factor2 = (SQRT2OVER2 * (theta2_x + theta2_y) + 1) / 2;
    const auto clr2 = BlendClr(border_color1, border_color2, color_scale_factor2);
    quads_colour_buf.store(clr2);
    quads_vert_buf.store(theta2_x, theta2_y);
    quads_colour_buf.store(clr2);
    quads_vert_buf.store(theta2_x * inner_radius, theta2_y * inner_radius);


    glEnableClientState(GL_COLOR_ARRAY);

    quads_vert_buf.activate();
    quads_colour_buf.activate();
    glDrawArrays(GL_QUAD_STRIP, 0, quads_vert_buf.size());

    glPopClientAttrib();
    glPopMatrix();
    glEnable(GL_TEXTURE_2D);
}

void RoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2,
                      unsigned int corner_radius, int thick)
{
    const int circle_diameter = corner_radius * 2;
    CircleArc(Pt(lr.x - circle_diameter, ul.y),                     Pt(lr.x, ul.y + circle_diameter),
              color, border_color2, border_color1, thick, 0, 0.5 * PI);  // ur corner
    CircleArc(Pt(ul.x, ul.y),                                       Pt(ul.x + circle_diameter, ul.y + circle_diameter),
              color, border_color2, border_color1, thick, 0.5 * PI, PI); // ul corner
    CircleArc(Pt(ul.x, lr.y - circle_diameter),                     Pt(ul.x + circle_diameter, lr.y),
              color, border_color2, border_color1, thick, PI, 1.5 * PI); // ll corner
    CircleArc(Pt(lr.x - circle_diameter, lr.y - circle_diameter),   Pt(lr.x, lr.y),
              color, border_color2, border_color1, thick, 1.5 * PI, 0);  // lr corner


    // lines connecting circle arcs and giving bevel appearance
    GL2DVertexBuffer vert_buf;
    vert_buf.reserve(28);
    GLRGBAColorBuffer colour_buf;   // need to give each vertex in lightness bar its own colour so can't just use a glColor call
    colour_buf.reserve(28);

    const int rad = static_cast<int>(corner_radius);

    static constexpr float color_scale_factor1 = (SQRT2OVER2 * (0 + 1) + 1) / 2;
    Clr clr = BlendClr(border_color1, border_color2, color_scale_factor1);
    // top
    vert_buf.store(lr.x - rad,      ul.y);
    vert_buf.store(ul.x + rad,      ul.y);
    vert_buf.store(ul.x + rad,      ul.y + thick);
    vert_buf.store(lr.x - rad,      ul.y + thick);
    // left
    vert_buf.store(ul.x + thick,    ul.y + rad);
    vert_buf.store(ul.x,            ul.y + rad);
    vert_buf.store(ul.x,            lr.y - rad);
    vert_buf.store(ul.x + thick,    lr.y - rad);
    for (unsigned int i = 0; i < 8; ++i)
        colour_buf.store(clr);


    static constexpr float color_scale_factor2 = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
    clr = BlendClr(border_color1, border_color2, color_scale_factor2);
    // right
    vert_buf.store(lr.x,            ul.y + rad);
    vert_buf.store(lr.x - thick,    ul.y + rad);
    vert_buf.store(lr.x - thick,    lr.y - rad);
    vert_buf.store(lr.x,            lr.y - rad);
    // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of right
    vert_buf.store(lr.x - rad,      lr.y - thick);
    vert_buf.store(ul.x + rad,      lr.y - thick);
    vert_buf.store(ul.x + rad,      lr.y);
    vert_buf.store(lr.x - rad,      lr.y);
    for (unsigned int i = 0; i < 8; ++i)
        colour_buf.store(clr);


    // middle
    vert_buf.store(lr.x - rad,      ul.y + thick);
    vert_buf.store(ul.x + rad,      ul.y + thick);
    vert_buf.store(ul.x + rad,      lr.y - thick);
    vert_buf.store(lr.x - rad,      lr.y - thick);

    vert_buf.store(lr.x - thick,    ul.y + rad);
    vert_buf.store(lr.x - rad,      ul.y + rad);
    vert_buf.store(lr.x - rad,      lr.y - rad);
    vert_buf.store(lr.x - thick,    lr.y - rad);

    vert_buf.store(ul.x + thick,    ul.y + rad);
    vert_buf.store(ul.x + rad,      ul.y + rad);
    vert_buf.store(ul.x + rad,      lr.y - rad);
    vert_buf.store(ul.x + thick,    lr.y - rad);
    for (unsigned int i = 0; i < 12; ++i)
        colour_buf.store(color);


    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();
    colour_buf.activate();

    glDrawArrays(GL_QUADS, 0, vert_buf.size());

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

}

void glColor(Clr clr)
{ glColor4ub(clr.r, clr.g, clr.b, clr.a); }

Clr GG::DisabledColor(Clr clr)
{
    Clr retval = clr;
    static constexpr double gray_factor = 0.75; // amount to move clr in the direction of gray
    retval.r = static_cast<int>(retval.r + (CLR_GRAY.r - retval.r) * gray_factor);
    retval.g = static_cast<int>(retval.g + (CLR_GRAY.g - retval.g) * gray_factor);
    retval.b = static_cast<int>(retval.b + (CLR_GRAY.b - retval.b) * gray_factor);
    return retval;
}

void GG::BeginScissorClipping(Pt ul, Pt lr)
{
    if (g_scissor_clipping_rects.empty()) {
        glPushAttrib(GL_SCISSOR_BIT | GL_ENABLE_BIT);
        glEnable(GL_SCISSOR_TEST);
        if (g_stencil_bit)
            glDisable(GL_STENCIL_TEST);
    } else {
        const Rect& r = g_scissor_clipping_rects.back();
        ul.x = std::max(r.Left(), std::min(ul.x, r.Right()));
        ul.y = std::max(r.Top(), std::min(ul.y, r.Bottom()));
        lr.x = std::max(r.Left(), std::min(lr.x, r.Right()));
        lr.y = std::max(r.Top(), std::min(lr.y, r.Bottom()));
    }
    glScissor(Value(ul.x), Value(GUI::GetGUI()->AppHeight() - lr.y),
                Value(lr.x - ul.x), Value(lr.y - ul.y));
    g_scissor_clipping_rects.emplace_back(ul, lr);
}

void GG::EndScissorClipping()
{
    assert(!g_scissor_clipping_rects.empty());
    g_scissor_clipping_rects.pop_back();
    if (g_scissor_clipping_rects.empty()) {
        if (g_stencil_bit)
            glEnable(GL_STENCIL_TEST);
        glPopAttrib();
    } else {
        const Rect& r = g_scissor_clipping_rects.back();
        glScissor(Value(r.Left()), Value(GUI::GetGUI()->AppHeight() - r.Bottom()),
                  Value(r.Width()), Value(r.Height()));
    }
}

Rect GG::ActiveScissorClippingRegion()
{
    if (g_scissor_clipping_rects.empty())
        return Rect();
    return g_scissor_clipping_rects.back();
}

void GG::BeginStencilClipping(Pt inner_ul, Pt inner_lr, Pt outer_ul, Pt outer_lr)
{
    if (!g_stencil_bit) {
        glPushAttrib(GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT);
        glClearStencil(0);
        glClear(GL_STENCIL_BUFFER_BIT);
        glEnable(GL_STENCIL_TEST);
        if (!g_scissor_clipping_rects.empty())
            glDisable(GL_SCISSOR_TEST);
    }

    GLboolean prev_color_writemask[4] = {};
    glGetBooleanv(GL_COLOR_WRITEMASK, prev_color_writemask);
    GLboolean prev_depth_writemask = 0;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &prev_depth_writemask);

    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    const GLuint mask = 1u << g_stencil_bit;

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glStencilFunc(GL_ALWAYS, mask, mask);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
    const GLint outer_vertices[] = {
        Value(outer_ul.x), Value(outer_ul.y),
        Value(outer_ul.x), Value(outer_lr.y),
        Value(outer_lr.x), Value(outer_lr.y),
        Value(outer_lr.x), Value(outer_ul.y)
    };
    glVertexPointer(2, GL_INT, 0, outer_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
    const GLint inner_vertices[] = {
        Value(inner_ul.x), Value(inner_ul.y),
        Value(inner_ul.x), Value(inner_lr.y),
        Value(inner_lr.x), Value(inner_lr.y),
        Value(inner_lr.x), Value(inner_ul.y)
    };
    glVertexPointer(2, GL_INT, 0, inner_vertices);
    glDrawArrays(GL_QUADS, 0, 4);

    glColorMask(prev_color_writemask[0],
                prev_color_writemask[1],
                prev_color_writemask[2],
                prev_color_writemask[3]);
    glDepthMask(prev_depth_writemask);

    glStencilFunc(GL_EQUAL, mask, mask);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    ++g_stencil_bit;

    glPopClientAttrib();
}

void GG::EndStencilClipping()
{
    assert(g_stencil_bit);
    --g_stencil_bit;
    if (!g_stencil_bit) {
        if (!g_scissor_clipping_rects.empty())
            glEnable(GL_SCISSOR_TEST);
        glPopAttrib();
    }
}

void GG::Line(Pt pt1, Pt pt2, Clr color, float thick)
{
    glLineWidth(thick);
    glColor(color);
    Line(pt1.x, pt1.y, pt2.x, pt2.y);
}

void GG::Line(X x1, Y y1, X x2, Y y2, Clr color, float thick)
{
    glLineWidth(thick);
    glColor(color);
    Line(x1, y1, x2, y2);
}

void GG::Line(X x1, Y y1, X x2, Y y2)
{
    const GLfloat vertices[4] = {GLfloat(Value(x1)), GLfloat(Value(y1)),
                                 GLfloat(Value(x2)), GLfloat(Value(y2))};

    glDisable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_LINES, 0, 2);

    glPopClientAttrib();
    glLineWidth(1.0f);
    glEnable(GL_TEXTURE_2D);
}

void GG::Triangle(Pt pt1, Pt pt2, Pt pt3, Clr color, Clr border_color, float border_thick)
{
    const GLfloat vertices[6] = {GLfloat(Value(pt1.x)), GLfloat(Value(pt1.y)), GLfloat(Value(pt2.x)),
                                 GLfloat(Value(pt2.y)), GLfloat(Value(pt3.x)), GLfloat(Value(pt3.y))};

    glDisable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glColor(color);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (border_color != GG::CLR_ZERO) {
        glLineWidth(border_thick);
        glColor(border_color);

        glDrawArrays(GL_LINE_LOOP, 0, 3);
        glLineWidth(1.0f);
    }

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void GG::Triangle(X x1, Y y1, X x2, Y y2, X x3, Y y3, bool filled)
{
    const GLfloat vertices[6] = {GLfloat(Value(x1)), GLfloat(Value(y1)), GLfloat(Value(x2)),
                                 GLfloat(Value(y2)), GLfloat(Value(x3)), GLfloat(Value(y3))};

    glDisable(GL_TEXTURE_2D);

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, vertices);

    if (filled)
        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    else
        glDrawArrays(GL_TRIANGLES, 0, 3);

    glPopClientAttrib();
    glEnable(GL_TEXTURE_2D);
}

void GG::FlatRectangle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int border_thick)
{
    Rectangle(ul, lr, color, border_color, border_color, border_thick,
              true, true, true, true);
}

void GG::BeveledRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int bevel_thick,
                          bool bevel_left, bool bevel_top, bool bevel_right, bool bevel_bottom)
{
    Rectangle(ul, lr, color,
              (up ? LightenClr(border_color) : DarkenClr(border_color)),
              (up ? DarkenClr(border_color) : LightenClr(border_color)),
              bevel_thick, bevel_left, bevel_top, bevel_right, bevel_bottom);
}

void GG::FlatRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color,
                              unsigned int corner_radius, unsigned int border_thick)
{
    RoundedRectangle(ul, lr, color, border_color, border_color,
                     corner_radius, border_thick);
}

void GG::BeveledRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up,
                                 unsigned int corner_radius, unsigned int bevel_thick)
{
    RoundedRectangle(ul, lr, color,
                     (up ? LightenClr(border_color) : DarkenClr(border_color)),
                     (up ? DarkenClr(border_color) : LightenClr(border_color)),
                     corner_radius, bevel_thick);
}

void GG::FlatCheck(Pt ul, Pt lr, Clr color)
{ Check(ul, lr, color, color, color); }

void GG::BeveledCheck(Pt ul, Pt lr, Clr color)
{ Check(ul, lr, color, LightenClr(color), DarkenClr(color)); }

void GG::FlatX(Pt ul, Pt lr, Clr color)
{ XMark(ul, lr, color, color, color); }

//void GG::Bubble(Pt ul, Pt lr, Clr color, bool up)
//{
//    BubbleArc(ul, lr, color,
//              (up ? DarkenClr(color) : LightenClr(color)),
//              (up ? LightenClr(color) : DarkenClr(color)),
//              0, 0);
//}

void GG::FlatCircle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int thick)
{ CircleArc(ul, lr, color, border_color, border_color, thick, 0, 0); }

void GG::BeveledCircle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int bevel_thick)
{
    CircleArc(ul, lr, color,
              (up ? DarkenClr(border_color) : LightenClr(border_color)),
              (up ? LightenClr(border_color) : DarkenClr(border_color)),
              bevel_thick, 0, 0);
}
