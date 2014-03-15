// -*- C++ -*-
#ifndef _TemporaryPtr_tcc_
#define _TemporaryPtr_tcc_

#include "EnableTemporaryFromThis.h"
#include "TemporaryPtr.h"
#include <boost/get_pointer.hpp>
#include <boost/mpl/if.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/type_traits/is_const.hpp>

#include <set>
#include <vector>

// Locking //

namespace detail {
    
/** RAll style refcount locking helper
 * 
  * What is secured here is neither the target object, nor the pointer itself.
  * What we absolutely need to synchronize is accessing and updating the 
  * reference counting bases of m_pvec, which are shared between all pointers 
  * to the same object (this is an internal structure of boost::shared_ptr and
  * boost::weak_ptr). If you need synchronization of the pointer value or the
  * target object, this is outside the scope of TemporaryPtr.
  *
  * Because we have to acquire two mutexes, we do so in order of their address
  * value. This prevents deadlocks between concurrent pointer assigments, but
  * it does not prevent other deadlocks. Keep in mind that pointer assignment
  * can block the current thread until other manipulations of the refcounting
  * base, like TemporaryFromThis(), have completed.
  */
class TemporaryPtrLock {
public:
    /** \name Structors
      * WARNING: argument passing must not cast nor copy! 
      */
    //@{
    template <class P>            TemporaryPtrLock(const P& p);
    template <class P1, class P2> TemporaryPtrLock(const P1& p1, const P2& p2);
    ~TemporaryPtrLock() { unlock(); }
    //@}
    
    void unlock();
private:
    class SingleLock : boost::noncopyable {
    public:
        SingleLock()  {};
        ~SingleLock() { unlock(); };
        template <class P> void lock(const EnableTemporary *, const P&); ///< failsafe but expensive version
        template <class T> void lock(const EnableTemporary *, const boost::shared_ptr<T>&); ///< possibly faster specialization
        template <class T> void lock(const EnableTemporary *, const boost::weak_ptr<T>&);   ///< possibly faster specialization
        void unlock();
    private:
        // NOTE: m_lock must die after m_ptr!
        boost::unique_lock<boost::mutex>       m_lock;
        boost::weak_ptr<const EnableTemporary> m_ptr;
    };
    
    SingleLock m_pvec[2];
};

template <class P>
TemporaryPtrLock::TemporaryPtrLock(const P& p) {
    const EnableTemporary* raw_p = boost::get_pointer(p);
    
    if (raw_p) m_pvec[0].lock(raw_p, p);
}

template <class P1, class P2>
TemporaryPtrLock::TemporaryPtrLock(const P1& p1, const P2& p2) {
    const EnableTemporary* raw_p1 = boost::get_pointer(p1);
    const EnableTemporary* raw_p2 = boost::get_pointer(p2);
    
    /* NOTE: comparison serves two purposes here.
     * 1. It avoids attempting to lock the same pointer twice
     * 2. It locks in ascending order
     * Both is necessary in order to avoid deadlocks.
     */
    if  (raw_p1 < raw_p2) {
        if (raw_p1) m_pvec[0].lock(raw_p1, p1);
        if (raw_p2) m_pvec[1].lock(raw_p2, p2);
    } else if (raw_p1 > raw_p2) {
        if (raw_p2) m_pvec[1].lock(raw_p2, p2);
        if (raw_p1) m_pvec[0].lock(raw_p1, p1);
    } else if (raw_p1 == raw_p2) {
        if (raw_p1) m_pvec[0].lock(raw_p1, p1);
    }
}

inline void TemporaryPtrLock::unlock()  { 
    m_pvec[0].unlock();
    m_pvec[1].unlock();
}

template <class P>
void TemporaryPtrLock::SingleLock::lock(const EnableTemporary* raw_p, const P& smart_p) {
    // test whether the object already owned by a shared_ptr
    // FIXME: thanks boost for this mess. Give us a proper test!
    try {
        boost::unique_lock<boost::mutex> guard(raw_p->m_ptr_mutex);
        
        // succeeds if object is already owned
        // NOTE: NOT unlocking here.
        m_ptr = raw_p->shared_from_this();
        m_lock.swap(guard);
    }
    catch (const boost::bad_weak_ptr&)
    {  } // object is unowned,  no need (and no way) to lock
}

template <class T>
void TemporaryPtrLock::SingleLock::lock(const EnableTemporary* raw_p, const boost::shared_ptr<T>& smart_p) {
    boost::unique_lock<boost::mutex> guard(raw_p->m_ptr_mutex);
        
    m_ptr = smart_p;
    m_lock.swap(guard);
}

template <class T>
void TemporaryPtrLock::SingleLock::lock(const EnableTemporary* raw_p, const boost::weak_ptr<T>& smart_p) {
    boost::unique_lock<boost::mutex> guard(raw_p->m_ptr_mutex);
        
    m_ptr = smart_p;
    m_lock.swap(guard);
}

inline void TemporaryPtrLock::SingleLock::unlock() {
    // NOTE: If the weak pointer is expired we must not access its mutex anymore
    // NOTE: holding the lock prevents the weak pointer from expiring right now
    // NOTE: first test, then reset, then unlock!
    if (m_ptr.expired()) {
        // this is also safe when m_ptr/m_lock are empty
        m_ptr.reset();
        m_lock.release();
    } else {
        m_ptr.reset();
        m_lock.unlock();
    }
}

} // namespace detail

