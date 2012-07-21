/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_KEYBOARD_HPP
#define ADOBE_KEYBOARD_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/poly_key_handler.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/forest.hpp>
#include <GG/adobe/widget_attributes.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct keyboard_t
{
    typedef forest<poly_key_handler_t>  keyboard_forest_t;
    typedef keyboard_forest_t::iterator iterator;

    static keyboard_t& get();

    iterator insert(iterator parent, const poly_key_handler_t& element);

    void erase(iterator position);

    bool dispatch(key_type             virtual_key,
                  bool                 pressed,
                  modifiers_t          modifiers,
                  const any_regular_t& base_handler);

private:
    iterator handler_to_iterator(const any_regular_t& handler);

    keyboard_forest_t forest_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
// ADOBE_KEYBOARD_HPP
#endif

/****************************************************************************************************/
