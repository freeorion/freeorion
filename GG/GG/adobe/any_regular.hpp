/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/**************************************************************************************************/


#ifndef ADOBE_ANY_REGULAR_HPP
#define ADOBE_ANY_REGULAR_HPP

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular_fwd.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/noncopyable.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/has_nothrow_copy.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/type_traits/is_reference.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/concept_check.hpp>

#include <GG/adobe/conversion.hpp>
#include <GG/adobe/cstdint.hpp>
#include <GG/adobe/empty.hpp>
#include <GG/adobe/memory.hpp>
#include <GG/adobe/move.hpp>
#include <GG/adobe/typeinfo.hpp>
#include <GG/adobe/regular_concept.hpp>

#include <GG/adobe/implementation/swap.hpp>

#if defined(ADOBE_STD_SERIALIZATION)
#include <iosfwd>
#endif

/**************************************************************************************************/

namespace adobe {
        
/*! 
\defgroup any_regular any_regular_t
\ingroup runtime_concepts abi_safe
*/


/*!
\fn adobe::any_regular_t::any_regular_t(const T& x)

\param x Value to copy as the instance value of this object.

\note
    What is stored is a copy of <code>x</code> promoted to <code>promote<T>::type</code>.
*/

/*!
\fn const typename promote<T>::type& adobe::any_regular_t::cast() const

\return
    A refernce to a promoted type for <code>T</code> containing the <code>any_regular_t</code>'s instance value.

\exception
    adobe::bad_cast Thrown if <code>adobe::promote<T></code> does not match the stored instance type.
*/

/*!
\fn typename promote<T>::type& adobe::any_regular_t::cast()

\return
    A refernce to a promoted type for <code>T</code> containing the <code>any_regular_t</code>'s instance value.

\exception
    adobe::bad_cast Thrown if <code>adobe::promote<T></code> does not match the stored instance type.
*/

/*!
\fn bool adobe::any_regular_t::cast(T& x) const

\param
    x Value that will be set to the value stored in the any_regular_t.

\return
    <code>true</code> if x was set successfully, <code>false</code> otherwise.

\exception
    Undefined Will relay anything thrown by <code>x.operator=()</code>.
*/

/*!
\fn adobe::any_regular_t& adobe::any_regular_t::assign(const T& x)

\param x The new value to assign to this any_regular_t.

\return
    The any_regular_t with its new value set.
*/

/*!
\fn const adobe::type_info_t& adobe::any_regular_t::type() const

\return
    The information returned by <code>typeid()</code> for the instance value.
*/

/*!
\class adobe::any_regular_t::transform
\ingroup any_regular

\brief Function object used in binding for instance value access.
*/

/*!
\typedef adobe::any_regular_t::transform::result_type

The promoted value type from T that will be returned by this function object's <code>operator()</code>s.
*/

/*!
\typedef adobe::any_regular_t::transform::argument_type

Parameter type required by this function object's <code>operator()</code>s.
*/

/*!
\fn promote<T>::type& adobe::any_regular_t::transform::operator()(any_regular_t &x) const

\param x The value from which the result is to be obtained.

\return
    A reference to the instance value stored in this any_regular_t.
*/

/*!
\fn const promote<T>::type& adobe::any_regular_t::transform::operator()(const any_regular_t &x) const

\param x The value from which the result is to be obtained.

\return
    A reference to the instance value stored in this any_regular_t.
*/

/**************************************************************************************************/

namespace implementation {

enum { vtable_version = 1 };

struct any_regular_interface_t;

struct vtable_t
{
    typedef any_regular_interface_t interface_type;

    adobe::uintptr_t    version;
    void                (*destruct)(const interface_type&);
    type_info_t         (*type_info)(const interface_type&);
    interface_type*     (*clone)(const interface_type&, void*);
    interface_type*     (*move_clone)(interface_type&, void*);
    void                (*assign)(interface_type&, const interface_type&);
    bool                (*equals)(const interface_type&, const interface_type&);
    void                (*exchange)(interface_type&, interface_type&);
};

// Ensure that the vtable_t has a fixed layout regardless of alignment or packing.

BOOST_STATIC_ASSERT(sizeof(vtable_t) == 8 * sizeof(void*));

/**************************************************************************************************/

union pad_vtable_t
{
    const vtable_t* vtable_m;
    double pad_m; // unused padding
};

BOOST_STATIC_ASSERT(sizeof(pad_vtable_t) == sizeof(double));

/**************************************************************************************************/

struct any_regular_interface_t {
    typedef any_regular_interface_t interface_type;

