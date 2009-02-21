#include "CUIDrawUtil.h"

#include "../util/MultiplayerCommon.h"

#include <GG/DrawUtil.h>

#include <cmath>


namespace {
    void FindIsoscelesTriangleVertices(const GG::Pt& ul, const GG::Pt& lr, ShapeOrientation orientation,
                                       double& x1_, double& y1_, double& x2_, double& y2_, double& x3_, double& y3_)
    {
        switch (orientation) {
        case SHAPE_UP:
            x1_ = Value(ul.x);
            y1_ = Value(lr.y);
            x2_ = Value(lr.x);
            y2_ = Value(lr.y);
            x3_ = Value((ul.x + lr.x) / 2.0);
            y3_ = Value(ul.y);
            break;
        case SHAPE_DOWN:
            x1_ = Value(lr.x);
            y1_ = Value(ul.y);
            x2_ = Value(ul.x);
            y2_ = Value(ul.y);
            x3_ = Value((ul.x + lr.x) / 2.0);
            y3_ = Value(lr.y);
            break;
        case SHAPE_LEFT:
            x1_ = Value(lr.x);
            y1_ = Value(lr.y);
            x2_ = Value(lr.x);
            y2_ = Value(ul.y);
            x3_ = Value(ul.x);
            y3_ = Value((ul.y + lr.y) / 2.0);
            break;
        case SHAPE_RIGHT:
            x1_ = Value(ul.x);
            y1_ = Value(ul.y);
            x2_ = Value(ul.x);
            y2_ = Value(lr.y);
            x3_ = Value(lr.x);
            y3_ = Value((ul.y + lr.y) / 2.0);
            break;
        }
    }

}

void AdjustBrightness(GG::Clr& color, int amount)
{
    color.r = std::max(0, std::min(color.r + amount, 255));
    color.g = std::max(0, std::min(color.g + amount, 255));
    color.b = std::max(0, std::min(color.b + amount, 255));
}

void AdjustBrightness(GG::Clr& color, double amount)
{
    color.r = std::max(0, std::min(static_cast<int>(color.r * amount), 255));
    color.g = std::max(0, std::min(static_cast<int>(color.g * amount), 255));
    color.b = std::max(0, std::min(static_cast<int>(color.b * amount), 255));
}

GG::Clr OpaqueColor(const GG::Clr& color)
{
    GG::Clr retval = color;
    retval.a = 255;
    return retval;
}

