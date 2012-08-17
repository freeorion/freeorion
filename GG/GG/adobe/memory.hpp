/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_MEMORY_HPP
#define ADOBE_MEMORY_HPP

#include <GG/adobe/config.hpp>

#include <cassert>
#include <functional>
#include <memory>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_const.hpp>

#include <GG/adobe/conversion.hpp>
#include <GG/adobe/functional.hpp>
#include <GG/adobe/memory_fwd.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\defgroup memory Memory
\ingroup utility

Augments the functionality of \<memory\>
@{
*/


template <typename T>
struct empty_ptr;


template <typename T>
struct empty_ptr<T*> : std::unary_function<T*, void>
{
    bool operator () (const T* x) const throw()
    { return x == NULL; }
};


template <typename T>
struct empty_ptr<T(*)[]> : std::unary_function<T*, void>
{
    bool operator () (const T* x) const throw()
    { return x == NULL; }
};

//@}

/*************************************************************************************************/

template<typename X, class Traits> class auto_ptr;
template<typename X, class Traits> class auto_resource;

template <typename ptrT>
struct ptr_traits;

//!\ingroup memory
template <typename T>
struct ptr_traits<T(*)[]>
{
    typedef T           element_type;
    typedef T*          pointer_type;
    typedef const T*    const_pointer_type;

    template <class U> struct rebind { typedef adobe::ptr_traits<U> other; };
    enum { is_array = true };
    
    static void delete_ptr(pointer_type x) throw() { adobe::delete_ptr<T(*)[]>()(x); }
    static bool empty_ptr(const_pointer_type x) throw() { return adobe::empty_ptr<T(*)[]>()(x); }
};

/*!
    The ptr_traits class provide basic information and operations associated with a pointer
    type.
*/

/*! \ingroup memory
   \brief This section defines requirements on classes representing <i>pointer traits</i>. The
    template class <code>ptr_traits\< ptrT \></code> is defined along with several specializations.
    
    A <i>pointer</i> may be any type used to refer to another, possibly not visible, type.
    
    In the following table, <code>X</code> denotes a Traits class defining types and functions for
    the pointer type PtrT. The type of the item refered to is T. T may be <code>void</code> or an
    incomplete type. The argument <code>p</code> is of type PtrT.
    
    <table>
        <tr>
            <td><b>expression</b></td>
            <td><b>return type</b></td>
            <td><b>notes</b></td>
            <td><b>complexity</b></td>
        </tr>
        <tr>
            <td><code>X::element_type</code></td>
            <td><code>T</code></td>
            <td>the type of the item refered to</td>
            <td>compile time</td>
        </tr>
        <tr>
            <td><code>X::pointer_type</code></td>
            <td><code>PtrT</code></td>
            <td>if opaque, may be any type used to refer to T</td>
            <td>compile time</td>
        </tr>
        <tr>
            <td><code>X::const_pointer_type</code></td>
            <td>implementation defined</td>
            <td>type corresponding to PtrT refering to a const T</td>
            <td>compile time</td>
        </tr>
        <tr>
            <td><code>X::is_array</code></td>
            <td><code>bool</code></td>
            <td>true iff T is an array; type may also be convertable to bool</td>
            <td>compile time</td>
        </tr>
        <tr>
            <td><code>X::delete_ptr(p)</code></td>
            <td><code>void</code></td>
            <td>destructs and deallocates item refered to by p; if p is empty, delete_ptr() has no effect</td>
            <td>implementation defined</td>
        </tr>
        <tr>
            <td><code>X::empty_ptr(p)</code></td>
            <td><code>bool</code></td>
            <td>result is <code>true</code> if <code>p</code> refers to nothing (corresponds to
                <code>NULL</code>); <code>false</code> otherwise</td>
            <td>constant</td>
        </tr>
    </table>
*/
template <typename T>
struct ptr_traits<T*>
{
    typedef T           element_type;
    typedef T*          pointer_type;
    typedef const pointer_type  const_pointer_type;
    
    template <class U> struct rebind { typedef adobe::ptr_traits<U> other; };
    enum { is_array = false };

    static void delete_ptr(pointer_type x) throw() { adobe::delete_ptr<T*>()(x); }
    static bool empty_ptr(const_pointer_type x) throw() { return adobe::empty_ptr<T*>()(x); }
};
    
//!\ingroup memory
template <typename T>
struct ptr_traits<std::auto_ptr<T> >
{
    typedef typename std::auto_ptr<T>::element_type element_type;
    typedef std::auto_ptr<T>                        pointer_type;
    typedef std::auto_ptr<const T>                  const_pointer_type;
    
    template <class U> struct rebind { typedef adobe::ptr_traits<U> other; };
    enum { is_array = false };
};

//!\ingroup memory
template <typename R, typename T>
struct runtime_cast_t<R, std::auto_ptr<T> > {
    R operator()(std::auto_ptr<T>& x) const
    {
        typedef typename R::element_type* dest_type;
        dest_type result = dynamic_cast<dest_type>(x.get());
        if (result) x.release();
        return R(result);
    }
};
    
//!\ingroup memory
template <typename T, class Traits>
struct ptr_traits<auto_ptr<T, Traits> >
{
    typedef typename auto_ptr<T, Traits>::element_type   element_type;
    typedef auto_ptr<T, Traits>                          pointer_type;
    typedef auto_ptr<const T, Traits>                    const_pointer_type;
    
    enum { is_array = Traits::is_array };
};

//!\ingroup memory
template <typename R, typename T, typename Traits>
struct runtime_cast_t<R, auto_ptr<T, Traits> > {
    R operator()(auto_ptr<T, Traits>& x) const
    {
        typedef typename R::element_type* dest_type;
        dest_type result = dynamic_cast<dest_type>(x.get());
        if (result) x.release();
        return R(result);
    }
};
    
//!\ingroup memory
template <typename T, class Traits>
struct ptr_traits<auto_resource<T, Traits> >
{
    typedef typename Traits::element_type           element_type;
    typedef auto_resource<T, Traits>         pointer_type;
    typedef auto_resource<const T, Traits>   const_pointer_type;
    
    enum { is_array = Traits::is_array };
};

//!\ingroup memory
template <typename R, typename T, typename Traits>
struct runtime_cast_t<R, auto_resource<T, Traits> > {
    R operator()(auto_resource<T, Traits>& x) const
    {
        typedef typename R::element_type* dest_type;
        dest_type result = dynamic_cast<dest_type>(x.get());
        if (result) x.release();
        return R(result);
    }
};


/*************************************************************************************************/

#ifndef NO_DOCUMENTATION
    
/*
    REVISIT (sparent) : This could use boost::static_assert but it doesn't seem worth adding a
    boost dependency just for this case.
*/

namespace implementation {
    template <bool x> struct adobe_static_assert;
    template <> struct adobe_static_assert<true> { };
}

#endif

/*************************************************************************************************/

/*! \addtogroup memory
  @{
*/
/*! \brief The template class <code>auto_resource\< X, Traits \></code> provides similar
    functionality to <code>auto_ptr</code> for resources for which the pointer is <i>opaque</i> 
    refered to by a non-pointer type.
    
    <code>auto_ptr\< Item \></code> is equivalent to <code>auto_resource\< Item* \></code> with
    the addition of <code>operator *()</code> and <code>operator ->()</code>.
    
    <b>Example:</b>
    \dontinclude auto_resource_test.cpp
    \skip start_of_example
    \until end_of_example
*/

template <typename X, class Traits = ptr_traits<X> >
class auto_resource
{
    struct clear_type { };
    operator int() const;

 public:
    typedef Traits                              traits_type;
    typedef typename traits_type::element_type  element_type;
    typedef typename traits_type::pointer_type  pointer_type;
    
    // 20.4.5.1 construct/copy/destroy:
    explicit auto_resource(pointer_type p = 0) throw();
                        
    auto_resource(auto_resource&) throw();
    template <typename Y> auto_resource(const auto_resource<Y, typename traits_type::template rebind<Y>::other>&) throw();
    
    auto_resource& operator=(auto_resource&) throw();
    template<typename Y> auto_resource& operator=(auto_resource<Y, typename traits_type::template rebind<Y>::other>) throw();
    
    ~auto_resource() throw();
    
    // assignment from NULL
    auto_resource& operator=(const clear_type*) throw();
    
    // 20.4.5.2 members:        
    pointer_type get() const throw();
    pointer_type release() throw();
    void reset(pointer_type p = 0) throw();
    
    // Safe bool conversion (private int conversion prevents unsafe use)
    operator bool () const throw() { return (pointer_m != NULL); }
    bool operator!() const throw();
    
 private:
    /*
        20.4.5.3 conversions:
    
        NOTE (spraent) : As per the recommendations on standard issue 463 the conversion
        operators through auto_ptr_ref have been removed in favor of using this conditional
        enabled trick.
    
        http://www.open-std.org/jtc1/sc22/wg21/docs/lwg-active.html#463
    */ 
// VC 2003 internal compiler error workaround. Some misues of auto_resource will go undetected under MSVC until fixed.
#ifndef BOOST_MSVC 
  template <typename Y> struct error_on_const_auto_type;
    template <typename Y> struct error_on_const_auto_type<auto_resource<Y, typename traits_type::template rebind<Y>::other> const>
    { typedef typename auto_resource<Y, typename traits_type::template rebind<Y>::other>::const_auto_type_is_not_allowed type; };
    
    template <class Y>
    auto_resource(Y& rhs, typename error_on_const_auto_type<Y>::type = 0);
#endif
    pointer_type pointer_m;
};

/*************************************************************************************************/

/*! \brief The <code>adobe::auto_ptr\<\></code> template adds a number of extensions to
    <code>std::auto_ptr\<\></code>.
    
    Although <code>std::auto_ptr\<\></code> is a non-regular type, it has proven to be valueable
    especially when converting C or non-exception aware C++ to exception aware C++ code. It is
    also useful when dealing with C libraries (such as OS libraries) that by necisity return
    pointers.
    
    However, <code>std::auto_ptr\<\></code> has a number of limitations that make use error prone,
    limit when it can be used, and make it's use more combursome than necessary.
    
    The <code>auto_ptr\<\></code> includes the following not present in
    <code>std::auto_ptr\<\></code>:
    
    - The inclusion of ptr_traits to support alternative delete functions.
    - Proper support for array types.
    - Support for assignment from function results.
    - Safe bool casts.
    - Assignment and construction from NULL.
    
    Also, <code>auto_resource\<\></code> is provided for non-pointer and opaque pointer
    references.

    <b>Rationals:</b>

    Rational for not going with boost: scoped_ptr interface: The interface to boost:scoped_ptr is a
    constrained subset of auto_ptr with out any improvements. I'm not a fan of providing a whole new
    interface to prevent the user from performing some operations.

    Rational for traits instead of policies:
        - Advantages of policies
            - allows for per-instance state.
            - would allow for use with boost::function.
        - Disadvanteages
            - no real world examples to demonstrate the advantages.
            - complicates implementation to properly handle exceptions from
                throwing when assigning policies.
            - prohibits use of no-throw in declarations (may not be an actual disadvantage).
            - would require more complex interface for constructors.
            - would require swap be added to the interface to achieve a non-throwing swap and
                it is unclear how swap could be generally implemented.
                
    In total I thought the advantages didn't warrant the effort. If someone come demonstrate a 
    concrete instance where there would be a strong advantage I'd reconsider.
        
    Differences between photoshop linear types and adobe:: types:
        - traits_type replaces deallocator and provides custom null checks
        - Safe bool casts.
        - Assignment and construction from NULL.
        - template based constructor, assignment, and conversion
        
    - linear_resource -> adobe::auto_resource
    - linear_base_ptr -> adobe::auto_ptr
    - linear_array<T> -> adobe::auto_ptr<T[]>
    - linear_ptr -> adobe::auto_ptr

*/

template<typename X, class Traits = ptr_traits<X*> >
class auto_ptr : public auto_resource<X*, Traits>
{
    typedef auto_resource<X*, Traits> inherited;
    struct clear_type { };
    
 public:
    typedef Traits                                              traits_type;
    typedef typename traits_type::element_type                  element_type;
    typedef typename traits_type::pointer_type                  pointer_type;
    // 20.4.5.1 construct/copy/destroy:
    explicit auto_ptr(pointer_type p = 0) throw();
                        
    auto_ptr(auto_ptr&) throw();
    template <typename Y> auto_ptr(const auto_ptr<Y, typename traits_type::template rebind<Y*>::other>&) throw();
    
    auto_ptr& operator=(auto_ptr&) throw();
    template<typename Y> auto_ptr& operator=(auto_ptr<Y, typename traits_type::template rebind<Y*>::other>) throw();
    
    // assignment from NULL
    auto_ptr& operator=(const clear_type*) throw();
    
    // additions for interop with std::auto_ptr                 
    auto_ptr(std::auto_ptr<X> r) throw();
    template <typename Y> auto_ptr(std::auto_ptr<Y> r) throw();
    
    auto_ptr& operator=(std::auto_ptr<X>) throw();
    template<typename Y> auto_ptr& operator=(std::auto_ptr<Y>) throw();
    
    operator std::auto_ptr<X> () throw() { return std::auto_ptr<X>(inherited::release()); }
        
    // 20.4.5.2 members:
    element_type& operator * () const throw();
    pointer_type operator -> () const throw();
    element_type& operator [] (std::size_t index) const throw(); // addition
            
 private:
    template <typename Y> struct error_on_const_auto_type;
    template <typename Y> struct error_on_const_auto_type<auto_ptr<Y, typename traits_type::template rebind<Y*>::other> const>
    { typedef typename auto_ptr<Y, typename traits_type::template rebind<Y*>::other>::const_auto_type_is_not_allowed type; };
    
    template <class U>
    auto_ptr(U& rhs, typename error_on_const_auto_type<U>::type = 0);
};

//! @}

/*************************************************************************************************/

template <typename X, class Traits>
inline auto_resource<X, Traits>::auto_resource(pointer_type p) throw() :
    pointer_m(p)
{ }

template <typename X, class Traits>
inline auto_resource<X, Traits>::auto_resource(auto_resource& x) throw() :
    pointer_m(x.release())
{ }

template <typename X, class Traits>
template <typename Y>
inline auto_resource <X, Traits>::auto_resource(auto_resource<Y, typename traits_type::template rebind<Y>::other> const& x) throw() :
    pointer_m(const_cast<auto_resource<Y, typename traits_type::template rebind<Y>::other>&>(x).release())
{ }

template <typename X, class Traits>
inline auto_resource<X, Traits>& auto_resource<X, Traits>::operator=(auto_resource& x) throw()
{
    reset(x.release());
    return *this;
}

template <typename X, class Traits>
template <typename Y>
inline auto_resource<X, Traits>& auto_resource<X, Traits>::operator=(
        auto_resource<Y, typename traits_type::template rebind<Y>::other> x) throw()
{
    reset(x.release());
    return *this;
}

template <typename X, class Traits>
inline auto_resource<X, Traits>::~auto_resource() throw()
    { traits_type::delete_ptr(pointer_m); }

/*************************************************************************************************/

template <typename X, class Traits>
inline auto_resource<X, Traits>& auto_resource<X, Traits>::operator=(const clear_type*) throw()
{
    reset();
    return *this;
}

/*************************************************************************************************/

template <typename X, class Traits>
inline typename auto_resource<X, Traits>::pointer_type auto_resource<X, Traits>::get() const throw()
{
    return pointer_m;
}

template <typename X, class Traits>
inline typename auto_resource<X, Traits>::pointer_type auto_resource<X, Traits>::release() throw()
{
    pointer_type result(pointer_m);
    pointer_m = NULL;
    return result;
}

template <typename X, class Traits>
inline void auto_resource<X, Traits>::reset(pointer_type p) throw()
{
    if (pointer_m != p)
    {
        traits_type::delete_ptr(pointer_m);
        pointer_m = p;
    }
}

/*************************************************************************************************/

template <typename X, class Traits>
inline bool auto_resource<X, Traits>::operator!() const throw()
{
    return !pointer_m;
}

/*************************************************************************************************/
    

template <typename X, class Traits>
inline auto_ptr<X, Traits>::auto_ptr(pointer_type p) throw() :
    inherited(p)
{ }

template <typename X, class Traits>                 
inline auto_ptr<X, Traits>::auto_ptr(auto_ptr& r) throw() :
    inherited(r)
{ }

template <typename X, class Traits>
template <typename Y>
inline auto_ptr<X, Traits>::auto_ptr(const auto_ptr<Y, typename traits_type::template rebind<Y*>::other>& r) throw() :
    inherited(const_cast<auto_ptr<Y, typename traits_type::template rebind<Y*>::other>&>(r))
{ }

template <typename X, class Traits>
inline auto_ptr<X, Traits>& auto_ptr<X, Traits>::operator=(auto_ptr& r) throw()
{
    inherited::operator=(r);
    return *this;
}

template <typename X, class Traits>
template<typename Y>
inline auto_ptr<X, Traits>& auto_ptr<X, Traits>::operator=(
        auto_ptr<Y, typename traits_type::template rebind<Y*>::other> r) throw()
{
    inherited::operator=(r);
    return *this;
}

/*************************************************************************************************/

template <typename X, class Traits>
inline auto_ptr<X, Traits>& auto_ptr<X, Traits>::operator=(const clear_type*) throw()
{
    inherited::reset();
    return *this;
}

/*************************************************************************************************/

template <typename X, class Traits>                 
inline auto_ptr<X, Traits>::auto_ptr(std::auto_ptr<X> r) throw() :
    inherited(r.release())
{ }

template <typename X, class Traits>
template <typename Y>
inline auto_ptr<X, Traits>::auto_ptr(std::auto_ptr<Y> r) throw() :
    inherited(r.release())
{ }

template <typename X, class Traits>
inline auto_ptr<X, Traits>& auto_ptr<X, Traits>::operator=(std::auto_ptr<X> r) throw()
{
    inherited::reset(r.release());
    return *this;
}

template <typename X, class Traits>
template<typename Y>
inline auto_ptr<X, Traits>& auto_ptr<X, Traits>::operator=(
        std::auto_ptr<Y> r) throw()
{
    inherited::reset(r.release());
    return *this;
}

/*************************************************************************************************/

template <typename X, class Traits>
inline typename auto_ptr<X, Traits>::element_type& auto_ptr<X, Traits>::operator * () const throw()
{
    assert(!traits_type::empty_ptr(this->get()));
    return *this->get();
}

template <typename X, class Traits>
inline typename auto_ptr<X, Traits>::pointer_type auto_ptr<X, Traits>::operator -> () const throw()
{
    assert(!traits_type::empty_ptr(this->get()));
    return this->get();
}

template <typename X, class Traits>
inline typename auto_ptr<X, Traits>::element_type& auto_ptr<X, Traits>::operator [] (std::size_t index) const throw()
{
implementation::adobe_static_assert<traits_type::is_array>();

assert(!traits_type::empty_ptr(this->get()));
return *(this->get() + index);
}

/*************************************************************************************************/

template <typename T> // T models Regular
inline void destroy(T* p) { p->~T(); }

template <typename T> // T models Regular
inline void construct(T* p) { ::new(static_cast<void*>(p)) T(); }

template <typename T> // T models Regular
inline void construct(T* p, const T& x) { ::new(static_cast<void*>(p)) T(x); }

template <typename F> // F models ForwardIterator
inline void destroy(F f, F l)
{
    while (f != l) {
        destroy(&*f);
        ++f;
    }
}

#if 0
template <typename A> // A models Allocator
typename A::value_type* alloc_default(A& a)
{
    typename A::value_type* result = a.allocate(1);
    try {
        construct(result);
    } catch (...) {
        a.deallocate(result, 1);
        throw;
    }
    return result;
}

template <typename A> // A models Allocator
typename A::value_type* alloc_copy(A& a, const typename A::value_type& x)
{
    typename A::value_type* result = a.allocate(1);
    try {
        construct(result, x);
    } catch (...) {
        a.deallocate(result, 1);
        throw;
    }
    return result;
}

template <typename A>
typename A::value_type* alloc_move(A& a, typename A::value_type x)
{
    typename A::value_type* result = a.allocate(1);
    try {
        move_construct(result, x);
    } catch (...) {
        a.deallocate(result, 1);
        throw;
    }
    return result;
}
#endif

/*************************************************************************************************/

namespace version_1 {

//! \addtogroup memory
//! @{

struct new_delete_t
{
    void* (*new_)(std::size_t);
    void (*delete_)(void*);
};

extern const new_delete_t local_new_delete_g;

template < >
class capture_allocator<void>
{
  public:
    void*               pointer;
    typedef const void* const_pointer;
    typedef void        value_type;
    template <class U> struct rebind { typedef capture_allocator<U> other; };

    friend inline bool operator==(const capture_allocator&, const capture_allocator&)
    { return true; }

    friend inline bool operator!=(const capture_allocator&, const capture_allocator&)
    { return false; }
};

template <typename T>
class capture_allocator
{
 public:
    typedef std::size_t         size_type;
    typedef std::ptrdiff_t      difference_type;
    typedef T*                  pointer;
    typedef const T*            const_pointer;
    typedef T&                  reference;
    typedef const T&            const_reference;
    typedef T                   value_type;
    template <typename U> struct rebind { typedef capture_allocator<U> other; };
    
    capture_allocator() : new_delete_m(&local_new_delete_g) { }
    template <typename U>
    capture_allocator(const capture_allocator<U>& x) : new_delete_m(x.new_delete()) { }
    
    pointer address(reference x) const { return &x; }
    const_pointer address(const_reference x) const { return &x; }
    pointer allocate(size_type n, capture_allocator<void>::const_pointer = 0)
    {
        if (n > max_size()) throw std::bad_alloc();
        pointer result = static_cast<pointer>(new_delete_m->new_(n * sizeof(T)));
        if (!result) throw std::bad_alloc();
        return result;
    }
    void deallocate(pointer p, size_type)
    {
        new_delete_m->delete_(p);
    }
    size_type max_size() const { return size_type(-1) / sizeof(T); }
    void construct(pointer p, const T& x) { adobe::construct(p, x); }
    void destroy(pointer p) { adobe::destroy(p); }

    friend inline bool operator==(const capture_allocator& x, const capture_allocator& y)
    { return x.new_delete_m == y.new_delete_m; }

    friend inline bool operator!=(const capture_allocator& x, const capture_allocator& y)
    { return x.new_delete_m != y.new_delete_m; }

    const new_delete_t* new_delete() const { return new_delete_m; }

 private:
    const new_delete_t* new_delete_m;
};

//! @} //end addtogroup memory
/*************************************************************************************************/

} // namespace version_1

/*************************************************************************************************/

/*
    Note (sparent) : The aligned storage class is intended to pad out an item of size_t such that 
    anything following it is aligned to the max alignement on the machine - in this case, quadword.
*/
    
//! \addtogroup memory
//! @{
template <typename T>
struct aligned_storage
{
    aligned_storage() { construct(&get()); }
    
    template <typename U>
    explicit aligned_storage(const U& x, typename copy_sink<U, T>::type = 0)
        { construct(&get(), x); }
    template <typename U>
    explicit aligned_storage(U x, typename move_sink<U, T>::type = 0)
        { move_construct(&get(), x); }
        
    ~aligned_storage() { destroy(&get()); }
    
    aligned_storage(const aligned_storage& x) { construct(&get(), x.get()); }
    aligned_storage(move_from<aligned_storage> x) { move_construct(&get(), x.source.get()); }
    
    aligned_storage& operator=(aligned_storage x) { swap(*this, x); return *this; }
        
    T& get() { return *static_cast<T*>(storage()); }
    const T& get() const { return *static_cast<const T*>(storage()); }
    
    friend inline void swap(aligned_storage& x, aligned_storage& y)
    { swap(x.get(), y.get()); }
    
 private:
    enum { word_size = 16 }; // quad word alignment
    
    typedef double storage_t[((sizeof(T) + (word_size - 1)) / word_size) * (word_size / sizeof(double))];
    
    void* storage() { return &data_m; }
    const void* storage() const { return &data_m; }
    storage_t data_m;
    
    BOOST_STATIC_ASSERT(sizeof(T) <= sizeof(storage_t));
};

//! @} //end addtogroup memory

/*************************************************************************************************/

} // namespace adobe

ADOBE_NAME_TYPE_1("capture_allocator:version_1:adobe", adobe::version_1::capture_allocator<T0>)

/*************************************************************************************************/

#endif

/*************************************************************************************************/
