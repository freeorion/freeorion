/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_FUNCTIONAL_HPP
#define ADOBE_FUNCTIONAL_HPP

#include <GG/adobe/config.hpp>

#include <functional>
#include <utility>

#include <boost/compressed_pair.hpp>
#include <boost/tuple/tuple.hpp>

#include <GG/adobe/functional/operator.hpp>
#include <GG/adobe/functional/is_member.hpp>
#include <GG/adobe/utility/pair.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\defgroup adobe_functional Functional
\ingroup asl_libraries
*/

/*
    REVISIT (sparent) : Documentation for delete_ptr moved to adobe/functional.hpp because doxygen 
    1.4.3 cannot handle the template specializations.
*/

/*!
\class adobe::generator_t
\ingroup misc_functional

\brief A function object for value generation within a domain.
*/

/*!
\class adobe::sequence_t 
\ingroup misc_functional

\brief A function object for sequence generation within a domain.
*/

/*!
\defgroup compare_members Object Member Comparsion
\ingroup adobe_functional

The library provides basic function object classes for comparing two member variables between object instances.
*/

/*!
\class adobe::compare_members_t
\ingroup compare_members

\brief Utility class for adobe::compare_members.
*/

/*!
\fn adobe::compare_members_t<T, R, std::less<R> > adobe::compare_members(R T::* member)
\ingroup compare_members

Used to compare members in two objects of type \c T using \c std::less<R>.

\param member Name of the member to compare.

\return
    An adobe::compare_members_t which can be used to make the comparison.
*/

/*!
\fn adobe::compare_members_t<T, R, Compare> adobe::compare_members(R T::* member, Compare compare)
\ingroup compare_members

Used to compare members in two objects of type \c T using a provided comparsion function object.

\param member Name of the member to compare.
\param compare \ref concept_convertible_to_function object used for comparing the instance members.

\return
    An adobe::compare_members_t which can be used to make the comparison.
*/

/*!
\defgroup mem_data Member Data Accessor
\ingroup adobe_functional

This library provides access to member data, similar to boost::mem_fn() except it returns a reference (or const reference for a const object) to the data.
*/

/*!
\class adobe::mem_data_t
\ingroup mem_data

\brief Adaptor similar to \c boost::mem_fn() used by \c boost::bind.

Unlike boost::mem_fn() this is of use when you want to use a pointer to a member and have it return a reference.

\todo
    (sparent) Propose this addition back to Boost.
*/

/*!
\fn adobe::mem_data_t<T, R> adobe::mem_data(R T::* member)
\ingroup mem_data

Returns a function object, which given an object of type \c T, will return the value of the member pointed to by \c member.

\param member pointer to member data.

\return
    An adobe::mem_data_t function object.
*/

/*!
\class adobe::indirect_t
\ingroup misc_functional

\brief Adapter used to convert pointers to references.
*/

/*!
\defgroup bitwise_operators Bitwise Operations
\ingroup adobe_functional

The library provides basic function object classes for all of the bitwise logical operators in the language.
*/

/*!
\class adobe::bitwise_or
\ingroup bitwise_operators

\brief \c operator() returns <code>x | y</code>.
*/

/*!
\class adobe::bitwise_and 
\ingroup bitwise_operators

\brief \c operator() returns <code>x & y</code>.
*/

/*!
\class adobe::bitwise_xor 
\ingroup bitwise_operators

\brief \c operator() returns <code>x ^ y</code>.
*/



/*! \addtogroup misc_functional
@{
*/

template <  typename F, // models UnaryFunction
            typename G> // models UnaryFunction -> argument_type(F)
struct unary_compose
{
    typedef typename F::result_type result_type;
    
    unary_compose() { }
    unary_compose(F f, G g) : data_m(f, g) { }

    template <typename U> // U models Regular
    result_type operator()(const U& x) const { return data_m.first()(data_m.second()(x)); }

