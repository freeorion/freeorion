// -*- C++ -*-
#ifndef _BSpline_h_
#define _BSpline_h_

#include <vector>

struct BSplinePoint {
    double x;
    double y;
    double z;
};

void BSpline(int t, const std::vector<BSplinePoint>& control, std::vector<BSplinePoint>& output, int num_output);

#endif // _BSpline_h_
