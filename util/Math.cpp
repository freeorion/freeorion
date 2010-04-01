#include "Math.h"

#include <boost/numeric/ublas/matrix.hpp>


bool PointInEllipse(double x, double y,
                    double ellipse_x, double ellipse_y,
                    double major, double minor,
                    double theta_major)
{
    typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::row_major> Matrix;
    Matrix v(2, 1);
    v(0, 0) = x - ellipse_x;
    v(0, 1) = y - ellipse_y;

    const double SIN = std::sin(-theta_major);
    const double COS = std::cos(-theta_major);
    Matrix rotate(2, 2);
    rotate(0, 0) = COS;
    rotate(0, 1) = -SIN;
    rotate(1, 0) = SIN;
    rotate(1, 1) = COS;

    v = prod(rotate, v);

    Matrix scale(2, 2);
    scale(0, 0) = 1.0 / major;
    scale(0, 1) = 0.0;
    scale(1, 0) = 0.0;
    scale(1, 1) = 1.0 / minor;

    v = prod(scale, v);

    return v(0, 0) * v(0, 0) + v(0, 1) * v(0, 1) < 1.0;
}

