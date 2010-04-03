// -*- C++ -*-
#ifndef _AIClientApp_h_
#define _AIClientApp_h_

#include "../ClientApp.h"

class AIBase;

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
   void                 operator()();   ///< external interface to Run()
   void                 Wait(int ms);   ///< put the main thread to sleep for \a ms milliseconds
   void                 Exit(int code); ///< does basic clean-up, then calls exit(); callable from anywhere in user code via GetApp()
   void                 SetPlayerName(const std::string& player_name) { m_player_name = player_name; }
   //@}

   /** \name Accessors */ //@{
   const std::string&   PlayerName() const { return m_player_name; }
   //@}

   static AIClientApp*  GetApp();       ///< returns a AIClientApp pointer to the singleton instance of the app
   const AIBase*        GetAI();        ///< returns pointer to AIBase implementation of AI for this client

private:
   void Run();              ///< initializes app state, then executes main event handler/render loop (PollAndRender())

   void HandleMessage(const Message& msg);

   AIBase*              m_AI;     ///< implementation of AI logic

   std::string          m_player_name;

   static AIClientApp*  s_app;
};

#endif // _AIClientApp_h_

