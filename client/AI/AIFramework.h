#ifndef _AIFramework_h_
#define _AIFramework_h_

#include "../../python/CommonFramework.h"
#include "AIInterface.h"

#include <string>

class PythonAI : public PythonBase, public AIBase {
public:
    bool Initialize();

    /** Initializes AI Python modules. */
    bool InitModules() override;
    void GenerateOrders() override;
    void HandleChatMessage(int sender_id, const std::string& msg) override;
    void HandleDiplomaticMessage(const DiplomaticMessage& msg) override;
    void HandleDiplomaticStatusUpdate(const DiplomaticStatusUpdateInfo& u) override;
    void StartNewGame() override;
    void ResumeLoadedGame(const std::string& save_state_string) override;

    const std::string&  GetSaveStateString() const override;

private:
    // reference to imported Python AI module
    boost::python::object m_python_module_ai;
};

#endif // _AIFramework_h_
