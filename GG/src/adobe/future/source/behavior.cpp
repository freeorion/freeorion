/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/behavior.hpp>

#ifndef NDEBUG
#include <iostream>
#endif

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

behavior_t::behavior_t(bool single_execution) :
    single_execution_m(single_execution)
{ }

/****************************************************************************************************/

void behavior_t::operator () ()
{
    assert(order_set_m.size() == verb_set_m.size() + behavior_set_m.size());

    if (order_set_m.empty())
        return;

    verb_set_t::iterator     vfirst(verb_set_m.begin());
    behavior_set_t::iterator bfirst(behavior_set_m.begin());
    order_set_t::iterator    first(order_set_m.begin());
    order_set_t::iterator    last(order_set_m.end());

    for (; first != last; ++first)
    {
        if (*first == behavior_t::order_verb_k)
        {
            (*vfirst)();

            ++vfirst;
        }
        else if (*first == behavior_t::order_behavior_k)
        {
            (*bfirst)();

            ++bfirst;
        }
    }

    if (single_execution_m)
    {
        behavior_set_m.clear();
        verb_set_m.clear();
        order_set_m.clear();
    }
}

/****************************************************************************************************/

behavior_t::behavior_token_t behavior_t::insert_behavior(bool single_execution)
{
    order_set_m.push_back(behavior_t::order_behavior_k);

    behavior_set_m.push_back(behavior_t(single_execution));

    return --(behavior_set_m.end());
}

/****************************************************************************************************/

behavior_t::verb_token_t behavior_t::insert(const verb_t& verb)
{
    order_set_m.push_back(behavior_t::order_verb_k);

    verb_set_m.push_back(verb);

    return --(verb_set_m.end());
}

/****************************************************************************************************/

void behavior_t::reset(verb_token_t token, const verb_t& verb)
{
    *token = verb;
}

/****************************************************************************************************/

void behavior_t::disconnect(behavior_token_t behavior)
{
    assert(behavior_set_m.empty() == false);

    std::size_t index(static_cast<std::size_t>(std::distance(behavior_set_m.begin(), behavior)));

    behavior_set_m.erase(behavior);

    erase_from_order(order_behavior_k, index);
}

/****************************************************************************************************/

void behavior_t::disconnect(verb_token_t verb)
{
    assert(verb_set_m.empty() == false);

    std::size_t index(static_cast<std::size_t>(std::distance(verb_set_m.begin(), verb)));

    verb_set_m.erase(verb);

    erase_from_order(order_verb_k, index);
}

/****************************************************************************************************/

void behavior_t::erase_from_order(order_t type, std::size_t index)
{
    order_set_t::iterator first(order_set_m.begin());
    order_set_t::iterator last(order_set_m.end());
    std::size_t           i(0);

    while (first != last)
    {
        if (*first == type)
        {
            if (i == index)
            {
                order_set_m.erase(first);

                break;
            }
            else
            {
                ++i;
            }
        }

        ++first;
    }
}

/****************************************************************************************************/

behavior_t& general_deferred_proc_queue()
{
    static behavior_t gdpq_s(true);

    return gdpq_s;
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
