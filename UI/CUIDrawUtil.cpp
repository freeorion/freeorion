#include "CUIDrawUtil.h"

#include "../util/MultiplayerCommon.h"

#include <cmath>

namespace {
    void FindIsoscelesTriangleVertices(int x1, int y1, int x2, int y2, ShapeOrientation orientation,
                                       double& x1_, double& y1_, double& x2_, double& y2_, double& x3_, double& y3_)
    {
        switch (orientation) {
        case SHAPE_UP:
            x1_ = x1;
            y1_ = y2;
            x2_ = x2;
            y2_ = y2;
            x3_ = (x1 + x2) / 2.0;
            y3_ = y1;
            break;
        case SHAPE_DOWN:
            x1_ = x2;
            y1_ = y1;
            x2_ = x1;
            y2_ = y1;
            x3_ = (x1 + x2) / 2.0;
            y3_ = y2;
            break;
        case SHAPE_LEFT:
            x1_ = x2;
            y1_ = y2;
            x2_ = x2;
            y2_ = y1;
            x3_ = x1;
            y3_ = (y1 + y2) / 2.0;
            break;
        case SHAPE_RIGHT:
            x1_ = x1;
            y1_ = y1;
            x2_ = x1;
            y2_ = y2;
            x3_ = x2;
            y3_ = (y1 + y2) / 2.0;
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

void AngledCornerRectangle(int x1, int y1, int x2, int y2, GG::Clr color, GG::Clr border, int angle_offset, int thick, 
                           bool upper_left_angled/* = true*/)
{
    glDisable(GL_TEXTURE_2D);

    int inner_x1 = x1 + thick;
    int inner_y1 = y1 + thick;
    int inner_x2 = x2 - thick;
    int inner_y2 = y2 - thick;

    // these are listed in CCW order for convenience
    int ul_corner_x1 = x1 + angle_offset;
    int ul_corner_y1 = y1;
    int ul_corner_x2 = x1;
    int ul_corner_y2 = y1 + angle_offset;
    int lr_corner_x1 = x2 - angle_offset;
    int lr_corner_y1 = y2;
    int lr_corner_x2 = x2;
    int lr_corner_y2 = y2 - angle_offset;

    int inner_ul_corner_x1 = ul_corner_x1 + thick;
    int inner_ul_corner_y1 = ul_corner_y1 + thick;
    int inner_ul_corner_x2 = ul_corner_x2 + thick;
    int inner_ul_corner_y2 = ul_corner_y2 + thick;
    int inner_lr_corner_x1 = lr_corner_x1 - thick;
    int inner_lr_corner_y1 = lr_corner_y1 - thick;
    int inner_lr_corner_x2 = lr_corner_x2 - thick;
    int inner_lr_corner_y2 = lr_corner_y2 - thick;

    // draw beveled edges
    if (thick) {
        glBegin(GL_QUADS);
        glColor4ubv(border.v);

        // the top
        glVertex2i(inner_x2, inner_y1);
        glVertex2i(x2, y1);
        if (upper_left_angled) {
            glVertex2i(ul_corner_x1, ul_corner_y1);
            glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
		} else {
            glVertex2i(x1, y1);
            glVertex2i(inner_x1, inner_y1);
        }

        // the upper-left angled side
        if (upper_left_angled) {
            glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
            glVertex2i(ul_corner_x1, ul_corner_y1);
            glVertex2i(ul_corner_x2, ul_corner_y2);
            glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
        }

        // the left side
        if (upper_left_angled) {
            glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
            glVertex2i(ul_corner_x2, ul_corner_y2);
        } else {
            glVertex2i(inner_x1, inner_y1);
            glVertex2i(x1, y1);
        }
        glVertex2i(x1, y2);
        glVertex2i(inner_x1, inner_y2);

        // the bottom
        glVertex2i(inner_x1, inner_y2);
        glVertex2i(x1, y2);
        glVertex2i(lr_corner_x1, lr_corner_y1);
        glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);

        // the lower-right angled side
        glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);
        glVertex2i(lr_corner_x1, lr_corner_y1);
        glVertex2i(lr_corner_x2, lr_corner_y2);
        glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);

        // the right side
        glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);
        glVertex2i(lr_corner_x2, lr_corner_y2);
        glVertex2i(x2, y1);
        glVertex2i(inner_x2, inner_y1);

        glEnd();
    }

    // draw interior of rectangle
    glColor4ubv(color.v);
    glBegin(GL_POLYGON);
    if (upper_left_angled) {
        glVertex2i(inner_ul_corner_x1, inner_ul_corner_y1);
        glVertex2i(inner_ul_corner_x2, inner_ul_corner_y2);
    } else {
        glVertex2i(inner_x1, inner_y1);
    }
    glVertex2i(inner_x1, inner_y2);
    glVertex2i(inner_lr_corner_x1, inner_lr_corner_y1);
    glVertex2i(inner_lr_corner_x2, inner_lr_corner_y2);
    glVertex2i(inner_x2, inner_y1);
    glEnd();

    glEnable(GL_TEXTURE_2D);
}

