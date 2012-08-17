/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_ALERT_HPP
#define ADOBE_ALERT_HPP

/****************************************************************************************************/

#include <GG/adobe/name.hpp>

#include <utility>

#include <boost/filesystem/path.hpp>
#include <boost/array.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

std::pair<const char*, bool> alert(const char*                    message_text = "There was an error.",
                                   const char*                    window_name = "Alert",
                                   const char*                    button_0_name = "OK",
                                   const char*                    button_1_name = 0,
                                   const char*                    button_2_name = 0,
                                   const char*                    checkbox_name = 0,
                                   const boost::filesystem::path& icon_path = boost::filesystem::path(),
                                   std::size_t                    default_button_index = 0,
                                   std::size_t                    cancel_button_index = 1);

/****************************************************************************************************/

struct alert_helper_t
{
    alert_helper_t() :
        window_name_m("Alert"),
        message_m("There was an error."),
        button_0_m("OK"),
        button_1_m(0),
        button_2_m(0),
        checkbox_name_m(0),
        default_button_index_m(0),
        cancel_button_index_m(1)
    { }

    std::pair<const char*, bool> run()
    {
        return alert(message_m,
                     window_name_m,
                     button_0_m,
                     button_1_m,
                     button_2_m,
                     checkbox_name_m,
                     icon_path_m,
                     default_button_index_m,
                     cancel_button_index_m);
    }

    const char*             window_name_m;
    const char*             message_m;
    const char*             button_0_m;
    const char*             button_1_m;
    const char*             button_2_m;
    const char*             checkbox_name_m;
    boost::filesystem::path icon_path_m;
    std::size_t             default_button_index_m;
    std::size_t             cancel_button_index_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
