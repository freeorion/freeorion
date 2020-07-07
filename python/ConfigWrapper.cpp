#include "../util/Directories.h"
#include "../util/OptionsDB.h"

#include <boost/python.hpp>

#include <string>

namespace py = boost::python;


namespace {
    py::object GetOptionsDBOptionStr(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? py::str(GetOptionsDB().Get<std::string>(option)) : py::str(); }

    py::object GetOptionsDBOptionInt(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<int>(option)) : py::object(); }

    py::object GetOptionsDBOptionBool(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<bool>(option)) : py::object(); }

    py::object GetOptionsDBOptionDouble(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? py::object(GetOptionsDB().Get<double>(option)) : py::object(); }

    py::str GetUserConfigDirWrapper()
    { return py::str(PathToString(GetUserConfigDir())); }

    py::str GetUserDataDirWrapper()
    { return py::str(PathToString(GetUserDataDir())); }
}

namespace FreeOrionPython {
    void WrapConfig()
    {
#ifdef FREEORION_BUILD_AI
        // For the AI client provide function names in camelCase,
        // as that's still the preferred style there (for the time being)
        py::def("getOptionsDBOptionStr",    GetOptionsDBOptionStr,     py::return_value_policy<py::return_by_value>(), "Returns the string value of option in OptionsDB or None if the option does not exist.");
        py::def("getOptionsDBOptionInt",    GetOptionsDBOptionInt,     py::return_value_policy<py::return_by_value>(), "Returns the integer value of option in OptionsDB or None if the option does not exist.");
        py::def("getOptionsDBOptionBool",   GetOptionsDBOptionBool,    py::return_value_policy<py::return_by_value>(), "Returns the bool value of option in OptionsDB or None if the option does not exist.");
        py::def("getOptionsDBOptionDouble", GetOptionsDBOptionDouble,  py::return_value_policy<py::return_by_value>(), "Returns the double value of option in OptionsDB or None if the option does not exist.");

        py::def("getUserConfigDir",         GetUserConfigDirWrapper,        /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific configuration.");
        py::def("getUserDataDir",           GetUserDataDirWrapper,          /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (saves, etc.).");
#endif

#ifdef FREEORION_BUILD_SERVER
        // For the server, provide the function names already in snake_case
        py::def("get_options_db_option_str",    GetOptionsDBOptionStr,     py::return_value_policy<py::return_by_value>(), "Returns the string value of option in OptionsDB or None if the option does not exist.");
        py::def("get_options_db_option_int",    GetOptionsDBOptionInt,     py::return_value_policy<py::return_by_value>(), "Returns the integer value of option in OptionsDB or None if the option does not exist.");
        py::def("get_options_db_option_bool",   GetOptionsDBOptionBool,    py::return_value_policy<py::return_by_value>(), "Returns the bool value of option in OptionsDB or None if the option does not exist.");
        py::def("get_options_db_option_double", GetOptionsDBOptionDouble,  py::return_value_policy<py::return_by_value>(), "Returns the double value of option in OptionsDB or None if the option does not exist.");

        py::def("get_user_config_dir",         GetUserConfigDirWrapper,        /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific configuration.");
        py::def("get_user_data_dir",           GetUserDataDirWrapper,          /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (saves, etc.).");
#endif
    }
}
