// -*- C++ -*-
#ifndef _TemporaryPtr_h_
#define _TemporaryPtr_h_

#include <boost/smart_ptr.hpp>
#include <boost/serialization/access.hpp>

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
        m_ptr = rhs.m_ptr;
        return *this;
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
    {}
    //@}

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
    { ar & m_ptr; }
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
