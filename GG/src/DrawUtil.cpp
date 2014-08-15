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

#include <GG/DrawUtil.h>

#include <GG/ClrConstants.h>
#include <GG/GUI.h>

#include <valarray>

namespace { // file-scope constants and functions
    using namespace GG;

    const double   PI = 3.14159426;
    const double   SQRT2OVER2 = std::sqrt(2.0) / 2.0;

    /// a stack of the currently-active clipping rects, in GG coordinates, not OpenGL scissor coordinates
    std::vector<Rect> g_scissor_clipping_rects;

    GLboolean g_prev_color_writemask[4];
    GLboolean g_prev_depth_writemask;

    /// the index of the next stencil bit to use for stencil clipping
    unsigned int g_stencil_bit = 0;

    /// whenever points on the unit circle are calculated with expensive sin() and cos() calls, the results are cached here
    std::map<int, std::valarray<double> > unit_circle_coords;
    /// this doesn't serve as a cache, but does allow us to prevent numerous constructions and destructions of Clr valarrays.
    std::map<int, std::valarray<Clr> > color_arrays;

    void Rectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2, unsigned int bevel_thick,
                   bool bevel_left, bool bevel_top, bool bevel_right, bool bevel_bottom)
    {
        glDisable(GL_TEXTURE_2D);

        X inner_x1 = ul.x + (bevel_left ? static_cast<int>(bevel_thick) : 0);
        Y inner_y1 = ul.y + (bevel_top ? static_cast<int>(bevel_thick) : 0);
        X inner_x2 = lr.x - (bevel_right ? static_cast<int>(bevel_thick) : 0);
        Y inner_y2 = lr.y - (bevel_bottom ? static_cast<int>(bevel_thick) : 0);

        int vertices[] = {
            Value(inner_x2), Value(inner_y1),
            Value(lr.x), Value(ul.y),
            Value(inner_x1), Value(inner_y1),
            Value(ul.x), Value(ul.y),
            Value(inner_x1), Value(inner_y2),
            Value(ul.x), Value(lr.y),
            Value(inner_x2), Value(inner_y2),
            Value(lr.x), Value(lr.y),
            Value(inner_x2), Value(inner_y1),
            Value(lr.x), Value(ul.y)
        };

        // draw beveled edges
        if (bevel_thick && (border_color1 != CLR_ZERO || border_color2 != CLR_ZERO)) {
            glColor(border_color1);
            if (border_color1 == border_color2) {
                glBegin(GL_QUAD_STRIP);
                for (int i = 0; i < 10; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
            } else {
                glBegin(GL_QUAD_STRIP);
                for (int i = 0; i < 6; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
                glColor(border_color2);
                glBegin(GL_QUAD_STRIP);
                for (int i = 4; i < 10; ++i) {
                    glVertex2i(vertices[i * 2 + 0], vertices[i * 2 + 1]);
                }
                glEnd();
            }
        }

        // draw interior of rectangle
        if (color != CLR_ZERO) {
            glColor(color);
            glBegin(GL_QUADS);
            glVertex(inner_x2, inner_y1);
            glVertex(inner_x1, inner_y1);
            glVertex(inner_x1, inner_y2);
            glVertex(inner_x2, inner_y2);
            glEnd();
        }

        glEnable(GL_TEXTURE_2D);
    }

    void Check(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3)
    {
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;
        glDisable(GL_TEXTURE_2D);

        // all vertices
        double verts[][2] = {{-0.2, 0.2}, {-0.6, -0.2}, {-0.6, 0.0}, {-0.2, 0.4}, {-0.8, 0.0},
                             {-0.2, 0.6}, { 0.8, -0.4}, {0.6, -0.4}, {0.8, -0.8}};

        glPushMatrix();
        const double sf = 1.25; // just a scale factor to make the check look right

        // move origin to the center of the rectangle
        glTranslated(Value(ul.x + wd / 2.0), Value(ul.y + ht / 2.0 * sf), 0.0);
        // map the range [-1,1] to the rectangle in both directions
        glScaled(Value(wd / 2.0 * sf), Value(ht / 2.0 * sf), 1.0);

        glColor(color3);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[2]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[7]);
        glEnd();

        glColor(color2);
        glBegin(GL_QUADS);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[6]);
        glEnd();

        glColor(color1);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[6]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[3]);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void XMark(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3)
    {
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;
        glDisable(GL_TEXTURE_2D);

        // all vertices
        double verts[][2] = {{-0.4, -0.6}, {-0.6, -0.4}, {-0.4, -0.4}, {-0.2, 0.0}, {-0.6, 0.4},
                             {-0.4, 0.6}, {-0.4, 0.4}, {0.0, 0.2}, {0.4, 0.6}, {0.6, 0.4},
                             {0.4, 0.4}, {0.2, 0.0}, {0.6, -0.4}, {0.4, -0.6}, {0.4, -0.4},
                             {0.0, -0.2}, {0.0, 0.0}};

        glPushMatrix();
        const double sf = 1.75; // just a scale factor; the check wasn't the right size as drawn originally
        glTranslatef(Value(ul.x + wd / 2.0), Value(ul.y + ht / 2.0), 0.0); // move origin to the center of the rectangle
        glScalef(Value(wd / 2.0 * sf), Value(ht / 2.0 * sf), 1.0); // map the range [-1,1] to the rectangle in both directions

        glColor(color1);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[12]);
        glVertex2dv(verts[13]);
        glVertex2dv(verts[14]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[15]);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[9]);
        glVertex2dv(verts[11]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[10]);
        glEnd();

        glColor(color2);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[0]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[2]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[13]);
        glVertex2dv(verts[15]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[14]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[16]);
        glEnd();

        glColor(color3);
        glBegin(GL_TRIANGLES);
        glVertex2dv(verts[4]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[9]);
        glVertex2dv(verts[10]);
        glEnd();
        glBegin(GL_QUADS);
        glVertex2dv(verts[14]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[11]);
        glVertex2dv(verts[12]);
        glVertex2dv(verts[2]);
        glVertex2dv(verts[1]);
        glVertex2dv(verts[3]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[6]);
        glVertex2dv(verts[5]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[16]);
        glVertex2dv(verts[7]);
        glVertex2dv(verts[8]);
        glVertex2dv(verts[10]);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void BubbleArc(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3, double theta1, double theta2)
    {
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;
        glDisable(GL_TEXTURE_2D);

        // correct theta* values to range [0, 2pi)
        if (theta1 < 0)
            theta1 += (int(-theta1 / (2 * PI)) + 1) * 2 * PI;
        else if (theta1 >= 2 * PI)
            theta1 -= int(theta1 / (2 * PI)) * 2 * PI;
        if (theta2 < 0)
            theta2 += (int(-theta2 / (2 * PI)) + 1) * 2 * PI;
        else if (theta2 >= 2 * PI)
            theta2 -= int(theta2 / (2 * PI)) * 2 * PI;

        const int      SLICES = std::min(3 + std::max(Value(wd), Value(ht)), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
        const double   HORZ_THETA = (2 * PI) / SLICES;

        std::valarray<double>& unit_vertices = unit_circle_coords[SLICES];
        std::valarray<Clr>&    colors = color_arrays[SLICES];
        bool calc_vertices = unit_vertices.size() == 0;
        if (calc_vertices) {
            unit_vertices.resize(2 * (SLICES + 1), 0.0);
            double theta = 0.0f;
            for (int j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
                unit_vertices[j*2] = cos(-theta);
                unit_vertices[j*2+1] = sin(-theta);
            }
            colors.resize(SLICES + 1, Clr()); // create but don't initialize (this is essentially just scratch space, since the colors are different call-to-call)
        }
        int first_slice_idx = int(theta1 / HORZ_THETA + 1);
        int last_slice_idx = int(theta2 / HORZ_THETA - 1);
        if (theta1 >= theta2)
            last_slice_idx += SLICES;
        for (int j = first_slice_idx; j <= last_slice_idx; ++j) { // calculate the color value for each needed point
            int X = (j > SLICES ? (j - SLICES) : j) * 2, Y = X + 1;
            double color_scale_factor = (SQRT2OVER2 * (unit_vertices[X] + unit_vertices[Y]) + 1) / 2; // this is essentially the dot product of (x,y) with (sqrt2over2,sqrt2over2), the direction of the light source, scaled to the range [0,1]
            colors[j] = Clr(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                            GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                            GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                            GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        }

        glPushMatrix();
        glTranslatef(Value(ul.x + wd / 2.0), Value(ul.y + ht / 2.0), 0.0);   // move origin to the center of the rectangle
        glScalef(Value(wd / 2.0), Value(ht / 2.0), 1.0);                 // map the range [-1,1] to the rectangle in both (x- and y-) directions

        glColor(color1);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        // point on circle at angle theta1
        double x = cos(-theta1),
            y = sin(-theta1);
        double color_scale_factor = (SQRT2OVER2 * (x + y) + 1) / 2;
        glColor4ub(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                   GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                   GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                   GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glVertex2f(x, y);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glColor(colors[i]);
            glVertex2f(unit_vertices[X], unit_vertices[Y]);
        }
        // theta2
        x = cos(-theta2);
        y = sin(-theta2);
        color_scale_factor = (SQRT2OVER2 * (x + y) + 1) / 2;
        glColor4ub(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                   GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                   GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                   GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glVertex2f(x, y);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void CircleArc(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2, unsigned int bevel_thick, double theta1, double theta2)
    {
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;
        glDisable(GL_TEXTURE_2D);

        // correct theta* values to range [0, 2pi)
        if (theta1 < 0)
            theta1 += (int(-theta1 / (2 * PI)) + 1) * 2 * PI;
        else if (theta1 >= 2 * PI)
            theta1 -= int(theta1 / (2 * PI)) * 2 * PI;
        if (theta2 < 0)
            theta2 += (int(-theta2 / (2 * PI)) + 1) * 2 * PI;
        else if (theta2 >= 2 * PI)
            theta2 -= int(theta2 / (2 * PI)) * 2 * PI;

        const int      SLICES = std::min(3 + std::max(Value(wd), Value(ht)), 50);  // this is a good guess at how much to tesselate the circle coordinates (50 segments max)
        const double   HORZ_THETA = (2 * PI) / SLICES;

        std::valarray<double>& unit_vertices = unit_circle_coords[SLICES];
        std::valarray<Clr>&    colors = color_arrays[SLICES];
        bool calc_vertices = unit_vertices.size() == 0;
        if (calc_vertices) {
            unit_vertices.resize(2 * (SLICES + 1), 0.0);
            double theta = 0.0f;
            for (int j = 0; j <= SLICES; theta += HORZ_THETA, ++j) { // calculate x,y values for each point on a unit circle divided into SLICES arcs
                unit_vertices[j*2] = cos(-theta);
                unit_vertices[j*2+1] = sin(-theta);
            }
            colors.resize(SLICES + 1, Clr()); // create but don't initialize (this is essentially just scratch space, since the colors are different call-to-call)
        }
        int first_slice_idx = int(theta1 / HORZ_THETA + 1);
        int last_slice_idx = int(theta2 / HORZ_THETA - 1);
        if (theta1 >= theta2)
            last_slice_idx += SLICES;
        for (int j = first_slice_idx; j <= last_slice_idx; ++j) { // calculate the color value for each needed point
            int X = (j > SLICES ? (j - SLICES) : j) * 2, Y = X + 1;
            double color_scale_factor = (SQRT2OVER2 * (unit_vertices[X] + unit_vertices[Y]) + 1) / 2; // this is essentially the dot product of (x,y) with (sqrt2over2,sqrt2over2), the direction of the light source, scaled to the range [0,1]
            colors[j] = Clr(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                            GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                            GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                            GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        }

        glPushMatrix();
        glTranslatef(Value(ul.x + wd / 2.0), Value(ul.y + ht / 2.0), 0.0);   // move origin to the center of the rectangle
        glScalef(Value(wd / 2.0), Value(ht / 2.0), 1.0);                 // map the range [-1,1] to the rectangle in both (x- and y-) directions

        double inner_radius = (std::min(Value(wd), Value(ht)) - 2.0 * bevel_thick) / std::min(Value(wd), Value(ht));
        glColor(color);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0, 0);
        // point on circle at angle theta1
        double theta1_x = cos(-theta1), theta1_y = sin(-theta1);
        glVertex2f(theta1_x * inner_radius, theta1_y * inner_radius);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glVertex2f(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
        }      // theta2
        double theta2_x = cos(-theta2), theta2_y = sin(-theta2);
        glVertex2f(theta2_x * inner_radius, theta2_y * inner_radius);
        glEnd();
        glBegin(GL_QUAD_STRIP);
        // point on circle at angle theta1
        double color_scale_factor = (SQRT2OVER2 * (theta1_x + theta1_y) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glVertex2f(theta1_x, theta1_y);
        glVertex2f(theta1_x * inner_radius, theta1_y * inner_radius);
        // angles in between theta1 and theta2, if any
        for (int i = first_slice_idx; i <= last_slice_idx; ++i) {
            int X = (i > SLICES ? (i - SLICES) : i) * 2, Y = X + 1;
            glColor(colors[i]);
            glVertex2f(unit_vertices[X], unit_vertices[Y]);
            glVertex2f(unit_vertices[X] * inner_radius, unit_vertices[Y] * inner_radius);
        }
        // theta2
        color_scale_factor = (SQRT2OVER2 * (theta2_x + theta2_y) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glVertex2f(theta2_x, theta2_y);
        glVertex2f(theta2_x * inner_radius, theta2_y * inner_radius);
        glEnd();
        glPopMatrix();
        glEnable(GL_TEXTURE_2D);
    }

    void RoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2, unsigned int corner_radius, int thick)
    {
        int circle_diameter = corner_radius * 2;
        CircleArc(Pt(lr.x - circle_diameter, ul.y), Pt(lr.x, ul.y + circle_diameter), color, border_color2, border_color1, thick, 0, 0.5 * PI);  // ur corner
        CircleArc(Pt(ul.x, ul.y), Pt(ul.x + circle_diameter, ul.y + circle_diameter), color, border_color2, border_color1, thick, 0.5 * PI, PI); // ul corner
        CircleArc(Pt(ul.x, lr.y - circle_diameter), Pt(ul.x + circle_diameter, lr.y), color, border_color2, border_color1, thick, PI, 1.5 * PI); // ll corner
        CircleArc(Pt(lr.x - circle_diameter, lr.y - circle_diameter), Pt(lr.x, lr.y), color, border_color2, border_color1, thick, 1.5 * PI, 0);  // lr corner

        glDisable(GL_TEXTURE_2D);

        // top
        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glBegin(GL_QUADS);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y);
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y);
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + thick);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + thick);
        glEnd();

        // left (uses color scale factor (SQRT2OVER2 * (1 + 0) + 1) / 2, which equals that of top
        glBegin(GL_QUADS);
        glVertex(ul.x + thick, ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x, ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x, lr.y - static_cast<int>(corner_radius));
        glVertex(ul.x + thick, lr.y - static_cast<int>(corner_radius));
        glEnd();

        // right
        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        glColor4ub(GLubyte(border_color2.r * (1 - color_scale_factor) + border_color1.r * color_scale_factor),
                   GLubyte(border_color2.g * (1 - color_scale_factor) + border_color1.g * color_scale_factor),
                   GLubyte(border_color2.b * (1 - color_scale_factor) + border_color1.b * color_scale_factor),
                   GLubyte(border_color2.a * (1 - color_scale_factor) + border_color1.a * color_scale_factor));
        glBegin(GL_QUADS);
        glVertex(lr.x, ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - thick, ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - thick, lr.y - static_cast<int>(corner_radius));
        glVertex(lr.x, lr.y - static_cast<int>(corner_radius));
        glEnd();

        // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of left
        glBegin(GL_QUADS);
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - thick);
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - thick);
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y);
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y);
        glEnd();

        // middle
        glColor(color);
        glBegin(GL_QUADS);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + thick);
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + thick);
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - thick);
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - thick);

        glVertex(lr.x - thick, ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glVertex(lr.x - thick, lr.y - static_cast<int>(corner_radius));

        glVertex(ul.x + thick, ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glVertex(ul.x + thick, lr.y - static_cast<int>(corner_radius));
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }

    void BubbleRectangle(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3, unsigned int corner_radius)
    {
        int circle_diameter = corner_radius * 2;
        BubbleArc(Pt(lr.x - circle_diameter, ul.y), Pt(lr.x, ul.y + circle_diameter), color1, color3, color2, 0, 0.5 * PI);  // ur corner
        BubbleArc(Pt(ul.x, ul.y), Pt(ul.x + circle_diameter, ul.y + circle_diameter), color1, color3, color2, 0.5 * PI, PI); // ul corner
        BubbleArc(Pt(ul.x, lr.y - circle_diameter), Pt(ul.x + circle_diameter, lr.y), color1, color3, color2, PI, 1.5 * PI); // ll corner
        BubbleArc(Pt(lr.x - circle_diameter, lr.y - circle_diameter), Pt(lr.x, lr.y), color1, color3, color2, 1.5 * PI, 0);  // lr corner

        glDisable(GL_TEXTURE_2D);

        // top
        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        Clr scaled_color(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                         GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                         GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                         GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glBegin(GL_QUADS);
        glColor(scaled_color);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y);
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y);
        glColor(color1);
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glEnd();

        // left (uses color scale factor (SQRT2OVER2 * (1 + 0) + 1) / 2, which equals that of top
        glBegin(GL_QUADS);
        glColor(scaled_color);
        glVertex(ul.x, ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x, lr.y - static_cast<int>(corner_radius));
        glColor(color1);
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glEnd();

        // right
        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        scaled_color = Clr(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                           GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                           GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                           GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glColor(scaled_color);
        glVertex(lr.x, lr.y - static_cast<int>(corner_radius));
        glVertex(lr.x, ul.y + static_cast<int>(corner_radius));
        glEnd();

        // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of left
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glColor(scaled_color);
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y);
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y);
        glEnd();

        // middle
        glBegin(GL_QUADS);
        glColor(color1);
        glVertex(lr.x - static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), ul.y + static_cast<int>(corner_radius));
        glVertex(ul.x + static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glVertex(lr.x - static_cast<int>(corner_radius), lr.y - static_cast<int>(corner_radius));
        glEnd();
        glEnable(GL_TEXTURE_2D);
    }
} // namespace


