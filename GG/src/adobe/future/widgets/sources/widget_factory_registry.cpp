/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_factory_registry.hpp>

#include <GG/adobe/string.hpp>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

widget_factory_t::map_type::iterator widget_factory_t::find(name_t name, noconst) const
{
    map_type::iterator result(const_cast<map_type&>(map_m).find(name));

    if (result == map_m.end())
    {
        throw std::runtime_error(make_string("Widget factory method for '", name.c_str(), "'not found."));
    }

    return result;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
