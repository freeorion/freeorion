// -*- C++ -*-
#ifndef _Enable_Temporary_From_This_tcc_
#define _Enable_Temporary_From_This_tcc_

#include "EnableTemporaryFromThis.h"
#include "TemporaryPtr.h"
#include <boost/thread/locks.hpp>

template <class T> 
TemporaryPtr<T> EnableTemporaryFromThis<T>::TemporaryFromThis() 
{ return TemporaryPtr<T>(dynamic_cast<T*>(this)); }

template <class T> 
TemporaryPtr<const T> EnableTemporaryFromThis<T>::TemporaryFromThis() const 
{ return TemporaryPtr<const T>(dynamic_cast<const T*>(this)); }

#endif // _Enable_Temporary_From_This_tcc_
