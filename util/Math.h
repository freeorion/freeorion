// -*- C++ -*-
#ifndef _Math_h_
#define _Math_h_

/** Returns true iff <i>(x, y)</i> falls inside the ellipse centered at
    <i>(ellipse_x, ellipse_y)</i>, with major and minor axes \a major and \a
    minor, with the major axis at angle \a theta_major. */
bool PointInEllipse(double x, double y,
                    double ellipse_x, double ellipse_y,
                    double major, double minor,
                    double theta_major);

/** Returns true iff <i>(x, y)</i> falls inside the angular portion
    <i>(theta_begin, theta_end)</i> ellipse centered at <i>(ellipse_x,
    ellipse_y)</i>, with major and minor axes \a major and \a minor, with the
    major axis at angle \a theta_major.  The angular region <i>(theta_begin,
    theta_end)</i> is taken to be relative to \a theta_major. */
bool PointInPartialEllipse(double x, double y,
                           double ellipse_x, double ellipse_y,
                           double major, double minor,
                           double theta_major,
                           double theta_begin,
                           double theta_end);

#endif // _Math_h_
