#ifndef _LoggerWithOptionsDB_h_
#define _LoggerWithOptionsDB_h_

#include "OptionsDB.h"

#include <string>
#include <set>
#include <tuple>

#include "Export.h"
#include "Logger.h"


/** \file
    \brief This extends the logging system to be configured from values stored in OptionsDB.

    Enabling/disabling the default executable logger and named sources is
    controlled statically on startup through OptionsDB, with either the files
    config.xml and/or persistent_config.xml or dynamically with the app options
    window.

    The logging section in the configuration file looks like:

    \code{.xml}
    <logging>
      <execs>
        <client>info</client>
        <server>debug</server>
        <ai>debug</ai>
      </execs>
      <sources>
        <combat>debug</combat>
        <combat-log>info</combat-log>
        <network>warn</network>
        <ai>debug</ai>
      </sources>
    </logging>
    \endcode

    The \<execs\> section controls the default log threshold of each client (freeorion.log), server
    (freeoriond.log) and AI (AI_x.log) files.

    The \<sources\> section controls the log threshold of the named loggers.

*/

DiscreteValidator<std::string> LogLevelValidator();

/** Initializes the logging system with settings from OptionsDB and starts capturing
 * LoggerCreatedSignal to add new loggers to OptionsDB on the fly. */
FO_COMMON_API void InitLoggingOptionsDBSystem();

// Configure a logger and lookup and/or register the \p name logger in OptionsDB.  Set the initial threshold.
FO_COMMON_API void RegisterLoggerWithOptionsDB(const std::string& logger_name, const bool is_exec_logger = false);

FO_COMMON_API void ChangeLoggerThresholdInOptionsDB(const std::string& option_name, LogLevel option_value);


///
enum class LoggerTypes {
    exec = 1,  ///< the unnamed logger for a particular executable
    named = 2, ///< a normal named source
    both = exec | named};

/** Return the option names, labels and levels for logger oy \p type from OptionsDB. */
FO_COMMON_API std::set<std::tuple<std::string, std::string, LogLevel>>
    LoggerOptionsLabelsAndLevels(const LoggerTypes types);

/** Sets the logger thresholds from a list of options, labels and thresholds. */
FO_COMMON_API void SetLoggerThresholds(const std::set<std::tuple<std::string, std::string, LogLevel>>& full_option_name_and_level);

#endif // _LoggerWithOptionsDB_h_
