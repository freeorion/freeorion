#include "CUIDrawUtil.h"

#include "ShaderProgram.h"
#include "../util/Logger.h"
#include "../util/Directories.h"
#include "../util/OptionsDB.h"

#include <GG/DrawUtil.h>

#include <cmath>


namespace {
    constexpr void FindIsoscelesTriangleVertices(const GG::Pt ul, const GG::Pt lr,
                                                 ShapeOrientation orientation, double& x1_, double& y1_,
                                                 double& x2_, double& y2_, double& x3_, double& y3_) noexcept
    {
        switch (orientation) {
        case ShapeOrientation::UP:
            x1_ = Value(ul.x);
            y1_ = Value(lr.y);
            x2_ = Value(lr.x);
            y2_ = Value(lr.y);
            x3_ = (ul.x + lr.x) / 2.0;
            y3_ = Value(ul.y);
            break;
        case ShapeOrientation::DOWN:
            x1_ = Value(lr.x);
            y1_ = Value(ul.y);
            x2_ = Value(ul.x);
            y2_ = Value(ul.y);
            x3_ = (ul.x + lr.x) / 2.0;
            y3_ = Value(lr.y);
            break;
        case ShapeOrientation::LEFT:
            x1_ = Value(lr.x);
            y1_ = Value(lr.y);
            x2_ = Value(lr.x);
            y2_ = Value(ul.y);
            x3_ = Value(ul.x);
            y3_ = (ul.y + lr.y) / 2.0;
            break;
        default:
            [[fallthrough]];
        case ShapeOrientation::RIGHT:
            x1_ = Value(ul.x);
            y1_ = Value(ul.y);
            x2_ = Value(ul.x);
            y2_ = Value(lr.y);
            x3_ = Value(lr.x);
            y3_ = (ul.y + lr.y) / 2.0;
            break;
        }
    }
}

void BufferStoreCircleArcVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                  const GG::Pt lr, double theta1, double theta2,
                                  bool filled_shape, int num_slices, bool fan)
{
    int wd = Value(lr.x - ul.x);
    int ht = Value(lr.y - ul.y);
    double center_x = ul.x + wd/2.0;
    double center_y = ul.y + ht/2.0;
    double r = std::min(wd / 2.0, ht / 2.0);
    static constexpr double PI = 3.141594; // intentionally slightly more than pi
    static constexpr double TWO_PI = 2.0 * PI;

    // correct theta* values to range [0, 2pi)
    if (theta1 < 0)
        theta1 += (int(-theta1 / TWO_PI) + 1) * TWO_PI;
    else if (theta1 >= TWO_PI)
        theta1 -= int(theta1 / TWO_PI) * TWO_PI;
    if (theta2 < 0)
        theta2 += (int(-theta2 / TWO_PI) + 1) * TWO_PI;
    else if (theta2 >= TWO_PI)
        theta2 -= int(theta2 / TWO_PI) * TWO_PI;

    std::size_t SLICES = 30;
    if (num_slices <= 0)
        SLICES = std::min(std::max(12u, 3u + std::max(wd, ht) / 4u), 30u);  // how much to tesselate the circle coordinates
    else
        SLICES = num_slices;
    const double HORZ_THETA = TWO_PI / SLICES;

    static std::map<int, std::vector<double>> unit_circle_coords;
    std::vector<double>& unit_vertices = unit_circle_coords[SLICES];
    bool calc_vertices = unit_vertices.size() == 0;
    if (calc_vertices) {
        unit_vertices.resize(2 * (SLICES + 1), 0.0);
        double theta = 0.0;
        for (std::size_t j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
            unit_vertices[j*2] = std::cos(-theta);
            unit_vertices[j*2+1] = std::sin(-theta);
        }
    }
    std::size_t first_slice_idx = static_cast<int>(theta1 / HORZ_THETA + 1);
    std::size_t last_slice_idx = static_cast<int>(
        theta2 / HORZ_THETA - 1 + ((theta1 >= theta2) ? SLICES : 0u));

    if (fan) {  // store a triangle fan vertex list, specifying each vertex just once
        if (filled_shape)   // central vertex first, to act as the pivot vertex for the fan
            buffer.store(static_cast<GLfloat>(center_x),
                         static_cast<GLfloat>(center_y));
        // if not filled_shape, assumes a previously-specified vertex in the buffer will act as the pivot for the fan

        // point on circle at angle theta1
        double theta1_x = std::cos(-theta1), theta1_y = std::sin(-theta1);
        buffer.store(static_cast<GLfloat>(center_x + theta1_x * r),
                     static_cast<GLfloat>(center_y + theta1_y * r));

        // angles in between theta1 and theta2, if any
        for (std::size_t i = first_slice_idx; i <= last_slice_idx + 1; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            buffer.store(static_cast<GLfloat>(center_x + unit_vertices[X] * r),
                         static_cast<GLfloat>(center_y + unit_vertices[Y] * r));
        }

        // theta2
        double theta2_x = std::cos(-theta2), theta2_y = std::sin(-theta2);
        buffer.store(static_cast<GLfloat>(center_x + theta2_x * r),
                     static_cast<GLfloat>(center_y + theta2_y * r));

    } else {    // (not a fan) store a list of complete lines / triangles
        // if storing a filled_shape, the first point in each triangle should be the centre of the arc
        std::pair<GLfloat, GLfloat> first_point{static_cast<GLfloat>(center_x),
                                                static_cast<GLfloat>(center_y)};
        // (not used for non-filled-shape)

        // angles in between theta1 and theta2, if any
        for (std::size_t i = first_slice_idx - 1; i <= last_slice_idx; ++i) {
            if (filled_shape) {
                buffer.store(first_point.first, first_point.second);
                // list of triangles: need two more vertices on the arc per starting vertex
            }
            // else: list of lines, with two vertices each

            int X = (i > SLICES ? (i - SLICES) : i) * 2;
            int Y = X + 1;
            buffer.store(static_cast<GLfloat>(center_x + unit_vertices[X] * r),
                         static_cast<GLfloat>(center_y + unit_vertices[Y] * r));

            auto next_i = i + 1;
            X = (next_i > SLICES ? (next_i - SLICES) : next_i) * 2;
            Y = X + 1;
            buffer.store(static_cast<GLfloat>(center_x + unit_vertices[X] * r),
                         static_cast<GLfloat>(center_y + unit_vertices[Y] * r));
        }

        // theta2
        if (filled_shape)
            buffer.store(first_point.first, first_point.second);

        auto i = last_slice_idx + 1;
        auto X = (i > SLICES ? (i - SLICES) : i) * 2;
        auto Y = X + 1;
        buffer.store(static_cast<GLfloat>(center_x + unit_vertices[X] * r),
                     static_cast<GLfloat>(center_y + unit_vertices[Y] * r));

        double theta2_x = std::cos(-theta2), theta2_y = std::sin(-theta2);
        buffer.store(static_cast<GLfloat>(center_x + theta2_x * r),
                     static_cast<GLfloat>(center_y + theta2_y * r));
    }
}

GG::Clr AdjustBrightness(GG::Clr color, int amount, bool jointly_capped)
{
    if (jointly_capped) {
        uint8_t max_val = std::max(std::max(color.r, color.g), color.b);
        amount = std::min(amount, 255-max_val);
    }
    return {
        std::max<uint8_t>(0, std::min(color.r + amount, 255)),
        std::max<uint8_t>(0, std::min(color.g + amount, 255)),
        std::max<uint8_t>(0, std::min(color.b + amount, 255)),
        color.a
    };
}

GG::Clr AdjustBrightness(GG::Clr color, double amount, bool jointly_capped)
{
    if (jointly_capped) {
        uint8_t max_val = std::max(std::max(color.r, color.g), color.b);
        amount = max_val > 0 ? std::min(amount, 255.0/max_val) : 255.0;
    }
    return {
        std::max<uint8_t>(0, std::min(color.r * amount, 255.0)),
        std::max<uint8_t>(0, std::min(color.g * amount, 255.0)),
        std::max<uint8_t>(0, std::min(color.b * amount, 255.0)),
        color.a
    };
}

GG::Clr OpaqueColor(GG::Clr color)
{
    color.a = 255;
    return color;
}

void BufferStoreRectangle(GG::GL2DVertexBuffer& buffer, GG::Rect area, GG::Rect border_thickness)
{
        GG::X inner_x1(area.ul.x + border_thickness.ul.x);
        GG::Y inner_y1(area.ul.y + border_thickness.ul.y);
        GG::X inner_x2(area.lr.x - border_thickness.lr.x);
        GG::Y inner_y2(area.lr.y - border_thickness.lr.y);

        buffer.reserve(14);
        buffer.store(inner_x2, inner_y1);
        buffer.store(area.lr.x, area.ul.y);
        buffer.store(inner_x1, inner_y1);
        buffer.store(area.ul.x, area.ul.y);
        buffer.store(inner_x1, inner_y2);
        buffer.store(area.ul.x, area.lr.y);
        buffer.store(inner_x2, inner_y2);
        buffer.store(area.lr.x, area.lr.y);
        buffer.store(inner_x2, inner_y1);
        buffer.store(area.lr.x, area.ul.y);

        buffer.store(inner_x2, inner_y1);
        buffer.store(inner_x1, inner_y1);
        buffer.store(inner_x1, inner_y2);
        buffer.store(inner_x2, inner_y2);
}