    any_regular_interface_t(const vtable_t& x) { object_m.vtable_m = &x; }

    pad_vtable_t    object_m;
    
    void            destruct() const { return object_m.vtable_m->destruct(*this); }
    type_info_t     type_info() const { return object_m.vtable_m->type_info(*this); }
    interface_type* clone(void* x) const { return object_m.vtable_m->clone(*this, x); }
    interface_type* move_clone(void* x) { return object_m.vtable_m->move_clone(*this, x); }
    void            assign(const interface_type& x) { object_m.vtable_m->assign(*this, x); }
    bool            equals(const interface_type& x) const { return object_m.vtable_m->equals(*this, x); }
    void            exchange(interface_type& x) { object_m.vtable_m->exchange(*this, x); }
};

/**************************************************************************************************/

template <typename T> // T models Regular
struct any_regular_model_local : any_regular_interface_t, boost::noncopyable
{
    typedef any_regular_interface_t interface_type;

    T object_m;
    
    static const vtable_t vtable_s;

    any_regular_model_local() : interface_type(vtable_s), object_m() { }
    
    template <typename U>
    explicit any_regular_model_local(const U& x, typename copy_sink<U, T>::type = 0)
        : interface_type(vtable_s), object_m(x) { }
    
    template <typename U>
    explicit any_regular_model_local(U x, typename move_sink<U, T>::type = 0)
        : interface_type(vtable_s), object_m(::adobe::move(x)) { }
        
    static const any_regular_model_local& self(const interface_type& x)
    { return static_cast<const any_regular_model_local&>(x); }
        
    static any_regular_model_local& self(interface_type& x)
    { return static_cast<any_regular_model_local&>(x); }
            
    static  type_info_t type_info(const interface_type&)
    { return adobe::type_info<T>(); }
    
    static void destruct(const interface_type& x)
    { self(x).~any_regular_model_local(); }
    
    static interface_type* clone(const interface_type& x, void* storage)
    { return ::new(storage) any_regular_model_local(self(x).object_m); }
    
    static interface_type* move_clone(interface_type& x, void* storage)
    { return ::new(storage) any_regular_model_local(::adobe::move(self(x).object_m)); }
    
    static void assign(interface_type& x, const interface_type& y)
    { self(x).object_m = self(y).object_m; }
    
    static bool equals(const interface_type& x, const interface_type& y)
    { return self(x).object_m == self(y).object_m; }
    
    static void exchange(interface_type& x, interface_type& y)
    { swap(self(x).object_m, self(y).object_m); }
    
    const T& get() const { return object_m; }
    T& get() { return object_m; }
};

BOOST_STATIC_ASSERT(sizeof(any_regular_model_local<double>) == 16);

template <typename T>
const vtable_t any_regular_model_local<T>::vtable_s
    = {
#ifndef NDEBUG
        short_name<T>::value,
#else
        vtable_version,
#endif
        &any_regular_model_local::destruct,
        &any_regular_model_local::type_info,
        &any_regular_model_local::clone,
        &any_regular_model_local::move_clone,
        &any_regular_model_local::assign,
        &any_regular_model_local::equals,
        &any_regular_model_local::exchange,
    };

template <typename T> // T models Regular
struct any_regular_model_remote : any_regular_interface_t, boost::noncopyable
{
    BOOST_CLASS_REQUIRE(T, adobe, RegularConcept);
    
    typedef any_regular_interface_t interface_type;
    
    struct object_t;
    typedef capture_allocator<object_t> allocator_type;
    
    struct object_t : boost::noncopyable
    {
        aligned_storage<allocator_type>   alloc_m;
        T                                 data_m;
    };

    static object_t* new_copy(const T& x)
    {
        allocator_type a;
        object_t* result = a.allocate(1);
        construct(&result->alloc_m, aligned_storage<allocator_type>(a));
        try {
            construct(&result->data_m, x);
        } catch (...) {
            destroy(&result->alloc_m);
            a.deallocate(result, 1);
        }
        return result;
    }

