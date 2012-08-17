/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#include <GG/adobe/basic_sheet.hpp>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/string.hpp>

#include <stdexcept>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

void basic_sheet_t::add_constant(name_t name, const any_regular_t& value)
{
    constant_cell_set_m.push_back(cell_t(value));
    
    const cell_t* cell(&constant_cell_set_m.back());
    
    variable_index_m.insert(std::make_pair(name.c_str(), cell));
}

/*************************************************************************************************/

void basic_sheet_t::add_interface(name_t name, const any_regular_t& value)
{
    interface_cell_set_m.push_back(interface_cell_t(value));
    
    interface_cell_t* cell(&interface_cell_set_m.back());
    
    variable_index_m.insert(std::make_pair(name.c_str(), cell));
    interface_index_m.insert(std::make_pair(name.c_str(), cell));
}

/*************************************************************************************************/

std::size_t basic_sheet_t::count_interface(name_t name) const
{ return interface_index_m.count(name.c_str()); }

/*************************************************************************************************/

basic_sheet_t::connection_t basic_sheet_t::monitor_value(name_t name,
        const monitor_value_t& monitor)
{
    interface_cell_t* cell(lookup_interface(name));

    monitor(cell->value_m);
    return (cell->monitor_value_m.connect(monitor));
}

/*************************************************************************************************/

void basic_sheet_t::set(const dictionary_t& cell_set)
{
    dictionary_t::const_iterator iter(cell_set.begin());
    dictionary_t::const_iterator last(cell_set.end());

    for (; iter != last; ++iter)    
        set(iter->first, iter->second);
}

/*************************************************************************************************/

void basic_sheet_t::set(name_t name, const any_regular_t& value)
{
    interface_cell_t* cell(lookup_interface(name));

    cell->value_m = value;

    cell->monitor_value_m(value);
}

/*************************************************************************************************/
    
const any_regular_t& basic_sheet_t::operator[](name_t name) const
{
    variable_index_t::const_iterator iter(variable_index_m.find(name.c_str()));

    if (iter == variable_index_m.end())
    {
        std::string error("basic_sheet_t variable cell does not exist: ");
        error << name.c_str();
        throw std::logic_error(error);
    }
    
    return iter->second->value_m;
}

/*************************************************************************************************/

adobe::dictionary_t basic_sheet_t::contributing() const
{
    interface_index_t::const_iterator iter(interface_index_m.begin());
    interface_index_t::const_iterator last(interface_index_m.end());
    adobe::dictionary_t         result;

    for (; iter != last; ++iter)
        result.insert(std::make_pair(adobe::name_t(iter->first), iter->second->value_m));

    return result;
}

/*************************************************************************************************/

basic_sheet_t::interface_cell_t* basic_sheet_t::lookup_interface(name_t name)
{
    interface_index_t::iterator iter(interface_index_m.find(name.c_str()));

    if (iter == interface_index_m.end())
    {
        std::string error("basic_sheet_t interface cell does not exist: ");
        error << name.c_str();
        throw std::logic_error(error);
    }
    
    return iter->second;
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/