    template <typename U> // U models Regular
    result_type operator()(U& x) const { return data_m.first()(data_m.second()(x)); }

 private:
    boost::compressed_pair<F, G> data_m;
};

/*************************************************************************************************/

template <  typename F, // models UnaryFunction
            typename G> // models UnaryFunction -> argument_type(F)
unary_compose<F, G> compose(F f, G g) { return unary_compose<F, G>(f, g); }

/*************************************************************************************************/

template <  typename F,     // models BinaryFunction
            typename G,     // models UnaryFunction -> argument_type(F, 0);
            typename H = G> // models UnaryFunction -> argument_type(F, 1);
struct binary_compose
{
    typedef typename F::result_type result_type;

    template <typename T, typename U> // models Regular
    result_type operator()(const T& x, const U& y) const { return f(g(x), h(y)); }

    template <typename T, typename U> // models Regular
    result_type operator()(T& x, U& y) const { return f(g(x), h(y)); }

    F f;
    G g;
    H h;
};

/*************************************************************************************************/

template <int N, typename T> // T is boost::tuple<>
struct element
{
    typedef typename boost::tuples::element<N, T>::type type;
};

template <typename T1, typename T2>
struct element<0, std::pair<T1, T2> >
{
    typedef typename std::pair<T1, T2>::first_type type;
};

template <typename T1, typename T2>
struct element<1, std::pair<T1, T2> >
{
    typedef typename std::pair<T1, T2>::second_type type;
};

template <typename T1, typename T2>
struct element<0, pair<T1, T2> >
{
    typedef typename pair<T1, T2>::first_type type;
};

template <typename T1, typename T2>
struct element<1, pair<T1, T2> >
{
    typedef typename pair<T1, T2>::second_type type;
};

/*************************************************************************************************/

template <int N, typename T> // T is pair or tuple
struct get_element : std::unary_function<T, typename element<N, T>::type>
{
    typename element<N, T>::type& operator()(T& x) const
    { return boost::get<N>(x); }
    
    const typename element<N, T>::type& operator()(const T& x) const
    { return boost::get<N>(x); }
};

/*************************************************************************************************/

template <typename T1, typename T2> // T is pair or tuple
struct get_element<0, std::pair<T1, T2> > :
        std::unary_function<std::pair<T1, T2>, typename std::pair<T1, T2>::first_type>
{
    typedef std::pair<T1, T2>                   argument_type;
    typedef typename argument_type::first_type  result_type;

    result_type& operator()(argument_type& x) const
    { return x.first; }
    
    const result_type& operator()(const argument_type& x) const
    { return x.first; }
};

/*************************************************************************************************/

template <typename T1, typename T2> // T is pair or tuple
struct get_element<0, pair<T1, T2> > :
        std::unary_function<pair<T1, T2>, typename pair<T1, T2>::first_type>
{
    typedef pair<T1, T2>                        argument_type;
    typedef typename argument_type::first_type  result_type;

    result_type& operator()(argument_type& x) const
    { return x.first; }
    
    const result_type& operator()(const argument_type& x) const
    { return x.first; }
};

/*************************************************************************************************/

template <typename T1, typename T2> // T is pair or tuple
struct get_element<1, std::pair<T1, T2> > :
        std::unary_function<std::pair<T1, T2>, typename std::pair<T1, T2>::second_type>
{
    typedef std::pair<T1, T2>                   argument_type;
    typedef typename argument_type::second_type result_type;
    
    result_type& operator()(argument_type& x) const
    { return x.second; }
    
    const result_type& operator()(const argument_type& x) const
    { return x.second; }
};

/*************************************************************************************************/

template <typename T1, typename T2> // T is pair or tuple
struct get_element<1, pair<T1, T2> > :
        std::unary_function<pair<T1, T2>, typename pair<T1, T2>::second_type>
{
    typedef pair<T1, T2>                        argument_type;
    typedef typename argument_type::second_type result_type;
    
    result_type& operator()(argument_type& x) const
    { return x.second; }
    
    const result_type& operator()(const argument_type& x) const
    { return x.second; }
};

