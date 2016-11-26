#ifndef _TemporaryPtr_h_
#define _TemporaryPtr_h_

#include <boost/smart_ptr/shared_ptr.hpp>

template <typename T>
using TemporaryPtr = boost::shared_ptr<T>;


#endif // _TemporaryPtr_h_
