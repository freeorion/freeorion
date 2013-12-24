#ifndef _TemporaryPtr_tcc_
#define _TemporaryPtr_tcc_

#include "EnableTemporaryFromThis.h"
#include "TemporaryPtr.h"
#include <boost/mpl/assert.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <algorithm>

template<class T>
T* TemporaryPtr<T>::get() const 
{ return m_ptr.get(); }

template<class T> template <class P>
TemporaryPtr<T>& TemporaryPtr<T>::internal_assign(const P& rhs) {
    BOOST_MPL_ASSERT((boost::is_convertible<typename P::element_type*, T*>));
    BOOST_MPL_ASSERT((boost::is_convertible<T*, const typename T::enable_temporary_from_this_type*>));
    T*                        our_raw = get();
    typename P::element_type* rhs_raw = rhs.get();
    if (our_raw != rhs_raw) {
        /* What is secured here is neither the target object, nor the pointer itself.
         * What we absolutely need to synchronize is accessing and updating the 
         * reference counting base of m_ptr, which is shared between all pointers to
         * the same object (this is an internal structure of boost::shared_ptr and
         * boost::weak_ptr). If you need synchronization of the pointer value or the
         * target object, this is outside the scope of TemporaryPtr.
         *
         * Because we have to acquire two mutexes, we do so in order of their address
         * value. This prevents deadlocks between concurrent pointer assigments, but
         * it does not prevent other deadlocks. Keep in mind that pointer assignment
         * can block the current thread until other manipulations of the refcounting
         * base, like TemporaryFromThis(), have completed.
         */
        boost::mutex sentinel1, sentinel2;
        boost::mutex* mutex_in_owned_object = our_raw? &( our_raw->m_ptr_mutex ) : &sentinel1;
        boost::mutex* mutex_in_other_object = rhs_raw? &( rhs_raw->m_ptr_mutex ) : &sentinel2;
        boost::mutex* mutex1 = (std::min)(mutex_in_owned_object, mutex_in_other_object);
        boost::mutex* mutex2 = (std::max)(mutex_in_owned_object, mutex_in_other_object);
        boost::unique_lock<boost::mutex> guard1(*mutex1);
        boost::unique_lock<boost::mutex> guard2(*mutex2);

        m_ptr = rhs; // both refcounting_bases are modified!
    }
    return *this;
}

/** TemporaryPtr friend operators */
//@{


template <class Y, class R>
bool operator ==(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second)
{ return first.get() == second.get(); }

template <class Y, class R>
bool operator ==(const TemporaryPtr<Y>& first, R* second)
{ return first.get() == second; }

template <class Y, class R>
bool operator <(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second)
{ return first.get() < second.get(); }

template <class Y, class R>
bool operator <(const TemporaryPtr<Y>& first, R* second)
{ return first.get() < second; }

template <class Y, class R>
bool operator <(Y* first, const TemporaryPtr<R>& second)
{ return first < second.get(); }

//@}

/** TemporaryPtr total order operators */
//@{

template <class Y, class R>
bool operator ==(Y* first, const TemporaryPtr<R>& second)
{ return second == first; }

#define TEMPORARY_PTR_OP(result_type, op, body) \
    template <class Y, class R> \
    result_type operator op (const TemporaryPtr<Y>& _1, const TemporaryPtr<R>& _2) \
    { return body; } \
    template <class Y, class R> \
    result_type operator op (const TemporaryPtr<Y>& _1,  R* _2) \
    { return body; } \
    template <class Y, class R> \
    result_type operator op (Y* _1, const TemporaryPtr<R>& _2) \
    { return body; }

TEMPORARY_PTR_OP(bool, !=, !(_1 == _2))
TEMPORARY_PTR_OP(bool, > ,   _2 <  _1 )
TEMPORARY_PTR_OP(bool, >=, !(_1 <  _2))
TEMPORARY_PTR_OP(bool, <=, !(_1 >  _2))

//@}

namespace boost {

template <class Y, class R>
TemporaryPtr<Y> static_pointer_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::static_pointer_cast<Y>(item.m_ptr)); }

template <class Y, class R>
TemporaryPtr<Y> dynamic_pointer_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::dynamic_pointer_cast<Y>(item.m_ptr)); }

template <class Y, class R>
TemporaryPtr<Y> const_pointer_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::const_pointer_cast<Y>(item.m_ptr)); }

template <class Y, class R>
TemporaryPtr<Y> reinterpret_pointer_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::reinterpret_pointer_cast<Y>(item.m_ptr)); }

} // namespace boost

#endif // _TemporaryPtr_tcc_
