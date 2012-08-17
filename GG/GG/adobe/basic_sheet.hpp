/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_BASIC_SHEET_HPP
#define ADOBE_BASIC_SHEET_HPP

#include <GG/adobe/config.hpp>

#include <deque>
#include <map>
#include <vector>

#include <boost/signal.hpp>
#include <boost/function.hpp>

#include <GG/adobe/dictionary_fwd.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>

/*************************************************************************************************/

namespace adobe {
/*!
\defgroup basic_property_model Basic Property Model
\ingroup property_model
 */

/*************************************************************************************************/

/*!
\ingroup basic_property_model
 */
class basic_sheet_t : boost::noncopyable
{
 public:
    
    typedef boost::signals::connection connection_t;
        typedef boost::function<void (const any_regular_t&)> monitor_value_t;
    typedef boost::signal<void (const any_regular_t&)>   monitor_value_list_t;
        void add_constant(name_t, const any_regular_t&);
    void add_interface(name_t, const any_regular_t&);
    
    std::size_t count_interface(name_t) const;
    
        connection_t monitor_value(name_t name, const monitor_value_t& monitor);
    
    void set(name_t, const any_regular_t&); // interface cell
    void set(const dictionary_t& cell_set); // interface cell set

    const any_regular_t& operator[](name_t) const; // variable lookup

    dictionary_t contributing() const;

 private:
    
    struct cell_t
    {
        cell_t(const any_regular_t& value) : value_m(value) { }
        any_regular_t value_m;
    };
    
    struct interface_cell_t : cell_t
    {
        interface_cell_t(const any_regular_t& value) : cell_t(value) { }
        
        interface_cell_t(const interface_cell_t& x) : cell_t(x.value_m) { }
        interface_cell_t& operator=(const interface_cell_t& x)
        {
            value_m = x.value_m; // copying a value could throw so it goes first
            // monitor_m is not copied - nor can it be
            return *this;
        }
                
                monitor_value_list_t monitor_value_m;
    };
    
    typedef std::map<const char*, interface_cell_t*, str_less_t>    interface_index_t;
    typedef std::map<const char*, const cell_t*, str_less_t>        variable_index_t;
    
    interface_cell_t* lookup_interface(name_t);
    
    variable_index_t                variable_index_m;
    interface_index_t               interface_index_m;

    std::deque<cell_t>              constant_cell_set_m;
    std::deque<interface_cell_t>    interface_cell_set_m;
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