    static object_t* new_move(T& x)
    {
        allocator_type a;
        object_t* result = a.allocate(1);
        construct(&result->alloc_m, aligned_storage<allocator_type>(a));
        try {
            move_construct(&result->data_m, x);
        } catch (...) {
            destroy(&result->alloc_m);
            a.deallocate(result, 1);
        }
        return result;
    }

    object_t* object_ptr_m;
    
    static const vtable_t vtable_s;

    template <typename U>
    explicit any_regular_model_remote(const U& x, typename copy_sink<U, T>::type = 0)
        : interface_type(vtable_s), object_ptr_m(new_copy(x))
        { }
    
    template <typename U>
    explicit any_regular_model_remote(U x, typename move_sink<U, T>::type = 0)
        : interface_type(vtable_s), object_ptr_m(new_move(x))
        { }

    any_regular_model_remote(move_from<any_regular_model_remote> x)
        : interface_type(vtable_s), object_ptr_m(x.source.object_ptr_m) { x.source.object_ptr_m = 0; }
    
    ~any_regular_model_remote()
    {
        if (object_ptr_m) {
            allocator_type a = object_ptr_m->alloc_m.get();
            destroy(&object_ptr_m->alloc_m);
            destroy(&object_ptr_m->data_m);
            a.deallocate(object_ptr_m, 1);
        }
    }
    
    static const any_regular_model_remote& self(const interface_type& x)
    { return static_cast<const any_regular_model_remote&>(x); }
        
    static any_regular_model_remote& self(interface_type& x)
    { return static_cast<any_regular_model_remote&>(x); }
    
    static  type_info_t type_info(const interface_type&)
    { return adobe::type_info<T>(); }
    
    static void destruct(const interface_type& x)
    { return self(x).~any_regular_model_remote(); }
    
    static interface_type* clone(const interface_type& x, void* storage)
    { return ::new(storage) any_regular_model_remote(self(x).get()); }
    
    static interface_type* move_clone(interface_type& x, void* storage)
    { return ::new(storage) any_regular_model_remote(move_from<any_regular_model_remote>(self(x))); }
    
    static void assign(interface_type& x, const interface_type& y)
    { self(x).get() = self(y).get(); }
    
    static bool equals(const interface_type& x, const interface_type& y)
    { return self(x).get() == self(y).get(); }
    
    static void exchange(interface_type& x, interface_type& y)
    { return swap(self(x).object_ptr_m, self(y).object_ptr_m); }
    
    const T& get() const { return object_ptr_m->data_m; }
    T& get() { return object_ptr_m->data_m; }
};

BOOST_STATIC_ASSERT(sizeof(any_regular_model_remote<double>) <= 16);

template <typename T>
const vtable_t any_regular_model_remote<T>::vtable_s
    = {
#ifndef NDEBUG
        short_name<T>::value,
#else
        vtable_version,
#endif
        &any_regular_model_remote::destruct,
        &any_regular_model_remote::type_info,
        &any_regular_model_remote::clone,
        &any_regular_model_remote::move_clone,
        &any_regular_model_remote::assign,
        &any_regular_model_remote::equals,
        &any_regular_model_remote::exchange,
    };

/**************************************************************************************************/

} // namespace implementation

/**************************************************************************************************/

namespace version_1 {

/**************************************************************************************************/

/*!
\ingroup any_regular

\brief A runtime polymorphic type similar to boost::any which can hold any type which models
    \ref concept_regular_type.

\model_of
    - \ref concept_movable

\details
\par Type Promotion
    \c any_regular_t leverages \c adobe::promote while storing instance values.
    \todo The reliance of any_regular_t on type promotion is current problematic and really
    ammounts to a poor-mans approach to refinements. In this case the desired functionality is to 
    be able to extract something which models a number regardless of the actual numeric type 
    stored. The feature becomes especially important if serialization happens between the
    writer and reader. This is still an open problem.

\par Getting a Value
    \c any_regular_t uses <code>cast<>()</code> to get its instance value. It behaves like 
    an explicit cast getting a any_regular_t out will always succeed and simply returns the same 
    any_regular_t.

\par Setting a Value
    \c any_regular_t uses <code>assign<>()</code> to set its instance value. It behaves like 
    an explicit assignment. Assigning an any_regular_t into a any_regular_t simply assigns the 
    values; it does not introduce another layer of indirection.

\par How does adobe::any_regular_t differ from boost::any?
    any_regular_t differs from <a href="http://www.boost.org/doc/html/any.html">
    <code>boost::any</code></a> in several ways:
        - any_regular_t models \ref stldoc_EqualityComparable, fully modeling a 
            \ref concept_regular_type.
        - any_regular_t supports type promotion. See <code>adobe::promote</code> for more details.
        - cast<>() results are returned by reference, making any_regular_t generally more efficient.
        - small values (less than or equal to 64 bits) with a non-throwing copy constructor or which
            model \ref concept_movable are stored without a free store allocation.
        
\note With currently compilers the trait 
<a href="http://www.boost.org/doc/libs/1_35_0/libs/type_traits/doc/html/boost_typetraits/reference/has_nothrow_copy.html">
<code>boost::has_no_throw_copy</code></a> must be manually declared for small types to be stored 
without a free store allocation.

\see_also
    adobe::runtime_cast

\tutorial
    A tutorial for \ref asl_tutorials_value_t is available.
*/

class any_regular_t : boost::equality_comparable<any_regular_t, any_regular_t>
{
    typedef implementation::any_regular_interface_t    interface_type;
    typedef double storage_t[2];
    
