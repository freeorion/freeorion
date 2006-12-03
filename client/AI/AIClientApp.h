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
    virtual void StartTurn();

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

   virtual void HandleMessage(const Message& msg);

   log4cpp::Category&   m_log_category; ///< reference to the log4cpp object used to log events to file

   static AIClientApp*  s_app;
};

#endif // _AIClientApp_h_

