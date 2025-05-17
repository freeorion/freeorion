#include "Process.h"

#include "Logger.h"

#include <boost/algorithm/string/trim.hpp>
#if BOOST_VERSION >= 108800
# include <boost/asio.hpp>
# include <boost/process/v1/args.hpp>
# include <boost/process/v1/async.hpp>
# include <boost/process/v1/child.hpp>
#else
# include <boost/process.hpp>
#endif

#include <stdexcept>

// assume Linux environment by default
#if (!defined(FREEORION_WIN32) && !defined(FREEORION_LINUX) && !defined(FREEORION_MACOSX))
#define FREEORION_LINUX
#endif

#ifdef FREEORION_WIN32
#define WIN32_LEAN_AND_MEAN
// define needed on Windows due to conflict with windows.h and std::min and std::max
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#include <windows.h>

std::wstring ToWString(const std::string& utf8_string) {
    // convert UTF-8 string to UTF-16
    int utf16_sz = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                       utf8_string.data(), utf8_string.length(), NULL, 0);
    std::wstring utf16_string(utf16_sz, 0);
    if (utf16_sz > 0)
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                            utf8_string.data(), utf8_string.size(),
                            utf16_string.data(), utf16_sz);
    return utf16_string;
}

template<typename InputIt>
std::vector<std::wstring> ToWStringArray(InputIt first, InputIt last) {
    std::vector<std::wstring> result;
    std::transform(first, last, std::back_inserter(result), ToWString);
    return result;
}
#endif

#if BOOST_VERSION >= 108800
    namespace bpv1 = boost::process::v1;
#else
    namespace bpv1 = boost::process;
#endif


class Process::Impl {
public:
    Impl(boost::asio::io_context& io_context, const std::string& cmd, const std::vector<std::string>& argv);
    ~Impl();

    bool SetLowPriority(bool low);
    bool Terminate();
    void Kill();
    void Free();

private:
    bool        m_free = false;
#if defined(FREEORION_MACOSX)
    pid_t       m_process_id;
#elif defined(FREEORION_WIN32) || defined(FREEORION_LINUX)
    bpv1::child m_child;
#endif
};

Process::Process() :
    m_empty(true)
{}

Process::Process(boost::asio::io_context& io_context, const std::string& cmd, const std::vector<std::string>& argv) :
    m_impl(std::make_unique<Impl>(io_context, cmd, argv)),
    m_empty(false)
{}

Process::~Process() noexcept = default;
Process::Process(Process&&) noexcept = default;
Process& Process::operator=(Process&&) noexcept = default;

bool Process::SetLowPriority(bool low) {
    if (m_empty)
        return false;

    if (m_low_priority == low)
        return true;

    if (m_impl->SetLowPriority(low)) {
        m_low_priority = low;
        return true;
    }

    return false;
}

void Process::Kill() {
    // Early exit if already killed.
    if (!m_impl && m_empty && !m_low_priority)
        return;

    DebugLogger() << "Process::Kill";
    if (m_impl) {
        DebugLogger() << "Process::Kill calling m_impl->Kill()";
        m_impl->Kill();
    } else {
        DebugLogger() << "Process::Kill found no m_impl";
    }
    DebugLogger() << "Process::Kill calling RequestTermination()";
    RequestTermination();
}

bool Process::Terminate() {
    // Early exit if already killed.
    if (!m_impl && m_empty && !m_low_priority)
        return true;

    bool result = true;
    DebugLogger() << "Process::Terminate";
    if (m_impl) {
        DebugLogger() << "Process::Terminate calling m_impl->Terminate()";
        result = m_impl->Terminate();
    } else {
        DebugLogger() << "Process::Terminate found no m_impl";
    }
    DebugLogger() << "Process::Terminate calling RequestTermination()";
    RequestTermination();
    return result;
}

void Process::RequestTermination() {
    m_impl.reset();
    m_empty = true;
    m_low_priority = false;
}

void Process::Free() {
    if (m_impl)
        m_impl->Free();
}




#if defined(FREEORION_MACOSX)

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <signal.h>
#include <cstdio>
#include <sys/wait.h>


Process::Impl::Impl(boost::asio::io_context& io_context, const std::string& cmd, const std::vector<std::string>& argv) {
    std::vector<char*> args;
    for (unsigned int i = 0; i < argv.size(); ++i)
        args.push_back(const_cast<char*>(&(const_cast<std::string&>(argv[i])[0])));
    args.push_back(nullptr);

    switch (m_process_id = fork()) {
    case -1: { // error
        throw std::runtime_error("Process::Process : Failed to fork a new process.");
        break;
    }

    case 0: { // child process side of fork
        execv(cmd.c_str(), &args[0]);
        perror(("execv failed: " + cmd).c_str());
        break;
    }

    default:
        break;
    }
}

