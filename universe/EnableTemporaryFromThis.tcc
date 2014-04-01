// -*- C++ -*-
#ifndef _Enable_Temporary_From_This_tcc_
#define _Enable_Temporary_From_This_tcc_

#include "EnableTemporaryFromThis.h"
#include "TemporaryPtr.h"
#include <boost/thread/locks.hpp>

template <class T>
EnableTemporaryFromThis<T>::EnableTemporaryFromThis() :
    boost::enable_shared_from_this<T>(), m_ptr_mutex()
{}

template <class T> 
TemporaryPtr<T> EnableTemporaryFromThis<T>::TemporaryFromThis() {
    boost::unique_lock<boost::mutex> guard(m_ptr_mutex);
    return TemporaryPtr<T>(boost::enable_shared_from_this<T>::shared_from_this());
}

template <class T> 
TemporaryPtr<const T> EnableTemporaryFromThis<T>::TemporaryFromThis() const {
    boost::unique_lock<boost::mutex> guard(m_ptr_mutex);
    return TemporaryPtr<const T>(boost::enable_shared_from_this<T>::shared_from_this());
}

#endif // _Enable_Temporary_From_This_tcc_