namespace GG {

    void glColor(Clr clr)
    { glColor4ub(clr.r, clr.g, clr.b, clr.a); }

    void glVertex(const Pt& pt)
    { glVertex2i(Value(pt.x), Value(pt.y)); }

    void glVertex(X x, Y y)
    { glVertex2i(Value(x), Value(y)); }

    void glVertex(X_d x, Y_d y)
    { glVertex2d(Value(x), Value(y)); }

    void glVertex(X x, Y_d y)
    { glVertex2d(Value(x), Value(y)); }

    void glVertex(X_d x, Y y)
    { glVertex2d(Value(x), Value(y)); }

    Clr LightColor(Clr clr)
    {
        const double scale_factor = 2.0;   // factor by which the color is lightened
        Clr retval = clr;
        retval.r = std::min(static_cast<int>(retval.r * scale_factor), 255);
        retval.g = std::min(static_cast<int>(retval.g * scale_factor), 255);
        retval.b = std::min(static_cast<int>(retval.b * scale_factor), 255);
        return retval;
    }

    Clr DarkColor(Clr clr)
    {
        const double scale_factor = 2.0;   // factor by which the color is darkened
        Clr retval = clr;
        retval.r = static_cast<int>(retval.r / scale_factor);
        retval.g = static_cast<int>(retval.g / scale_factor);
        retval.b = static_cast<int>(retval.b / scale_factor);
        return retval;
    }

