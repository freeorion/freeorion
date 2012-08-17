/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/localization.hpp>

#include <stdexcept>

/*************************************************************************************************/

namespace {

/*************************************************************************************************/

adobe::localization_lookup_proc_t& localization_proc()
{
    static adobe::localization_lookup_proc_t localization_lookup_proc_s;

    return localization_lookup_proc_s;
}

/*************************************************************************************************/

} // namespace

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void localization_register(const localization_lookup_proc_t& proc)
{
    localization_proc() = proc;
}

std::string localization_invoke(const std::string& source)
{
    if (!localization_proc())
        throw std::runtime_error("Attempting to call an unregistered localization routine.");

    return localization_proc()(source);
}

bool localization_ready()
{
    return localization_proc();
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
