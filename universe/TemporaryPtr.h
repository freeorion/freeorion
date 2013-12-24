// -*- C++ -*-
#ifndef _TemporaryPtr_h_
#define _TemporaryPtr_h_

#include "EnableTemporaryFromThis.h"
#include <boost/mpl/assert.hpp>
#include <boost/serialization/access.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>

template <class T> class EnableTemporaryFromThis;
template <class T> class TemporaryPtr;

namespace boost {

template <class Y, class R> TemporaryPtr<Y> static_pointer_cast     (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> dynamic_pointer_cast    (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> const_pointer_cast      (const TemporaryPtr<R>& item);
template <class Y, class R> TemporaryPtr<Y> reinterpret_pointer_cast(const TemporaryPtr<R>& item);

}

template <class T>
class TemporaryPtr {
public:
    /** \name Structors */ //@{
    TemporaryPtr() : m_ptr()
    {}

    /* NOTE: pointer assignment can block the current thread until other 
     * manipulations of the refcounting base, like TemporaryFromThis(), have 
     * completed.
     */
    template<class Y>
    TemporaryPtr(const TemporaryPtr<Y>& rhs) :
        m_ptr()
    { internal_assign(rhs.m_ptr); }

    template<class Y>
    TemporaryPtr(EnableTemporaryFromThis<Y>* rhs) :
        m_ptr(rhs->TemporaryFromThis())
    { BOOST_MPL_ASSERT((boost::is_convertible<Y*, T*>)); }

    template<class Y>
    TemporaryPtr(const EnableTemporaryFromThis<Y>* rhs) :
        m_ptr(rhs->TemporaryFromThis())
    { BOOST_MPL_ASSERT((boost::is_convertible<const Y*, T*>)); }

    template<class Y>
    TemporaryPtr<T>& operator =(const TemporaryPtr<Y>& rhs) 
    { return internal_assign(rhs.m_ptr); }
    //@}

    /** \name Accessors */ //@{
    void reset() { internal_assign(boost::shared_ptr<T>()); };
    T*   get() const;

    operator bool() const { return get() != NULL; }
    T* operator ->() const { return get(); }
    T& operator *() const  { return *get(); }
    //@}

private:
    template <class Y>
    friend class TemporaryPtr;
    template <class Y>
    friend class EnableTemporaryFromThis;
    friend class ObjectMap; // FIXME: use TemporaryPtr in ObjectMap

    boost::shared_ptr<T> m_ptr;

    /** \name Structors */ //@{
    template<class Y>
    explicit TemporaryPtr(const boost::shared_ptr<Y>& rhs) :
        m_ptr(rhs)
    { BOOST_MPL_ASSERT((boost::is_convertible<Y*, T*>)); }

    template<class Y>
    explicit TemporaryPtr(const boost::weak_ptr<Y>& rhs) :
        m_ptr(rhs)
    { BOOST_MPL_ASSERT((boost::is_convertible<Y*, T*>)); }

    template<class P>
    TemporaryPtr<T>& internal_assign(const P& rhs);
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
    friend TemporaryPtr<Y> boost::static_pointer_cast(TemporaryPtr<R> const & item);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::dynamic_pointer_cast(TemporaryPtr<R> const & item);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::const_pointer_cast(TemporaryPtr<R> const & item);

    template <class Y, class R>
    friend TemporaryPtr<Y> boost::reinterpret_pointer_cast(TemporaryPtr<R> const & item);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    { ar & BOOST_SERIALIZATION_NVP(m_ptr); }
};

#include "TemporaryPtr.tcc"

#endif // _TemporaryPtr_h_
