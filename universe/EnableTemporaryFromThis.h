// -*- C++ -*-
#ifndef _Enable_Temporary_From_This_h_
#define _Enable_Temporary_From_This_h_

#include <boost/smart_ptr/enable_shared_from_this.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>

template <class T> class TemporaryPtr;

template <class T>
class EnableTemporaryFromThis : public boost::enable_shared_from_this<T> {
public:
    typedef EnableTemporaryFromThis<T> enable_temporary_from_this_type;
		EnableTemporaryFromThis();
    TemporaryPtr<T> TemporaryFromThis();
    TemporaryPtr<const T> TemporaryFromThis() const;

private:
    // this is only necessary when we inherit privately from boost::enable_shared_from_this
    // currently, it is safe to publicly inherit, so we make MSVC happy and do so.
    // template <class Y>
    // friend class boost::shared_ptr; 
    template <class Y>
    friend class TemporaryPtr;
    mutable boost::mutex m_ptr_mutex;
};

#include "EnableTemporaryFromThis.tcc"

#endif // _Enable_Temporary_From_This_h_
