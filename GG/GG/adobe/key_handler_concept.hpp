/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_KEY_HANDLER_HPP
#define ADOBE_KEY_HANDLER_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/future/platform_primitives.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/concept_check.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <class H> // H models KeyHandler
inline bool handle_key(H& handler, key_type key, bool pressed, modifiers_t modifiers)
{ return handler.handle_key(key, pressed, modifiers); }

/*************************************************************************************************/

template <class H> // H models KeyHandler
inline any_regular_t underlying_handler(H& handler)
{ return handler.underlying_handler(); }

/*************************************************************************************************/

    template <class Handler>
    struct KeyHandlerConcept
    {
        void constraints() {
            handle_key(handler, key, pressed, modifiers);
            underlying_handler(handler);
        }
        Handler handler;
        key_type key;
        bool pressed;
        modifiers_t modifiers;
    };

/*************************************************************************************************/

} //namespace adobe

/*************************************************************************************************/

#endif
