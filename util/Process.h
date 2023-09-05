#ifndef _Process_h_
#define _Process_h_


#include <memory>
#include <string>
#include <vector>

#include "Export.h"


/** Encapsulates a spawned child process in a platform-independent manner. A Process object holds a shared_ptr to the
   data on the process it creates; therefore Process objects can be freely copied, with the same copy semantics as
   a shared_ptr.  In addition, the created process is automatically killed when its owning Process object is
   destroyed, unless it is explicitly Free()d.  Note that whether or not the process is explicitly Free()d, it may be
   explicitly Kill()ed at any time.
   <br>
   Currently, creating processes is supported on these operating systems:
   - Linux (this is the default): requires definition of FREEORION_LINUX
   - Win32 (use for MinGW apps as well): requires definition of FREEORION_WIN32
   one of them *must* be used.  Note that the Win32 version of Process calls TerminateProcess(), and so the killed
   process does minimal cleanup; in particular, it will not terminate any of its child processes and may not release
   DLLs it may be using.*/
class FO_COMMON_API Process {
public:
    /** Creates a Process with no associated child process.  A child process
        will never be associated with this default-constructed Process unless
        another Process is assigned to it. */
    Process();

    /** Ctor requiring a command and a full argv-style command line.  The
        command may be a relative or an absolute path name.  The first token on
        the command line must be the name of the executable of the process to be
        created.

        Example:
        cmd: "/usr/bin/cvs", argv: "cvs update -C project_file.cpp".

        Of course, each arg should be in its own string within argv, and argv
        strings containing spaces must be enclosed in quotes.

        @throw std::runtime_error  If the process cannot be successfully
            created.

        Process returns immediately. */
    Process(const std::string& cmd, const std::vector<std::string>& argv);

    ~Process() noexcept;
    Process(Process&&) noexcept;
    Process(const Process&) = delete;
    Process& operator=(Process&&) noexcept;
    Process& operator=(const Process&) = delete;

    bool Empty() const noexcept { return m_empty; }           ///< true if this is a default-constructed object with no associated process
    bool HasLowPriority() noexcept { return m_low_priority; } ///< true if process is set to low priority

    /** sets process priority */
    bool SetLowPriority(bool low);

    /** kills the controlled process immediately. */
    void Kill();

    /** terminate the controlled process and wait for finish.
     *  Return true if exit status was 0. */
    bool Terminate();

    /** kills the controlled process iff it has not been freed. */
    void RequestTermination();

    /** frees the controlled process from auto-deletion when this Process object is destroyed. */
    void Free();

private:
    class Impl;

    std::unique_ptr<Impl>   m_impl;
    bool                    m_empty = false;        ///< true iff this is a default-constructed Process (no associated process exists)
    bool                    m_low_priority = false; ///< true if this process is set to low priority
};


#endif
