#ifndef _Process_h_
#define _Process_h_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

// assume Linux environment by default
#if (!defined(WIN32_CHILD_PROCESSES) && !defined(LINUX_CHILD_PROCESSES))
#define LINUX_CHILD_PROCESSES
#endif

/** encapsulates a spawned child process in a platform-independent manner. A Process object holds a shared_ptr to the 
   data on the process it creates; therefore Process objects can be freely copied, with the same copy semantics as 
   a shared_ptr.  In addition, the created process is automatically killed when its owning Process object is 
   destroyed.  Currently, creating processes is supported on these operating systems:
   - Linux (this is the default): requires definition of LINUX_CHILD_PROCESSES
   - Win32 (use for MinGW apps as well): requires definition of WIN32_CHILD_PROCESSES
   one of them *must* be used.  Note that the Win32 version of Process calls TerminateProcess(), and so the killed
   process does minimal cleanup; in particular, it will not terminate any of its child processes and may not release
   DLLs it may be useing.*/
class Process
{
public:
   /** \name Structors */ //@{
   /** default ctor.  Creates a Process with no associated child process.  A child process will never be associated 
      with this default-constructed Process unless another Process is assigned to it.*/
   Process();
   
   /** ctor requiring a command and a full argv-style command line.  The command may be a relative or an absolute path 
      name. The first token on the command line must be the name of the executable of the process to be created.  Example: 
      cmd: "/usr/bin/cvs", argv: "cvs update -C project_file.cpp". Of course, each arg should be in its own string within 
      argv, and argv strings containing spaces must be enclosed in quotes.  \throw std::runtime_error Throws 
      std::runtime_error if the process cannot be successfully created.*/
   Process(const std::string& cmd, const std::vector<std::string>& argv);
   //@}

   /** \name Accessors */ //@{
   bool Empty() const {return m_empty;} ///< returns true if this is a default-constructed object with no associated process
   //@}

   /** \name Mutators */ //@{
   /** equivalent to "*this = Process();".  This essentially reduces the reference count of Process objects associated 
      with the underlying child process.  If this is the last reference to the child process, it is killed.*/   
   void Kill();
   //@}
   
private:
   class ProcessImpl
   {
   public:
      ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv);
      ~ProcessImpl();
      
   private:
   #if defined(WIN32_CHILD_PROCESSES)
      STARTUPINFO          m_startup_info;
      PROCESS_INFORMATION  m_process_info;
   #elif defined(LINUX_CHILD_PROCESSES)
      pid_t                m_process_id;
   #endif
   };

   boost::shared_ptr<ProcessImpl>   m_impl;
   bool                             m_empty; ///< true iff this is a default-constructed Process (no associated process exists)
};

#endif // _Process_h_