bool InAngledCornerRect(const GG::Pt& pt, int x1, int y1, int x2, int y2, int angle_offset, 
                        bool upper_left_angled/* = true*/)
{
    bool retval = false;
    GG::Pt ul(x1, y1);
    GG::Pt lr(x2, y2);
    if (retval = (ul <= pt && pt < lr)) {
        GG::Pt dist_from_ul = pt - ul;
        GG::Pt dist_from_lr = lr - pt;
        bool inside_upper_left_corner = upper_left_angled ? (angle_offset < dist_from_ul.x + dist_from_ul.y) : true;
        bool inside_lower_right_corner = angle_offset < dist_from_lr.x + dist_from_lr.y;
        retval = inside_upper_left_corner && inside_lower_right_corner;
    }
    return retval;
}

void Triangle(double x1, double y1, double x2, double y2, double x3, double y3, GG::Clr color, bool border/*= true*/)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ubv(color.v);
    glBegin(GL_TRIANGLES);
    glVertex2d(x1, y1);
    glVertex2d(x2, y2);
    glVertex2d(x3, y3);
    glEnd();
    if (border) {
		AdjustBrightness(color, 75);
		// trace the lines both ways, to ensure that this small polygon looks symmetrical
        glColor4ubv(color.v);
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
    // take dot products of perpendicular vectors (normals of sides) with point pt, and sum the signs of these products
    int sum = (0 < (pt.x - x1) * vec_A_y + (pt.y - y1) * -vec_A_x ? 1 : 0) + 
              (0 < (pt.x - x2) * vec_B_y + (pt.y - y2) * -vec_B_x ? 1 : 0) +
              (0 < (pt.x - x3) * vec_C_y + (pt.y - y3) * -vec_C_x ? 1 : 0);
    // if the products are all the same sign, the point is in the triangle
    return (sum == 3 || sum == 0);
}

void IsoscelesTriangle(int x1, int y1, int x2, int y2, ShapeOrientation orientation, GG::Clr color, bool border/* = true*/)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(x1, y1, x2, y2, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    Triangle(x1_, y1_, x2_, y2_, x3_, y3_, color, border);
}