void AngledCornerRectangle(const GG::Pt ul, const GG::Pt lr, GG::Clr color, GG::Clr border,
                           int angle_offset, int thick, bool upper_left_angled,
                           bool lower_right_angled, bool draw_bottom)
{
    glDisable(GL_TEXTURE_2D);

    GG::GL2DVertexBuffer vert_buf;
    vert_buf.reserve(14);
    GG::Pt thick_pt = GG::Pt(GG::X(thick), GG::Y(thick));
    BufferStoreAngledCornerRectangleVertices(vert_buf, ul + thick_pt, lr - thick_pt, angle_offset,
                                             upper_left_angled, lower_right_angled, draw_bottom);

    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();

    glColor(color);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vert_buf.size());
    if (thick > 0) {
        glColor(border);
        glLineWidth(thick);
        glDrawArrays(GL_LINE_STRIP, 0, vert_buf.size());
        glLineWidth(1.0f);
    }

    glPopClientAttrib();

    glEnable(GL_TEXTURE_2D);
}

void BufferStoreAngledCornerRectangleVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                              const GG::Pt lr, int angle_offset,
                                              bool upper_left_angled, bool lower_right_angled,
                                              bool connect_bottom_line)
{
    // these are listed in CCW order
    if (connect_bottom_line)
        buffer.store(Value(ul.x),                   Value(lr.y));

    if (lower_right_angled) {
        buffer.store(Value(lr.x) - angle_offset - 3,Value(lr.y));   // don't know why, but - 3 here and the next line seem to make things symmetric top-left and bottom-right
        buffer.store(Value(lr.x),                   Value(lr.y) - angle_offset - 3);
    } else {
        buffer.store(Value(lr.x),                   Value(lr.y));
    }

    buffer.store(Value(lr.x),                       Value(ul.y));

    if (upper_left_angled) {
        buffer.store(Value(ul.x) + angle_offset,    Value(ul.y));
        buffer.store(Value(ul.x),                   Value(ul.y) + angle_offset);
    } else {
        buffer.store(Value(ul.x),                   Value(ul.y));
    }

    buffer.store(Value(ul.x),                       Value(lr.y));
}

bool InAngledCornerRect(const GG::Pt pt, const GG::Pt ul, const GG::Pt lr, int angle_offset,
                        bool upper_left_angled, bool lower_right_angled) noexcept
{
    bool retval = (ul <= pt) && (pt < lr);
    if (retval) {
        GG::Pt dist_from_ul = pt - ul;
        GG::Pt dist_from_lr = lr - pt;
        bool inside_upper_left_corner = upper_left_angled ? (angle_offset < Value(dist_from_ul.x) + Value(dist_from_ul.y)) : true;
        bool inside_lower_right_corner = lower_right_angled ? (angle_offset < Value(dist_from_lr.x) + Value(dist_from_lr.y)) : true;
        retval = inside_upper_left_corner && inside_lower_right_corner;
    }
    return retval;
}

void Triangle(double x1, double y1, double x2, double y2, double x3, double y3, GG::Clr color, bool border) {
    const GG::Clr border_clr = border ? AdjustBrightness(color, 75) : color;
    GG::Triangle(GG::Pt(GG::X(x1), GG::Y(y1)),
                 GG::Pt(GG::X(x2), GG::Y(y2)),
                 GG::Pt(GG::X(x3), GG::Y(y3)),
                 color, border ? border_clr : GG::CLR_ZERO);
}

