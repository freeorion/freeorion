#include "ServerFramework.h"

#include "ServerWrapper.h"
#include "../python/SetWrapper.h"
#include "../python/CommonWrappers.h"

#include "../util/Logger.h"
#include "../util/Directories.h"
#include "../universe/Condition.h"
#include "../universe/Universe.h"

#include <boost/filesystem.hpp>
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/python/suite/indexing/map_indexing_suite.hpp>
#include <boost/python/docstring_options.hpp>
#include <boost/python/stl_iterator.hpp>

#include <stdexcept>

namespace fs = boost::filesystem;
namespace py = boost::python;


BOOST_PYTHON_MODULE(freeorion) {
    py::docstring_options doc_options(true, true, false);

    FreeOrionPython::WrapGameStateEnums();
    FreeOrionPython::WrapGalaxySetupData();
    FreeOrionPython::WrapEmpire();
    FreeOrionPython::WrapUniverseClasses();
    FreeOrionPython::WrapServer();
    FreeOrionPython::WrapConfig();

    // STL Containers
    py::class_<std::vector<int>>("IntVec")
        .def(py::vector_indexing_suite<std::vector<int>>())
    ;
    py::class_<std::vector<std::string>>("StringVec")
        .def(py::vector_indexing_suite<std::vector<std::string>>())
    ;

    py::class_<std::map<int, bool>>("IntBoolMap")
        .def(py::map_indexing_suite<std::map<int, bool>>())
    ;

    FreeOrionPython::SetWrapper<std::set<int>>::Wrap("IntSet");
    FreeOrionPython::SetWrapper<std::set<std::string>>::Wrap("StringSet");
}


auto PythonServer::InitImports() -> bool
{
    DebugLogger() << "Initializing server Python imports";
    // Allow the "freeorion" C++ module to be imported within Python code
    return PyImport_AppendInittab("freeorion", &PyInit_freeorion) != -1;
}

auto PythonServer::InitModules() -> bool
{
    DebugLogger() << "Initializing server Python modules";

    // Confirm existence of the directory containing the universe generation
    // Python scripts and add it to Pythons sys.path to make sure Python will
    // find our scripts
    auto python_universe_generator_dir = GetPythonUniverseGeneratorDir();
    if (!fs::exists(python_universe_generator_dir)) {
        ErrorLogger() << "Can't find folder containing universe generation scripts: " << PathToString(python_universe_generator_dir);
        return false;
    }
    AddToSysPath(python_universe_generator_dir);

    // Confirm existence of the directory containing the turn event Python
    // scripts and add it to Pythons sys.path to make sure Python will find
    // our scripts
    auto python_turn_events_dir = GetPythonTurnEventsDir();
    if (!fs::exists(python_turn_events_dir)) {
        ErrorLogger() << "Can't find folder containing turn events scripts:" << PathToString(python_turn_events_dir);
        return false;
    }
    AddToSysPath(python_turn_events_dir);

    // import universe generator script file
    m_python_module_turn_events = py::import("turn_events");

    // Confirm existence of the directory containing the auth Python scripts
    // and add it to Pythons sys.path to make sure Python will find our scripts
    auto python_auth_dir = GetPythonAuthDir();
    if (!fs::exists(python_auth_dir)) {
        ErrorLogger() << "Can't find folder containing auth scripts:" << PathToString(python_auth_dir);
        return false;
    }
    AddToSysPath(python_auth_dir);

    // import auth script file
    m_python_module_auth = py::import("auth");

    // Save AuthProvider instance in auth module's namespace
    m_python_module_auth.attr("__dict__")["auth_provider"] = m_python_module_auth.attr("AuthProvider")();

    AddToSysPath(GetPythonChatDir());

    // import chat script file
    m_python_module_chat = py::import("chat");

    // Save ChatProvider instance in chat module's namespace
    m_python_module_chat.attr("__dict__")["chat_history_provider"] = m_python_module_chat.attr("ChatHistoryProvider")();

    // Save asyncio event loop
    if (GetOptionsDB().Get<int>("network.server.python.asyncio-interval") > 0) {
        py::object asyncio = py::import("asyncio");

        m_asyncio_event_loop = asyncio.attr("new_event_loop")();
	asyncio.attr("set_event_loop")(m_asyncio_event_loop);
    }

    DebugLogger() << "Server Python modules successfully initialized!";
    return true;
}

