// -*- C++ -*-
#ifndef _CUIDrawUtil_h_
#define _CUIDrawUtil_h_

#ifndef _GGClr_h_
#include "GGClr.h"
#endif

/** adjusts the intensity of the color up or down by \a amount units per color channel; leaves alpha unchanged */
void AdjustBrightness(GG::Clr& color, int amount);

/** renders a rectangle whose upper left and lower right corners are angled.  If \a upper_left_angled == false, 
    the upper left corner is drawn as a normal corner */
void AngledCornerRectangle(int x1, int y1, int x2, int y2, GG::Clr color, GG::Clr border, int angle_offset, int thick, 
                           bool upper_left_angled = true);

/** returns true iff \a pt falls within \a rect, with the missing bits of the angled corners not catching the point. 
    If \a upper_left_angled == false, the upper left corner is treated as a normal corner. */
bool InAngledCornerRect(const GG::Pt& pt, int x1, int y1, int x2, int y2, int angle_offset, bool upper_left_angled = true);

/** the orientations used to render some shapes used in the UI; the orientations usually refer to the direction
    in which the shape is pointing */
enum ShapeOrientation {SHAPE_UP, SHAPE_DOWN, SHAPE_LEFT, SHAPE_RIGHT};

/** renders a triangle of arbitrary size and shape, having an optional 1-pixel-thick border */
void Triangle(int x1, int y1, int x2, int y2, int x3, int y3, GG::Clr color, bool border = true);

/** returns true iff \a pt lies within the triangle described by the other parameters */
bool InTriangle(const GG::Pt& pt, int x1, int y1, int x2, int y2, int x3, int y3);

/** renders a triangle with two equal-length sides, oriented in the desired direction.  The triangle will have a base length
    of one of (<i>x2</i> - <i>x1</i>) and (<i>y2</i> - <i>y1</i>), depending on \a orientation, and a height of the other. */
void IsoscelesTriangle(int x1, int y1, int x2, int y2, ShapeOrientation orientation, GG::Clr color, bool border = true);

/** returns true iff \a pt falls within the isosceles triangle described by the other parameters */
bool InIsoscelesTriangle(const GG::Pt& pt, int x1, int y1, int x2, int y2, ShapeOrientation orientation);

/** renders a semi-triangular shape with two equal-length sides, oriented in the desired direction.  
    Only SHAPE_LEFT and SHAPE_RIGHT are supported. */
void FleetMarker(int x1, int y1, int x2, int y2, ShapeOrientation orientation, GG::Clr color);

/** returns true iff \a pt falls within the fleet marker described by the other parameters */
bool InFleetMarker(const GG::Pt& pt, int x1, int y1, int x2, int y2, ShapeOrientation orientation);

inline std::pair<std::string, std::string> CUIDrawUtilRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _CUIDrawUtil_h_