bool InIsoscelesTriangle(const GG::Pt& pt, int x1, int y1, int x2, int y2, ShapeOrientation orientation)
{
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(x1, y1, x2, y2, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    return InTriangle(pt, x1_, y1_, x2_, y2_, x3_, y3_);
}

void FleetMarker(int x1, int y1, int x2, int y2, ShapeOrientation orientation, GG::Clr color)
{
    if (orientation == SHAPE_UP || orientation == SHAPE_DOWN)
        orientation = SHAPE_LEFT;

    // make sure triangle width is odd, for symmetry
    if (!((y2 - y1) % 2))
        ++y2;

    double point = orientation == SHAPE_LEFT ? x1 : x2;
    double base =  orientation == SHAPE_LEFT ? x2 : x1;
    double cutout_point = base + (point - base) * 0.2;
    double top = y1;
    double middle = (y2 + y1) * 0.5;
    double bottom = y2;
    double cutout_top = top + (y2 - y1) * 0.25;
    double cutout_bottom = bottom - (y2 - y1) * 0.25;

    GG::Clr border_color = color;
    AdjustBrightness(border_color, 75);
    border_color.a = 255;
    if (orientation == SHAPE_LEFT) {
        glColor4ubv(color.v);
        glBegin(GL_QUADS);
        glVertex2d(cutout_point, middle);
        glVertex2d(base, cutout_top);
        glVertex2d(base, top);
        glVertex2d(point, middle);
        glVertex2d(point, middle);
        glVertex2d(base, bottom);
        glVertex2d(base, cutout_bottom);
        glVertex2d(cutout_point, middle);
        glEnd();

        glColor4ubv(border_color.v);
        glBegin(GL_LINE_STRIP);
        glVertex2d(cutout_point, middle);
        glVertex2d(base, cutout_top);
        glVertex2d(base, top);
        glVertex2d(point, middle);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glVertex2d(point, middle);
        glVertex2d(base, bottom);
        glVertex2d(base, cutout_bottom);
        glVertex2d(cutout_point, middle);
        glEnd();
    } else {
        glColor4ubv(color.v);
        glBegin(GL_QUADS);
        glVertex2d(point, middle);
        glVertex2d(base, top);
        glVertex2d(base, cutout_top);
        glVertex2d(cutout_point, middle);
        glVertex2d(cutout_point, middle);
        glVertex2d(base, cutout_bottom);
        glVertex2d(base, bottom);
        glVertex2d(point, middle);
        glEnd();

        glColor4ubv(border_color.v);
        glBegin(GL_LINE_STRIP);
        glVertex2d(point, middle);
        glVertex2d(base, top);
        glVertex2d(base, cutout_top);
        glVertex2d(cutout_point, middle);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glVertex2d(cutout_point, middle);
        glVertex2d(base, cutout_bottom);
        glVertex2d(base, bottom);
        glVertex2d(point, middle);
        glEnd();
    }
}

bool InFleetMarker(const GG::Pt& pt, int x1, int y1, int x2, int y2, ShapeOrientation orientation)
{
    if (orientation == SHAPE_UP || orientation == SHAPE_DOWN)
        orientation = SHAPE_LEFT;
    double x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(x1, y1, x2, y2, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    return InTriangle(pt, x1_, y1_, x2_, y2_, x3_, y3_);
}

void CircleArc(int x1, int y1, int x2, int y2, double theta1, double theta2, bool filled_shape)
{
    int wd = x2 - x1, ht = y2 - y1;
    double center_x = x1 + wd / 2.0;
    double center_y = y1 + ht / 2.0;
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

    const int      SLICES = std::min(3 + std::max(wd, ht), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
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
        glVertex2f(center_x, center_y);
    }
    // point on circle at angle theta1
    double theta1_x = std::cos(-theta1), theta1_y = std::sin(-theta1);
    glVertex2f(center_x + theta1_x * r, center_y + theta1_y * r);
    // angles in between theta1 and theta2, if any
    for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
        int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
        glVertex2f(center_x + unit_vertices[X] * r, center_y + unit_vertices[Y] * r);
    }
    // theta2
    double theta2_x = std::cos(-theta2), theta2_y = std::sin(-theta2);
    glVertex2f(center_x + theta2_x * r, center_y + theta2_y * r);
    if (filled_shape)
        glEnd();
}

void PartlyRoundedRect(const GG::Pt& ul, const GG::Pt& lr, int radius, bool ur_round, bool ul_round, bool ll_round, bool lr_round, bool fill)
{
    const double PI = 3.141594;
    if (fill) {
        if (ur_round)
            CircleArc(lr.x - 2 * radius, ul.y, lr.x, ul.y + 2 * radius, 0.0, PI / 2.0, fill);
        if (ul_round)
            CircleArc(ul.x, ul.y, ul.x + 2 * radius, ul.y + 2 * radius, PI / 2.0, PI, fill);
        if (ll_round)
            CircleArc(ul.x, lr.y - 2 * radius, ul.x + 2 * radius, lr.y, PI, 3.0 * PI / 2.0, fill);
        if (lr_round)
            CircleArc(lr.x - 2 * radius, lr.y - 2 * radius, lr.x, lr.y, 3.0 * PI / 2.0, 0.0, fill);
        glBegin(GL_QUADS);
        glVertex2i(lr.x, ul.y + radius);
        glVertex2i(ul.x, ul.y + radius);
        glVertex2i(ul.x, lr.y - radius);
        glVertex2i(lr.x, lr.y - radius);
        glVertex2i(lr.x - radius, ul.y);
        glVertex2i(ul.x + radius, ul.y);
        glVertex2i(ul.x + radius, ul.y + radius);
        glVertex2i(lr.x - radius, ul.y + radius);
        glVertex2i(lr.x - radius, lr.y - radius);
        glVertex2i(ul.x + radius, lr.y - radius);
        glVertex2i(ul.x + radius, lr.y);
        glVertex2i(lr.x - radius, lr.y);
        if (!ur_round) {
            glVertex2i(lr.x, ul.y);
            glVertex2i(lr.x - radius, ul.y);
            glVertex2i(lr.x - radius, ul.y + radius);
            glVertex2i(lr.x, ul.y + radius);
        }
        if (!ul_round) {
            glVertex2i(ul.x + radius, ul.y);
            glVertex2i(ul.x, ul.y);
            glVertex2i(ul.x, ul.y + radius);
            glVertex2i(ul.x + radius, ul.y + radius);
        }
        if (!ll_round) {
            glVertex2i(ul.x + radius, lr.y - radius);
            glVertex2i(ul.x, lr.y - radius);
            glVertex2i(ul.x, lr.y);
            glVertex2i(ul.x + radius, lr.y);
        }
        if (!lr_round) {
            glVertex2i(lr.x, lr.y - radius);
            glVertex2i(lr.x - radius, lr.y - radius);
            glVertex2i(lr.x - radius, lr.y);
            glVertex2i(lr.x, lr.y);
        }
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        if (ur_round)
            CircleArc(lr.x - 2 * radius, ul.y, lr.x, ul.y + 2 * radius, 0.0, PI / 2.0, fill);
        else
            glVertex2i(lr.x, ul.y);
        if (ul_round)
            CircleArc(ul.x, ul.y, ul.x + 2 * radius, ul.y + 2 * radius, PI / 2.0, PI, fill);
        else
            glVertex2i(ul.x, ul.y);
        if (ll_round)
            CircleArc(ul.x, lr.y - 2 * radius, ul.x + 2 * radius, lr.y, PI, 3.0 * PI / 2.0, fill);
        else
            glVertex2i(ul.x, lr.y);
        if (lr_round)
            CircleArc(lr.x - 2 * radius, lr.y - 2 * radius, lr.x, lr.y, 3.0 * PI / 2.0, 0.0, fill);
        else
            glVertex2i(lr.x, lr.y);
        glEnd();
    }
}
