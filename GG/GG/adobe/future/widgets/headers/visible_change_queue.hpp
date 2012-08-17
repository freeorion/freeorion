/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_VISIBLE_CHANGE_QUEUE_HPP
#define ADOBE_VISIBLE_CHANGE_QUEUE_HPP

/****************************************************************************************************/

#include <vector>

#include <GG/adobe/eve.hpp>
#include <GG/adobe/future/behavior.hpp>
#include <GG/adobe/name.hpp>

/****************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*
    REVISIT (sparent) : As a current "hack" for handling the drawing order of widgets when optional
    panels come and go we are going to define a dpq of callbacks which need to be shown or hidden
    and the window will handle the logic. Still need to figure out the right place to put this.
*/
struct visible_change_queue_t
{
    visible_change_queue_t(eve_t& eve) :
        root_m(false),
        hide_queue_m(root_m.insert_behavior(true)),
        eve_eval_token_m(root_m.insert(boost::bind(&eve_t::evaluate, boost::ref(eve), eve_t::evaluate_nested, 0, 0))),
        show_queue_m(root_m.insert_behavior(true)),
        force_m(false)
    { }

    behavior_t                   root_m;
    behavior_t::behavior_token_t hide_queue_m;
    behavior_t::verb_token_t     eve_eval_token_m;
    behavior_t::behavior_token_t show_queue_m;
    bool                         force_m; // force an update irrespective of the show/hide queues being empty

    void update()
    {
        if (force_m || hide_queue_m->empty() == false || show_queue_m->empty() == false)
            root_m();

        force_m = false;
    }
};


/****************************************************************************************************/

typedef std::vector<name_t> touch_set_t;

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