    #ifndef ADOBE_NO_DOCUMENTATION
    
    template <typename T>
    struct traits
    {
        typedef typename promote<T>::type                               promote_type;
        BOOST_CLASS_REQUIRE(promote_type, adobe, RegularConcept);

        typedef promote_type&                                           reference_type;
        typedef const promote_type&                                     const_reference_type;
        
        typedef implementation::any_regular_model_local<promote_type>   regular_model_local_type;
        typedef implementation::any_regular_model_remote<promote_type>  regular_model_remote_type;
        
        typedef boost::mpl::bool_<(sizeof(regular_model_local_type)
                <= sizeof(storage_t))
                && (boost::has_nothrow_copy<promote_type>::value
                || is_movable<promote_type>::value)>                    use_local_type;
        
        typedef typename boost::mpl::if_<use_local_type,
                regular_model_local_type,
                regular_model_remote_type>::type                        model_type;
            
        typedef typename boost::mpl::if_c<
            boost::is_same<promote_type, T>::value,
            reference_type, T>::type                                    result_type;
    
        typedef typename boost::mpl::if_c<
            boost::is_same<promote_type, T>::value,
            const_reference_type, T>::type                              const_result_type;
    };
    
    template <typename T> struct helper;
    template <typename T> friend struct helper;
    
    #endif
    
 public:
    /*!
    \name Movable Functions
    \see_only
        \ref concept_movable
    */
    /*!@{*/
    any_regular_t() : data_ptr_m(&data_m[0]) { ::new (storage()) traits<empty_t>::model_type(); }

    any_regular_t(const any_regular_t& x) : data_ptr_m(&data_m[0]) { x.object().clone(storage()); }
    
    any_regular_t(move_from<any_regular_t> x) : data_ptr_m(&data_m[0]) { x.source.object().move_clone(storage()); }
    
    any_regular_t& operator=(any_regular_t x)
    {
        object().destruct();
        x.object().move_clone(storage());
        return *this;
    }

    ~any_regular_t() { object().destruct(); }

    friend inline bool operator == (const any_regular_t& x, const any_regular_t& y)
    { return (x.type_info() == y.type_info()) && x.object().equals(y.object()); }
    
    friend inline void swap(any_regular_t& x, any_regular_t& y)
    {
        interface_type& a(x.object());
        interface_type& b(y.object());
    
        if (a.type_info() == b.type_info()) { a.exchange(b); return; }
        
        // x->tmp
        any_regular_t tmp((move_from<interface_type>(a)));
        a.destruct();
        
        // y->x
        b.move_clone(x.storage());
        b.destruct();
        
        // tmp->y
        tmp.object().move_clone(y.storage());
    }
    /*!@}*/
    
    /*!@{*/
    
    /*!
    \param x Value is promoted to type \c promote<T>::type and stored in the object.
    */

    template <typename T>
    explicit any_regular_t(const T& x, typename copy_sink<T>::type = 0) : data_ptr_m(&data_m[0])
    { ::new (storage()) typename traits<T>::model_type(x); }
    
    template <typename T>
    explicit any_regular_t(T x, typename move_sink<T>::type = 0) : data_ptr_m(&data_m[0])
    { ::new (storage()) typename traits<T>::model_type(::adobe::move(x)); }
    
