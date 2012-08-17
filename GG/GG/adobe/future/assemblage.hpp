/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ASSEMBLAGE_HPP
#define ADOBE_ASSEMBLAGE_HPP

#include <GG/adobe/config.hpp>

#include <list>

#include <boost/bind/apply.hpp>
#include <boost/function.hpp>
#include <boost/signals/connection.hpp>

#include <GG/adobe/algorithm/for_each.hpp>
#include <GG/adobe/memory.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

class assemblage_t
{
public:
    ~assemblage_t()
    {
        // disconnect all our connections
        adobe::for_each(cleanup_m, boost::bind(boost::apply<void>(), _1));
    }

    void cleanup(boost::function<void()> f)
    {
        cleanup_m.push_front(f);
    }

private:
    std::list<boost::function<void ()> >          cleanup_m;
};

/****************************************************************************************************/

template <typename T>
inline void assemblage_cleanup_ptr(assemblage_t& assemblage, T* x)
{ assemblage.cleanup(boost::bind(delete_ptr<T*>(), x)); }

/****************************************************************************************************/

inline void assemblage_cleanup_connection(assemblage_t& assemblage, boost::signals::connection& x)
{ assemblage.cleanup(boost::bind(&boost::signals::connection::disconnect, x)); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
