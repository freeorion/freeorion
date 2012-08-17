/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_RESOURCES_HPP
#define ADOBE_WIDGET_RESOURCES_HPP

/****************************************************************************************************/

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void push_resource_root_path(const boost::filesystem::path& root);
void pop_resource_root_path();

boost::filesystem::path find_resource(const boost::filesystem::path& name);

/****************************************************************************************************/

struct resource_context_t
{
    explicit resource_context_t(const boost::filesystem::path& root)
    {
        push_resource_root_path(root);
    }

    ~resource_context_t()
    {
        pop_resource_root_path();
    }
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

// ADOBE_WIDGET_RESOURCES_HPP
#endif

/****************************************************************************************************/
