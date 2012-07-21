/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_DIRTY_VALUE_HPP
#define ADOBE_DIRTY_VALUE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <typename T>
struct dirty_value
{
    typedef T value_type;

    dirty_value(const value_type& x = value_type()) :
        value_m(x), dirty_m(false)
    { }

    dirty_value(const dirty_value& rhs) :
        value_m(rhs.value_m), dirty_m(false)
    { }

    dirty_value& operator = (const value_type& rhs)
    {
        if (value_m == rhs)
            return *this;

        value_m = rhs;
        dirty_m = true;
        return *this;
    }

    dirty_value& operator = (const dirty_value& rhs)
    { return *this = rhs.value_m; }

    const value_type& operator*() const
    { return value_m; }
    const value_type* operator->() const
    { return &value_m; }
    operator const value_type& () const
    { return value_m; }

    void clean()
    { dirty_m = false; }

    bool is_dirty() const
    { return dirty_m; }

    struct cleaner
    {
        typedef dirty_value first_argument_type;
        typedef void        result_type;

        inline result_type operator()(first_argument_type& x) const
        { x.clean(); }
    };

    value_type value_m;
    bool       dirty_m;
};

/*************************************************************************************************/

/*
    NOTE (fbrereto) : We cannot use boost::totaly_ordered to implement
    these operations portably. The problem is that we do not know if T
    satifies the requirments for totally ordered or not - dirty_value is
    only totally ordered if T is. By splitting the operations out to
    seperate template functions they are only instantiated if and where
    they are used.
*/

template <typename T>
inline bool operator<(const dirty_value<T>& x, const dirty_value<T>& y)
{ return *x < *y; }

template <typename T>
inline bool operator>(const dirty_value<T>& x, const dirty_value<T>& y)
{ return y < x; }

template <typename T>
inline bool operator<=(const dirty_value<T>& x, const dirty_value<T>& y)
{ return !(y < x); }

template <typename T>
inline bool operator>=(const dirty_value<T>& x, const dirty_value<T>& y)
{ return !(x < y); }

template <typename T>
inline bool operator==(const dirty_value<T>& x, const dirty_value<T>& y)
{ return *x == *y; }

template <typename T>
inline bool operator!=(const dirty_value<T>& x, const dirty_value<T>& y)
{ return !(x == y); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_DIRTY_VALUE_HPP
#endif

/*************************************************************************************************/