bool InTriangle(const GG::Pt pt, double x1, double y1, double x2, double y2, double x3, double y3) noexcept {
    double vec_A_x = x2 - x1; // side A is the vector from pt1 to pt2
    double vec_A_y = y2 - y1; // side A is the vector from pt1 to pt2
    double vec_B_x = x3 - x2; // side B is the vector from pt2 to pt3
    double vec_B_y = y3 - y2; // side B is the vector from pt2 to pt3
    double vec_C_x = x1 - x3; // side C is the vector from pt3 to pt1
    double vec_C_y = y1 - y3; // side C is the vector from pt3 to pt1
    int pt_x = Value(pt.x);
    int pt_y = Value(pt.y);
    // take dot products of perpendicular vectors (normals of sides) with point pt, and sum the signs of these products
    int sum = (0 < (pt_x - x1) * vec_A_y + (pt_y - y1) * -vec_A_x ? 1 : 0) +
              (0 < (pt_x - x2) * vec_B_y + (pt_y - y2) * -vec_B_x ? 1 : 0) +
              (0 < (pt_x - x3) * vec_C_y + (pt_y - y3) * -vec_C_x ? 1 : 0);
    // if the products are all the same sign, the point is in the triangle
    return (sum == 3 || sum == 0);
}

void IsoscelesTriangle(const GG::Pt ul, const GG::Pt lr, ShapeOrientation orientation,
                       GG::Clr color, bool border)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(ul, lr, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    Triangle(x1_, y1_, x2_, y2_, x3_, y3_, color, border);
}

void BufferStoreIsoscelesTriangle(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                  const GG::Pt lr, ShapeOrientation orientation)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(ul, lr, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    buffer.store(x1_,   y1_);
    buffer.store(x2_,   y2_);
    buffer.store(x3_,   y3_);
}

bool InIsoscelesTriangle(const GG::Pt pt, const GG::Pt ul, const GG::Pt lr,
                         ShapeOrientation orientation) noexcept
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(ul, lr, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    return InTriangle(pt, x1_, y1_, x2_, y2_, x3_, y3_);
}

void CircleArc(const GG::Pt ul, const GG::Pt lr, double theta1, double theta2,
               bool filled_shape)
{
    //std::cout << "CircleArc ul: " << ul << "  lr: " << lr << "  theta1: " << theta1 << "  theta2: " << theta2 << "  filled: " << filled_shape << std::endl;
    GG::GL2DVertexBuffer vert_buf;
    vert_buf.reserve(31);   // max number that BufferStoreCircleArcVertices might add

    BufferStoreCircleArcVertices(vert_buf, ul, lr, theta1, theta2, filled_shape, 0, true);

    //glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();

    if (filled_shape)
        glDrawArrays(GL_TRIANGLE_FAN, 0, vert_buf.size());
    else
        glDrawArrays(GL_LINE_STRIP, 0, vert_buf.size());

    glPopClientAttrib();
    //glEnable(GL_TEXTURE_2D);
}

void CircleArcSegments(GG::Pt ul, GG::Pt lr, int segments, bool filled_shape) {
    if (segments < 2)
        return;

    GG::GL2DVertexBuffer vert_buf;
    vert_buf.reserve(5 * segments); // guesstimate for how many verts might be added

    constexpr double TWO_PI = 2.0*3.1415926536;

    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    float segment = static_cast<float>(TWO_PI) / segments;
    for (int n = 0; n < 24; n = n + 2) {
        auto theta1 = n * segment;
        auto theta2 = (n+1) * segment;

        BufferStoreCircleArcVertices(vert_buf, ul, lr, theta1, theta2, filled_shape, 0, false);
    }

    vert_buf.activate();

    if (filled_shape)
        glDrawArrays(GL_TRIANGLES, 0, vert_buf.size());
    else
        glDrawArrays(GL_LINES, 0, vert_buf.size());

    glPopClientAttrib();
}

void PartlyRoundedRect(const GG::Pt ul, const GG::Pt lr, int radius,
                       bool ur_round, bool ul_round,
                       bool ll_round, bool lr_round, bool fill)
{
    GG::GL2DVertexBuffer vert_buf;
    vert_buf.reserve(210);  // should be enough for 4 corners with 50 verts each, plus a bit extra to be safe

    BufferStorePartlyRoundedRectVertices(vert_buf, ul, lr, radius, ur_round,
                                         lr_round, ll_round, lr_round);

    //glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    glEnableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    vert_buf.activate();

    if (fill)
        glDrawArrays(GL_TRIANGLE_FAN, 0, vert_buf.size());
    else
        glDrawArrays(GL_LINE_LOOP, 0, vert_buf.size());

    glPopClientAttrib();
    //glEnable(GL_TEXTURE_2D);
}

