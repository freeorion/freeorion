/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_STRING_POOL_HPP
#define ADOBE_STRING_POOL_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/noncopyable.hpp>

/*************************************************************************************************/

namespace adobe {

/**************************************************************************************************/

class unique_string_pool_t : boost::noncopyable
{
public:
    unique_string_pool_t();

    ~unique_string_pool_t();

    const char* add(const char* str);

private:
    struct implementation_t;

    implementation_t* object_m;
};


/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

#endif

/**************************************************************************************************/
