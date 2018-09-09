#include "../util/Directories.h"
#include "../util/OptionsDB.h"

#include <boost/python.hpp>

#include <string>

using boost::python::def;
using boost::python::return_value_policy;
using boost::python::return_by_value;

namespace {
    boost::python::object GetOptionsDBOptionStr(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? boost::python::str(GetOptionsDB().Get<std::string>(option)) : boost::python::str(); }

    boost::python::object GetOptionsDBOptionInt(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? boost::python::object(GetOptionsDB().Get<int>(option)) : boost::python::object(); }

    boost::python::object GetOptionsDBOptionBool(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? boost::python::object(GetOptionsDB().Get<bool>(option)) : boost::python::object(); }

    boost::python::object GetOptionsDBOptionDouble(std::string const &option)
    { return GetOptionsDB().OptionExists(option) ? boost::python::object(GetOptionsDB().Get<double>(option)) : boost::python::object(); }

    boost::python::str GetUserConfigDirWrapper()
    { return boost::python::str(PathToString(GetUserConfigDir())); }

    boost::python::str GetUserDataDirWrapper()
    { return boost::python::str(PathToString(GetUserDataDir())); }
}

namespace FreeOrionPython {
    void WrapConfig()
    {
        // We provide the Python wrappers both with camelCase and with snake_case names,
        // as these wrappers are used both by the AI and the universe generation scripts.
        // The former still uses camelCase, while the latter uses snake_case, so we need
        // to support both styles for the time being.

        def("getOptionsDBOptionStr",    GetOptionsDBOptionStr,     return_value_policy<return_by_value>(), "Returns the string value of option in OptionsDB or None if the option does not exist.");
        def("getOptionsDBOptionInt",    GetOptionsDBOptionInt,     return_value_policy<return_by_value>(), "Returns the integer value of option in OptionsDB or None if the option does not exist.");
        def("getOptionsDBOptionBool",   GetOptionsDBOptionBool,    return_value_policy<return_by_value>(), "Returns the bool value of option in OptionsDB or None if the option does not exist.");
        def("getOptionsDBOptionDouble", GetOptionsDBOptionDouble,  return_value_policy<return_by_value>(), "Returns the double value of option in OptionsDB or None if the option does not exist.");

        def("get_options_db_option_str",    GetOptionsDBOptionStr,     return_value_policy<return_by_value>(), "Returns the string value of option in OptionsDB or None if the option does not exist.");
        def("get_options_db_option_int",    GetOptionsDBOptionInt,     return_value_policy<return_by_value>(), "Returns the integer value of option in OptionsDB or None if the option does not exist.");
        def("get_options_db_option_bool",   GetOptionsDBOptionBool,    return_value_policy<return_by_value>(), "Returns the bool value of option in OptionsDB or None if the option does not exist.");
        def("get_options_db_option_double", GetOptionsDBOptionDouble,  return_value_policy<return_by_value>(), "Returns the double value of option in OptionsDB or None if the option does not exist.");

        def("getUserConfigDir",         GetUserConfigDirWrapper,        /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific configuration.");
        def("getUserDataDir",           GetUserDataDirWrapper,          /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (saves, etc.).");

        def("get_user_config_dir",         GetUserConfigDirWrapper,        /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific configuration.");
        def("get_user_data_dir",           GetUserDataDirWrapper,          /* no return value policy, */ "Returns path to directory where FreeOrion stores user specific data (saves, etc.).");
    }
}
