#include "Process.h"
#include <stdexcept>


Process::Process() : 
   m_empty(true)
{
}

Process::Process(const std::string& cmd, const std::vector<std::string>& argv) : 
   m_impl(new ProcessImpl(cmd, argv)), 
   m_empty(false)
{
}

void Process::Kill()
{
   m_impl.reset();
}


#if defined(WIN32_CHILD_PROCESSES)

Process::ProcessImpl::ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv)
{
   std::string args;
   for (unsigned int i = 0; i < argv.size(); ++i) {
      args += argv[i];
      if (i + 1 < argv.size())
         args += ' ';
   }

   ZeroMemory(&m_startup_info, sizeof(STARTUPINFO));
   m_startup_info.cb = sizeof(STARTUPINFO);
   ZeroMemory(&m_process_info, sizeof(PROCESS_INFORMATION));
   
   if (!CreateProcess(const_cast<char*>(cmd.c_str()), const_cast<char*>(args.c_str()), 0, 0, false, 
                      CREATE_NO_WINDOW, 0, 0, &m_startup_info, &m_process_info)) {
      std::string err_str;
      DWORD err = GetLastError();
      DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
      LPSTR buf;
      if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
         err_str += buf;
         LocalFree(buf);
      }
      throw std::runtime_error("Process::Process : Failed to create child process.  Windows error was: \"" + err_str + "\"");
   }
   WaitForInputIdle(m_process_info.hProcess, 1000); // wait for process to finish setting up, or for 1 sec, which ever comes first
}

Process::ProcessImpl::~ProcessImpl()
{
   if (!TerminateProcess(m_process_info.hProcess, 0)) {
      std::string err_str;
      DWORD err = GetLastError();
      DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM;
      LPSTR buf;
      if (FormatMessageA(flags, 0, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buf, 0, 0)) {
         err_str += buf;
         LocalFree(buf);
      }
   }
   CloseHandle(m_process_info.hProcess);
   CloseHandle(m_process_info.hThread);
}

#elif defined(LINUX_CHILD_PROCESSES)

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

Process::ProcessImpl::ProcessImpl(const std::string& cmd, const std::vector<std::string>& argv)
{
   std::vector<char*> args;
   for (unsigned int i = 0; i < argv.size(); ++i) {
      args.push_back(const_cast<char*>(&(const_cast<std::string&>(argv[i])[0])));
   }
   args.push_back(0);
   
   switch (m_process_id = fork()) {
   case -1: { // error
      throw std::runtime_error("Process::Process : Failed to fork a new process.");
      break;
   }

   case 0: { // child process side of fork
      execv(cmd.c_str(), &args[0]);
      break;
   }

   default: // original process side of fork (execution continues)
      break;
   }
}

Process::ProcessImpl::~ProcessImpl()
{
   kill(m_process_id, SIGHUP);
}

#endif