    Clr DisabledColor(Clr clr)
    {
        Clr retval = clr;
        const double gray_factor = 0.75; // amount to move clr in the direction of gray
        retval.r = static_cast<int>(retval.r + (CLR_GRAY.r - retval.r) * gray_factor);
        retval.g = static_cast<int>(retval.g + (CLR_GRAY.g - retval.g) * gray_factor);
        retval.b = static_cast<int>(retval.b + (CLR_GRAY.b - retval.b) * gray_factor);
        return retval;
    }

    void BeginScissorClipping(Pt ul, Pt lr)
    {
        if (g_scissor_clipping_rects.empty()) {
            glPushAttrib(GL_SCISSOR_BIT);
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
        g_scissor_clipping_rects.push_back(Rect(ul, lr));
    }

    void EndScissorClipping()
    {
        assert(!g_scissor_clipping_rects.empty());
        g_scissor_clipping_rects.pop_back();
        if (g_scissor_clipping_rects.empty()) {
            glPopAttrib();
            if (g_stencil_bit)
                glEnable(GL_STENCIL_TEST);
        } else {
            const Rect& r = g_scissor_clipping_rects.back();
            glScissor(Value(r.Left()), Value(GUI::GetGUI()->AppHeight() - r.Bottom()),
                      Value(r.Width()), Value(r.Height()));
        }
    }

    void BeginStencilClipping(Pt inner_ul, Pt inner_lr,
                              Pt outer_ul, Pt outer_lr)
    {
        if (!g_stencil_bit) {
            glPushAttrib(GL_STENCIL_BUFFER_BIT);
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            if (!g_scissor_clipping_rects.empty())
                glDisable(GL_SCISSOR_TEST);
        }

        glGetBooleanv(GL_COLOR_WRITEMASK, g_prev_color_writemask);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &g_prev_depth_writemask);

        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        GLuint mask = 1u << g_stencil_bit;

        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
        glEnableClientState(GL_VERTEX_ARRAY);

        glStencilFunc(GL_ALWAYS, mask, mask);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
        GLint outer_vertices[] = {
            Value(outer_ul.x), Value(outer_ul.y),
            Value(outer_ul.x), Value(outer_lr.y),
            Value(outer_lr.x), Value(outer_lr.y),
            Value(outer_lr.x), Value(outer_ul.y)
        };
        glVertexPointer(2, GL_INT, 0, outer_vertices);
        glDrawArrays(GL_QUADS, 0, 4);

        glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
        GLint inner_vertices[] = {
            Value(inner_ul.x), Value(inner_ul.y),
            Value(inner_ul.x), Value(inner_lr.y),
            Value(inner_lr.x), Value(inner_lr.y),
            Value(inner_lr.x), Value(inner_ul.y)
        };
        glVertexPointer(2, GL_INT, 0, inner_vertices);
        glDrawArrays(GL_QUADS, 0, 4);

        glColorMask(g_prev_color_writemask[0],
                    g_prev_color_writemask[1],
                    g_prev_color_writemask[2],
                    g_prev_color_writemask[3]);
        glDepthMask(g_prev_depth_writemask);

        glStencilFunc(GL_EQUAL, mask, mask);
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
        ++g_stencil_bit;

        glPopClientAttrib();
    }