// Accessors //

template<class T>
T* TemporaryPtr<T>::get() const 
{ return m_ptr.get(); }

template<class T>
void TemporaryPtr<T>::reset() { 
    Lock guard(m_ptr);
    
    m_ptr.reset();
}

// Assignment, Swap, Cast, Serialization //

template<class T> template<class Y> TemporaryPtr<T>& TemporaryPtr<T>::operator =(Y* rhs) {
    if (rhs == NULL) {
        reset();
    } else {
        Lock guard(m_ptr, rhs);
        
        /* NOTE: we have to cast three times here to get correct results in all cases.
         * 1. Y* is implicitly upcast to EnableTemporary* by shared_from_this()
         * 2. We revert this by a polymorphic downcast to Y*. 
         *    Virtual inheritance prevents a static cast here.
         * 3. Finally we implicitly cast to T*. This is a type check, 
         *    so that only assignments succeed which allow implicit cast from Y* to T* 
         */
        typedef typename boost::mpl::if_<boost::is_const<Y>, const EnableTemporary, EnableTemporary>::type B;
        boost::shared_ptr<B> shared_B; // implicit upcast to the common base   
        
        // test whether the object already owned by a shared_ptr
        // FIXME: thanks boost for this mess. Give us a proper test!
        try 
        { shared_B = rhs->shared_from_this(); }   // object is already owned
        catch (const boost::bad_weak_ptr&)
        { shared_B = boost::shared_ptr<Y>(rhs); } // obtain ownership
        
        // polymorphic downcast to argument type (Y)
        // implicit cast to result type (T)
        m_ptr = boost::dynamic_pointer_cast<Y>(shared_B);
    }
    return *this;
}

template<class T> TemporaryPtr<T>& TemporaryPtr<T>::swap(boost::shared_ptr<T>& rhs) {
    /* There is no need to modify refcounts for swap(), but some implementations
     * may do so temporarily, so we better acquire locks. */
    Lock guard(m_ptr, rhs);
    
    m_ptr.swap(rhs);
    return *this;
}

template<class T> template <class P>
void TemporaryPtr<T>::interlocked_assign(const P& rhs) {
    Lock guard(m_ptr, rhs);
    
    m_ptr = rhs;
}

template<class T> template <class P, class F>
TemporaryPtr<T> TemporaryPtr<T>::interlocked_cast(const P& rhs, F convert) {
    Lock guard(rhs);
    TemporaryPtr<T> result;
    
    result.m_ptr = convert(rhs);
    return result;
}

template<class T> template <class Archive>
void TemporaryPtr<T>::serialize(Archive& ar, const unsigned int version) {
    /* Unfortunately, we cannot serialize m_ptr directly here. The object pointed 
     * to may already be constructed, in which case its refcount is increased 
     * during deserialization, but we do not know the pointer being deserialized
     * yet, so we cannot lock the refcount base. We also do not know anything
     * about temporary copies created during de/serialization that might affect
     * the refcount. Raw pointers should be safe here, the refcount will be 
     * reconstructed by operator = during deserialization.
     */
    T* raw_ptr;
    
    if (Archive::is_saving::value)
        raw_ptr = get();
    ar & boost::serialization::make_nvp("m_ptr", raw_ptr);
    if (Archive::is_loading::value)
        *this = raw_ptr;
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
TemporaryPtr<Y> static_pointer_cast(const TemporaryPtr<R>& rhs)
{ return TemporaryPtr<Y>::template interlocked_cast<shared_ptr<R>, shared_ptr<Y> (const shared_ptr<R>&)>(rhs.m_ptr, static_pointer_cast<Y>); }

template <class Y, class R>
TemporaryPtr<Y> dynamic_pointer_cast(const TemporaryPtr<R>& rhs) 
{ return TemporaryPtr<Y>::template interlocked_cast<shared_ptr<R>, shared_ptr<Y> (const shared_ptr<R>&)>(rhs.m_ptr, dynamic_pointer_cast<Y>); }

template <class Y, class R>
TemporaryPtr<Y> const_pointer_cast(const TemporaryPtr<R>& rhs)
{ return TemporaryPtr<Y>::template interlocked_cast<shared_ptr<R>, shared_ptr<Y> (const shared_ptr<R>&)>(rhs.m_ptr, const_pointer_cast<Y>); }

template <class Y, class R>
TemporaryPtr<Y> reinterpret_pointer_cast(const TemporaryPtr<R>& rhs)
{ return TemporaryPtr<Y>::template interlocked_cast<shared_ptr<R>, shared_ptr<Y> (const shared_ptr<R>&)>(rhs.m_ptr, reinterpret_pointer_cast<Y>); }

template <class T> T* get_pointer(const TemporaryPtr<T>& rhs)
{ return rhs.get(); }

} // namespace boost

#endif // _TemporaryPtr_tcc_
