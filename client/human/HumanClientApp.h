#ifndef _HumanClientApp_h_
#define _HumanClientApp_h_

#ifndef _SDLGGApp_h_
#include "SDLGGApp.h"
#endif

#ifndef _ClientApp_h_
#include "../ClientApp.h"
#endif

#ifndef _Process_h_
#include "../../util/Process.h"
#endif

/** the application framework class for the human player FreeOrion client.  */
class HumanClientApp : public ClientApp, public SDLGGApp
{
public:
   /** \name Structors */ //@{
   HumanClientApp(const GG::XMLElement& elem);
   virtual ~HumanClientApp();
   //@}

   /** \name Mutators */ //@{
   virtual void Enter2DMode();
   virtual void Exit2DMode();
   //@}

   static HumanClientApp* GetApp(); ///< returns HumanClientApp pointer to the single instance of the app
   
protected:
   /** \name Mutators */ //@{
   virtual void SDLInit();
   virtual void GLInit();
   virtual void Initialize();

   virtual void HandleSDLEvent(const SDL_Event& event);

   virtual void Update();
   virtual void Render();

   virtual void FinalCleanup();
   //@}

private:
   virtual void HandleMessageImpl(const Message& msg);
   
   Process  m_server_process; ///< the server process (when hosting a game or playing single player); will be empty when playing multiplayer as a non-host player
};

#endif // _HumanClientApp_h_