auto PythonServer::IsRequireAuthOrReturnRoles(const std::string& player_name, const std::string& ip_address,
                                              bool &result, Networking::AuthRoles& roles) const -> bool
{
    py::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    py::object f = auth_provider.attr("is_require_auth_or_return_roles");
    if (!f) {
        ErrorLogger() << "Unable to call Python method is_require_auth";
        return false;
    }
    roles.Clear();
    py::object r = f(player_name, ip_address);
    py::extract<py::list> py_roles(r);
    if (py_roles.check()) {
        result = false;
        py::stl_input_iterator<Networking::RoleType> role_begin(py_roles), role_end;
        for (auto& it = role_begin; it != role_end; ++it)
            roles.SetRole(*it, true);
    } else {
        result = py::extract<bool>(r)();
    }
    return true;
}

auto PythonServer::IsSuccessAuthAndReturnRoles(const std::string& player_name, const std::string& auth,
                                               bool &result, Networking::AuthRoles& roles) const -> bool
{
    py::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    py::object f = auth_provider.attr("is_success_auth_and_return_roles");
    if (!f) {
        ErrorLogger() << "Unable to call Python method is_success_auth";
        return false;
    }
    py::object r = f(player_name, auth);
    py::extract<py::list> py_roles(r);
    if (py_roles.check()) {
        result = true;

        py::stl_input_iterator<Networking::RoleType> role_begin(py_roles), role_end;
        for (auto& it = role_begin; it != role_end; ++it)
             roles.SetRole(*it, true);
    } else {
        result = false;
        DebugLogger() << "Wrong auth data for \"" << player_name << "\": check returns " << py::extract<std::string>(py::str(r))();
    }
    return true;
}

auto PythonServer::FillListPlayers(std::vector<PlayerSetupData>& players) const -> bool
{
    const py::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    const py::object f = auth_provider.attr("list_players");
    if (!f) {
        ErrorLogger() << "Unable to call Python method list_players";
        return false;
    }
    const py::object r = f();
    const py::extract<py::list> py_players(r);
    if (py_players.check()) {
        py::stl_input_iterator<PlayerSetupData> players_begin(py_players), players_end;
        players.reserve(py::len(py_players));
        players.insert(players.end(), players_begin, players_end);
    } else {
        DebugLogger() << "Wrong players list data: check returns "
                      << py::extract<std::string>(py::str(r))();
        return false;
    }
    return true;
}

auto PythonServer::GetPlayerDelegation(const std::string& player_name,
                                       std::vector<std::string>& result) const -> bool
{
    py::object auth_provider = m_python_module_auth.attr("__dict__")["auth_provider"];
    if (!auth_provider) {
        ErrorLogger() << "Unable to get Python object auth_provider";
        return false;
    }
    py::object f = auth_provider.attr("get_player_delegation");
    if (!f) {
        ErrorLogger() << "Unable to call Python method get_player_delegation";
        return false;
    }
    py::object r = f(player_name);
    py::extract<py::list> py_players(r);
    if (py_players.check()) {
        py::stl_input_iterator<std::string> players_begin(py_players), players_end;
        result.insert(result.end(), players_begin, players_end);
    } else {
        DebugLogger() << "Wrong delegated players list data: check returns "
                      << py::extract<std::string>(py::str(r))();
        return false;
    }

    return true;
}

