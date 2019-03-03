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
#include <GG/GLClientAndServerBuffer.h>


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
    std::map<int, std::valarray<double>> unit_circle_coords;
    /// this doesn't serve as a cache, but does allow us to prevent numerous constructions and destructions of Clr valarrays.
    std::map<int, std::valarray<Clr>> color_arrays;

    void Rectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2, unsigned int bevel_thick,
                   bool bevel_left, bool bevel_top, bool bevel_right, bool bevel_bottom)
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
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;

        // all vertices
        GLfloat verts[][2] = {{-0.2f,  0.2f}, {-0.6f, -0.2f}, {-0.6f,  0.0f}, {-0.2f,  0.4f}, {-0.8f,  0.0f},
                             { -0.2f,  0.6f}, { 0.8f, -0.4f}, { 0.6f, -0.4f}, { 0.8f, -0.8f}};

        glPushMatrix();
        const float sf = 1.25f;                                                     // scale factor to make the check look right
        glTranslatef(Value(ul.x + wd / 2.0f), Value(ul.y + ht / 2.0f * sf), 0.0f);  // move origin to the center of the rectangle
        glScalef(Value(wd / 2.0f * sf), Value(ht / 2.0f * sf), 1.0f);               // map the range [-1,1] to the rectangle in both directions

        static std::size_t indices[22] = { 1,  4,  2,
                                           8,  0,  3,  7,
                                           2,  4,  5,  3,  7,  3,  5,  6,
                                           8,  7,  6,
                                           0,  1,  2,  3};

        GL2DVertexBuffer vert_buf;
        vert_buf.reserve(22);
        for (std::size_t i = 0; i < 22; ++i)
            vert_buf.store(verts[indices[i]][0], verts[indices[i]][1]);

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
        X wd = lr.x - ul.x;
        Y ht = lr.y - ul.y;
        glDisable(GL_TEXTURE_2D);

        // all vertices
        GLfloat verts[][2] = {{-0.4f, -0.6f}, {-0.6f, -0.4f}, {-0.4f, -0.4f}, {-0.2f,  0.0f}, {-0.6f,  0.4f},
                              {-0.4f,  0.6f}, {-0.4f,  0.4f}, { 0.0f,  0.2f}, { 0.4f,  0.6f}, { 0.6f,  0.4f},
                              { 0.4f,  0.4f}, { 0.2f,  0.0f}, { 0.6f, -0.4f}, { 0.4f, -0.6f}, { 0.4f, -0.4f},
                              { 0.0f, -0.2f}, { 0.0f,  0.0f}};

        glPushMatrix();
        const float sf = 1.75f;                                                 // scale factor; the check wasn't the right size as drawn originally
        glTranslatef(Value(ul.x + wd / 2.0f), Value(ul.y + ht / 2.0f), 0.0f);   // move origin to the center of the rectangle
        glScalef(Value(wd / 2.0f * sf), Value(ht / 2.0f * sf), 1.0f);           // map the range [-1,1] to the rectangle in both directions

        static std::size_t indices[44] = {12, 13, 14,
                                          15,  0,  2, 16,  9, 11, 16, 10,
                                           0,  1,  2,
                                          13, 15, 16, 14,  3,  4,  6, 16,
                                           4,  5,  6,  8,  9, 10,
                                          14, 16, 11, 12,  2,  1,  3, 16, 16,  6,  5,  7, 16,  7,  8, 10};

        GL2DVertexBuffer vert_buf;
        vert_buf.reserve(44);
        for (std::size_t i = 0; i < 44; ++i)
            vert_buf.store(verts[indices[i]][0], verts[indices[i]][1]);

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

    void CircleArc(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2,
                   unsigned int bevel_thick, double theta1, double theta2)
    {
        //std::cout << "GG::CircleArc ul: " << ul << "  lr: " << lr << " bevel thick: " << bevel_thick << "  theta1: " << theta1 << "  theta2: " << theta2 << std::flush << std::endl;
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

    void RoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color1, Clr border_color2,
                          unsigned int corner_radius, int thick)
    {
        int circle_diameter = corner_radius * 2;
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

        int rad = static_cast<int>(corner_radius);

        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        GG::Clr clr = border_color2 * (1 - color_scale_factor) + border_color1 * color_scale_factor;
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


        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        clr = border_color2 * (1 - color_scale_factor) + border_color1 * color_scale_factor;
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

    void BubbleRectangle(Pt ul, Pt lr, Clr color1, Clr color2, Clr color3, unsigned int corner_radius)
    {
        int circle_diameter = corner_radius * 2;
        BubbleArc(Pt(lr.x - circle_diameter, ul.y), Pt(lr.x, ul.y + circle_diameter), color1, color3, color2, 0, 0.5 * PI);  // ur corner
        BubbleArc(Pt(ul.x, ul.y), Pt(ul.x + circle_diameter, ul.y + circle_diameter), color1, color3, color2, 0.5 * PI, PI); // ul corner
        BubbleArc(Pt(ul.x, lr.y - circle_diameter), Pt(ul.x + circle_diameter, lr.y), color1, color3, color2, PI, 1.5 * PI); // ll corner
        BubbleArc(Pt(lr.x - circle_diameter, lr.y - circle_diameter), Pt(lr.x, lr.y), color1, color3, color2, 1.5 * PI, 0);  // lr corner

        int rad = static_cast<int>(corner_radius);

        // top
        double color_scale_factor = (SQRT2OVER2 * (0 + 1) + 1) / 2;
        Clr scaled_color(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                         GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                         GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                         GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));

        GL2DVertexBuffer verts;
        verts.reserve(20);
        GLRGBAColorBuffer colours;
        colours.reserve(20);

        colours.store(scaled_color);
        colours.store(scaled_color);
        verts.store(lr.x - rad, ul.y);
        verts.store(ul.x + rad, ul.y);
        colours.store(color1);
        colours.store(color1);
        verts.store(ul.x + rad, ul.y + rad);
        verts.store(lr.x - rad, ul.y + rad);

        // left (uses color scale factor (SQRT2OVER2 * (1 + 0) + 1) / 2, which equals that of top
        colours.store(scaled_color);
        colours.store(scaled_color);
        verts.store(ul.x, ul.y + rad);
        verts.store(ul.x, lr.y - rad);
        colours.store(color1);
        colours.store(color1);
        verts.store(ul.x + rad, lr.y - rad);
        verts.store(ul.x + rad, ul.y + rad);

        // right
        color_scale_factor = (SQRT2OVER2 * (-1 + 0) + 1) / 2;
        scaled_color = Clr(GLubyte(color3.r * (1 - color_scale_factor) + color2.r * color_scale_factor),
                           GLubyte(color3.g * (1 - color_scale_factor) + color2.g * color_scale_factor),
                           GLubyte(color3.b * (1 - color_scale_factor) + color2.b * color_scale_factor),
                           GLubyte(color3.a * (1 - color_scale_factor) + color2.a * color_scale_factor));
        colours.store(color1);
        colours.store(color1);
        verts.store(lr.x - rad, ul.y + rad);
        verts.store(lr.x - rad, lr.y - rad);
        colours.store(scaled_color);
        colours.store(scaled_color);
        verts.store(lr.x, lr.y - rad);
        verts.store(lr.x, ul.y + rad);

        // bottom (uses color scale factor (SQRT2OVER2 * (0 + -1) + 1) / 2, which equals that of left
        colours.store(color1);
        colours.store(color1);
        verts.store(lr.x - rad, lr.y - rad);
        verts.store(ul.x + rad, lr.y - rad);
        colours.store(scaled_color);
        colours.store(scaled_color);
        verts.store(ul.x + rad, lr.y);
        verts.store(lr.x - rad, lr.y);

        // middle
        colours.store(color1);
        colours.store(color1);
        verts.store(lr.x - rad, ul.y + rad);
        verts.store(ul.x + rad, ul.y + rad);
        colours.store(color1);
        colours.store(color1);
        verts.store(ul.x + rad, lr.y - rad);
        verts.store(lr.x - rad, lr.y - rad);


        glDisable(GL_TEXTURE_2D);
        glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        verts.activate();
        colours.activate();
        glDrawArrays(GL_QUADS, 0, verts.size());

        glPopClientAttrib();
        glEnable(GL_TEXTURE_2D);
    }
} // namespace