    /*!@}*/
    
    /*!@{*/

    /*!
    \param x
        If \c promote<T>::type is equal to \c T then the value stored will be assigned to \c x, 
        otherwise \c x is unchanged
    
    \return
        \true if \c x was assigned to, \false otherwise.
    */
    
    template <typename T>
    bool cast(T& x) const
    {
        if (type_info() != adobe::type_info<typename promote<T>::type>()) return false;
        x = cast<T>();
        return true;
    }

    template <typename T>
    typename traits<T>::const_result_type cast() const
    { return helper<T>::cast(*this); }
    
    /*!
    \return
        A [const]referece to the stored value if \c T is equal to \c promote<T>::type otherwise 
        \c static_cast<T>(value) where value is the stored value is return.
    
    \exception adobe::bad_cast
        Thrown if \c promote<T>::type does not match the store instance type.
    */
    
    template <typename T>
    typename traits<T>::result_type cast()
    { return helper<T>::cast(*this); }
    
    /*!@}*/

    /*!@{*/
    
    /*!
    
    \param x
        Assigned to the stored value.
    
    \return
        The any_regular_t with its new value.
    */
    
    template <typename T>
    any_regular_t& assign(const T& x, typename copy_sink<T>::type = 0)
    { return helper<T>::assign(*this, x); }
    
    template <typename T>
    any_regular_t& assign(T x, typename move_sink<T>::type = 0)
    { 
        object().destruct();
        ::new (storage()) typename traits<T>::model_type(::adobe::move(x));
        return *this;
    }
    
    any_regular_t& assign(any_regular_t x)
    { 
        object().destruct();
        x.object().move_clone(storage());
        return *this;
    }
    
    /*!@}*/

    /*!@{*/

    /*!
    \return
        The information returned by <code>adobe::type_info<T>()</code> for the stored value.
    */
    
    type_info_t type_info() const { return object().type_info(); } 

    /*!
    \brief Function object used in binding for instance value access.
    */
    
    /*!@}*/

    
    template <typename T>
    struct transform
    {
        typedef typename traits<T>::promote_type    result_type;
        
        typename traits<T>::result_type operator () (any_regular_t& x) const
        { return x.cast<T>(); }
        
        typename traits<T>::const_result_type operator () (const any_regular_t& x) const
        { return x.cast<T>(); }
    };
    
#if defined(ADOBE_STD_SERIALIZATION)
    friend std::ostream& operator<<(std::ostream& out, const any_regular_t& value);
#endif

 private:    
    any_regular_t(move_from<interface_type> x) : data_ptr_m(&data_m[0]) { x.source.move_clone(storage()); }
 
    interface_type& object() { return *static_cast<interface_type*>(storage()); }
    const interface_type& object() const { return *static_cast<const interface_type*>(storage()); }
    
    void* storage() { return static_cast<void*>(data_ptr_m); }
    const void* storage() const { return static_cast<const void*>(data_ptr_m); }
    
    storage_t data_m;
    void* data_ptr_m;

#ifndef ADOBE_NO_DOCUMENTATION

#ifdef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
    template <typename, typename> friend struct adobe::runtime_cast_t;
#endif

    template <typename T>
    T* ptr_cast()
    { return helper<T>::ptr_cast(*this); }

#endif
};

// disabled due to reports of problems on some architectures, and apparent
// lack of necessity for FreeOrion
//#ifndef ADOBE_NO_DOCUMENTATION
#if 0
    BOOST_STATIC_ASSERT((sizeof(any_regular_t) == 16 + sizeof(void*)));
#endif

/**************************************************************************************************/

#ifndef ADOBE_NO_DOCUMENTATION

template <typename T>
struct any_regular_t::helper
{
    static inline T* ptr_cast(any_regular_t& r)
    {
        if (r.type_info() != adobe::type_info<T>())
            return 0;
        return &reinterpret_cast<typename traits<T>::model_type&>(r.object()).get();
    }

    static inline typename traits<T>::const_result_type cast(const any_regular_t& r)
    {
        typedef typename traits<T>::promote_type promote_type;
    
        if (r.type_info() != adobe::type_info<promote_type>())
            throw bad_cast(r.type_info(), adobe::type_info<promote_type>());

        const void* void_ptr = static_cast<const void*>(&r.object());
        const typename traits<T>::model_type* model_ptr = static_cast<const typename traits<T>::model_type*>(void_ptr);
        return static_cast<typename traits<T>::const_result_type>(model_ptr->get());
    }
    
