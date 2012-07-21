/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_STATIC_TABLE_HPP
#define ADOBE_STATIC_TABLE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <utility>
#include <stdexcept>

#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/algorithm/sort.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

//***************************************************************************//
//***************************************************************************//
//***************************************************************************//

/*!
\class adobe::static_table
\ingroup other_container

\brief A simple lookup table of fixed size. [under review]

static_table is intended to encapsulate the code used to initialize, sort, and perform lookups on lookup tables of static size. They are intended to be initialized without runtime code, though they must be sorted at runtime before any lookups can occur. Performing a lookup before the table is sorted will have undefined results.

\example
\code
    typdef adobe::static_table<adobe::name_t, boost::function<void (int)>, 4> table_t;

    static table_t some_table_s =
    {{
        table_t::entry_type(adobe::static_name_t("foo"),    &do_foo),
        table_t::entry_type(adobe::static_name_t("bar"),    &do_bar),
        table_t::entry_type(adobe::static_name_t("baz"),    &do_baz),
        table_t::entry_type(adobe::static_name_t("snafu"),  &do_snafu)
    }};

    some_table_s.sort();

    some_table_s(adobe::static_name_t("baz"))(42); // calls do_baz
\endcode
*/

/*!
\typedef adobe::static_table::traits_type

The static_table_traits used to extend this static_table's functionality.
*/

/*!
\typedef adobe::static_table::key_type

The type used for lookups in the table.
*/

/*!
\typedef adobe::static_table::value_type

The resultant type from a table lookup.
*/

/*!
\typedef adobe::static_table::entry_type

A pair comprised of a key_type and a value_type.
*/

/*!
\var adobe::static_table::table_m

The static lookup table contents. <i>This variable is not intended to be manipulated directly.</i> It is publicly available to support static table initialization by the C++ compiler.
*/

/*!
\fn const adobe::static_table::value_type& adobe::static_table::operator()(const adobe::static_table::key_type& key) const

\param key The key whose stored value we are searching for.

\exception std::logic_error Thrown if the key does not exist in the table.

\return a reference to the value found associated with <code>key</code>.

\note Calling this function before calling sort() yields undefined results.
*/

/*!
\fn bool adobe::static_table::operator()(const adobe::static_table::key_type& key, adobe::static_table::value_type& result) const

\param key The key whose stored value we are searching for.
\param result Set to the value associated with the key if <code>key</code> is found.

\exception None Guaranteed not to throw.

\return <code>true</code> if <code>key</code> was found and result's assignment did not throw. <code>false</code> otherwise.

\note Calling this function before calling sort() yields undefined results.
*/

/*!
\fn void adobe::static_table::sort()

Sorts the contents of the table according to the static_table_traits type.
*/

//***************************************************************************//
//***************************************************************************//
//***************************************************************************//

/*! 
\class adobe::static_table_traits
\ingroup other_container

\brief A traits class for use with adobe::static_table.

static_table_traits provides functionality lifted out of the static_table class so clients can add their own traits should their key types require custom comparison and equality functionality. An example key type that would require a customized static_table_traits class would be <code>boost::reference_wrapper<const std::type_info></code>, as the default-supplied functionality is not compatible.
*/

/*!
\typedef adobe::static_table_traits::result_type

The result type retured by operator() in this function object. Must always be <code>bool</code>. Required by boost::bind.
*/

/*!
\typedef adobe::static_table_traits::key_type

The type used for lookups in the table.
*/

/*!
\typedef adobe::static_table_traits::value_type

The resultant type from a table lookup.
*/

/*!
\typedef adobe::static_table_traits::entry_type

A pair comprised of a key_type and a value_type.
*/

/*!
\fn adobe::static_table_traits::result_type adobe::static_table_traits::operator()(const adobe::static_table_traits::entry_type& x, const adobe::static_table_traits::entry_type& y) const

\param x The first entry
\param y The second entry

\return
    <code>true</code> if <code>x</code>'s key &lt; <code>y</code>'s key; <code>false</code> otherwise.
*/

/*!
\fn adobe::static_table_traits::result_type adobe::static_table_traits::operator()(const adobe::static_table_traits::entry_type& x, const adobe::static_table_traits::key_type& y) const

\param x The table entry
\param y An arbitrary key

\return
    <code>true</code> if <code>x</code>'s key &lt; <code>y</code>; <code>false</code> otherwise.
*/

/*!
\fn adobe::static_table_traits::result_type adobe::static_table_traits::equal(const adobe::static_table_traits::key_type& x, const adobe::static_table_traits::key_type& y) const

\param x The first key
\param y The second key

\return
    <code>true</code> if <code>x</code> == <code>y</code>; <code>false</code> otherwise.
*/


template <typename KeyType, typename ValueType>
struct static_table_traits
{
    typedef bool                            result_type;
    typedef KeyType                         key_type;
    typedef ValueType                       value_type;
    typedef std::pair<key_type, value_type> entry_type;

    result_type operator()(const entry_type& x, const entry_type& y) const
    {
        return (*this)(x, y.first);
    }

    // revisit: MM. For debugging purposes, VC 8 requires the definition of
    // this (unnecessary overload) in debug versions.
    result_type operator()(const key_type& x, const entry_type& y) const
    {
        return x < y.first;
    }

    result_type operator()(const entry_type& x, const key_type& y) const
    {
        return x.first < y;
    }

    result_type equal(const key_type& x, const key_type& y) const
    {
        return x == y;
    }
};

/*************************************************************************************************/

template <typename KeyType, typename ValueType, std::size_t Size, typename Traits = static_table_traits<KeyType, ValueType> >
struct static_table
{
    typedef Traits                              traits_type;
    typedef typename traits_type::key_type      key_type;
    typedef typename traits_type::value_type    value_type;
    typedef typename traits_type::entry_type    entry_type;

    const value_type& operator()(const key_type& key) const
    {
        const entry_type* iter(adobe::lower_bound(table_m, key, traits_type()));

        if (iter == boost::end(table_m) || !traits_type().equal(iter->first, key))
            throw std::logic_error("static_table key not found");

        return iter->second;
    }

    bool operator()(const key_type& key, value_type& result) const
    {
        const entry_type* iter(adobe::lower_bound(table_m, key, traits_type()));

        if (iter == boost::end(table_m) || !traits_type().equal(iter->first, key))
            return false;

        result = iter->second;

        return true;
    }

    void sort()
    {
        adobe::sort(table_m, traits_type());
    }

public:
    entry_type table_m[Size];
};

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif // ADOBE_STATIC_TABLE_HPP

/*************************************************************************************************/
