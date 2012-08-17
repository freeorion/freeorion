/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_UTILITY_PAIR_HPP
#define ADOBE_UTILITY_PAIR_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <utility>

#include <boost/operators.hpp>

#include <GG/adobe/empty.hpp>
#include <GG/adobe/move.hpp>

#include <GG/adobe/implementation/swap.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\defgroup asl_pair pair
\ingroup utility
*/

//! \ingroup asl_pair
template <typename T1, typename T2 = T1>
struct aggregate_pair
{
    typedef T1 first_type;
    typedef T2 second_type;
    
    T1 first;
    T2 second;
        
    friend inline bool operator==(const aggregate_pair& x, const aggregate_pair&y)
    { return x.first == y.first && x.second == y.second; }
    
    friend inline bool operator<(const aggregate_pair& x, const aggregate_pair& y)
    { return x.first < y.first || (!(y.first < x.first) && x.second < y.second); }

    friend inline bool operator!=(const aggregate_pair& x, const aggregate_pair&y)
    { return !(x == y); }
    
    friend inline bool operator>(const aggregate_pair& x, const aggregate_pair& y)
    { return  y < x; }
    
    friend inline bool operator<=(const aggregate_pair& x, const aggregate_pair& y)
    { return  !(y < x); }
    
    friend inline bool operator>=(const aggregate_pair& x, const aggregate_pair& y)
    { return  !(x < y); }
    
    friend inline void swap(aggregate_pair& x, aggregate_pair& y)
    { swap(x.first, y.first); swap(x.second, y.second); }
};

/*************************************************************************************************/

//! \ingroup asl_pair
template <typename T1, typename T2 = T1>
struct pair : boost::totally_ordered<pair<T1, T2>, pair<T1, T2>, empty_base<pair<T1, T2> > >
{
    typedef T1 first_type;
    typedef T2 second_type;
    
    T1 first;
    T2 second;
    
    pair() : first(), second() { }
    
    pair(move_from<pair> x) : first(::adobe::move(x.source.first)), second(::adobe::move(x.source.second)) { }
    
    pair& operator=(pair x) { first = ::adobe::move(x.first); second = ::adobe::move(x.second); return *this; }
    
    pair(T1 x, T2 y) : first(::adobe::move(x)), second(::adobe::move(y)) { }
    
    template <typename U1, typename U2>
    pair(const pair<U1, U2>& p) : first(p.first), second(p.second) { }
    
    template <typename U1, typename U2>
    pair(const std::pair<U1, U2>& p) : first(p.first), second(p.second) { }
    
    template <typename U1, typename U2>
    pair(const aggregate_pair<U1, U2>& p) : first(p.first), second(p.second) { }
    
    friend inline bool operator==(const pair& x, const pair&y)
    { return x.first == y.first && x.second == y.second; }
    
    friend inline bool operator<(const pair& x, const pair& y)
    { return x.first < y.first || (!(y.first < x.first) && x.second < y.second); }
    
    friend inline void swap(pair& x, pair& y)
    { swap(x.first, y.first); swap(x.second, y.second); }
};

//! \ingroup asl_pair
template <typename T1, typename T2>
inline pair<T1, T2> make_pair(T1 x, T2 y)
{ return pair<T1, T2>(::adobe::move(x), ::adobe::move(y)); }

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

// ADOBE_UTILITY_PAIR_HPP
#endif

/*************************************************************************************************/
