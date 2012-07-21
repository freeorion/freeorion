/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FIND_CLOSEST_HPP
#define ADOBE_FIND_CLOSEST_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <iterator>

#include <GG/adobe/future/ternary_function.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*
    find_closest takes a range and a value, and returns the closest value in the sequence to the
    value passed, for some definition of "closest". By default it compares the cardinal difference
    between the values to find the closest, though the client can pass their own TernaryPredicate
    to override this functionality.

    Preconditions:
        - The sequence to be searched must be sorted according to the comparison method used in
            the supplied TernaryPredicate.

    Postconditions:
        - As long as the size of the sequence is greater than 0, the result will always be a valid
            value inside the sequence. (i.e., the only time the end of the sequence is returned is
            when the sequence is empty).

    Additional Concepts:
        - Subtractable      :   Subtraction yields a difference type; T - T -> D
        - TernaryPredicate  :   A TernanyPredicate is called with three arguments
                                and returns true or false.
*/

/*************************************************************************************************/

template <  typename T  // T models Subtractable
         >
struct closer_predicate : ternary_function<T, T, T, bool>
{
    typedef ternary_function<T, T, T, bool>         _super;
    typedef typename _super::first_argument_type    first_argument_type;
    typedef typename _super::second_argument_type   second_argument_type;
    typedef typename _super::third_argument_type    third_argument_type;
    typedef typename _super::result_type            result_type;

    result_type operator () (const first_argument_type& a, const second_argument_type& b, const third_argument_type& x) const
    {
        // precondition: a <= b

        return x - a < b - x;
    }
};

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename D, // D models LessThanComparable  
            typename T, // T models Subtractable
            typename C  // C models TernaryPredicate
         >
I find_closest(I first, D n, const T& value, C pred, std::forward_iterator_tag)
{
    if (n < D(2)) return first;

    while (n != D(2))
    {
        D third(n / D(3));
        D new_n(n - third);
        I first_third(first);
        I last_third(first);

        std::advance(first_third, third);
        std::advance(last_third, new_n);

        if (!pred(*first_third, *last_third, value))
            first = first_third;

        n = new_n;
    }

    I second(first);

    std::advance(second, 1);

    return pred(*first, *second, value) ? first : second;
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename D, // D models LessThanComparable  
            typename T, // T models Subtractable
            typename C  // C models TernaryPredicate
         >
I find_closest(I first, D n, const T& value, C pred, std::random_access_iterator_tag)
{
    if (n < D(2)) return first;

    while (n != D(2))
    {
        D third(n / D(3));
        D new_n(n - third);

        if (!pred(*(first + third), *(first + new_n), value))
            first += third;

        n = new_n;
    }

    I second(first + 1);

    return pred(*first, *second, value) ? first : second;
}

/*************************************************************************************************/

} // namespace implementation

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename D, // D models LessThanComparable  
            typename T, // T models Subtractable
            typename C  // C models TernaryPredicate
         >
inline I find_closest(I first, D n, const T& value, C pred)
{
    typedef typename std::iterator_traits<I>::iterator_category category;

    return implementation::find_closest(first, n, value, pred, category());
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename D, // D models LessThanComparable  
            typename T  // T models Subtractable
         >
inline I find_closest(I first, D n, const T& value)
{
    typedef typename std::iterator_traits<I>::iterator_category category;

    return implementation::find_closest(first, n, value, closer_predicate<T>(), category()); 
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename T, // T models Subtractable
            typename C  // C models TernaryPredicate
         >
inline I find_closest(I first, I last, const T& value, C pred)
{
    typedef typename std::iterator_traits<I>::iterator_category category;

    return implementation::find_closest(first, std::distance(first, last), value, pred, category());
}

/*************************************************************************************************/

template <  typename I, // I models ForwardIterator
            typename T  // T models Subtractable
         >
inline I find_closest(I first, I last, const T& value)
{
    typedef typename std::iterator_traits<I>::iterator_category category;

    return implementation::find_closest(first, std::distance(first, last), value, closer_predicate<T>(), category());
}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
