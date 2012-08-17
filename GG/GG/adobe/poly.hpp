/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_POLY_HPP
#define ADOBE_POLY_HPP

#include <GG/adobe/config.hpp>


#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/type_traits/remove_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/has_nothrow_constructor.hpp>

#include <GG/adobe/move.hpp>
#include <GG/adobe/implementation/swap.hpp>
#include <GG/adobe/typeinfo.hpp>

/*************************************************************************************************/

namespace adobe {


/*!
\defgroup runtime_concepts Runtime Concepts
\ingroup utility
*/

/*!
\defgroup poly_related Poly Library
\ingroup runtime_concepts
\ingroup layout_library\

\brief Support for creating Regular, runtime-polymorphic objects with value semantics
@{
*/


#if !defined(ADOBE_NO_DOCUMENTATION)

template <typename T, typename U>
struct is_base_derived_or_same : boost::mpl::or_<boost::is_base_of<T, U>,
                                                 boost::is_base_of<U, T>,
                                                 boost::is_same<T, U> > {};
#endif
// !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

/*!
\ingroup poly_related
\brief Abstract interface providing signatures needed to implement "handle" objects modeling a
Value (Copyable/Movable) concept. Authors of \ref adobe::poly classes must derive their interfaces from
this. See \ref adobe::poly_placeable_interface for an example.
*/

struct poly_copyable_interface {
    virtual poly_copyable_interface* clone(void*) const = 0;
    virtual poly_copyable_interface* move_clone(void*) = 0;
    virtual void* cast() = 0;
    virtual const void* cast() const = 0;
    virtual const std::type_info& type_info() const = 0;

    // Precondition of assignment: this->type_info() == x.type_info()
    virtual void assign(const poly_copyable_interface& x) = 0;

    // Precondition of exchange: this->type_info() == x.type_info()
    virtual void exchange(poly_copyable_interface& x) = 0;

    virtual ~poly_copyable_interface() {}
};

/*************************************************************************************************/

#if !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

namespace implementation {

/*************************************************************************************************/

template <typename ConcreteType, typename Interface>
struct poly_state_remote : Interface
{
    typedef ConcreteType value_type;
    typedef Interface    interface_type;

    const value_type& get() const { return *value_ptr_m; }
    value_type& get() { return *value_ptr_m; }

    poly_state_remote(move_from<poly_state_remote> x)
        : value_ptr_m(x.source.value_ptr_m){ x.source.value_ptr_m = NULL; }

    template <typename U>
    explicit poly_state_remote(const U& x, typename copy_sink<U, value_type>::type = 0)
        : value_ptr_m(::new value_type(x)) { }
    
    template <typename U>
    explicit poly_state_remote(U x, typename move_sink<U, value_type>::type = 0)
        : value_ptr_m(::new value_type(::adobe::move(x))) { }
    
    ~poly_state_remote()
    { delete value_ptr_m; }

    // Precondition : this->type_info() == x.type_info()
    void assign(const poly_copyable_interface& x)
    { *value_ptr_m = *static_cast<const poly_state_remote&>(x).value_ptr_m; }

    const std::type_info& type_info() const
    { return typeid(value_type); }
    const void* cast() const { return value_ptr_m; }
    void* cast() { return value_ptr_m; }

    // Precondition : this->type_info() == x.type_info()
    void exchange(poly_copyable_interface& x)
    { return std::swap(value_ptr_m, static_cast<poly_state_remote&>(x).value_ptr_m); }

    // Precondition : this->type_info() == x.type_info()
    friend bool operator==(const poly_state_remote& x, const poly_state_remote& y)
    { return *x.value_ptr_m == *y.value_ptr_m; }

    value_type* value_ptr_m;
};

/*************************************************************************************************/

template <typename ConcreteType, typename Interface>
struct poly_state_local : Interface
{
    typedef ConcreteType value_type;
    typedef Interface    interface_type;