namespace GG {
    std::ostream& operator<<(std::ostream& os, const Clr& clr)
    {
        os << "(" << +clr.r << ", " << +clr.g << ", " << +clr.b << ", " << +clr.a << ")";
        return os;
    }

    void glColor(Clr clr)
    { glColor4ub(clr.r, clr.g, clr.b, clr.a); }

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
        g_scissor_clipping_rects.push_back(Rect(ul, lr));
    }

    void EndScissorClipping()
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

    Rect ActiveScissorClippingRegion()
    {
        if (g_scissor_clipping_rects.empty())
            return Rect();
        return g_scissor_clipping_rects.back();
    }

    void BeginStencilClipping(Pt inner_ul, Pt inner_lr, Pt outer_ul, Pt outer_lr)
    {
        if (!g_stencil_bit) {
            glPushAttrib(GL_STENCIL_BUFFER_BIT | GL_ENABLE_BIT);
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
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

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
            if (!g_scissor_clipping_rects.empty())
                glEnable(GL_SCISSOR_TEST);
            glPopAttrib();
        }
    }

    void Line(Pt pt1, Pt pt2, Clr color, float thick)
    {
        glLineWidth(thick);
        glColor(color);
        Line(pt1.x, pt1.y, pt2.x, pt2.y);
    }

    void Line(X x1, Y y1, X x2, Y y2)
    {
        GLfloat vertices[4] = {GLfloat(Value(x1)), GLfloat(Value(y1)),
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

    void Triangle(Pt pt1, Pt pt2, Pt pt3, Clr color, Clr border_color, float border_thick)
    {
        GLfloat vertices[6] = {GLfloat(Value(pt1.x)), GLfloat(Value(pt1.y)), GLfloat(Value(pt2.x)),
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

    void Triangle(X x1, Y y1, X x2, Y y2, X x3, Y y3, bool filled)
    {
        GLfloat vertices[6] = {GLfloat(Value(x1)), GLfloat(Value(y1)), GLfloat(Value(x2)),
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

    void FlatRectangle(Pt ul, Pt lr, Clr color, Clr border_color,
                       unsigned int border_thick/* = 2*/)
    {
        Rectangle(ul, lr, color, border_color, border_color, border_thick,
                  true, true, true, true);
    }

    void BeveledRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up,
                          unsigned int bevel_thick/* = 2*/, bool bevel_left/* = true*/,
                          bool bevel_top/* = true*/, bool bevel_right/* = true*/,
                          bool bevel_bottom/* = true*/)
    {
        Rectangle(ul, lr, color,
                  (up ? LightColor(border_color) : DarkColor(border_color)),
                  (up ? DarkColor(border_color) : LightColor(border_color)),
                  bevel_thick, bevel_left, bevel_top, bevel_right, bevel_bottom);
    }

    void FlatRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color,
                              unsigned int corner_radius/* = 5*/,
                              unsigned int border_thick/* = 2*/)
    {
        RoundedRectangle(ul, lr, color, border_color, border_color,
                         corner_radius, border_thick);
    }

    void BeveledRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up,
                                 unsigned int corner_radius/* = 5*/,
                                 unsigned int bevel_thick/* = 2*/)
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
