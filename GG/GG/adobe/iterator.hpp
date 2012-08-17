/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ITERATOR_HPP
#define ADOBE_ITERATOR_HPP

#include <GG/adobe/config.hpp>

/*
    NOTE (sparent) : GCC 3.1 defines std::distance in <algorithm> instead of <iterator> so we go
    ahead and include both here to work around the issue.
*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <utility>

#include <boost/range.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/iterator_traits.hpp>

#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/empty.hpp>

#include <GG/adobe/implementation/swap.hpp>

namespace adobe {

/*************************************************************************************************/

/*
    Just counts the number of outputs; doesn't copy anything. More efficient than a
    back_insert_iterator into a container if all you're interested in is the size of
    the result.
*/

/*************************************************************************************************/

/*!
\addtogroup adobe_iterator
@{
*/
class counting_output_iterator
{
public:
    typedef std::output_iterator_tag        iterator_category;
    typedef counting_output_iterator                value_type;
    typedef counting_output_iterator&           reference;
    typedef std::size_t                     size_type;

    counting_output_iterator() :
        count_m(0)
        { }

    counting_output_iterator(const counting_output_iterator& x) :
        count_m(x.count_m)
        { }

    size_type count() const
        { return count_m; }

    template <typename T>
    reference operator = (const T&)
        { return *this; }

    reference operator * ()
        { return *this; }

    bool operator == (counting_output_iterator const& rhs) const
        { return this == &rhs; }

    counting_output_iterator operator ++ (int)
        { ++count_m; return *this; }

    reference operator ++ ()
        { ++count_m; return *this; }

private:
    std::size_t     count_m;
};

/*************************************************************************************************/

/*
    top iterator            bottom iterator
    
    random access           random access           bidirectional
    bidirectional           random access           bidirectional
    forward                 random access           forward
    input                   random access           forward
    output                  random access           ???
    
    random access           bidirectional           bidirectional
    bidirectional           bidirectional           bidirectional
    forward                 bidirectional           forward
    input                   bidirectional           forward
    output                  bidirectional           ???
    
    random access           forward                 forward
    bidirectional           forward                 forward
    forward                 forward                 forward
    input                   forward                 forward
    output                  forward                 ???
    
    random access           input                   input
    bidirectional           input                   input
    forward                 input                   input
    input                   input                   input
    output                  input                   ???
    
    random access           output                  output
    bidirectional           output                  output
    forward                 output                  output
    input                   output                  output
    output                  output                  ???
    
    
    if (catgory(bottom) == bidirectional && catgory(top) == bidirectional) return bidirectional
    else if (catgory(bottom) == forward) return forward
    else return category(bottom)
*/

template <typename I> // I models an InputIterator where value_type(I) models Range
class segmented_iterator : public boost::iterator_facade<segmented_iterator<I>,
    typename boost::range_value<typename boost::iterator_value<I>::type>::type,
    std::bidirectional_iterator_tag,
    typename boost::iterator_reference<typename boost::range_iterator<typename boost::iterator_value<I>::type>::type>::type,
    typename boost::iterator_difference<typename boost::range_iterator<typename boost::iterator_value<I>::type>::type>::type>
{
 public:
    segmented_iterator() : bucket_m(), end_m(), current_m() { }
    segmented_iterator(I first, I last): bucket_m(first), end_m(last)
    {
        while(bucket_m != end_m && boost::empty(*bucket_m))
        {
            ++bucket_m;
        }
        if (bucket_m != end_m) current_m = boost::begin(*bucket_m);
    }
    
    segmented_iterator(const segmented_iterator& x) :
        bucket_m(x.bucket_m),
        end_m(x.end_m),
        current_m(x.current_m)
    { }
    
    segmented_iterator& operator=(segmented_iterator x)
    {
        swap(*this, x); return *this;
    }
    
    friend inline void swap(segmented_iterator& x, segmented_iterator& y)
    {
        swap(x.bucket_m, y.bucket_m);
        swap(x.end_m, y.end_m);
        swap(x.curent_m, y.curent_m);
    }
    
 private:
    typedef typename boost::iterator_reference<typename boost::range_iterator<
        typename boost::iterator_value<I>::type>::type>::type   reference_t;
    typedef I top_iterator_t;
    typedef typename boost::range_iterator<typename boost::iterator_value<I>::type>::type bottom_iterator_t;
 
    top_iterator_t      bucket_m;
    top_iterator_t      end_m;
    bottom_iterator_t   current_m;
    
// boost iterator_facade access functions

    friend class boost::iterator_core_access;
                
    reference_t dereference() const { return *current_m; }

    bool equal(const segmented_iterator& x) const
    {
        /*
        If the end of the top sequences are the same and we are in the same bucket then if we are
        at the very end or we are at the same local position then we are equal.
        */
        
        return end_m == x.end_m && bucket_m == x.bucket_m
            && (bucket_m == end_m || current_m == x.current_m);
    }

    void increment()
    {
        ++current_m;
        
        while (current_m == boost::end(*bucket_m))
        {
            ++bucket_m;
            if (bucket_m == end_m) break;
            current_m = boost::begin(*bucket_m);
        }
    }
    void decrement()
    {
        while (bucket_m == end_m || current_m == boost::begin(*bucket_m))
        {
            --bucket_m;
            current_m = boost::end(*bucket_m);
        }
        
        --current_m;
    }
};