    const value_type& get() const { return value_m; }
    value_type& get() { return value_m; }

    poly_state_local(move_from<poly_state_local> x)
        : value_m(::adobe::move(x.source.value_m)){ }

    template <typename U>
    explicit poly_state_local(const U& x, typename copy_sink<U, value_type>::type = 0)
        : value_m(x) { }
    
    template <typename U>
    explicit poly_state_local(U x, typename move_sink<U, value_type>::type = 0)
        : value_m(::adobe::move(x)) { }
    
    // Precondition : this->type_info() == x.type_info()
    void assign(const poly_copyable_interface& x)
    { value_m = static_cast<const poly_state_local&>(x).value_m; }

    const std::type_info& type_info() const
    { return typeid(value_type); }
    const void* cast() const { return &value_m; }
    void* cast() { return &value_m; }
     
    // Precondition : this->type_info() == x.type_info()
    void exchange(poly_copyable_interface& x)
    { return std::swap(value_m, static_cast<poly_state_local&>(x).value_m); }

    // Precondition : this->type_info() == x.type_info()
    friend bool operator==(const poly_state_local& x, const poly_state_local& y)
    { return x.value_m == y.value_m; }

    value_type value_m;
};


/*************************************************************************************************/

typedef double storage_t[2];

template<typename T, int N=sizeof(storage_t)>
struct is_small
{
    enum { value = sizeof(T) <= N && (boost::has_nothrow_constructor<typename T::value_type>::value ||
                                      boost::is_same<std::string, typename T::value_type>::value) };

};

/*************************************************************************************************/

template <typename F>
struct poly_instance : F {
    typedef typename F::value_type value_type;
    typedef typename F::interface_type interface_type;

    poly_instance(const value_type& x): F(x){ }
    poly_instance(move_from<poly_instance> x) : F(move_from<F>(x.source)) { }

    poly_copyable_interface* clone(void* storage) const
    { return ::new (storage) poly_instance(this->get()); }

    poly_copyable_interface* move_clone(void* storage)
    { return ::new (storage) poly_instance(move_from<poly_instance>(*this)); }
};

/*************************************************************************************************/

template <typename T> 
class has_equals { 
    typedef bool (T::*E)(const T&) const; 
    typedef char (&no_type)[1]; 
    typedef char (&yes_type)[2]; 
    template <E e> struct sfinae { typedef yes_type type; }; 
    template <class U> 
    static typename sfinae<&U::equals>::type test(int); 
    template <class U> 
    static no_type test(...); 
public: 
    enum {value = sizeof(test<T>(1)) == sizeof(yes_type)}; 
}; 

/*************************************************************************************************/

} //namespace implementation

/*************************************************************************************************/

#endif
//  !defined(ADOBE_NO_DOCUMENTATION)

/*************************************************************************************************/

/*!
  \ingroup poly_related
  \brief Authors of \ref adobe::poly concept representatives must derive their instance class from this.
  See of \ref adobe::poly_placeable_instance, for example. This metafunction is used in the implementation 
  of the small storage optimization.
*/

template <typename ConcreteType, typename Interface>
struct optimized_storage_type : 
    boost::mpl::if_<implementation::is_small<implementation::poly_state_local<ConcreteType, Interface> >,
                    implementation::poly_state_local<ConcreteType, Interface>,
                    implementation::poly_state_remote<ConcreteType, Interface> > {
};


/*************************************************************************************************/

/*!  
\brief Authors of a Concept representative F, intended as a template
parameter to \ref adobe::poly, will inherit from \ref adobe::poly_base.  The first
template parameter for \ref adobe::poly_base provides the virtual interface
for the concept representative.  The second template parameter for
\ref adobe::poly_base must inherit from the
Concept interface representative.  The author's third duty is to
provide forwarding functions in a their Concept representative. See
the \ref placeable_concept.hpp header file for details.
*/

template <typename I, template <typename> class Instance>
struct poly_base {

