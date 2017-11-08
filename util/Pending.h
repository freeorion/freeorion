#ifndef _Pending_h_
#define _Pending_h_

#include "Export.h"
#include "Logger.h"

#include <boost/filesystem/path.hpp>
#include <boost/optional/optional.hpp>

#include <future>
#include <string>

/** namespace Pending collection classes and functions used for
    asynchronously parsing universe types, statistics etc.*/
namespace Pending {
    /** Pending holds the std::future results of a T.*/
    template <typename T>
    struct FO_COMMON_API Pending {
        Pending(
            boost::optional<std::future<T>>&& pending_,
            const std::string& name_
        ) :
            pending(std::move(pending_)),
            filename(name_)
        {}

        Pending(Pending&& other) :
            pending(std::move(other.pending)),
            filename(std::move(other.filename))
        {}

        Pending& operator=(Pending&& other) {
            pending = std::move(other.pending);
            filename = std::move(other.filename);
            return *this;
        }

        boost::optional<std::future<T>> pending = boost::none;
        std::string filename;
    };

    /** Wait for the \p pending parse to complete.  Set pending to boost::none
        and return the parsed T.  Return boost::none on errors.*/
    template <typename T>
    boost::optional<T> WaitForPending(boost::optional<Pending<T>>& pending) {
        if (!pending)
            return boost::none;

        std::future_status status;
        do {
            status = pending->pending->wait_for(std::chrono::seconds(1));
            if (status == std::future_status::timeout)
                DebugLogger() << "Waiting for parse of \"" << pending->filename << "\" to complete.";

            if (status == std::future_status::deferred) {
                ErrorLogger() << "Pending parse is unable to handle deferred future.";
                throw "deferred future not handled";
            }

        } while (status != std::future_status::ready);

        try {
            auto x = std::move(pending->pending->get());
            pending = boost::none;
            return std::move(x);
        } catch (const std::exception& e) {
            ErrorLogger() << "Parsing of \"" << pending->filename << "\" failed with error: " << e.what();
            pending = boost::none;
        }

        return boost::none;
    }

    /** If there is a pending parse, wait for it and swap it with the stored
        value.  Return the stored value.*/
    template <typename T>
    T& SwapPending(boost::optional<Pending<T>>& pending, T& stored) {
        if (auto tt = WaitForPending(pending))
            std::swap(*tt, stored);
        return stored;
    }

    /** If there is a pending parse, wait for it and swap it with the stored
        value.  Return the stored value.

        TODO: remove this function once all of the raw pointer containers are removed.
    */
    template <typename T>
    T& SwapPendingRawPointers(boost::optional<Pending<T>>& pending, T& stored) {
        if (auto parsed = WaitForPending(pending)) {
            std::swap(*parsed, stored);

            // Don't leak old types
            for (auto& entry : *parsed)
                delete entry.second;
        }
        return stored;
    }

    /** Return a Pending<T> constructed with \p parser and \p path*/
    template <typename Func>
    auto StartParsing(const Func& parser, const boost::filesystem::path& path)
        -> Pending<decltype(parser(path))>
    {
        return Pending<decltype(parser(path))>(
            std::async(std::launch::async, parser, path), path.filename().string());
    }
}

#endif // _Pending_h_
