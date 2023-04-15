#include "CommonWrappers.h"

#include "../util/boost_fix.h"
#include <string>
#include "../util/Directories.h"
#include "../util/OptionsDB.h"
#include <boost/python.hpp>

namespace py = boost::python;


void FreeOrionPython::WrapConfig()
{
    // For the AI client provide function names in camelCase,
    // as that's still the preferred style there (for the time being)
    // For the server, provide the function names already in snake_case

    py::def(
#ifdef FREEORION_BUILD_AI
            "getOptionsDBOptionStr"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_options_db_option_str"
#endif
            ,
            +[](std::string const &option) -> py::object { return GetOptionsDB().OptionExists(option) ? py::str(GetOptionsDB().Get<std::string>(option)) : py::str(); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the string value of option in OptionsDB or None if the"
            " option does not exist.");

    py::def(
#ifdef FREEORION_BUILD_AI
            "getOptionsDBOptionInt"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_options_db_option_int"
#endif
            ,
            +[](std::string const &option) -> py::object { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<int>(option)) : py::object(); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the integer value of option in OptionsDB or None if the"
            " option does not exist.");

    py::def(
#ifdef FREEORION_BUILD_AI
            "getOptionsDBOptionBool"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_options_db_option_bool"
#endif
            ,
            +[](std::string const &option) -> py::object { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<bool>(option)) : py::object(); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the bool value of option in OptionsDB or None if the"
            " option does not exist.");

    py::def(
#ifdef FREEORION_BUILD_AI
            "getOptionsDBOptionDouble"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_options_db_option_double"
#endif
            ,
            +[](std::string const &option) -> py::object { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<double>(option)) : py::object(); },
            py::return_value_policy<py::return_by_value>(),
            "Returns the double value of option in OptionsDB or None if the"
            " option does not exist.");

    py::def(
#ifdef FREEORION_BUILD_AI
            "getUserConfigDir"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_user_config_dir"
#endif
            ,
            +[]() -> py::str { return py::str(PathToString(GetUserConfigDir())); },
            "Returns path to directory where FreeOrion stores user specific"
            " configuration.");

    py::def(
#ifdef FREEORION_BUILD_AI
            "getUserDataDir"
#endif
#ifdef FREEORION_BUILD_SERVER
            "get_user_data_dir"
#endif
            ,
            +[]() -> py::str { return py::str(PathToString(GetUserDataDir())); },
            "Returns path to directory where FreeOrion stores user specific"
            " data (saves, etc.).");
}
