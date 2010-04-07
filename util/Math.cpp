#include "Math.h"

#include <boost/numeric/ublas/matrix.hpp>


namespace {
    bool PointInEllipseImpl(double x, double y,
                            double ellipse_x, double ellipse_y,
                            double major, double minor,
                            double theta_major,
                            double theta_begin,
                            double theta_end)
    {
        typedef boost::numeric::ublas::matrix<double, boost::numeric::ublas::row_major> Matrix;
        Matrix v(2, 1);
        v(0, 0) = x - ellipse_x;
        v(1, 0) = y - ellipse_y;

        const double SIN = std::sin(-theta_major);
        const double COS = std::cos(-theta_major);
        Matrix rotate(2, 2);
        rotate(0, 0) = COS;
        rotate(0, 1) = -SIN;
        rotate(1, 0) = SIN;
        rotate(1, 1) = COS;

        v = prod(rotate, v);

        if (theta_begin != theta_end) {
            double theta = std::atan2(v(1, 0), v(0, 0));
            if (theta < theta_begin || theta_end < theta)
                return false;
        }

        Matrix scale(2, 2);
        scale(0, 0) = 1.0 / major;
        scale(0, 1) = 0.0;
        scale(1, 0) = 0.0;
        scale(1, 1) = 1.0 / minor;

        v = prod(scale, v);

        return v(0, 0) * v(0, 0) + v(1, 0) * v(1, 0) < 1.0;
    }

}

bool PointInEllipse(double x, double y,
                    double ellipse_x, double ellipse_y,
                    double major, double minor,
                    double theta_major)
{ return PointInEllipseImpl(x, y, ellipse_x, ellipse_y, major, minor, theta_major, 0, 0); }

bool PointInPartialEllipse(double x, double y,
                           double ellipse_x, double ellipse_y,
                           double major, double minor,
                           double theta_major,
                           double theta_begin,
                           double theta_end)
{ return PointInEllipseImpl(x, y, ellipse_x, ellipse_y, major, minor, theta_major, theta_begin, theta_end); }