void BufferStorePartlyRoundedRectVertices(GG::GL2DVertexBuffer& buffer, const GG::Pt ul,
                                          const GG::Pt lr, int radius, bool ur_round,
                                          bool ul_round, bool ll_round, bool lr_round)
{
    static constexpr double PI = 3.141594; // probably intentionally sightly more than Pi

    buffer.store(lr.x, ul.y + radius);

    if (ur_round)
        BufferStoreCircleArcVertices(buffer, GG::Pt(lr.x - 2 * radius, ul.y),
                                     GG::Pt(lr.x, ul.y + 2 * radius), 0.0, PI / 2.0, false);
    else
        buffer.store(lr.x, ul.y);

    if (ul_round)
        BufferStoreCircleArcVertices(buffer, ul, GG::Pt(ul.x + 2 * radius, ul.y + 2 * radius),
                                     PI / 2.0, PI, false);
    else
        buffer.store(ul.x, ul.y);

    if (ll_round)
        BufferStoreCircleArcVertices(buffer, GG::Pt(ul.x, lr.y - 2 * radius),
                                     GG::Pt(ul.x + 2 * radius, lr.y),
                                     PI, 3.0 * PI / 2.0, false);
    else
        buffer.store(ul.x, lr.y);

    if (lr_round)
        BufferStoreCircleArcVertices(buffer, GG::Pt(lr.x - 2 * radius, lr.y - 2 * radius),
                                     lr, 3.0 * PI / 2.0, 0.0, false);
    else
        buffer.store(lr.x, lr.y);

    buffer.store(lr.x, ul.y + radius);
}

namespace {
    constexpr double TWO_PI = 2.0 * 3.14159;
}

class ScanlineRenderer::Impl {
public:
    Impl() :
        m_scanline_shader(),
        m_failed_init(false)
    { m_color = GG::CLR_BLACK; }

    void StartUsing() {
        if (m_failed_init)
            return;

        if (!m_scanline_shader) {
            boost::filesystem::path shader_path = GetRootDataDir() / "default" / "shaders" / "scanlines.frag";
            std::string shader_text;
            if (!ReadFile(shader_path, shader_text)) {
                ErrorLogger() << "ScanlineRenderer failed to read shader at path " << shader_path.string();
                m_failed_init = true;
                return;
            }
            m_scanline_shader = ShaderProgram::shaderProgramFactory("", shader_text);

            if (!m_scanline_shader) {
                ErrorLogger() << "ScanlineRenderer failed to initialize shader.";
                m_failed_init = true;
                return;
            }
        }

        float fog_scanline_spacing = static_cast<float>(GetOptionsDB().Get<double>("ui.map.system.scanlines.spacing"));
        m_scanline_shader->Use();
        m_scanline_shader->Bind("scanline_spacing", fog_scanline_spacing);
        m_scanline_shader->Bind("line_color", m_color.r * (1.f / 255.f), m_color.g * (1.f / 255.f), m_color.b * (1.f / 255.f), m_color.a * (1.f / 255.f));
    }

    void SetColor(GG::Clr clr) noexcept { m_color = clr; }

    void StopUsing() { m_scanline_shader->stopUse(); }

    void RenderCircle(const GG::Pt ul, const GG::Pt lr) {
        StartUsing();
        CircleArc(ul, lr, 0.0, TWO_PI, true);
        StopUsing();
    }

    void RenderRectangle(const GG::Pt ul, const GG::Pt lr) {
        StartUsing();
        GG::FlatRectangle(ul, lr, GG::CLR_WHITE, GG::CLR_WHITE, 0u);
        StopUsing();
    }

    std::unique_ptr<ShaderProgram> m_scanline_shader;
    bool m_failed_init;
    GG::Clr m_color;
};


ScanlineRenderer::ScanlineRenderer() :
    m_impl(std::make_unique<Impl>())
{}

// This destructor is required here because ~ScanlineRendererImpl is declared here.
ScanlineRenderer::~ScanlineRenderer() = default;

void ScanlineRenderer::RenderCircle(const GG::Pt ul, const GG::Pt lr)
{ m_impl->RenderCircle(ul, lr); }

void ScanlineRenderer::RenderRectangle(const GG::Pt ul, const GG::Pt lr)
{ m_impl->RenderRectangle(ul, lr); }

void ScanlineRenderer::StartUsing()
{ m_impl->StartUsing(); }

void ScanlineRenderer::SetColor(GG::Clr clr) noexcept
{ m_impl->SetColor(clr); }

void ScanlineRenderer::StopUsing()
{ m_impl->StopUsing(); }