Process::Impl::~Impl()
{ if (!m_free) Kill(); }

bool Process::Impl::SetLowPriority(bool low) {
    if (low)
        return (setpriority(PRIO_PROCESS, m_process_id, 10) == 0);
    else
        return (setpriority(PRIO_PROCESS, m_process_id, 0) == 0);
}

bool Process::Impl::Terminate() {
    if (m_free) {
        DebugLogger() << "Process::Impl::Terminate called but m_free is true so returning with no action";
        return true;
    }
    int status = -1;
    DebugLogger() << "Process::Impl::Terminate calling kill(m_process_id, SIGINT)";
    kill(m_process_id, SIGINT);
    DebugLogger() << "Process::Impl::Terminate calling waitpid(m_process_id, &status, 0)";
    waitpid(m_process_id, &status, 0);
    DebugLogger() << "Process::Impl::Terminate done";
    if (status != 0) {
        WarnLogger() << "Process::Impl::Terminate got failure status " << status;
        return false;
    }
    return true;
}

void Process::Impl::Kill() {
    if (m_free) {
        DebugLogger() << "Process::Impl::Kill called but m_free is true so returning with no action";
        return;
    }
    int status;
    DebugLogger() << "Process::Impl::Kill calling kill(m_process_id, SIGKILL)";
    kill(m_process_id, SIGKILL);
    DebugLogger() << "Process::Impl::Kill calling waitpid(m_process_id, &status, 0)";
    waitpid(m_process_id, &status, 0);
    DebugLogger() << "Process::Impl::Kill done";
}

#elif defined(FREEORION_WIN32) || defined(FREEORION_LINUX)

Process::Impl::Impl(boost::asio::io_context& io_context, const std::string& cmd, const std::vector<std::string>& argv) :
#if defined(FREEORION_LINUX)
    m_child(cmd, bpv1::args = std::vector(argv.cbegin() + 1, argv.cend()), io_context)
#elif defined(FREEORION_WIN32)
    m_child(ToWString(cmd), bpv1::args = ToWStringArray(argv.cbegin() + 1, argv.cend()), io_context)
#endif
{
    std::error_code ec;
    if (!m_child.running(ec)) {
        std::string error_message = "Process::Process : Failed to run a new process: ";
        error_message += ec.message();
        ErrorLogger() << error_message;
        throw std::runtime_error(error_message);
    }
}

Process::Impl::~Impl()
{ if (!m_free) Kill(); }

#if defined(FREEORION_LINUX)
#include <sys/resource.h>

bool Process::Impl::SetLowPriority(bool low) {
    if (low)
        return (setpriority(PRIO_PROCESS, m_child.id(), 10) == 0);
    else
        return (setpriority(PRIO_PROCESS, m_child.id(), 0) == 0);
}
#elif defined(FREEORION_WIN32)
bool Process::Impl::SetLowPriority(bool low) {
    const DWORD priority = low ? BELOW_NORMAL_PRIORITY_CLASS : NORMAL_PRIORITY_CLASS;
    return SetPriorityClass(m_child.native_handle(), priority);
}
#endif

bool Process::Impl::Terminate() {
    if (m_free) {
        DebugLogger() << "Process::Impl::Terminate called but m_free is true so returning with no action";
        return true;
    }
    DebugLogger() << "Process::Impl::Terminate calling kill(m_process_id, SIGINT)";
    std::error_code term_ec;
    m_child.terminate(term_ec);
    DebugLogger() << "Process::Impl::Terminate calling waitpid(m_process_id, &status, 0)";
    std::error_code wait_ec;
    m_child.wait(wait_ec);
    int status = m_child.exit_code();
    DebugLogger() << "Process::Impl::Terminate done";
    if (status != 0) {
        WarnLogger() << "Process::Impl::Terminate got failure status " << status << ", termination error code " << term_ec.message() << ", waiting error code " << wait_ec.message();
        return false;
    }
    return true;
}

void Process::Impl::Kill() {
    if (m_free) {
        DebugLogger() << "Process::Impl::Kill called but m_free is true so returning with no action";
        return;
    }
    DebugLogger() << "Process::Impl::Kill calling kill(m_process_id, SIGKILL)";
    std::error_code term_ec;
    m_child.terminate(term_ec);
    DebugLogger() << "Process::Impl::Kill calling waitpid(m_process_id, &status, 0)";
    std::error_code wait_ec;
    m_child.wait(wait_ec);
    DebugLogger() << "Process::Impl::Kill done";
}

#endif

void Process::Impl::Free()
{ m_free = true; }
