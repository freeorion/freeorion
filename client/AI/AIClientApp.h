// -*- C++ -*-
#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#ifndef _ClientApp_h_
#include "../ClientApp.h"
#endif

namespace log4cpp {class Category;}

/** the application framework for an AI player FreeOrion client.*/
class AIClientApp : public ClientApp
{
public:
   /** \name Structors */ //@{   
   AIClientApp(int argc, char* argv[]);
   ~AIClientApp();
   //@}
   
   /** \name Mutators */ //@{   
   void                 operator()();     ///< external interface to Run()
   void                 Exit(int code);   ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
   log4cpp::Category&   Logger();         ///< returns the debug logging object for the app
   //@}
   
   static AIClientApp*  GetApp();         ///< returns a AIClientApp pointer to the singleton instance of the app
   
private:
   void Run();             ///< initializes app state, then executes main event handler/render loop (PollAndRender())

   void SDLInit();         ///< initializes SDL and SDL-related libs
   void Initialize();      ///< app initialization
   
   void Poll();            ///< handles all waiting SDL messages

   void FinalCleanup();    ///< app final cleanup
   void SDLQuit();         ///< cleans up FE and SDL

   virtual void HandleMessageImpl(const Message& msg);
   virtual void HandleServerDisconnectImpl();

   log4cpp::Category&   m_log_category; ///< reference to the log4cpp object used to log events to file

   static AIClientApp*  s_app;
};

inline std::pair<std::string, std::string> AIClientAppRevision()
{return std::pair<std::string, std::string>("$RCSfile$", "$Revision$");}

#endif // _AIClientApp_h_

