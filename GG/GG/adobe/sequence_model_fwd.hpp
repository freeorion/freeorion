/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/******************************************************************************/

#ifndef ADOBE_SEQUENCE_MODEL_FWD_HPP
#define ADOBE_SEQUENCE_MODEL_FWD_HPP

/******************************************************************************/

#ifdef ADOBE_STD_SERIALIZATION
    #include <iostream>
#endif

#include <boost/operators.hpp>

#include <GG/adobe/copy_on_write.hpp>

/******************************************************************************/

namespace adobe {

/******************************************************************************/

template <typename T>
class sequence_model;

/******************************************************************************/

namespace version_0 {

/******************************************************************************/

template <typename T>
class sequence_key : boost::equality_comparable<sequence_key<T> >
{
    typedef T                         value_type;
    typedef copy_on_write<value_type> cow_value_type;

public:
    static const sequence_key nkey;

    sequence_key() :
        value_m(0)
    { }

    sequence_key(const sequence_key& rhs) :
        value_m(rhs.value_m)
    { }

    sequence_key& operator=(const sequence_key& rhs)
    { value_m = rhs.value_m; return *this; }

    inline friend bool operator==(const sequence_key& x, const sequence_key& y)
    { return x.value_m == y.value_m; }

private:
    friend class sequence_model<T>;

    explicit sequence_key(cow_value_type& value) :
        value_m(&value)
    { }

    cow_value_type* value_m;

#ifdef ADOBE_STD_SERIALIZATION
    inline friend std::ostream& operator<<(std::ostream& s, const sequence_key& key)
    {
        return key == nkey ?
               s << "nkey" :
               s << reinterpret_cast<std::size_t>(key.value_m);
    }
#endif
};

template <typename T>
const sequence_key<T> sequence_key<T>::nkey;

/******************************************************************************/

} // namespace version_0

/******************************************************************************/

using adobe::version_0::sequence_key;

/******************************************************************************/

} // namespace adobe

/******************************************************************************/

// This makes sequence keys binary compatible, so you can pass them across DLL
// boundaries. (They are (and should always remain) simply pointers, after all.)
ADOBE_NAME_TYPE_1("sequence_key:version_0:adobe", adobe::version_0::sequence_key<T0>)

/******************************************************************************/
// ADOBE_SEQUENCE_MODEL_FWD_HPP
#endif

/******************************************************************************/
