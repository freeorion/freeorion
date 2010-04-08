#include "Math.h"


namespace {
    bool PointInEllipseImpl(double x, double y,
                            double ellipse_x, double ellipse_y,
                            double major, double minor,
                            double theta_major,
                            double theta_begin,
                            double theta_end)
    {
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

Matrix Inverse4(const Matrix& m)
{
    assert(m.size1() == 4u);
    assert(m.size2() == 4u);

    Matrix retval(4, 4);

    double m00 = m(0, 0), m01 = m(0, 1), m02 = m(0, 2), m03 = m(0, 3);
    double m10 = m(1, 0), m11 = m(1, 1), m12 = m(1, 2), m13 = m(1, 3);
    double m20 = m(2, 0), m21 = m(2, 1), m22 = m(2, 2), m23 = m(2, 3);
    double m30 = m(3, 0), m31 = m(3, 1), m32 = m(3, 2), m33 = m(3, 3);

    double v0 = m20 * m31 - m21 * m30;
    double v1 = m20 * m32 - m22 * m30;
    double v2 = m20 * m33 - m23 * m30;
    double v3 = m21 * m32 - m22 * m31;
    double v4 = m21 * m33 - m23 * m31;
    double v5 = m22 * m33 - m23 * m32;

    double t00 = + (v5 * m11 - v4 * m12 + v3 * m13);
    double t10 = - (v5 * m10 - v2 * m12 + v1 * m13);
    double t20 = + (v4 * m10 - v2 * m11 + v0 * m13);
    double t30 = - (v3 * m10 - v1 * m11 + v0 * m12);

    double inv_det = 1 / (t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03);

    retval(0, 0) = t00 * inv_det;
    retval(1, 0) = t10 * inv_det;
    retval(2, 0) = t20 * inv_det;
    retval(3, 0) = t30 * inv_det;

    retval(0, 1) = - (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
    retval(1, 1) = + (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
    retval(2, 1) = - (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
    retval(3, 1) = + (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

    v0 = m10 * m31 - m11 * m30;
    v1 = m10 * m32 - m12 * m30;
    v2 = m10 * m33 - m13 * m30;
    v3 = m11 * m32 - m12 * m31;
    v4 = m11 * m33 - m13 * m31;
    v5 = m12 * m33 - m13 * m32;

    retval(0, 2) = + (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
    retval(1, 2) = - (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
    retval(2, 2) = + (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
    retval(3, 2) = - (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

    v0 = m21 * m10 - m20 * m11;
    v1 = m22 * m10 - m20 * m12;
    v2 = m23 * m10 - m20 * m13;
    v3 = m22 * m11 - m21 * m12;
    v4 = m23 * m11 - m21 * m13;
    v5 = m23 * m12 - m22 * m13;

    retval(0, 3) = - (v5 * m01 - v4 * m02 + v3 * m03) * inv_det;
    retval(1, 3) = + (v5 * m00 - v2 * m02 + v1 * m03) * inv_det;
    retval(2, 3) = - (v4 * m00 - v2 * m01 + v0 * m03) * inv_det;
    retval(3, 3) = + (v3 * m00 - v1 * m01 + v0 * m02) * inv_det;

    return retval;
}

Matrix Matrix4xVector3(const Matrix& m, const Matrix& v)
{
    Matrix retval(3, 1);
    double inv_w = 1.0 / ( m(3, 0) * v(0, 0) + m(3, 1) * v(1, 0) + m(3, 2) * v(2, 0) + m(3, 3) );
    retval(0, 0) = ( m(0, 0) * v(0, 0) + m(0, 1) * v(1, 0) + m(0, 2) * v(2, 0) + m(0, 3) ) * inv_w;
    retval(1, 0) = ( m(1, 0) * v(0, 0) + m(1, 1) * v(1, 0) + m(1, 2) * v(2, 0) + m(1, 3) ) * inv_w;
    retval(2, 0) = ( m(2, 0) * v(0, 0) + m(2, 1) * v(1, 0) + m(2, 2) * v(2, 0) + m(2, 3) ) * inv_w;
    return retval;
}

std::pair<bool, double> Intersects(double ray_origin[3], double ray_direction[3],
                                   double plane_normal[3], double plane_point[3])
{
    double denominator =
        plane_normal[0] * ray_direction[0] +
        plane_normal[1] * ray_direction[1] +
        plane_normal[2] * ray_direction[2];
    if (std::abs(denominator) < std::numeric_limits<double>::epsilon())
    {
        // Parallel
        return std::pair<bool, double>(false, 0);
    }
    else
    {
        double nominator =
            plane_normal[0] * ray_origin[0] + plane_point[0] +
            plane_normal[1] * ray_origin[1] + plane_point[1] +
            plane_normal[2] * ray_origin[2] + plane_point[2];
        double t = -nominator / denominator;
        return std::pair<bool, double>(0 <= t, t);
    }
}
