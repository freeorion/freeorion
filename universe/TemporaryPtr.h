// -*- C++ -*-
#ifndef _TemporaryPtr_h_
#define _TemporaryPtr_h_

#include <boost/serialization/access.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <ostream>

template <class T> class TemporaryPtr;

namespace boost {

template <class Y, class R> TemporaryPtr<Y> static_pointer_cast     (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> dynamic_pointer_cast    (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> const_pointer_cast      (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> reinterpret_pointer_cast(const TemporaryPtr<R>& item);

}

namespace detail
{ class TemporaryPtrLock; /**< Multi-pointer Locking helper*/ }

template <class T>
class TemporaryPtr {
public:
    /** \name Structors */ //@{
    TemporaryPtr() : m_ptr() {}

    /* NOTE: pointer assignment can block the current thread until other 
     * manipulations of the refcounting base, like TemporaryFromThis(), have 
     * completed.
     */
    template<class Y>          TemporaryPtr(const TemporaryPtr<Y>& rhs)      { *this = rhs; }
    template<class Y> explicit TemporaryPtr(const boost::shared_ptr<Y>& rhs) { *this = rhs; }
    template<class Y> explicit TemporaryPtr(const boost::weak_ptr<Y>& rhs)   { *this = rhs; }
    template<class Y> explicit TemporaryPtr(Y* rhs)                          { *this = rhs; }

    template<class Y> TemporaryPtr<T>& operator =(const TemporaryPtr<Y>& rhs)      { interlocked_assign(rhs.m_ptr); return *this; }
    template<class Y> TemporaryPtr<T>& operator =(const boost::shared_ptr<Y>& rhs) { interlocked_assign(rhs);       return *this; }
    template<class Y> TemporaryPtr<T>& operator =(const boost::weak_ptr<Y>& rhs)   { interlocked_assign(rhs);       return *this; }
    template<class Y> TemporaryPtr<T>& operator =(Y* rhs);

    // no refcounting access -> no locking necessary
    TemporaryPtr<T>& swap(TemporaryPtr     <T>& rhs) { swap(rhs.m_ptr); return *this; }
    TemporaryPtr<T>& swap(boost::shared_ptr<T>& rhs);
    
    // lock the internal pointer while releasing the reference
    ~TemporaryPtr() { reset(); }
   //@}

    /** \name Accessors */ //@{
    void reset();
    T*   get() const;

    operator bool() const { return get() != NULL; }
    T* operator ->() const { return get(); }
    T& operator *() const  { return *get(); }
    //@}

    typedef T element_type;
    template <class Y> struct rebind
    { typedef TemporaryPtr<Y> type; };
private:
    boost::shared_ptr<T> m_ptr;
    template <class Y> friend class TemporaryPtr;
    
    typedef detail::TemporaryPtrLock Lock;
    friend class detail::TemporaryPtrLock;

    /** \name Structors */ //@{

    operator int() const;   // disabled. Call ->ID() to get object's ID

    template<class P>
    void interlocked_assign(const P& rhs);
    
    template <class P, class F>
    static TemporaryPtr<T> interlocked_cast(const P& rhs, F convert);
    
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version);
    //@}

    template <class Y, class R>
    friend bool operator ==(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator ==(const TemporaryPtr<Y>& first, R* second);

    template <class Y, class R>
    friend bool operator <(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator <(const TemporaryPtr<Y>& first, R* second);

    template <class Y, class R>
    friend bool operator <(Y* first, const TemporaryPtr<R>&  second);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::static_pointer_cast(TemporaryPtr<R> const & rhs);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::dynamic_pointer_cast(TemporaryPtr<R> const & rhs);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::const_pointer_cast(TemporaryPtr<R> const & rhs);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::reinterpret_pointer_cast(TemporaryPtr<R> const & rhs);
};

template <class Y>
std::ostream& operator<<(std::ostream& o, const TemporaryPtr<Y>& p)
{ return (o << ""); }

#include "TemporaryPtr.tcc"

#endif // _TemporaryPtr_h_
