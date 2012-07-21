/*
    Copyright 2008 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FUNCTIONAL_IS_MEMBER_HPP
#define ADOBE_FUNCTIONAL_IS_MEMBER_HPP

#include <GG/adobe/config.hpp>

#include <boost/iterator/iterator_traits.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/const_iterator.hpp>
#include <boost/range/end.hpp>

#include <GG/adobe/algorithm/binary_search.hpp>
#include <GG/adobe/functional/operator.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*! \addtogroup misc_functional
@{
*/

template <typename I,           // I ForwardIterator
          typename O = less>    // O modles StrictWeakOrdering
struct is_member
{
    typedef bool result_type;

    is_member(I f, I l, O o = O()) : first(f), last(l), compare(o) { }

    bool operator()(const typename boost::iterator_value<I>::type& x) const
    { return binary_search(first, last, x, compare) != last; }

    I first;
    I last;
    O compare;
};

template <typename I> // I models ForwardIterator
is_member<I, less> make_is_member(I f, I l) { return is_member<I, less>(f, l); }

template <typename I, // I models ForwardIterator
          typename O> // O modles StrictWeakOrdering
is_member<I, O> make_is_member(I f, I l, O o) { return is_member<I, O>(f, l, o); }

template <typename I> // I models ForwardRange
is_member<typename boost::range_const_iterator<I>::type, less> make_is_member(const I& r)
{ return make_is_member(begin(r), end(r)); }

template <typename I, // I models ForwardRange
          typename O> // O modles StrictWeakOrdering
is_member<typename boost::range_const_iterator<I>::type, O> make_is_member(const I& r, O o)
{ return make_is_member(begin(r), end(r), o); }

//!@}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
