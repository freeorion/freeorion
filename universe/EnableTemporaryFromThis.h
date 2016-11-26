#ifndef _Enable_Temporary_From_This_h_
#define _Enable_Temporary_From_This_h_

#include <boost/smart_ptr/enable_shared_from_this.hpp>

template <typename T>
using EnableTemporaryFromThis = boost::enable_shared_from_this<T>;

#define TemporaryFromThis shared_from_this

#endif // _Enable_Temporary_From_This_h_
