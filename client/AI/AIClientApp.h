// -*- C++ -*-
#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#include "../ClientApp.h"
#include "../../AI/AIInterface.h"

namespace log4cpp {class Category;}

/** the application framework for an AI player FreeOrion client.*/
class AIClientApp : public ClientApp
{
public:
    virtual void StartTurn();       ///< encodes order sets and sends turn orders message

   /** \name Structors */ //@{   
   AIClientApp(int argc, char* argv[]);
   ~AIClientApp();
   //@}

   /** \name Mutators */ //@{   
   void                 operator()();     ///< external interface to Run()
   void                 Wait(int ms);     ///< put the main thread to sleep for \a ms milliseconds
   void                 Exit(int code);   ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
   //@}
   
   static AIClientApp*  GetApp();         ///< returns a AIClientApp pointer to the singleton instance of the app
   
private:
   void Run();              ///< initializes app state, then executes main event handler/render loop (PollAndRender())

   void HandleMessage(const Message& msg);

   AIBase*              m_AI;           ///< implementation of AI logic

   static AIClientApp*  s_app;
};

#endif // _AIClientApp_h_