    template <typename T, template <typename> class U>
    friend struct poly_base;

    typedef I interface_type;

    // Construct from value type
    template <typename T>
    explicit poly_base(const T& x, 
        typename copy_sink<T>::type = 0, 
        typename boost::disable_if<boost::is_base_of<poly_base, T> >::type* = 0)
    { ::new (storage()) implementation::poly_instance<Instance<T> >(x); }
    
    template <typename T>
    explicit poly_base(T x, 
        typename move_sink<T>::type = 0,
        typename boost::disable_if<boost::is_base_of<poly_base, T> >::type* = 0)
    { ::new (storage()) implementation::poly_instance<Instance<T> >(::adobe::move(x)); }




    // Construct from related interface (might throw on downcast)
    template <typename J, template <typename> class K> 
    explicit poly_base(const poly_base<J, K>& x , 
        typename boost::enable_if<is_base_derived_or_same<I, J> >::type* dummy = 0)
    {
        if(boost::is_base_of<J, I>::value)
            dynamic_cast<const I&>(static_cast<const poly_copyable_interface&>(x.interface_ref()));
       x.interface_ref().clone(storage());    
    }

    poly_base(const poly_base& x) { x.interface_ref().clone(storage()); }

    poly_base(move_from<poly_base> x) { x.source.interface_ref().move_clone(storage()); }

    friend inline void swap(poly_base& x, poly_base& y)
    {
        interface_type& a(x.interface_ref());
        interface_type& b(y.interface_ref());
    
        if (a.type_info() == b.type_info()) { a.exchange(b); return; }

        // x->tmp
        poly_base tmp(::adobe::move(x));
        a.~interface_type();
        
        // y->x
        b.move_clone(x.storage());
        b.~interface_type();
        
        // tmp->y
        tmp.interface_ref().move_clone(y.storage());
    }

    poly_base& operator=(poly_base x)
    {
        interface_ref().~interface_type();
        x.interface_ref().move_clone(storage());
        return *this;
    }
    ~poly_base() { interface_ref().~interface_type(); }

    template <typename J, template <typename> class K>
    static bool is_dynamic_convertible_from(const poly_base<J, K>& x)
    { 
        return dynamic_cast<const I*>(static_cast<const poly_copyable_interface*>(&x.interface_ref())); 
    }

    template <typename J>
    bool is_dynamic_convertible_to() const
    { 
        return dynamic_cast<const J*>(static_cast<const poly_copyable_interface*>(&interface_ref())) != NULL; 
    }   

    const std::type_info& type_info() const
        { return interface_ref().type_info(); }

    template <typename T> const T& cast() const
    {
        if (type_info() != typeid(T))
            throw bad_cast(type_info(), typeid(T));
        return *static_cast<const T*>(interface_ref().cast());
    }

    template <typename T> T& cast()
    {
        if (type_info() != typeid(T))
            throw bad_cast(type_info(), typeid(T));
        return *static_cast<T*>(interface_ref().cast());
    }

    template <typename T> bool cast(T& x) const
    {
        if (type_info() != typeid(T))
            return false;
        x = cast<T>();
        return true;
    }

    template <typename T> poly_base& assign(const T& x)
    {
        if (type_info() == typeid(T))
            cast<T>() = x;
        else
        {
            poly_base tmp(x);
            swap(*this, tmp);
        }
        return *this;
    }

        // Assign from related (may throw if downcastisng)
    template <typename J, template <typename> class K> 
    typename boost::enable_if<is_base_derived_or_same<I, J> >::type
    assign(const poly_base<J, K>& x)
    {
        if(boost::is_base_of<J, I>::value)
            dynamic_cast<I&>(static_cast<J&>(*x.interface_ptr())); //make sure type safe
        interface_ref().~interface_type();
        x.interface_ref().clone(storage());      
    }

    const interface_type* operator->() const
    { return &interface_ref(); }

    interface_type* operator->()
    { return &interface_ref(); }