template <typename R> // R models ConvertibleToRange
inline boost::iterator_range<segmented_iterator<typename boost::range_iterator<R>::type> >
    make_segmented_range(R& r)
{
    typedef segmented_iterator<typename boost::range_iterator<R>::type> iterator;

    return boost::make_iterator_range(iterator(boost::begin(r), boost::end(r)),
        iterator(boost::end(r), boost::end(r)));
}


template <typename R> // R models ConvertibleToRange
inline segmented_iterator<typename boost::range_iterator<R>::type> make_segmented_iterator(R& r)
{
    typedef segmented_iterator<typename boost::range_iterator<R>::type> iterator;
    
    return iterator(boost::begin(r), boost::end(r));
}

/*************************************************************************************************/

/*
    NOTE (sparent) : The asserts are comment only because we don't require that our function
    object be equality comparable.
*/


template <  typename F, // F models Unary Function object
            typename T, // T models Regular Type
            typename R = T&, // R models Reference Type of T
            typename I = std::size_t, // I models Unsigned Integer
            typename D = std::ptrdiff_t // D models Signed Integer
        >
// I is convertible to argument[1] of F
// result of F is R
// D is the difference type of I (must be signed)

class index_iterator : public boost::iterator_facade<index_iterator<F, T, R, I, D>, T,
    std::random_access_iterator_tag, R, D>
{
 public:
    index_iterator() : index_m(0) { }
    index_iterator(F f, I i): dereference_m(f), index_m(i) { }
    
    index_iterator(const index_iterator& x) :
        dereference_m(x.dereference_m),
        index_m(x.index_m)
    { }
    
    index_iterator& operator=(const index_iterator& x)
    {
        index_iterator temp(x);
        swap(temp, *this);
        return *this;
    }
    
    friend inline void swap(index_iterator& x, index_iterator& y)
    {
        swap(x.dereference_m, y.dereference_m);
        swap(x.index_m, y.index_m);
    }
    
    I base() const { return index_m; }
    
 private:
    F dereference_m;
    I index_m;
    
// boost iterator_facade access functions

    friend class boost::iterator_core_access;
                
    R dereference() const { return dereference_m(this->index_m); }

    bool equal(const index_iterator& x) const
    {
        // assert(dereference_m == x.dereference_m);
        
        return index_m == x.index_m;
    }

    void increment() { ++index_m; }
    void decrement() { --index_m; }
    void advance(D x) { index_m += x; }
    
    D distance_to(const index_iterator& x) const
    {
        // assert(dereference_m == x.dereference_m);
        
        /*
            REVISIT (sparent) : There isn't a difference type for unsigned integers - because an
            index is usually denoted by an unsigned type, but a difference is signed we should
            have some mechanism to peform the subtraction and guarantee a valid result. We don't
            currently have said mechanism, so we simply cast from (possibly)unsigned to signed.
            
            This limits the range within which we can perform this operation, but practically it
            shouldn't be an issue.
        */
        return D(x.index_m) - D(index_m);
    }
};

