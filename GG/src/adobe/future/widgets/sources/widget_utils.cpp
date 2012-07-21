/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <GG/adobe/name.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

name_t state_cell_unique_name()
{
    // this takes an increasing counter and converts it to base 26,
    // turning it into a string, and returning the string. It prepends
    // a ZUID to dramatically increase the avoidance of a collision with
    // a cell already named in the sheet.
    //
    // (I'd use a zuid here, but there are symbols missing on Mac OS X:)
    //      _sprintf$LDBLStub
    //      _sscanf$LDBLStub

    static long count(-1);
    std::string result("__22c8c184-fb4e-1fff-8f9e-8dc7baa26187_");
    long        tmp(++count);

    while(true)
    {
        long n(tmp / 26);
        long r(tmp % 26);

        tmp = n;        

        result += static_cast<char>('a' + r);

        if (tmp == 0)
            break;
    }

    return name_t(result.c_str());
}

/****************************************************************************************************/

void align_slices(extents_t::slice_t& slice_one, extents_t::slice_t slice_two)
{
    //
    // Make sure that we have points of interest to align by.
    //
    // REVISIT (fbrereto) Are we sure we want to assert here?
    //

    assert(!slice_one.guide_set_m.empty());   
    assert(!slice_two.guide_set_m.empty());

    //
    // The eventual point of interest (or baseline) is the maxiumum
    // of the two we have.
    //

    long poi = (std::max)(slice_one.guide_set_m[0], slice_two.guide_set_m[0]);

    //
    // The length of the slice is the maximum amount before the
    // point of interest (ascent above baseline) plus the maximum
    // amount after the point of interest (descent below the
    // baseline).
    //
    // The maximum before the point of interest is obviously the
    // the point of interest.
    //

    long length = poi + (std::max)( slice_one.length_m - slice_one.guide_set_m[0],
                                    slice_two.length_m - slice_two.guide_set_m[0]);

    //
    // Set our new values into the first slice.
    //

    slice_one.guide_set_m[0] = poi;
    slice_one.length_m = length;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
