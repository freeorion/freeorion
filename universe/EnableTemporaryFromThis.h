// -*- C++ -*-
#ifndef _Enable_Temporary_From_This_h_
#define _Enable_Temporary_From_This_h_

#include <boost/serialization/access.hpp>
#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>

class EnableTemporary;
template <class T> class TemporaryPtr;
template <class T> class EnableTemporaryFromThis;

namespace detail
{ class TemporaryPtrLock; }

/* WARNING:only inherit this virtually.
 * It may fail horribly if the base exists
 * multiple times within the same object.
 */
class EnableTemporary 
    : public boost::enable_shared_from_this<EnableTemporary>
{
public:
    EnableTemporary() {};
    virtual ~EnableTemporary() {};
private:
    // retrieving unprotected shared pointers only for friends
    template <class T> friend class boost::enable_shared_from_this;
    template <class T> friend class boost::shared_ptr;
    template <class T> friend class TemporaryPtr;
    boost::enable_shared_from_this<EnableTemporary>::shared_from_this;
    
    friend class detail::TemporaryPtrLock;
    mutable boost::mutex m_ptr_mutex;
    
    friend class boost::serialization::access;
    template <class Archive> 
    void serialize(Archive& ar, const unsigned int version) {}
};

template <class T>
class EnableTemporaryFromThis
    : virtual public EnableTemporary
{
public:
    EnableTemporaryFromThis() {};
    TemporaryPtr<T>       TemporaryFromThis();
    TemporaryPtr<const T> TemporaryFromThis() const;
};

#include "EnableTemporaryFromThis.tcc"

#endif // _Enable_Temporary_From_This_h_