////////////////////////////////////////////////////////////////////////////////////////
///                 
//                  STEP ITERATOR ADAPTOR
/// \brief step iterator adaptor
///
/// An adaptor over an existing iterator that changes the step unit
/// (i.e. distance(it,it+1)) by a given predicate. Instead of calling base's 
/// operators ++, --, +=, -=, etc. the adaptor is using the passed policy object S_FN
/// for advancing and for computing the distance between iterators.
///
////////////////////////////////////////////////////////////////////////////////////////


template <typename DERIVED, // type of the derived class
        typename IT,    // Models Iterator
        typename S_FN>  // A policy object that can compute the distance between two iterators of type IT
                        // and can advance an iterator of type IT a given number of IT's units  
class step_iterator_adaptor : public boost::iterator_adaptor<DERIVED, IT, boost::use_default, boost::use_default, boost::use_default, typename S_FN::difference_type> {
public:
    typedef boost::iterator_adaptor<DERIVED, IT, boost::use_default, boost::use_default, boost::use_default, typename S_FN::difference_type> parent_type;
    typedef typename std::iterator_traits<IT>::difference_type base_difference_type;
    typedef typename S_FN::difference_type                     difference_type;
    typedef typename std::iterator_traits<IT>::reference       reference;

    step_iterator_adaptor() {}
    step_iterator_adaptor(const IT& it, S_FN step_fn=S_FN()) : parent_type(it), _step_fn(step_fn) {}

    difference_type step() const { return _step_fn.step(); }

protected:
    S_FN _step_fn;
private:
    friend class boost::iterator_core_access;

    void increment() { _step_fn.advance(this->base_reference(),1); }
    void decrement() { _step_fn.advance(this->base_reference(),-1); }
    void advance(base_difference_type d) { _step_fn.advance(this->base_reference(),d); }
    difference_type distance_to(const step_iterator_adaptor& it) const { return _step_fn.difference(this->base_reference(),it.base_reference()); }
};

// although boost::iterator_adaptor defines these, the default implementation computes distance and compares for zero.
// it is often faster to just apply the relation operator to the base
/*!
\ingroup adobe_iterator
*/
template <typename D,typename IT,typename S_FN> inline
bool operator>(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.step()>0 ? p1.base()> p2.base() : p1.base()< p2.base(); 
}


template <typename D,typename IT,typename S_FN> inline
bool operator<(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.step()>0 ? p1.base()< p2.base() : p1.base()> p2.base(); 
}

template <typename D,typename IT,typename S_FN> inline
bool operator>=(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.step()>0 ? p1.base()>=p2.base() : p1.base()<=p2.base(); 
}


template <typename D,typename IT,typename S_FN> inline
bool operator<=(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.step()>0 ? p1.base()<=p2.base() : p1.base()>=p2.base(); 
}


template <typename D,typename IT,typename S_FN> inline
bool operator==(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.base()==p2.base(); 
}


template <typename D,typename IT,typename S_FN> inline
bool operator!=(const step_iterator_adaptor<D,IT,S_FN>& p1, const step_iterator_adaptor<D,IT,S_FN>& p2) 
{ 
        return p1.base()!=p2.base(); 
}

/*************************************************************************************************/

/*!
    \brief A stub iterator that models OutputIterator and outputs nothing.
*/
struct null_output_iterator_t
{
    typedef std::output_iterator_tag iterator_category;
    typedef null_output_iterator_t   value_type;
    typedef std::ptrdiff_t           difference_type;
    typedef value_type*              pointer;
    typedef value_type&              reference;

    null_output_iterator_t& operator ++ (int) { return *this; }
    null_output_iterator_t& operator ++ () { return *this; }
    reference               operator * () { return *this; }

    template <typename T>
    null_output_iterator_t& operator = (const T&) { return *this; }
};

//! @}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
