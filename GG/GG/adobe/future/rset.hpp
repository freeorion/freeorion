/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_RSET_HPP
#define ADOBE_RSET_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <functional>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/type_traits.hpp>
#include <boost/utility/enable_if.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\ingroup apl_libraries

\brief A bidirectional lookup table intended for a small set of elements

rset (reversible set) allows the client to specify a set of one-to-one relationships between
values of different types. The client is then able to search on any value in either type to
retrieve the corresponding value in the other type. The two base types can be the same. When
they are not, additional syntactic sugar is enabled to make for prettier code.

\note
Boost has a library that is far more robust than this one, with an extensive set of features
beyond what rset has to offer. However, for relatively small sets, this container can
prove faster than a more heavyweight counterpart. (See
http://www.boost.org/libs/multi_index/doc/index.html)
*/

template <typename Type1,
          typename Type2,
          typename Type1Equality = std::equal_to<Type1>,
          typename Type2Equality = std::equal_to<Type2> >
class rset
{
public:
    typedef Type1                              first_type;
    typedef Type2                              second_type;
    typedef Type1Equality                      first_compare_type;
    typedef Type2Equality                      second_compare_type;
    typedef std::pair<first_type, second_type> value_type;
    typedef std::vector<value_type>            set_type;
    typedef typename set_type::iterator        iterator;
    typedef typename set_type::const_iterator  const_iterator;
    typedef typename set_type::reference       reference;
    typedef typename set_type::const_reference const_reference;

    rset() :
        pred1_m(Type1Equality()),
        pred2_m(Type2Equality())
    { }

    rset(const rset& rhs) :
        set_m(rhs.set_m),
        pred1_m(rhs.pred1_m),
        pred2_m(rhs.pred2_m)
    { }

    rset& operator= (const rset& rhs)
    {
        set_m = rhs.set_m;
        pred1_m = rhs.pred1_m;
        pred2_m = rhs.pred2_m;

        return *this;
    }

    inline first_compare_type  first_compare() const  { return pred1_m; }
    inline second_compare_type second_compare() const { return pred2_m; }

    inline bool        empty() const { return set_m.empty(); }
    inline std::size_t size() const  { return set_m.size(); }

    inline void push_back(const value_type& value)
    { set_m.push_back(value); }

    inline void push_back(const first_type& first, const second_type& second)
    { push_back(value_type(first, second)); }

    inline iterator begin() { return set_m.begin(); }
    inline iterator end()   { return set_m.end(); }

    inline const_iterator begin() const { return set_m.begin(); }
    inline const_iterator end() const   { return set_m.end(); }

    inline reference at(std::size_t n)             { assert(n < size()); return set_m[n]; }
    inline const_reference at(std::size_t n) const { assert(n < size()); return set_m[n]; }

    first_type& find1(const second_type& key)
    {
        for (iterator iter(begin()), last(end()); iter != last; ++iter)
            if (pred2_m(key, iter->second))
                return iter->first;

        throw std::runtime_error("find2 key not found in set");
    }

    inline const first_type& find1(const second_type& key) const
    { return const_cast<rset*>(this)->find1(key); }

    second_type& find2(const first_type& key)
    {
        for (iterator iter(begin()), last(end()); iter != last; ++iter)
            if (pred1_m(key, iter->first))
                return iter->second;

        throw std::runtime_error("find1 key not found in set");
    }

    inline const second_type& find2(const first_type& key) const
    { return const_cast<rset*>(this)->find2(key); }

    inline typename boost::disable_if<boost::is_same<first_type, second_type>, second_type>::type&
    operator[] (const first_type& key)
    { return find2(key); }

    inline const typename boost::disable_if<boost::is_same<first_type, second_type>, second_type>::type&
    operator[] (const first_type& key) const
    { return find2(key); }

    inline typename boost::disable_if<boost::is_same<first_type, second_type>, first_type>::type&
    operator[] (const second_type& key)
    { return find1(key); }

    inline const typename boost::disable_if<boost::is_same<first_type, second_type>, first_type>::type&
    operator[] (const second_type& key) const
    { return find1(key); }

private:
    set_type            set_m;
    first_compare_type  pred1_m;
    second_compare_type pred2_m;
};

/*************************************************************************************************/

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
bool operator== (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                 const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{
    typedef rset<Type1, Type2, Type1Equality, Type2Equality> set_type;
    typedef typename set_type::const_iterator                iterator;
    typedef typename set_type::first_compare_type            first_compare_type;
    typedef typename set_type::second_compare_type           second_compare_type;

    if (x.size() != y.size())
        return false;

    iterator            x_iter(x.begin());
    iterator            x_last(x.end());
    iterator            y_iter(y.begin());
    first_compare_type  pred1(x.first_compare());
    second_compare_type pred2(x.second_compare());

    for (; x_iter != x_last; ++x_iter, ++y_iter)
        if (!pred1(x_iter->first, y_iter->first) ||
            !pred2(x_iter->second, y_iter->second))
            return false;

    return true;
}

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
inline bool operator!= (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                        const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{ return !(x == y); }

/*************************************************************************************************/

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
bool operator< (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{
    typedef typename rset<Type1, Type2, Type1Equality, Type2Equality>::reference reference;

    const std::size_t x_size(x.size());
    const std::size_t y_size(y.size());

    for (std::size_t i(0); ; ++i)
    {
        if (i >= y_size)
            return false;

        if (i >= x_size)
            return true;

        reference x_ref(x.at(i));
        reference y_ref(y.at(i));

        if (y_ref.first < x_ref.first ||
                (y_ref.first == x_ref.first && y_ref.second < x_ref.second))
            return false;

        if (x_ref.first < y_ref.first ||
                (x_ref.first == y_ref.first && x_ref.second < y_ref.second))
            return true;
    }
}

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
inline bool operator> (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                       const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{ return y < x; }

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
inline bool operator<= (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                        const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{ return !(y < x); }

template <typename Type1, typename Type2,
          typename Type1Equality, typename Type2Equality>
inline bool operator>= (const rset<Type1, Type2, Type1Equality, Type2Equality>& x,
                        const rset<Type1, Type2, Type1Equality, Type2Equality>& y)
{ return !(x < y); } 

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_RSET_HPP
#endif

/*************************************************************************************************/
