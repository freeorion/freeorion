#ifndef _Pending_h_
#define _Pending_h_

#include "Export.h"
#include "Logger.h"

#include <boost/filesystem/path.hpp>
#include <boost/optional/optional.hpp>

#include <future>
#include <mutex>
#include <string>

/** namespace Pending collection classes and functions used for
    asynchronously parsing universe types, statistics etc.*/
namespace Pending {
    /** Pending holds the std::future results of a T.*/
    template <typename T>
    struct FO_COMMON_API Pending {
        using result_type = T;

        Pending(boost::optional<std::future<T>>&& pending_,
                const std::string& name_) :
            pending(std::move(pending_)),
            filename(name_)
        {}

        Pending(Pending&& other) noexcept :
            pending(std::move(other.pending)),
            filename(std::move(other.filename))
        {}

        Pending(const Pending& other) = delete;

        Pending& operator=(Pending&& other) noexcept {
            pending = std::move(other.pending);
            filename = std::move(other.filename);
            return *this;
        }

        Pending& operator=(const Pending& other) = delete;

        boost::optional<std::future<T>> pending = boost::none;
        std::string filename;
        std::mutex m_mutex;
    };

    template <typename T>
    [[nodiscard]] boost::optional<T> WaitForPendingUnlocked(Pending<T>&& pending, bool do_not_care_about_result = false) {
        std::future_status status = std::future_status::deferred;
        do {
            if (!pending.pending->valid())
                return boost::none;

            status = pending.pending->wait_for(std::chrono::seconds(1));
            if (status == std::future_status::timeout)
                DebugLogger() << "Waiting for parse of \"" << pending.filename << "\" to complete.";

            if (status == std::future_status::deferred) {
                DebugLogger() << "Pending parse awaiting deferred future.";
                pending.pending->wait();
            }
            DebugLogger() << "WaitForPendingUnlocked another wait_for round";
        } while (status != std::future_status::ready);

        try {
            // multiple threads might be waiting but not care about the results
            if (do_not_care_about_result) {
                if (pending.pending->valid()) {
                    DebugLogger() << "Don't care for result of parsing \"" << pending.filename << "\". Have to get() once to release shared state in pending future.";
                    pending.pending->get(); // needs to be called once to release state
                }
                DebugLogger() << "Don't care for result of parsing \"" << pending.filename << "\". Was already released.";
                return boost::none;
            }
            DebugLogger() << "Retrieve result of parsing \"" << pending.filename << "\".";
            return pending.pending->get();
        } catch (const std::exception& e) {
            ErrorLogger() << "Parsing of \"" << pending.filename << "\" failed with error: " << e.what();
        }

        return boost::none;
    }
    /** Wait for the \p pending parse to complete.  Set pending to boost::none
        and return the parsed T. Destroys the shared state in the wrapped std::future.
        Return boost::none on errors.*/
    template <typename T>
    boost::optional<T> WaitForPending(boost::optional<Pending<T>>& pending, bool do_not_care_about_result = false) {
        if (!pending)
            return boost::none;
        std::scoped_lock lock(pending->m_mutex);
        if (!pending || !(pending->pending)) {
            // another thread in the meantime transferred the pending to stored
            return boost::none;
        }
        if (auto tt = WaitForPendingUnlocked(std::move(*pending), do_not_care_about_result)) {
            pending = boost::none;
            return tt;
        } else {
            pending = boost::none;
            return boost::none;
        }
    }

    /** If there is a pending parse, wait for it and swap it with the stored
        value.  Return the stored value.*/
    template <typename T>
    T& SwapPending(boost::optional<Pending<T>>& pending, T& stored) {
        if (pending) {
            std::scoped_lock lock(pending->m_mutex);
            if (!pending)
                return stored; // another thread in the meantime transferred the pending to stored

            if (auto tt = WaitForPendingUnlocked(std::move(*pending)))
                std::swap(*tt, stored);
            pending = boost::none;
        }
        return stored;
    }

