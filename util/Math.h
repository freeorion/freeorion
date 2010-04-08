// -*- C++ -*-
#ifndef _Math_h_
#define _Math_h_

#include <boost/numeric/ublas/matrix.hpp>


/** Boost.UBlas-based matrix type. */
typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::row_major> Matrix;

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

/** Returns the inverse of the 4x4 matrix \a m. */
Matrix Inverse4(const Matrix& m);

/** Returns the product of 4x4 Matrix \a m and 3x1 "vector" Matrix \a v. */
Matrix Matrix4xVector3(const Matrix& m, const Matrix& v);

/** Returns whether the ray described by \a ray_origin and \a ray_direction
    intersects the plane described by \a plane_normal and \a plane_point, and
    the distance along the ray of the intersection.  If there was no
    intersection, the distance returned is undefined. */
std::pair<bool, double> Intersects(double ray_origin[3], double ray_direction[3],
                                   double plane_normal[3], double plane_point[3]);

#endif // _Math_h_