    static inline typename traits<T>::result_type cast(any_regular_t& r)
    {
        typedef typename traits<T>::promote_type promote_type;
        
        if (r.type_info() != adobe::type_info<promote_type>())
            throw bad_cast(r.type_info(), adobe::type_info<promote_type>());

        void* void_ptr = static_cast<void*>(&r.object());
        typename traits<T>::model_type* model_ptr = static_cast<typename traits<T>::model_type*>(void_ptr);
        return static_cast<typename traits<T>::result_type>(model_ptr->get());
    }
    
    static inline any_regular_t& assign(any_regular_t& r, const T& x)
    {
        typedef typename promote<T>::type promote_type;

        if (r.type_info() == adobe::type_info<promote_type>())
            r.cast<promote_type>() = static_cast<promote_type>(x);
        else
        {
            any_regular_t result(x);
            swap(r, result);
        }
        return r;
    }
};

/**************************************************************************************************/

template <>
struct any_regular_t::helper<any_regular_t>
{
    static inline any_regular_t* ptr_cast(any_regular_t& r)
    { return &r; }

    static inline const any_regular_t& cast(const any_regular_t& r)
    { return r; }
    
    static inline any_regular_t& cast(any_regular_t& r)
    { return r; }
};

#endif

/**************************************************************************************************/

} // namespace version_1

/**************************************************************************************************/

/*!
    \return \true if \c x contains \c empty_t(), \false otherwise.
   
    \related adobe::version_1::any_regular_t
*/

inline bool empty(const any_regular_t& x) { return x.type_info() == type_info<empty_t>(); }

/**************************************************************************************************/

/*!
    \related adobe::version_1::any_regular_t
*/

template <typename R>
struct runtime_cast_t<R, const any_regular_t>
{
    R operator()(const any_regular_t& x) const
    {
        typedef typename boost::remove_const<typename boost::remove_reference<R>::type>::type
                result_type;

        BOOST_STATIC_ASSERT((boost::is_reference<R>::value));

        /* There is no auto-promotion through the new interface. Soon promotion will be disabled. */
        BOOST_STATIC_ASSERT((boost::is_same<typename promote<result_type>::type, result_type>::value));

        return x.cast<result_type>();
    }
};

/**************************************************************************************************/

template <typename R>
struct runtime_cast_t<R, any_regular_t>
{
    R operator()(any_regular_t& x) const
    {
        typedef typename boost::remove_reference<R>::type result_type;

        BOOST_STATIC_ASSERT((boost::is_reference<R>::value));

        /* There is no auto-promotion through the new interface. Soon promotion will be disabled. */
        BOOST_STATIC_ASSERT((boost::is_same<typename promote<result_type>::type, result_type>::value));

        return x.cast<result_type>();
    }
};

/**************************************************************************************************/

template <typename R>
struct runtime_cast_t<R, any_regular_t*>
{
    R operator()(any_regular_t* x) const
    {
        typedef typename boost::remove_pointer<R>::type result_type;

        BOOST_STATIC_ASSERT((boost::is_pointer<R>::value));

        /* There is no auto-promotion through the new interface. Soon promotion will be disabled. */
        BOOST_STATIC_ASSERT((boost::is_same<typename promote<result_type>::type, result_type>::value));

        return x->ptr_cast<result_type>();
    }
};

/**************************************************************************************************/

template <typename R>
struct runtime_cast_t<R, const any_regular_t*>
{
    R operator()(const any_regular_t* x) const
    {
        typedef typename boost::remove_const<typename boost::remove_pointer<R>::type>::type
                result_type;

        BOOST_STATIC_ASSERT((boost::is_pointer<R>::value));

        /* There is no auto-promotion through the new interface. Soon promotion will be disabled. */
        BOOST_STATIC_ASSERT((boost::is_same<typename promote<result_type>::type, result_type>::value));

        if (x->type_info() != type_info<result_type>()) return 0;
        return &x->cast<result_type>();
    }
};

/**************************************************************************************************/

} // namespace adobe

/**************************************************************************************************/

ADOBE_NAME_TYPE_0("any_regular_t:version_1:adobe", adobe::version_1::any_regular_t)

/**************************************************************************************************/

#endif

/**************************************************************************************************/
