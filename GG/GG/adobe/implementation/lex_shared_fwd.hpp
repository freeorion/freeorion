/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_LEX_SHARED_FWD_HPP
#define ADOBE_LEX_SHARED_FWD_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <functional>
#include <iterator>
#include <iterator>
#include <utility>

#include <boost/operators.hpp>
#include <boost/static_assert.hpp>

#include <GG/adobe/algorithm/copy.hpp>
#include <GG/adobe/algorithm/lower_bound.hpp>
#include <GG/adobe/algorithm/mismatch.hpp>
#include <GG/adobe/algorithm/sort.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/utility.hpp>

#ifndef NDEBUG
    #include <iostream>
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

typedef const unsigned char* uchar_ptr_t;

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

Compares two ranges of data with a binary predicate. 

\param first1 Iterator to the first element of the first range
\param last1 Iterator to one-past the last element of the first range
\param first2 Iterator to the first element of the second range
\param last2 Iterator to one-past the last element of the second range
\param pred \ref concept_convertible_to_function that models a BinaryPredicate

\return
When the two ranges are of equal length and pairwise-comparison of the
ranges' elements returns true, the function itself returns true.
Otherwise, the function returns false.
*/

template <  typename I1, // models InputIterator
            typename I2, // models InputIterator
            typename BP> // models BinaryPredicate
inline bool bounded_equal(I1 first1, I1 last1, I2 first2, I2 last2, BP pred)
{
    for (; first1 != last1 && first2 != last2; ++first1, ++first2)
        if (!pred(*first1, *first2)) return false;

    return first1 == last1 && first2 == last2;
}

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

Compares two ranges of data with a default binary predicate (std::equal_to<>()). 

\param first1 Iterator to the first element of the first range
\param last1 Iterator to one-past the last element of the first range
\param first2 Iterator to the first element of the second range
\param last2 Iterator to one-past the last element of the second range

\return
When the two ranges are of equal length and pairwise-comparison of the
ranges' elements returns true, the function itself returns true.
Otherwise, the function returns false.
*/

template <  typename I1, // models InputIterator
            typename I2> // models InputIterator
inline bool bounded_equal(I1 first1, I1 last1, I2 first2, I2 last2)
{
    typedef typename std::iterator_traits<I1>::value_type value_type;

    return bounded_equal(first1, last1, first2, last2, std::equal_to<value_type>());
}

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

Compares two ranges of data with a default binary predicate (std::equal_to<>()). 

\param range1 Iterator range to the first range
\param range2 Iterator range to the second range

\return
When the two ranges are of equal length and pairwise-comparison of the
ranges' elements returns true, the function itself returns true.
Otherwise, the function returns false.
*/

template <  typename R1, // models InputRange
            typename R2> // models InputRange
inline bool bounded_equal(R1& range1, R2& range2)
{
    return bounded_equal(   boost::begin(range1), boost::end(range1),
                            boost::begin(range2), boost::end(range2));
}

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

\brief A range of pointers denoting a token within a character stream.

token_range_t is a utility class mostly used by the in-memory ASL
parsers such as xml_parser_t. Token ranges are accompanied by a
host of algorithms used to compare them to one another. Because they are
a pair of pointers they do not copy any memory dynamically, and so are
very fast to construct and copy. The data to which they point could be
shared across countless other token_range_t instances, so the
modification of the tokens they represent is prohibited.

A NULL-token as it relates to token_range_t is one where both the first
and second value are equivalent, but any value. Typically this value is
0, but this is not required.
*/

typedef std::pair<uchar_ptr_t, uchar_ptr_t> token_range_t;

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

Compares two token ranges for equality.

For two token ranges to be equal their lengths must be the same and a
character-by-character comparison of the two ranges must be true.

\param x the first range to be compared
\param y the second range to be compared

\return true when a character-wise comparsion of the two ranges results
in equality for each set of characters. false otherwise.
*/

inline bool token_range_equal(const token_range_t& x, const token_range_t& y)
{
    return boost::size(x) == boost::size(y) && adobe::bounded_equal(x, y);
}

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

Compares two token ranges for strict weak ordering.

Unlike standard string comparison this algorithm optimizes sorting based
on the lengths of the tokens involved. If a token is smaller than
another it is considered to be "less" than the other. Character-based
sorting only takes place when the length of the two tokens is the same.
When this is the case the algorithm sorts the tokens in the same manner
as adobe::mismatch.

\param x the first range to be compared
\param y the second range to be compared

\return true when:
    - the first range is smaller in length than the second range, or
    - in the case when the ranges are the same length, that the
    character at the point when the two ranges differ is smaller in the
    first range.
    - (false otherwise.)
*/

inline bool token_range_less(const token_range_t& x, const token_range_t& y)
{
    std::size_t sizex(boost::size(x));
    std::size_t sizey(boost::size(y));

    if (sizex < sizey) return true;
    else if (sizey < sizex) return false;

    std::pair<uchar_ptr_t, uchar_ptr_t> diff(adobe::mismatch(x, boost::begin(y)));

    if (diff.first == boost::end(x)) return false;

    return *diff.first < *diff.second;
}

/*************************************************************************************************/

#ifndef NDEBUG
/*!
\ingroup asl_xml_parser

Serializes a token_range_t to an output stream

\param s stream to take the token_range_t
\param x the token_range_t to be output to the stream

\return the original output stream.
*/

inline std::ostream& operator << (std::ostream& s, const token_range_t& x)
{
    adobe::copy(x, std::ostream_iterator<char>(s));

    return s;
}
#endif

/*************************************************************************************************/

/*!
\ingroup asl_xml_parser

static_token_range is a utility class that creates an
token_range_t from a compile-time NTBS (null-terminated byte
string). The precondition for this function is that the token's source
is an NTBS that exists at least as long as the token itself. Note that
this function does not copy the character range internally, and makes no
warranties as to string ownership.
*/

template <typename T>
inline token_range_t static_token_range(T* begin)
{
    BOOST_STATIC_ASSERT(sizeof(T) == sizeof(unsigned char));

    T* end(begin);

    while (*end != 0)
        ++end;

    return token_range_t(reinterpret_cast<uchar_ptr_t>(begin), reinterpret_cast<uchar_ptr_t>(end));
}

/*************************************************************************************************/

template <typename E> // E models Enumeration
struct lex_token_t
{
    lex_token_t()
    { }

    explicit lex_token_t(E enumeration, uchar_ptr_t first = 0, uchar_ptr_t last = 0) :
        enum_m(enumeration), range_m(first, last)
    { }

    E               enum_m;
    token_range_t   range_m;
};

/*************************************************************************************************/

typedef pair<name_t, any_regular_t> stream_lex_token_t;

/*************************************************************************************************/

extern aggregate_name_t eof_k; // EOF token name

/*************************************************************************************************/

template <typename E> inline E eof_token(); // specialize

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
