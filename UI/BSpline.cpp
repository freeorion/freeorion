/*********************************************************************

 Simple b-spline curve algorithm

 Copyright 1994 by Keith Vertanen (vertankd@cda.mrs.umn.edu)

 Released to the public domain (your mileage may vary)

**********************************************************************/

#include "BSpline.h"

#include <cassert>

namespace {
    double blend(int k, int t, std::vector<int>& u, double v)  // calculate the blending value
    {
        double value;

        if (t==1)			// base case for the recursion
        {
            if ((u[k]<=v) && (v<u[k+1]))
                value=1;
            else
                value=0;
        }
        else
        {
            if ((u[k+t-1]==u[k]) && (u[k+t]==u[k+1]))  // check for divide by zero
                value = 0;
            else
                if (u[k+t-1]==u[k]) // if a term's denominator is zero,use just the other
                    value = (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
                else
                    if (u[k+t]==u[k+1])
                        value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v);
                    else
                        value = (v - u[k]) / (u[k+t-1] - u[k]) * blend(k, t-1, u, v) +
                            (u[k+t] - v) / (u[k+t] - u[k+1]) * blend(k+1, t-1, u, v);
        }
        return value;
    }

    void compute_intervals(std::vector<int>& u, int n, int t)   // figure out the knots
    {
        int j;

        for (j=0; j<=n+t; j++)
        {
            if (j<t)
                u[j]=0;
            else
                if ((t<=j) && (j<=n))
                    u[j]=j-t+1;
                else
                    if (j>n)
                        u[j]=n-t+2;  // if n-t=-2 then we're screwed, everything goes to 0
        }
    }

    void compute_point(std::vector<int>& u, int n, int t, double v, const std::vector<BSplinePoint>& control,
                       BSplinePoint *output)
    {
        int k;
        double temp;

        // initialize the variables that will hold our outputted point
        output->x=0;
        output->y=0;
        output->z=0;

        for (k=0; k<=n; k++)
        {
            temp = blend(k,t,u,v);  // same blend is used for each dimension coordinate
            output->x = output->x + (control[k]).x * temp;
            output->y = output->y + (control[k]).y * temp;
            output->z = output->z + (control[k]).z * temp;
        }
    }
}

/*********************************************************************
Parameters:
  n          - the number of control points minus 1
  t          - the degree of the polynomial plus 1
  control    - control point array made up of point stucture
  output     - vector in which the calculate spline points are to be put
  num_output - how many points on the spline are to be calculated

Pre-conditions:
  n+2>t  (no curve results if n+2<=t)
  control array contains the number of points specified by n
**********************************************************************/
void BSpline(int t, const std::vector<BSplinePoint>& control, std::vector<BSplinePoint>& output, int num_output)
{
    double increment,interval;
    BSplinePoint calcxyz;

    const int n = control.size() - 1;
    assert(t < n + 2);
    output.resize(num_output);

    std::vector<int> u(n+t+1);
    compute_intervals(u, n, t);

    increment=static_cast<double>(n-t+2)/(num_output-1);  // how much parameter goes up each time
    interval=0;

    for (int output_index=0; output_index<num_output-1; output_index++)
    {
        compute_point(u, n, t, interval, control, &calcxyz);
        output[output_index].x = calcxyz.x;
        output[output_index].y = calcxyz.y;
        output[output_index].z = calcxyz.z;
        interval=interval+increment;  // increment our parameter
    }
    output[num_output-1].x=control[n].x;   // put in the last point
    output[num_output-1].y=control[n].y;
    output[num_output-1].z=control[n].z;
}
