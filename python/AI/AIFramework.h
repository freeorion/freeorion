#ifndef __FreeOrion__Python__AIFramework__
#define __FreeOrion__Python__AIFramework__

#include "../CommonFramework.h"
#include "../../AI/AIInterface.h"

#include <string>

class PythonAI : public PythonBase, public AIBase {
public:
    bool Initialize();
    bool InitModules(); // Initializes AI Python modules

    virtual void                GenerateOrders();
    virtual void                HandleChatMessage(int sender_id, const std::string& msg);
    virtual void                HandleDiplomaticMessage(const DiplomaticMessage& msg);
    virtual void                HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u);
    virtual void                StartNewGame();
    virtual void                ResumeLoadedGame(const std::string& save_state_string);
    virtual const std::string&  GetSaveStateString();

private:
    // reference to imported Python AI module
    boost::python::object m_python_module_ai;
};

#endif // __FreeOrion__Python__AIFramework__
