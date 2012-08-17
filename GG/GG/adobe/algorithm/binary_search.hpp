/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ALGORITHM_BINARY_SEARCH_HPP
#define ADOBE_ALGORITHM_BINARY_SEARCH_HPP

#include <GG/adobe/config.hpp>

#include <boost/range/begin.hpp>
#include <boost/range/const_iterator.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>

#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/functional/operator.hpp>

/*************************************************************************************************/

namespace adobe {

namespace implementation {

template <typename I, // I models ForwardIterator
          typename T, // T models Regular
          typename C, // C models StrictWeakOrdering
          typename P> // P models UnaryFunction(value_type(I)) -> T
inline I binary_search(I f, I l, const T& x, C c, P p)
{
    I result = adobe::lower_bound(f, l, x, c, p);
    if (result != l && p(*result) == x) return result;
    return l;
}

}

/*************************************************************************************************/
/*!
\defgroup binary_search binary_search
\ingroup sorting

\c binary_search() is a version of binary search: it attempts to find the element value in an
ordered range <code>[f, l)</code>. It returns an iterator to the first instance that is equivalent
to [1] \c x if such a value s present and \c l if no such elment exists. \c binary_search()
takes an optional comparision function and uses \c adobe::less() if not provided.

\requirements
    - \c I is a model of \ref stldoc_ForwardIterator.
    - \c C is a model of \ref stldoc_LessThanComparable.
    - <code>value_type(I) == argument_type(C)</code>

\pre
    - <code>[f, l)</code> is a valid range.
    - <code>[f, l)</code> is ordered in ascending order according to \c c. That is, for every pair
of iterators \c i and \c j in <code>[f, l)</code> such that \c i precedes \c j,
<code>c(*j, *i)</code> is false.

\complexity
The number of comparisons is logarithmic: at most <code>log(l - f) + 2</code>. If I is a
\ref stldoc_RandomAccessIterator then the number of steps through the range is also logarithmic;
otherwise, the number of steps is proportional to last - first. [2]

\notes
[1] Note that you may use an ordering that is a strict weak ordering but not a total ordering; that 
is, there might be values x and y such that x < y, x > y, and x == y are all false. 
(See the \ref stldoc_LessThanComparable requirements for a more complete discussion.) Finding \c x
in the range <code>[f, l)</code>, then, doesn't mean finding an element that is equal to \c x but
rather one that is equivalent to \c x: one that is neither greater than nor less than \c x. If
you're using a total ordering, however (if you're using \c strcmp, for example, or if you're using 
ordinary arithmetic comparison on integers), then you can ignore this technical distinction: for a
total ordering, equality and equivalence are the same.

[2] This difference between \ref stldoc_RandomAccessIterator and \ref stldoc_ForwardIterator is 
simply because advance is constant time for \ref stldoc_RandomAccessIterator and linear time for 
\ref stldoc_ForwardIterator.

\note
\c binary_search() differs from \ref stldoc_binary_search in that it returns an iterator to the
first element rather than simply a \c bool. This is commonly a more useful function.
\c binary_search is similar to \ref lower_bound except it returns \c l if no element matching \c x
exists.

\see
    - STL documentation for \ref stldoc_binary_search
    - \ref lower_bound
    - \ref upper_bound
    - \ref equal_range

    @{
*/
/*************************************************************************************************/

template <typename I, // I models ForwardIterator
          typename T, // T models Regular
          typename C, // C models StrictWeakOrdering
          typename P> // P models UnaryFunction(value_type(I)) -> T
inline I binary_search(I f, I l, const T& x, C c, P p)
{
    return implementation::binary_search(f, l, x, c, boost::bind(p, _1));
}


template <typename I, // I models ForwardIterator
          typename T, // T models Regular
          typename C> // C models StrictWeakOrdering
inline I binary_search(I f, I l, const T& x, C c)
{
    return adobe::binary_search(f, l, x, c, identity<T>());
}

template <typename I, // I models ForwardIterator
          typename T> // T models Regular
inline I binary_search(I f, I l, const T& x)
{ return binary_search(f, l, x, less()); }

template <typename I, typename T>
inline typename boost::range_iterator<I>::type binary_search(I& range, const T& x)
{
    return adobe::binary_search(boost::begin(range), boost::end(range), x);
}

template <typename I, typename T>
inline typename boost::range_const_iterator<I>::type binary_search(const I& range, const T& x)
{
    return adobe::binary_search(boost::begin(range), boost::end(range), x);
}

template <typename I, typename T, typename C>
inline typename boost::range_iterator<I>::type binary_search(I& range, const T& x, C c)
{
    return adobe::binary_search(boost::begin(range), boost::end(range), x, c);
}

template <typename I, typename T, typename C>
inline typename boost::range_const_iterator<I>::type binary_search(const I& range, const T& x, C c)
{
    return adobe::binary_search(boost::begin(range), boost::end(range), x, c);
}

/*************************************************************************************************/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
    typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_iterator<I> >::type
        binary_search(I& r, const T& x, C c, P p)
{ return adobe::binary_search(boost::begin(r), boost::end(r), x, c, p); }

/*************************************************************************************************/

template <  typename I, // I models ForwardRange
            typename T, // T == result_type(P)
            typename C, // C models StrictWeakOrdering(T, T)
            typename P> // P models UnaryFunction(value_type(I)) -> T
inline
    typename boost::lazy_disable_if<boost::is_same<I, T>, boost::range_const_iterator<I> >::type
        binary_search(const I& r, const T& x, C c, P p)
{ return adobe::binary_search(boost::begin(r), boost::end(r), x, c, p); }

/*!
@}
*/

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