    void EndStencilClipping()
    {
        assert(g_stencil_bit);
        --g_stencil_bit;
        if (!g_stencil_bit) {
            glPopAttrib();
            if (!g_scissor_clipping_rects.empty())
                glEnable(GL_SCISSOR_TEST);
        }
    }

    void FlatRectangle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int border_thick/* = 2*/)
    { Rectangle(ul, lr, color, border_color, border_color, border_thick, true, true, true, true); }

    void BeveledRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int bevel_thick/* = 2*/,
                          bool bevel_left/* = true*/, bool bevel_top/* = true*/, bool bevel_right/* = true*/, bool bevel_bottom/* = true*/)
    {
        Rectangle(ul, lr, color,
                  (up ? LightColor(border_color) : DarkColor(border_color)),
                  (up ? DarkColor(border_color) : LightColor(border_color)),
                  bevel_thick, bevel_left, bevel_top, bevel_right, bevel_bottom);
    }

    void FlatRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int corner_radius/* = 5*/, unsigned int border_thick/* = 2*/)
    { RoundedRectangle(ul, lr, color, border_color, border_color, corner_radius, border_thick); }

    void BeveledRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int corner_radius/* = 5*/, unsigned int bevel_thick/* = 2*/)
    {
        RoundedRectangle(ul, lr, color,
                         (up ? LightColor(border_color) : DarkColor(border_color)),
                         (up ? DarkColor(border_color) : LightColor(border_color)),
                         corner_radius, bevel_thick);
    }

    void FlatCheck(Pt ul, Pt lr, Clr color)
    { Check(ul, lr, color, color, color); }

    void BeveledCheck(Pt ul, Pt lr, Clr color)
    { Check(ul, lr, color, LightColor(color), DarkColor(color)); }

    void FlatX(Pt ul, Pt lr, Clr color)
    { XMark(ul, lr, color, color, color); }

    void Bubble(Pt ul, Pt lr, Clr color, bool up/* = true*/)
    {
        BubbleArc(ul, lr, color,
                  (up ? DarkColor(color) : LightColor(color)),
                  (up ? LightColor(color) : DarkColor(color)),
                  0, 0);
    }

    void FlatCircle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int thick/* = 2*/)
    { CircleArc(ul, lr, color, border_color, border_color, thick, 0, 0); }

    void BeveledCircle(Pt ul, Pt lr, Clr color, Clr border_color, bool up/* = true*/, unsigned int bevel_thick/* = 2*/)
    {
        CircleArc(ul, lr, color, 
                  (up ? DarkColor(border_color) : LightColor(border_color)),
                  (up ? LightColor(border_color) : DarkColor(border_color)),
                  bevel_thick, 0, 0);
    }

    void BubbleRectangle(Pt ul, Pt lr, Clr color, bool up, unsigned int corner_radius/* = 5*/)
    {
        ::BubbleRectangle(ul, lr, color,
                          (up ? LightColor(color) : DarkColor(color)),
                          (up ? DarkColor(color) : LightColor(color)),
                          corner_radius);
    }

} // namespace GG
