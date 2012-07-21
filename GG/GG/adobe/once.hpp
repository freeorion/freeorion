/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_ONCE_HPP
#define ADOBE_ONCE_HPP

/*************************************************************************************************/

#include <GG/adobe/config.hpp>

#if defined(BOOST_HAS_THREADS)
    #include <boost/thread.hpp>
#endif

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

#if defined(BOOST_HAS_THREADS)

/*************************************************************************************************/

typedef boost::once_flag    once_flag;
#define ADOBE_ONCE_INIT     BOOST_ONCE_INIT

inline void call_once(void (*func)(), adobe::once_flag& flag)
{
    boost::call_once(func, flag);
}

/*************************************************************************************************/

#else

/*************************************************************************************************/

typedef bool                once_flag;
#define ADOBE_ONCE_INIT     false

inline void call_once(void (*func)(), adobe::once_flag& flag)
{
    if (!flag)
    {
        (*func)();
        flag = true;
    }
}

/*************************************************************************************************/

#endif

/*************************************************************************************************/

} // namespace adobe

/*************************************************************************************************/

#define ADOBE_ONCE_DECLARATION(signature)               \
    struct adobe_initialize_constants_##signature##_t   \
    {                                                   \
        adobe_initialize_constants_##signature##_t();   \
    };

#define ADOBE_ONCE_DEFINITION(signature, func)                                                  \
    namespace {                                                                                 \
    adobe::once_flag adobe_once_flag_##signature##_s = ADOBE_ONCE_INIT;                         \
    }                                                                                           \
    adobe_initialize_constants_##signature##_t::adobe_initialize_constants_##signature##_t()    \
    {                                                                                           \
        adobe::call_once(&func, adobe_once_flag_##signature##_s);                               \
    }

#define ADOBE_ONCE_INSTANCE(signature) \
    adobe_initialize_constants_##signature##_t adobe_initialize_constants_##signature##_s

#define ADOBE_ONCE_STATIC_INSTANCE(signature) \
    namespace { ADOBE_ONCE_INSTANCE(signature); }

#if defined(BOOST_HAS_THREADS)

#define ADOBE_GLOBAL_MUTEX_DEFINITION(signature)                            \
    namespace {                                                             \
    adobe::once_flag    adobe_once_flag_##signature##_s = ADOBE_ONCE_INIT;  \
    boost::mutex*       adobe_mutex_ptr_##signature##_s = 0;                \
    inline void adobe_init_once_##signature()                               \
    {                                                                       \
        static boost::mutex mutex_s;                                        \
        adobe_mutex_ptr_##signature##_s = &mutex_s;                         \
    }                                                                       \
    }

#define ADOBE_GLOBAL_MUTEX_INSTANCE(signature)                                          \
    boost::call_once(&adobe_init_once_##signature, adobe_once_flag_##signature##_s);    \
    boost::mutex::scoped_lock lock(*adobe_mutex_ptr_##signature##_s)

#else

#define ADOBE_GLOBAL_MUTEX_DEFINITION(signature)
#define ADOBE_GLOBAL_MUTEX_INSTANCE(signature)

#endif

/*************************************************************************************************/

#if defined(BOOST_HAS_THREADS)

#define ADOBE_THREAD_LOCAL_STORAGE_1(type, signature, ctor_p1) \
    namespace { \
    typedef boost::thread_specific_ptr< type > adobe_thread_local_storage_##signature##_t; \
    adobe_thread_local_storage_##signature##_t* adobe_thread_local_storage_##signature##_g = 0;\
    type& adobe_thread_local_storage_##signature##_access(); \
    type& adobe_thread_local_storage_##signature##_access() \
    { \
        type* result = adobe_thread_local_storage_##signature##_g->get(); \
        if (result) return *result; \
        result = new type(ctor_p1); \
        adobe_thread_local_storage_##signature##_g->reset(result); \
        return *result; \
    } }

#define ADOBE_THREAD_LOCAL_STORAGE(type, signature) \
    namespace { \
    typedef boost::thread_specific_ptr< type > adobe_thread_local_storage_##signature##_t; \
    adobe_thread_local_storage_##signature##_t* adobe_thread_local_storage_##signature##_g = 0;\
    type& adobe_thread_local_storage_##signature##_access(); \
    type& adobe_thread_local_storage_##signature##_access() \
    { \
        type* result = adobe_thread_local_storage_##signature##_g->get(); \
        if (result) return *result; \
        result = new type(); \
        adobe_thread_local_storage_##signature##_g->reset(result); \
        return *result; \
    } }

#define ADOBE_THREAD_LOCAL_STORAGE_INITIALIZE(signature) \
    static adobe_thread_local_storage_##signature##_t adobe_thread_local_storage_##signature##_s; \
    adobe_thread_local_storage_##signature##_g = &adobe_thread_local_storage_##signature##_s

#else

#define ADOBE_THREAD_LOCAL_STORAGE_1(type, signature, ctor_p1) \
    type& adobe_thread_local_storage_##signature##_access(); \
    type& adobe_thread_local_storage_##signature##_access() \
    { \
        static type adobe_thread_local_storage_##signature##_s(ctor_p1); \
        return adobe_thread_local_storage_##signature##_s; \
    }

#define ADOBE_THREAD_LOCAL_STORAGE(type, signature) \
    type& adobe_thread_local_storage_##signature##_access(); \
    type& adobe_thread_local_storage_##signature##_access() \
    { \
        static type adobe_thread_local_storage_##signature##_s; \
        return adobe_thread_local_storage_##signature##_s; \
    }

#define ADOBE_THREAD_LOCAL_STORAGE_INITIALIZE(signature)

#endif

#define ADOBE_THREAD_LOCAL_STORAGE_ACCESS(signature) \
    adobe_thread_local_storage_##signature##_access()

/*************************************************************************************************/

#endif // ADOBE_ONCE_HPP

/*************************************************************************************************/