    interface_type& interface_ref() 
    { return *static_cast<interface_type*>(storage()); }
    
    const interface_type& interface_ref() const 
    { return *static_cast<const interface_type *>(storage()); }

    void* storage() { return &data_m; }
    const void* storage() const { return &data_m; }
    
    implementation::storage_t data_m;

};

template <class J, template <typename> class K>
inline typename boost::enable_if<implementation::has_equals<J>, bool>::type
operator==(const poly_base<J, K>& x, const poly_base<J, K>& y)
{ return x.interface_ref().equals(y.interface_ref()); }


/*************************************************************************************************/

/*!

\ingroup poly_related

\brief \ref adobe::poly\<foo\> will be a runtime polymorphic value type wrapper
modelling a concept represented by foo

\sa poly_base
*/

template <class F>   
class poly : public F
{
public:  
/*!

T must be a regular type modeling the concept represented by F

*/
    template <typename T>
    explicit poly(const T& x) : F(x) {}    

    poly(move_from<poly> x) : F(move_from<F>(x.source)) {}
    
    poly& operator=(poly x) { static_cast<F&>(*this) = ::adobe::move(static_cast<F&>(x)); return *this; }

    poly() : F() {}
};

/*************************************************************************************************/

/*!
\ingroup poly_related

Polymorphic cast from \ref poly \<U\> & to T&, where T is another \ref
poly instance. Throws \ref adobe::bad_cast if x does not dynamically model
T's Concept requirement. For example,

\code 
  poly<base_concept> x(....);
  //... 
  poly<refined_concept> & y = poly_cast<poly<refined_concept&>>(x); 
\endcode

*/
template <typename T, typename U>
T poly_cast(poly<U>& x)
{ 
    typedef typename boost::remove_reference<T>::type target_type;
    typedef typename target_type::interface_type target_interface_type;
    if(!x.template is_dynamic_convertible_to<target_interface_type>())
        throw bad_cast(typeid(poly<U>), typeid(T));
    return reinterpret_cast<T>(x);
}

/*************************************************************************************************/

/*!  

\ingroup poly_related

\sa poly_cast(poly <U>& x);
*/

template <typename T, typename U>
T poly_cast(const poly<U>& x)
{ 
    typedef typename boost::remove_reference<T>::type target_type;
    typedef typename target_type::interface_type target_interface_type;
    if(!x.template is_dynamic_convertible_to<target_interface_type>())
        throw bad_cast(typeid(poly<U>), typeid(T));
    return reinterpret_cast<T>(x);
}

/*************************************************************************************************/

/*!
\ingroup poly_related

Polymorphic cast from \ref poly \<U\> * to T *, where T is another \ref
poly instance. Returns NULL if x does not dynamically model T's
Concept requirement. For example,

\code 
  poly<base_concept> *x;
  //... 
  poly<refined_concept> * y = poly_cast<poly<refined_concept *>>(x); 
\endcode

*/

template <typename T, typename U>
T poly_cast(poly<U>* x)
{ 
    typedef typename boost::remove_pointer<T>::type target_type;
    typedef typename target_type::interface_type target_interface_type;
    return x->template is_dynamic_convertible_to<target_interface_type>()
        ? reinterpret_cast<T>(x)
        : NULL;
}

/*************************************************************************************************/

/*!  

\ingroup poly_related

\sa poly_cast(poly <U>* x);
*/


template <typename T, typename U>
T poly_cast(const poly<U>* x)
{ 
    typedef typename boost::remove_pointer<T>::type target_type;
    typedef typename target_type::interface_type target_interface_type;
    return x->template is_dynamic_convertible_to<target_interface_type>()
        ? reinterpret_cast<T>(x)
        : NULL;
}

/*************************************************************************************************/

/*!

\ingroup poly_related

\brief inequality comparison
*/

template <class T>
inline bool operator!=(const poly<T>& x, const poly<T>& y)
{
    return !(x == y);
}


//! @}

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