void AngledCornerRectangle(const GG::Pt& ul, const GG::Pt& lr, GG::Clr color, GG::Clr border, int angle_offset, int thick,
                           bool upper_left_angled/* = true*/, bool lower_right_angled/* = true*/, bool draw_bottom/* = true*/)
{
    glDisable(GL_TEXTURE_2D);

    GG::X inner_x1 = ul.x + thick;
    GG::Y inner_y1 = ul.y + thick;
    GG::X inner_x2 = lr.x - thick;
    GG::Y inner_y2 = lr.y - thick;

    // these are listed in CCW order for convenience
    GG::X ul_corner_x1 = ul.x + angle_offset;
    GG::Y ul_corner_y1 = ul.y;
    GG::X ul_corner_x2 = ul.x;
    GG::Y ul_corner_y2 = ul.y + angle_offset;
    GG::X lr_corner_x1 = lr.x - angle_offset;
    GG::Y lr_corner_y1 = lr.y;
    GG::X lr_corner_x2 = lr.x;
    GG::Y lr_corner_y2 = lr.y - angle_offset;

    GG::X inner_ul_corner_x1 = ul_corner_x1 + thick;
    GG::Y inner_ul_corner_y1 = ul_corner_y1 + thick;
    GG::X inner_ul_corner_x2 = ul_corner_x2 + thick;
    GG::Y inner_ul_corner_y2 = ul_corner_y2 + thick;
    GG::X inner_lr_corner_x1 = lr_corner_x1 - thick;
    GG::Y inner_lr_corner_y1 = lr_corner_y1 - thick;
    GG::X inner_lr_corner_x2 = lr_corner_x2 - thick;
    GG::Y inner_lr_corner_y2 = lr_corner_y2 - thick;

    // draw border
    if (thick) {
        glBegin(GL_QUADS);
        glColor(border);

        // the top
        glVertex(inner_x2, inner_y1);
        glVertex(lr.x, ul.y);
        if (upper_left_angled) {
            glVertex(ul_corner_x1, ul_corner_y1);
            glVertex(inner_ul_corner_x1, inner_ul_corner_y1);
        } else {
            glVertex(ul.x, ul.y);
            glVertex(inner_x1, inner_y1);
        }

        // the upper-left angled side
        if (upper_left_angled) {
            glVertex(inner_ul_corner_x1, inner_ul_corner_y1);
            glVertex(ul_corner_x1, ul_corner_y1);
            glVertex(ul_corner_x2, ul_corner_y2);
            glVertex(inner_ul_corner_x2, inner_ul_corner_y2);
        }

        // the left side
        if (upper_left_angled) {
            glVertex(inner_ul_corner_x2, inner_ul_corner_y2);
            glVertex(ul_corner_x2, ul_corner_y2);
        } else {
            glVertex(inner_x1, inner_y1);
            glVertex(ul.x, ul.y);
        }
        glVertex(ul.x, lr.y);
        glVertex(inner_x1, inner_y2);

        // the bottom
        if (draw_bottom) {
            glVertex(inner_x1, inner_y2);
            glVertex(ul.x, lr.y);
            if (lower_right_angled) {
                glVertex(lr_corner_x1, lr_corner_y1);
                glVertex(inner_lr_corner_x1, inner_lr_corner_y1);
            } else {
                glVertex(lr.x, lr.y);
                glVertex(inner_x2, inner_y2);
            }
        }

        // the lower-right angled side
        if (lower_right_angled) {
            glVertex(inner_lr_corner_x1, inner_lr_corner_y1);
            glVertex(lr_corner_x1, lr_corner_y1);
            glVertex(lr_corner_x2, lr_corner_y2);
            glVertex(inner_lr_corner_x2, inner_lr_corner_y2);
        }

        // the right side
        if (lower_right_angled) {
            glVertex(inner_lr_corner_x2, inner_lr_corner_y2);
            glVertex(lr_corner_x2, lr_corner_y2);
        } else {
            glVertex(lr.x, lr.y);
            glVertex(inner_x2, inner_y2);
        }
        glVertex(lr.x, ul.y);
        glVertex(inner_x2, inner_y1);

        glEnd();
    }

    // draw interior of rectangle
    glColor(color);
    glBegin(GL_POLYGON);
    if (upper_left_angled) {
        glVertex(inner_ul_corner_x1, inner_ul_corner_y1);
        glVertex(inner_ul_corner_x2, inner_ul_corner_y2);
    } else {
        glVertex(inner_x1, inner_y1);
    }
    glVertex(inner_x1, inner_y2);
    if (lower_right_angled) {
        glVertex(inner_lr_corner_x1, inner_lr_corner_y1);
        glVertex(inner_lr_corner_x2, inner_lr_corner_y2);
    } else {
        glVertex(inner_x2, inner_y2);
    }
    glVertex(inner_x2, inner_y1);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

bool InAngledCornerRect(const GG::Pt& pt, const GG::Pt& ul, const GG::Pt& lr, int angle_offset, 
                        bool upper_left_angled/* = true*/, bool lower_right_angled/* = true*/)
{
    bool retval = false;
    if (retval = (ul <= pt && pt < lr)) {
        GG::Pt dist_from_ul = pt - ul;
        GG::Pt dist_from_lr = lr - pt;
        bool inside_upper_left_corner = upper_left_angled ? (angle_offset < Value(dist_from_ul.x) + Value(dist_from_ul.y)) : true;
        bool inside_lower_right_corner = lower_right_angled ? (angle_offset < Value(dist_from_lr.x) + Value(dist_from_lr.y)) : true;
        retval = inside_upper_left_corner && inside_lower_right_corner;
    }
    return retval;
}

void Triangle(double x1, double y1, double x2, double y2, double x3, double y3, GG::Clr color, bool border/*= true*/)
{
    glDisable(GL_TEXTURE_2D);
    glColor(color);
    glBegin(GL_TRIANGLES);
    glVertex2d(x1, y1);
    glVertex2d(x2, y2);
    glVertex2d(x3, y3);
    glEnd();
    if (border) {
        AdjustBrightness(color, 75);
        // trace the lines both ways, to ensure that this small polygon looks symmetrical
        glColor(color);
        glBegin(GL_LINE_LOOP);
        glVertex2d(x3, y3);
        glVertex2d(x2, y2);
        glVertex2d(x1, y1);
        glEnd();
        glBegin(GL_LINE_LOOP);
        glVertex2d(x1, y1);
        glVertex2d(x2, y2);
        glVertex2d(x3, y3);
        glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

bool InTriangle(const GG::Pt& pt, double x1, double y1, double x2, double y2, double x3, double y3)
{
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

void IsoscelesTriangle(const GG::Pt& ul, const GG::Pt& lr, ShapeOrientation orientation, GG::Clr color, bool border/* = true*/)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(ul, lr, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    Triangle(x1_, y1_, x2_, y2_, x3_, y3_, color, border);
}

bool InIsoscelesTriangle(const GG::Pt& pt, const GG::Pt& ul, const GG::Pt& lr, ShapeOrientation orientation)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(ul, lr, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    return InTriangle(pt, x1_, y1_, x2_, y2_, x3_, y3_);
}

void CircleArc(const GG::Pt& ul, const GG::Pt& lr, double theta1, double theta2, bool filled_shape)
{
    int wd = Value(lr.x - ul.x), ht = Value(lr.y - ul.y);
    double center_x = Value(ul.x + wd / 2.0);
    double center_y = Value(ul.y + ht / 2.0);
    double r = std::min(wd / 2.0, ht / 2.0);
    const double PI = 3.141594;

    // correct theta* values to range [0, 2pi)
    if (theta1 < 0)
        theta1 += (int(-theta1 / (2 * PI)) + 1) * 2 * PI;
    else if (theta1 >= 2 * PI)
        theta1 -= int(theta1 / (2 * PI)) * 2 * PI;
    if (theta2 < 0)
        theta2 += (int(-theta2 / (2 * PI)) + 1) * 2 * PI;
    else if (theta2 >= 2 * PI)
        theta2 -= int(theta2 / (2 * PI)) * 2 * PI;

    const int      SLICES = std::min(std::max(12, 3 + std::max(wd, ht)), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
    const double   HORZ_THETA = (2 * PI) / SLICES;

    static std::map<int, std::vector<double> > unit_circle_coords;
    std::vector<double>& unit_vertices = unit_circle_coords[SLICES];
    bool calc_vertices = unit_vertices.size() == 0;
    if (calc_vertices) {
        unit_vertices.resize(2 * (SLICES + 1), 0.0);
        double theta = 0.0f;
        for (int j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
            unit_vertices[j*2] = std::cos(-theta);
            unit_vertices[j*2+1] = std::sin(-theta);
        }
    }
    int first_slice_idx = int(theta1 / HORZ_THETA + 1);
    int last_slice_idx = int(theta2 / HORZ_THETA - 1);
    if (theta1 >= theta2)
        last_slice_idx += SLICES;

    if (filled_shape) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(static_cast<GLfloat>(center_x), static_cast<GLfloat>(center_y));
    }
    // point on circle at angle theta1
    double theta1_x = std::cos(-theta1), theta1_y = std::sin(-theta1);
    glVertex2f(static_cast<GLfloat>(center_x + theta1_x * r), static_cast<GLfloat>(center_y + theta1_y * r));
    // angles in between theta1 and theta2, if any
    for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
        int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
        glVertex2f(static_cast<GLfloat>(center_x + unit_vertices[X] * r), static_cast<GLfloat>(center_y + unit_vertices[Y] * r));
    }
    // theta2
    double theta2_x = std::cos(-theta2), theta2_y = std::sin(-theta2);
    glVertex2f(static_cast<GLfloat>(center_x + theta2_x * r), static_cast<GLfloat>(center_y + theta2_y * r));
    if (filled_shape)
        glEnd();
}

void PartlyRoundedRect(const GG::Pt& ul, const GG::Pt& lr, int radius, bool ur_round, bool ul_round, bool ll_round, bool lr_round, bool fill)
{
    const double PI = 3.141594;
    if (fill) {
        if (ur_round)
            CircleArc(GG::Pt(lr.x - 2 * radius, ul.y), GG::Pt(lr.x, ul.y + 2 * radius), 0.0, PI / 2.0, fill);
        if (ul_round)
            CircleArc(ul, GG::Pt(ul.x + 2 * radius, ul.y + 2 * radius), PI / 2.0, PI, fill);
        if (ll_round)
            CircleArc(GG::Pt(ul.x, lr.y - 2 * radius), GG::Pt(ul.x + 2 * radius, lr.y), PI, 3.0 * PI / 2.0, fill);
        if (lr_round)
            CircleArc(GG::Pt(lr.x - 2 * radius, lr.y - 2 * radius), lr, 3.0 * PI / 2.0, 0.0, fill);
        glBegin(GL_QUADS);
        glVertex(lr.x, ul.y + radius);
        glVertex(ul.x, ul.y + radius);
        glVertex(ul.x, lr.y - radius);
        glVertex(lr.x, lr.y - radius);
        glVertex(lr.x - radius, ul.y);
        glVertex(ul.x + radius, ul.y);
        glVertex(ul.x + radius, ul.y + radius);
        glVertex(lr.x - radius, ul.y + radius);
        glVertex(lr.x - radius, lr.y - radius);
        glVertex(ul.x + radius, lr.y - radius);
        glVertex(ul.x + radius, lr.y);
        glVertex(lr.x - radius, lr.y);
        if (!ur_round) {
            glVertex(lr.x, ul.y);
            glVertex(lr.x - radius, ul.y);
            glVertex(lr.x - radius, ul.y + radius);
            glVertex(lr.x, ul.y + radius);
        }
        if (!ul_round) {
            glVertex(ul.x + radius, ul.y);
            glVertex(ul.x, ul.y);
            glVertex(ul.x, ul.y + radius);
            glVertex(ul.x + radius, ul.y + radius);
        }
        if (!ll_round) {
            glVertex(ul.x + radius, lr.y - radius);
            glVertex(ul.x, lr.y - radius);
            glVertex(ul.x, lr.y);
            glVertex(ul.x + radius, lr.y);
        }
        if (!lr_round) {
            glVertex(lr.x, lr.y - radius);
            glVertex(lr.x - radius, lr.y - radius);
            glVertex(lr.x - radius, lr.y);
            glVertex(lr.x, lr.y);
        }
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        if (ur_round)
            CircleArc(GG::Pt(lr.x - 2 * radius, ul.y), GG::Pt(lr.x, ul.y + 2 * radius), 0.0, PI / 2.0, fill);
        else
            glVertex(lr.x, ul.y);
        if (ul_round)
            CircleArc(ul, GG::Pt(ul.x + 2 * radius, ul.y + 2 * radius), PI / 2.0, PI, fill);
        else
            glVertex(ul.x, ul.y);
        if (ll_round)
            CircleArc(GG::Pt(ul.x, lr.y - 2 * radius), GG::Pt(ul.x + 2 * radius, lr.y), PI, 3.0 * PI / 2.0, fill);
        else
            glVertex(ul.x, lr.y);
        if (lr_round)
            CircleArc(GG::Pt(lr.x - 2 * radius, lr.y - 2 * radius), lr, 3.0 * PI / 2.0, 0.0, fill);
        else
            glVertex(lr.x, lr.y);
        glEnd();
    }
}
