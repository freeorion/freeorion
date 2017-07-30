#ifndef __FreeOrion__Python__ServerFramework__
#define __FreeOrion__Python__ServerFramework__

#include "../CommonFramework.h"
#include "../../util/MultiplayerCommon.h"

#include <map>


class PythonServer : public PythonBase {
public:
    /** Initializes server Python modules. */
    bool InitModules() override;

    bool CreateUniverse(std::map<int, PlayerSetupData>& player_setup_data); // Wraps call to the main Python universe generator function
    bool ExecuteTurnEvents();    // Wraps call to the main Python turn events function

private:
    // reference to imported Python universe generator module
    boost::python::object m_python_module_universe_generator;

    // reference to imported Python turn events module
    boost::python::object m_python_module_turn_events;

    // reference to imported Python auth module
    boost::python::object m_python_module_auth;
};

// Returns folder containing the Python universe generator scripts
const std::string GetPythonUniverseGeneratorDir();

// Returns folder containing the Python turn events scripts
const std::string GetPythonTurnEventsDir();

// Returns folder containing the Python auth scripts
const std::string GetPythonAuthDir();

#endif /* defined(__FreeOrion__Python__ServerFramework__) */
