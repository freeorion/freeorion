/*
    notifyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a notify at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_NOTIFY_ON_WRITE_HPP
#define ADOBE_NOTIFY_ON_WRITE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <boost/function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

template <typename T, typename Notifier>
class notify_on_write
{
public:
    typedef T                                          value_type;
    typedef Notifier                                   notifier_type;
    typedef notify_on_write<value_type, notifier_type> self_type;

    explicit notify_on_write(const value_type& x = value_type(),
                             const Notifier&   notifier = notifier_type()) :
        value_m(x),
        notifier_m(notifier)
    { notifier_m.ctor(const_cast<const self_type&>(*this)); }

    notify_on_write(const notify_on_write& rhs) :
        value_m(rhs.value_m),
        notifier_m(rhs.notifier_m)
    { notifier_m.ctor(const_cast<const self_type&>(*this)); }

    ~notify_on_write()
    { notifier_m.dtor(const_cast<const self_type&>(*this)); }

    notify_on_write& operator=(const value_type& x)
    {
        if (value_m == x)
            return *this;

        value_m = x;

        notifier_m.modify(const_cast<const self_type&>(*this));

        return *this;
    }

    notify_on_write& operator=(const notify_on_write& x)
    { return *this = x.value_m; }

    template <typename UnaryFunction>
    void write(UnaryFunction proc)
    {
        proc(value_m);

        notifier_m.modify(const_cast<const self_type&>(*this));
    }

    operator const value_type& () const
    { return value_m; }

    const value_type& operator*() const { return value_m; }
    const value_type* operator->() const { return &value_m; }

private:
#if !defined(ADOBE_NO_DOCUMENTATION)
    value_type    value_m;
    notifier_type notifier_m;
#endif
};

/*************************************************************************************************/
#if !defined(ADOBE_NO_DOCUMENTATION)
/*
    NOTE (sparent) : We cannot use boost::totaly_ordered to implement these operations portably
    (although it works with most compilers, it doesn't with CW 9.6). The problem is that 
    we do not know if T satifies the requirements for totally ordered or not - notify_on_write is
    only totally ordered if T is. By splitting the operations out to seperate template functions
    they are only instantiated if and where they are used.
*/

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator<(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return *x < *y; }

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator>(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return y < x; }

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator<=(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return !(y < x); }

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator>=(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return !(x < y); }

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator==(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return *x == *y; }

template <typename T, typename Notifier1, typename Notifier2>
inline bool operator!=(const notify_on_write<T, Notifier1>& x, const notify_on_write<T, Notifier2>& y)
{ return !(x == y); }
#endif
/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
