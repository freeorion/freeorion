#include "CUIDrawUtil.h"

#include "../util/MultiplayerCommon.h"

namespace {
    void FindIsoscelesTriangleVertices(int x1, int y1, int x2, int y2, ShapeOrientation orientation,
                                       int& x1_, int& y1_, int& x2_, int& y2_, int& x3_, int& y3_)
    {
        switch (orientation) {
        case SHAPE_UP:
            x1_ = x1;
            y1_ = y2;
            x2_ = x2;
            y2_ = y2;
            x3_ = (x1 + x2) / 2;
            y3_ = y1;
            break;
        case SHAPE_DOWN:
            x1_ = x2;
            y1_ = y1;
            x2_ = x1;
            y2_ = y1;
            x3_ = (x1 + x2) / 2;
            y3_ = y2;
            break;
        case SHAPE_LEFT:
            x1_ = x2;
            y1_ = y2;
            x2_ = x2;
            y2_ = y1;
            x3_ = x1;
            y3_ = (y1 + y2) / 2;
            break;
        case SHAPE_RIGHT:
            x1_ = x1;
            y1_ = y1;
            x2_ = x1;
            y2_ = y2;
            x3_ = x2;
            y3_ = (y1 + y2) / 2;
            break;
        }
    }

    bool temp_header_bool = RecordHeaderFile(CUIDrawUtilRevision());
    bool temp_source_bool = RecordSourceFile("$RCSfile$", "$Revision$");
}

void AdjustBrightness(GG::Clr& color, int amount)
{
	color.r = (color.r + amount < 0 ? 0 : (255 < color.r + amount ? 255 : color.r + amount));
	color.g = (color.g + amount < 0 ? 0 : (255 < color.g + amount ? 255 : color.g + amount));
	color.b = (color.b + amount < 0 ? 0 : (255 < color.b + amount ? 255 : color.b + amount));
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

void Triangle(int x1, int y1, int x2, int y2, int x3, int y3, GG::Clr color, bool border/*= true*/)
{
    glDisable(GL_TEXTURE_2D);
    glColor4ubv(color.v);
    glBegin(GL_TRIANGLES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glVertex2i(x3, y3);
    glEnd();
    if (border) {
		AdjustBrightness(color, 75);
		// trace the lines both ways, to ensure that this small polygon looks symmetrical
        glColor4ubv(color.v);
		glBegin(GL_LINE_LOOP);
		glVertex2i(x3, y3);
		glVertex2i(x2, y2);
		glVertex2i(x1, y1);
		glEnd();
		glBegin(GL_LINE_LOOP);
		glVertex2i(x1, y1);
		glVertex2i(x2, y2);
		glVertex2i(x3, y3);
		glEnd();
    }
    glEnable(GL_TEXTURE_2D);
}

bool InTriangle(const GG::Pt& pt, int x1, int y1, int x2, int y2, int x3, int y3)
{
    GG::Pt vec_A(x2 - x1, y2 - y1); // side A is the vector from pt1 to pt2
    GG::Pt vec_B(x3 - x2, y3 - y2); // side B is the vector from pt2 to pt3
    GG::Pt vec_C(x1 - x3, y1 - y3); // side C is the vector from pt3 to pt1
    // take dot products of perpendicular vectors (normals of sides) with point pt, and sum the signs of these products
    int sum = (0 < (pt.x - x1) * vec_A.y + (pt.y - y1) * -vec_A.x ? 1 : 0) + 
              (0 < (pt.x - x2) * vec_B.y + (pt.y - y2) * -vec_B.x ? 1 : 0) +
              (0 < (pt.x - x3) * vec_C.y + (pt.y - y3) * -vec_C.x ? 1 : 0);
    // if the products are all the same sign, the point is in the triangle
    return (sum == 3 || sum == 0);
}

void IsoscelesTriangle(int x1, int y1, int x2, int y2, ShapeOrientation orientation, GG::Clr color, bool border/* = true*/)
{
    int x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(x1, y1, x2, y2, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    Triangle(x1_, y1_, x2_, y2_, x3_, y3_, color, border);
}

bool InIsoscelesTriangle(const GG::Pt& pt, int x1, int y1, int x2, int y2, ShapeOrientation orientation)
{
    int x1_, y1_, x2_, y2_, x3_, y3_;
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
    int x1_, y1_, x2_, y2_, x3_, y3_;
    FindIsoscelesTriangleVertices(x1, y1, x2, y2, orientation, x1_, y1_, x2_, y2_, x3_, y3_);
    return InTriangle(pt, x1_, y1_, x2_, y2_, x3_, y3_);
}