auto PythonServer::LoadChatHistory(boost::circular_buffer<ChatHistoryEntity>& chat_history) -> bool
{
    py::object chat_provider = m_python_module_chat.attr("__dict__")["chat_history_provider"];
    if (!chat_provider) {
        ErrorLogger() << "Unable to get Python object chat_history_provider";
        return false;
    }
    py::object f = chat_provider.attr("load_history");
    if (!f) {
        ErrorLogger() << "Unable to call Python method load_history";
        return false;
    }
    py::object r = f();
    py::extract<py::list> py_history(r);
    if (py_history.check()) {
        py::stl_input_iterator<py::tuple> entity_begin(py_history), entity_end;
        for (auto& it = entity_begin; it != entity_end; ++it) {
            ChatHistoryEntity e;
            e.timestamp = boost::posix_time::from_time_t(py::extract<time_t>((*it)[0]));
            e.player_name = py::extract<std::string>((*it)[1]);
            e.text = py::extract<std::string>((*it)[2]);
            py::tuple color = py::extract<py::tuple>((*it)[3]);
            e.text_color = std::array<uint8_t, 4>{{
                py::extract<uint8_t>(color[0]),
                py::extract<uint8_t>(color[1]),
                py::extract<uint8_t>(color[2]),
                py::extract<uint8_t>(color[3])
            }};
            chat_history.push_back(e);
        }
    }

    return true;
}

auto PythonServer::PutChatHistoryEntity(const ChatHistoryEntity& chat_history_entity) -> bool
{
    py::object chat_provider = m_python_module_chat.attr("__dict__")["chat_history_provider"];
    if (!chat_provider) {
        ErrorLogger() << "Unable to get Python object chat_history_provider";
        return false;
    }
    py::object f = chat_provider.attr("put_history_entity");
    if (!f) {
        ErrorLogger() << "Unable to call Python method put_history_entity";
        return false;
    }

    auto color = chat_history_entity.text_color;

    return f(py::long_(to_time_t(chat_history_entity.timestamp)),
             chat_history_entity.player_name,
             chat_history_entity.text,
             py::make_tuple(std::get<0>(color), std::get<1>(color), std::get<2>(color), std::get<3>(color)));
}

auto PythonServer::CreateUniverse(std::map<int, PlayerSetupData>& player_setup_data) -> bool
{
    // Confirm existence of the directory containing the universe generation
    // Python scripts and add it to Pythons sys.path to make sure Python will
    // find our scripts
    auto python_universe_generator_dir = GetPythonUniverseGeneratorDir();
    if (!fs::exists(python_universe_generator_dir)) {
        ErrorLogger() << "Can't find folder containing universe generation scripts: " << PathToString(python_universe_generator_dir);
        return false;
    }
    AddToSysPath(python_universe_generator_dir);

    // import universe generator script file
    m_python_module_universe_generator = py::import("universe_generator");

    py::dict py_player_setup_data;

    // the universe generator module should contain an "error_report" function,
    // so set the ErrorReport member function to use it
    SetErrorModule(m_python_module_universe_generator);

    for (auto& psd : player_setup_data) {
        py_player_setup_data[psd.first] = py::object(psd.second);
    }

    py::object f = m_python_module_universe_generator.attr("create_universe");
    if (!f) {
        ErrorLogger() << "Unable to call Python function create_universe ";
        return false;
    }

    return f(py_player_setup_data);
}

auto PythonServer::ExecuteTurnEvents() -> bool
{
    py::object f = m_python_module_turn_events.attr("execute_turn_events");
    if (!f) {
        ErrorLogger() << "Unable to call Python function execute_turn_events ";
        return false;
    }
    return f();
}

auto PythonServer::AsyncIOTick() -> bool
{
    py::object stop = m_asyncio_event_loop.attr("stop");
    if (!stop) {
        ErrorLogger() << "Unable to call Python function stop";
        return false;
    }
    stop();
    py::object run_forever = m_asyncio_event_loop.attr("run_forever");
    if (!run_forever) {
        ErrorLogger() << "Unable to call Python function run_forever";
        return false;
    }
    run_forever();

    return true;
}

auto GetPythonUniverseGeneratorDir() -> fs::path
{ return GetPythonDir() / "universe_generation"; }

auto GetPythonTurnEventsDir() -> fs::path
{ return GetPythonDir() / "turn_events"; }

auto GetPythonAuthDir() -> fs::path
{ return GetPythonDir() / "auth"; }

auto GetPythonChatDir() -> fs::path
{ return GetPythonDir() / "chat"; }
