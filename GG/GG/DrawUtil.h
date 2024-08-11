//! GiGi - A GUI for OpenGL
//!
//!  Copyright (C) 2003-2008 T. Zachary Laine <whatwasthataddress@gmail.com>
//!  Copyright (C) 2013-2020 The FreeOrion Project
//!
//! Released under the GNU Lesser General Public License 2.1 or later.
//! Some Rights Reserved.  See COPYING file or https://www.gnu.org/licenses/lgpl-2.1.txt
//! SPDX-License-Identifier: LGPL-2.1-or-later

//! @file GG/DrawUtil.h
//!
//! Contains numerous 2D rendering convenience functions, for rendering
//! rectangles, circles, etc.

#ifndef _GG_DrawUtil_h_
#define _GG_DrawUtil_h_


#include <GG/Base.h>
#include <GG/ClrConstants.h>


namespace GG {

/** Returns the "disabled" (grayed) version of color clr.  DisabledColor
    leaves the alpha channel unchanged, and adjusts the other channels in
    the direction of gray (GG_CLR_GRAY) by a factor between 0.0 and 1.0.
    (The factor is defined within DisabledColor().)  This is used
    throughout the GG classes to render disabled controls. */
GG_API Clr DisabledColor(Clr clr);

/** Sets up a GL scissor box, so that everything outside of the screen
    region defined by points \a ul and \a lr is clipped out.  These
    coordinates should be in GG screen coordinates, with +y downward,
    instead of GL's screen coordinates.  \note Failing to call
    EndScissorClipping() after calling this function and before the next
    unmatched glPopAttrib() call may produce unexpected results. */
GG_API void BeginScissorClipping(Pt ul, Pt lr);

/** Ends the current GL scissor box, restoring GL scissor state to what it
    was before the corresponding call to BeginScissorClipping().  \pre
    There must be an outstanding call to BeginScissorClipping(). */
GG_API void EndScissorClipping();

/** Returns the most recently begun and not yet ended scissor clipping
    region. */
GG_API Rect ActiveScissorClippingRegion();

/** Sets up a GL stencil, so that anything inside of the screen region
    defined by points \a inner_ul and \a inner_lr, or outside of the
    screen region defined by points \a outer_ul and \a outer_lr, is
    clipped out.  \note Failing to call EndStencilClipping() after calling
    this function and before the next unmatched glPopAttrib() call may
    produce unexpected results.  \note An unnested call to
    BeginStencilClipping() will clear the stencil buffer.  \pre There are
    no more than GL_STENCIL_BITS - 1 nested calls to
    BeginStencilClipping() currently outstanding (each nested call uses a
    separate bit in the stencil buffer). */
GG_API void BeginStencilClipping(Pt inner_ul, Pt inner_lr, Pt outer_ul, Pt outer_lr);

/** Ends the current GL stencil, restoring GL stencil state to what it was
    before the corresponding call to BeginScissorClipping().  \pre There
    must be an outstanding call to BeginStencilClipping(). */
GG_API void EndStencilClipping();

/** Renders a line between the two specified points, with the specified
    color and thickness. */
GG_API void Line(Pt pt1, Pt pt2, Clr color, float thick = 1.0f);

/** Renders a line between the specified coordinates, with the specified
    color and thickness. */
GG_API void Line(X x1, Y y1, X x2, Y y2, Clr color, float thick = 1.0f);

/** Renders line between specified coordinates. */
GG_API void Line(X x1, Y y1, X x2, Y y2);

/** Renders a triangle between the specified points, with the specified
    color and (if specified) border color and thickness. */
GG_API void Triangle(Pt pt1, Pt pt2, Pt pt3, Clr color, Clr border_color = CLR_ZERO, float border_thick = 1.0f);

/** Renders triangle between the specified coordinates. */
GG_API void Triangle(X x1, Y y1, X x2, Y y2, X x3, Y y3, bool filled = false);

/** Renders a rectangle starting at ul and ending just before lr, and
    assumes that OpenGL in in a "2D" state.  The border is drawn in the
    desired thickness and color, then whatever is space is left inside
    that is filled with color \a color.  No checking is done to make sure
    that \a border_thick * 2 is <= \a lr.x - \a ul.x (or <= \a lr.y - \a
    ul.y, for that matter).  This method of drawing and the 2D
    requirements are true for all functions that follow. */
GG_API void FlatRectangle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int border_thick = 2);

/** Like FlatRectangle(), but with a "beveled" appearance.  The
    border_color used to create a lighter and a darker version of
    border_color, which are used to draw beveled edges around the inside
    of the rectangle to the desired thickness.  If \a up is true, the
    beveled edges are lighter on the top and left, darker on the bottom
    and right, effecting a raised appearance.  If \a up is false, the
    opposite happens, and the rectangle looks depressed.  This is true of
    all the Beveled*() functions. */
GG_API void BeveledRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int bevel_thick = 2,
                                bool bevel_left = true, bool bevel_top = true, bool bevel_right = true, bool bevel_bottom = true);

/** Draws a checkmark used to draw state buttons. */
GG_API void FlatCheck(Pt ul, Pt lr, Clr color);

/** Like FlatCheck(), but with a raised appearance. */
GG_API void BeveledCheck(Pt ul, Pt lr, Clr color);

/** Draws an X-mark used to draw state buttons. */
GG_API void FlatX(Pt ul, Pt lr, Clr color);

/** Draws a disk that appears to be a portion of a lit sphere.  The
    portion may appear raised or depressed. */
GG_API void Bubble(Pt ul, Pt lr, Clr color, bool up = true);

/** Draws a circle of thick pixels thickness in the color specified. */
GG_API void FlatCircle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned thick = 2);

/** Draws a circle of \a thick pixels thickness in the color specified.
    The circle appears to be beveled, and may be beveled in such a way as
    to appear raised or depressed. */
GG_API void BeveledCircle(Pt ul, Pt lr, Clr color, Clr border_color, bool up = true, unsigned int bevel_thick = 2);

/** Draws a rounded rectangle of the specified thickness. The radius of
    the circles used to draw the corners is specified by \a corner_radius.
    Note that this means the rectangle should be at least 2 * \a
    corner_radius on a side, but as with all the other functions, no such
    check is performed. */
GG_API void FlatRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, unsigned int corner_radius = 5, unsigned int border_thick = 2);

/** Like the FlatRoundedRectangle() function, but beveled (raised or
    depressed). */
GG_API void BeveledRoundedRectangle(Pt ul, Pt lr, Clr color, Clr border_color, bool up, unsigned int corner_radius = 5, unsigned int bevel_thick = 2);

/** Using the same techniques as in Bubble(), creates a rounded, bubbly
    rectangle. */
GG_API void BubbleRectangle(Pt ul, Pt lr, Clr color, bool up, unsigned int corner_radius = 5);

}


#endif