    /** Return a Pending<T> constructed with \p parser and \p path*/
    template <typename Func>
    [[nodiscard]] auto StartAsyncParsing(const Func& parser, const boost::filesystem::path& path)
        -> Pending<decltype(parser(path))>
    {
        return Pending<decltype(parser(path))>(
            std::async(std::launch::async, parser, path), path.filename().string());
    }

    /** Return a Pending<T> constructed with \p parser, \p arg1, and \p path*/
    template <typename Func, typename Arg1>
    [[nodiscard]] auto ParseSynchronously(const Func& parser, const Arg1& arg1, const boost::filesystem::path& path)
        -> Pending<decltype(parser(arg1, path, std::declval<bool&>()))>
    {
        bool success = true;
        auto result = parser(arg1, path, success);
        auto promise = std::promise<decltype(parser(arg1, path, std::declval<bool&>()))>();
        if (success)
            promise.set_value(std::move(result));
        else
            promise.set_exception(std::make_exception_ptr(std::runtime_error(path.string())));
        return Pending<decltype(parser(arg1, path, std::declval<bool&>()))>(promise.get_future(), path.filename().string());
    }

    /** Return a Pending<T> constructed with \p parser, \p arg1, and \p path*/
    template <typename Func, typename Arg1>
    [[nodiscard]] auto ParseSynchronously(const Func& parser, const Arg1& arg1, const boost::filesystem::path& path,
                            std::promise<void>&& barrier)
        -> Pending<decltype(parser(arg1, path, std::declval<bool&>()))>
    {
        bool success = true;
        auto result = parser(arg1, path, success);
        auto promise = std::promise<decltype(parser(arg1, path, std::declval<bool&>()))>();
        if (success) {
            promise.set_value(std::move(result));
            barrier.set_value();
        } else {
            promise.set_exception(std::make_exception_ptr(std::runtime_error(path.string())));
            barrier.set_exception(std::make_exception_ptr(std::runtime_error(path.string())));
        }
        return Pending<decltype(parser(arg1, path, std::declval<bool&>()))>(promise.get_future(), path.filename().string());
    }

    /** Helper struct for use with std::async. operator() evaluates \a _parser
      * on \a path and then flags \a barrier to indicate that the \a _parser
      * call is finished. */
    template <typename Func>
    struct Parsing {
        Func parser;
        std::promise<void> barrier;

        Parsing(Func _parser, std::promise<void>&& _barrier) :
            parser(_parser),
            barrier(std::move(_barrier))
        { }

        auto operator()(const boost::filesystem::path& path)
            -> decltype(parser(path))
        {
            auto ret = parser(path);
            barrier.set_value();
            return ret;
        }
    };

    /** Return a Pending<T> constructed with \p parser and \p path
     * and notify \p barrier*/
    template <typename Func>
    [[nodiscard]] auto StartAsyncParsing(Func parser, const boost::filesystem::path& path, std::promise<void>&& barrier)
        -> Pending<decltype(parser(path))>
    {
        return Pending<decltype(parser(path))>(
            std::async(std::launch::async,
                       Parsing<Func>(parser, std::move(barrier)),
                       path),
            path.filename().string());
    }

    /** Return a Pending<T> constructed with \p parser and \p path which
      * executes the parser in the calling thread and stores the result
      * before returning. */
    template <typename Func>
    [[nodiscard]] auto ParseSynchronously(const Func& parser, const boost::filesystem::path& path)
        -> Pending<decltype(parser(path))>
    {
        auto retval = std::async(std::launch::deferred, parser, path);
        retval.wait();
        return Pending<decltype(parser(path))>(std::move(retval), path.filename().string());
    }

    /** Return a Pending<T> constructed with \p parser and \p path and notify \p barrier which
      * executes the parser in the calling thread and stores the result
      * before returning. */
    template <typename Func>
    [[nodiscard]] auto ParseSynchronously(const Func& parser, const boost::filesystem::path& path, std::promise<void>&& barrier)
        -> Pending<decltype(parser(path))>
    {
        auto retval = std::async(std::launch::deferred, parser, path);
        retval.wait();
        barrier.set_value();
        return Pending<decltype(parser(path))>(std::move(retval), path.filename().string());
    }

}


#endif
