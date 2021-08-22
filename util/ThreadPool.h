#ifndef _ThreadPool_h_
#define _ThreadPool_h_

//! @file
//!     Includes boost asio thread_pool headers, or defines a dummy implementation.

#if BOOST_VERSION >= 106600
#  include <boost/asio/thread_pool.hpp>
#  include <boost/asio/post.hpp>
#else
namespace boost::asio {
    // dummy implementation of thread_pool and post that just immediately executes the passed-in function
    struct thread_pool {
        thread_pool(int) {}
        void join() const {}
    };
    inline void post(const thread_pool&, std::function<void()> func) { func(); }
}
#endif

#endif
