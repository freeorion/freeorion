// -*- C++ -*-
#ifndef _TemporaryPtr_h_
#define _TemporaryPtr_h_

#include <boost/smart_ptr.hpp>
#include <boost/serialization/access.hpp>

template <class T> class EnableTemporaryFromThis;

template <class T>
class TemporaryPtr {
public:
    /** \name Structors */ //@{
    TemporaryPtr()
    {}

    template<class Y>
    TemporaryPtr(const TemporaryPtr<Y>& rhs) :
        m_ptr(rhs.m_ptr)
    {}
    
    template<class Y>
    TemporaryPtr& operator =(const TemporaryPtr<Y>& rhs) {
        if (rhs != *this) {
            ClearTemporaryPtr(m_ptr.get(), m_ptr.use_count());
            m_ptr = boost::const_pointer_cast<T>(boost::static_pointer_cast<const T>(rhs.m_ptr)); // TODO: Not sure why this jugglery is needed...
        }
        return *this;
    }

    ~TemporaryPtr() {
        ClearTemporaryPtr(m_ptr.get(), m_ptr.use_count());
    }
    //@}
    
    /** \name Accessors */ //@{
    operator bool() const
    { return (bool)m_ptr; }
    
    T* operator ->()
    { return m_ptr.get(); }
    
    T* operator ->() const
    { return m_ptr.get(); }
    //@}
    
private:
    template <class Y>
    friend class TemporaryPtr;
    friend class ObjectMap;

    boost::shared_ptr<T> m_ptr;
    
    /** \name Structors */ //@{
    template<class Y>
    explicit TemporaryPtr(const boost::shared_ptr<Y>& obj) :
        m_ptr(obj)
    {
        if (m_ptr)
            AssignTemporaryPtr(obj.get());
    }
    //@}


    template <class Y>
    void AssignTemporaryPtr(const EnableTemporaryFromThis<Y>* enabled)
    { enabled->m_ptr = *this; }

    void AssignTemporaryPtr(...)
    {}

    template <class Y>
    void ClearTemporaryPtr(const EnableTemporaryFromThis<Y>* enabled, long pointer_count) {
        // This is called whenever a shared_ptr is about to be destroyed, or assigned over.
        // If there remains only that one, and the one inside \a enabled, then we get rid of that one as well.
        if (pointer_count == 2)
            enabled->m_ptr = TemporaryPtr<Y>();
    }

    void ClearTemporaryPtr(...)
    {}

    template <class Y, class R>
    friend bool operator ==(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator ==(const Y* first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator ==(const TemporaryPtr<Y>& first, const R* second);

    template <class Y, class R>
    friend bool operator !=(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator !=(const Y* first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend bool operator !=(const TemporaryPtr<Y>& first, const R* second);

    template <class Y, class R>
    friend bool operator <(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second);

    template <class Y, class R>
    friend TemporaryPtr<Y> static_ptr_cast(const TemporaryPtr<R>& item);

    template <class Y, class R>
    friend TemporaryPtr<Y> dynamic_ptr_cast(const TemporaryPtr<R>& item);

    template <class Y, class R>
    friend TemporaryPtr<Y> const_ptr_cast(const TemporaryPtr<R>& item);

    /** If the payload of \a item is convertible to Y*, returns a TemporaryPtr<Y>
      * which shares ownership with \a item.  Otherwise, returns TemporaryPtr<Y>(). */
    //template <class Y, class R>
    //friend TemporaryPtr<Y> universe_object_ptr_cast(const TemporaryPtr<R>& item);

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    { ar & BOOST_SERIALIZATION_NVP(m_ptr); }
};

template <class Y, class R>
bool operator ==(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second)
{ return first.m_ptr == second.m_ptr; }

template <class Y, class R>
bool operator ==(const Y* first, const TemporaryPtr<R>& second)
{ return first == second.m_ptr.get(); }

template <class Y, class R>
bool operator ==(const TemporaryPtr<Y>& first, const R* second)
{ return first.m_ptr.get() == second; }

template <class Y, class R>
bool operator !=(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second)
{ return first.m_ptr != second.m_ptr; }

template <class Y, class R>
bool operator !=(const Y* first, const TemporaryPtr<R>& second)
{ return first != second.m_ptr.get(); }

template <class Y, class R>
bool operator !=(const TemporaryPtr<Y>& first, const R* second)
{ return first.m_ptr.get() != second; }

template <class Y, class R>
bool operator <(const TemporaryPtr<Y>& first, const TemporaryPtr<R>& second)
{ return first.m_ptr < second.m_ptr; }

template <class Y, class R>
TemporaryPtr<Y> static_ptr_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::static_pointer_cast<Y>(item.m_ptr)); }

template <class Y, class R>
TemporaryPtr<Y> dynamic_ptr_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::dynamic_pointer_cast<Y>(item.m_ptr)); }

template <class Y, class R>
TemporaryPtr<Y> const_ptr_cast(const TemporaryPtr<R>& item)
{ return TemporaryPtr<Y>(boost::const_pointer_cast<Y>(item.m_ptr)); }

#endif // _TemporaryPtr_h_
