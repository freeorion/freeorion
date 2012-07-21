/*
    Copyright 2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/

#ifndef ADOBE_VIRTUAL_MACHINE_EXTENSION_HPP
#define ADOBE_VIRTUAL_MACHINE_EXTENSION_HPP

/**************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/closed_hash.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>
#include <GG/adobe/virtual_machine.hpp>
#include <GG/adobe/adam.hpp>

#include <boost/bind.hpp>
#include <boost/function.hpp>


namespace GG {
    class Texture;
}

/**************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

namespace implementation {

/**************************************************************************************************/

any_regular_t vm_dictionary_image_proc(const dictionary_t& named_argument_set);
any_regular_t vm_array_image_proc(const array_t& argument_set);

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/
/*!
    This is a hack to give this particular GIL image type serialization support so it can be
    put into an any_regular_t. It is not intended to be used-- ever.

    \todo replace this with something correct.
*/
std::ostream& operator<<(std::ostream& s, const boost::shared_ptr<GG::Texture>& image);

/**************************************************************************************************/

any_regular_t asl_standard_dictionary_function_lookup(name_t              function_name,
                                                      const dictionary_t& named_argument_set);

any_regular_t asl_standard_array_function_lookup(name_t         function_name,
                                                 const array_t& argument_set);

/**************************************************************************************************/

inline
void set_dictionary_function_lookup_to(virtual_machine_t&                                     machine,
                                       const virtual_machine_t::dictionary_function_lookup_t& proc)
{
    machine.set_dictionary_function_lookup(proc);
}

/**************************************************************************************************/

inline
void set_array_function_lookup_to(virtual_machine_t&                                machine,
                                  const virtual_machine_t::array_function_lookup_t& proc)
{
    machine.set_array_function_lookup(proc);
}

/**************************************************************************************************/

inline void default_vm_function_lookup(virtual_machine_t& machine)
{
    machine.set_dictionary_function_lookup(&asl_standard_dictionary_function_lookup);
    machine.set_array_function_lookup(&asl_standard_array_function_lookup);
}

/**************************************************************************************************/

struct vm_lookup_t
{
public:
    typedef boost::function<any_regular_t (const dictionary_t&)> dictionary_function_t;
    typedef boost::function<any_regular_t (const array_t&)>      array_function_t;
    typedef boost::function<any_regular_t (name_t)>              variable_function_t;

    typedef closed_hash_map<name_t, dictionary_function_t> dictionary_function_map_t;
    typedef closed_hash_map<name_t, array_function_t >      array_function_map_t;

    vm_lookup_t();

    void insert_dictionary_function(name_t name, const dictionary_function_t& proc);
    void insert_array_function(name_t name, const array_function_t& proc);

    void attach_to(virtual_machine_t& vm);
    void attach_to(sheet_t&);

private:
    any_regular_t dproc(name_t name, const dictionary_t& argument_set) const;
    any_regular_t aproc(name_t name, const array_t& argument_set) const;

    dictionary_function_map_t dmap_m;
    array_function_map_t      amap_m;
    
    /*
        REVISIT (sparent) : Currently there is no mechanism for adding variable lookup functions.
        This is a shim used by attach_to(sheet) - which should be replaced with a general
        mechanism later.
    */
    
    variable_function_t variable_lookup_m;
};

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif

/**************************************************************************************************/
