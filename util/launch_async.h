#ifndef _launch_async_h_
#define _launch_async_h_

#include "Logger.h"

#include <future>


// FIXME avoid unnecessary copies/moves


//! Execute a callable in an async thread.
template <typename Fn>
void launch_async(Fn fn)
{
    std::async(std::launch::async, [fn] () {
        try {
            fn();
        } catch (const std::exception& ex) {
            ErrorLogger() << "Exception in async thread: " << ex.what();
        }
    });
}

#endif // _launch_async_h_
