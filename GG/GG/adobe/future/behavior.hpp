/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_DEFERRED_PROC_SYSTEM_HPP
#define ADOBE_DEFERRED_PROC_SYSTEM_HPP

/****************************************************************************************************/

#include <GG/adobe/memory.hpp>
#include <GG/adobe/name.hpp>

#include <boost/function.hpp>

#include <cassert>
#include <list>
#include <vector>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

typedef boost::function<void ()> verb_t;

class behavior_t
{
public:
    typedef std::list<verb_t>     verb_set_t;
    typedef std::list<behavior_t> behavior_set_t;

    typedef verb_set_t::iterator     verb_token_t;
    typedef behavior_set_t::iterator behavior_token_t;

    explicit behavior_t(bool single_execution);

    // Executes the behavior
    void operator () ();

    behavior_token_t insert_behavior(bool single_execution);
    verb_token_t     insert(const verb_t&);
    void             reset(verb_token_t, const verb_t&);
    void             disconnect(verb_token_t token);
    void             disconnect(behavior_token_t token);

    bool        empty() const
        { return order_set_m.empty(); }

    std::size_t size() const
        { return order_set_m.size(); }

private:
    enum order_t
    {
        order_verb_k = 0,
        order_behavior_k = 1
    };

    typedef std::vector<order_t> order_set_t;

    void erase_from_order(order_t type, std::size_t index);

    bool           single_execution_m;
    verb_set_t     verb_set_m;
    behavior_set_t behavior_set_m;
    order_set_t    order_set_m;
};

/****************************************************************************************************/

behavior_t& general_deferred_proc_queue();

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
