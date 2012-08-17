/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP
#define ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <string>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

struct edit_text_ctor_block_t
{
    edit_text_ctor_block_t() :
        scrollable_m(false),
        password_m(false),
        monospaced_m(false),
        min_characters_m(10),
        max_characters_m(0),
        num_lines_m(1)
    { }

    std::string name_m;
    std::string alt_text_m;
    bool        scrollable_m;
    bool        password_m;
    bool        monospaced_m;
    long        min_characters_m;
    long        max_characters_m; // 0 here means unlimited. Only matters when num_lines_m == 1
    long        num_lines_m;
    theme_t     theme_m;
};

/*************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGETS_EDIT_TEXT_COMMON_HPP
#endif

/****************************************************************************************************/
