/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_EDIT_NUMBER_EDGE_HPP
#define ADOBE_WIDGET_EDIT_NUMBER_EDGE_HPP

#include <GG/PtRect.h>


/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

class EditNumberLabelFilter;

}

/****************************************************************************************************/

struct edit_number_t;

/****************************************************************************************************/

struct edit_number_platform_data_t
{
    explicit edit_number_platform_data_t(edit_number_t* edit_number);

    ~edit_number_platform_data_t();

    void initialize();
    void increment_n(long n);

    edit_number_t* control_m;
    GG::Pt         last_point_m;
    boost::shared_ptr<implementation::EditNumberLabelFilter>
                   filter_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGET_EDIT_NUMBER_EDGE_HPP
#endif

/****************************************************************************************************/
