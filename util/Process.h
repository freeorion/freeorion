// -*- C++ -*-
#ifndef _Process_h_
#define _Process_h_

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>

// assume Linux environment by default
#if (!defined(FREEORION_WIN32) && !defined(FREEORION_LINUX))
#define FREEORION_LINUX
#endif

#ifdef FREEORION_WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/** encapsulates a spawned child process in a platform-independent manner. A Process object holds a shared_ptr to the 
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
   /** kills the controlled process immediately. */   
   void Kill();

   /** kills the controlled process iff it has not been freed. */
   void RequestTermination();

   /** frees the controlled process from auto-deletion when this Process object is destroyed. */   
   void Free();
   //@}
   
private:
   class ProcessImpl
   {
   public:
      ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv);
      ~ProcessImpl();

      void Kill();
      void Free();
      
   private:
       bool                m_free;
   #if defined(FREEORION_WIN32)
      STARTUPINFO          m_startup_info;
      PROCESS_INFORMATION  m_process_info;
   #elif defined(FREEORION_LINUX)
      pid_t                m_process_id;
   #endif
   };

   boost::shared_ptr<ProcessImpl>   m_impl;
   bool                             m_empty; ///< true iff this is a default-constructed Process (no associated process exists)
};

inline std::pair<std::string, std::string> ProcessRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _Process_h_