/*************************************************************************************************/

template <typename T>
struct always_true : std::unary_function<T, bool>
{
    bool operator()(const T&) const { return true; }
};

/*************************************************************************************************/

template <class Result>
struct generator_t
{
    typedef Result result_type;
};

/*************************************************************************************************/


template <typename T>
struct sequence_t
#if !defined(ADOBE_NO_DOCUMENTATION)
: generator_t<T>
#endif
{
    explicit sequence_t(const T& x) : data_m(x) { }
    T operator () () { return data_m++; }
private:
    T data_m;
};

/*************************************************************************************************/

template <class T, typename R, class Compare>
struct compare_members_t : std::binary_function<T, T, bool>
{
    compare_members_t(R T::* member, Compare compare) :
        compare_m(compare),
        member_m(member)
        { }

    bool operator () (const T& x, const T& y) const
        { return compare_m(x.*member_m, y.*member_m); }
    
    bool operator () (const T& x, const R& y) const
        { return compare_m(x.*member_m, y); }
    
    bool operator () (const R& x, const T& y) const
        { return compare_m(x, y.*member_m); }

 private:
 
/*
    REVISIT (sparent) : This could probably use an empty member optimization.
*/

    Compare compare_m;
    R T::*  member_m;
};

template <class T, typename R>
compare_members_t<T, R, std::less<R> > compare_members(R T::* member)
{
    return compare_members_t<T, R, std::less<R> >(member, std::less<R>() );
}

template <class T, typename R, class Compare>
compare_members_t<T, R, Compare> compare_members(R T::* member, Compare compare)
{
    return compare_members_t<T, R, Compare>(member, compare);
}

/*************************************************************************************************/

template <class T, typename R>
struct mem_data_t : std::unary_function<T, R&>
{
    mem_data_t() { }

    explicit mem_data_t(R T::* member) :
        member_m(member)
        { }
    
    R& operator () (T& x) const
        { return  x.*member_m; }
        
    const R& operator () (const T& x) const
        { return x.*member_m; }

 private:
    R T::* member_m;
};

template <class T, typename R>
struct mem_data_t<const T, R> : std::unary_function<T, const R&>
{
    explicit mem_data_t(R T::* member) :
        member_m(member)
        { }
    
    const R& operator () (const T& x) const
        { return x.*member_m; }

 private:
    R T::* member_m;
};

template <class T, typename R>
mem_data_t<T, R> mem_data(R T::* member)
{
    return mem_data_t<T, R>(member);
}

/*************************************************************************************************/

template <typename O> // O models StrictWeakOrdering
struct equivalent : std::binary_function<typename O::first_argument_type,
                                         typename O::second_argument_type,
                                         bool>
{
    public:
      explicit equivalent(const O& x) : o_m(x) { }

      bool operator()(  const typename O::first_argument_type& x,
                        const typename O::second_argument_type& y) const
      { return !o_m(x, y) && !o_m(y, x); }
 private:
    O o_m;
};

/*************************************************************************************************/

template <class F> // F models a BinaryFunction
struct transposer : std::binary_function<typename F::second_argument_type,
                                         typename F::first_argument_type,
                                         typename F::result_type>
{
    typedef typename F::second_argument_type first_argument_type;
    typedef typename F::first_argument_type  second_argument_type;
    typedef typename F::result_type          result_type;

    F fun;

    transposer(const F& f) :
        fun(f)
    { }

    result_type operator()(const first_argument_type& x, const second_argument_type& y) const
    {
        return fun(y, x);
    }
};

template <typename F> // F models BinaryFunction
inline transposer<F> f_transpose(F f)
{
    return transposer<F>(f);
}

//!@}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
