#ifndef _ServerFramework_h_
#define _ServerFramework_h_

#include "../python/PythonBase.h"
#include "../util/MultiplayerCommon.h"

#include <boost/circular_buffer.hpp>

#include <map>


class PythonServer : public PythonBase {
public:
    /** Initializes server Python imports. */
    bool InitImports() override;
    /** Initializes server Python modules. */
    bool InitModules() override;

    bool CreateUniverse(std::map<int, PlayerSetupData>& player_setup_data); // Wraps call to the main Python universe generator function
    bool ExecuteTurnEvents();    // Wraps call to the main Python turn events function
    bool IsRequireAuthOrReturnRoles(const std::string& player_name, const std::string& ip_address, bool &result, Networking::AuthRoles& roles) const; // Wraps call to AuthProvider's method is_require_auth
    bool IsSuccessAuthAndReturnRoles(const std::string& player_name, const std::string& auth, bool &result, Networking::AuthRoles& roles) const; // Wraps call to AuthProvider's method is_success_auth
    bool FillListPlayers(std::vector<PlayerSetupData>& players) const; // Wraps call to AuthProvider's method list_player
    bool GetPlayerDelegation(const std::string& player_name, std::vector<std::string>& result) const; // Wraps call to AuthProvider's method get_player_delegation
    bool LoadChatHistory(boost::circular_buffer<ChatHistoryEntity>& chat_history); // Wraps call to ChatProvider's method load_history
    bool PutChatHistoryEntity(const ChatHistoryEntity& chat_history_entity); // Wraps call to ChatProvider's method put_history_entity
    bool AsyncIOTick(); // Executes awaiting asyncio callbacks, see https://docs.python.org/3/library/asyncio-eventloop.html#asyncio.loop.run_forever

private:
    // reference to imported Python universe generator module
    boost::python::object m_python_module_universe_generator;

    // reference to imported Python turn events module
    boost::python::object m_python_module_turn_events;

    // reference to imported Python auth module
    boost::python::object m_python_module_auth;

    // reference to importer Python chat module
    boost::python::object m_python_module_chat;

    // reference to asyncio event loop
    boost::python::object m_asyncio_event_loop;
};

// Returns folder containing the Python universe generator scripts
boost::filesystem::path GetPythonUniverseGeneratorDir();

// Returns folder containing the Python turn events scripts
boost::filesystem::path GetPythonTurnEventsDir();

// Returns folder containing the Python auth scripts
boost::filesystem::path GetPythonAuthDir();

// Returns folder containing the Python chat scripts
boost::filesystem::path GetPythonChatDir();


#endif
