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

        ...

        <ai>debug</ai>
      </sources>
    </logging>

    The <execs> section controls the default log threshold of each client (freeorion.log), server
    (freeoriond.log) and AI (AI_x.log) files.

    The <sources> section controls the log threshold of the named loggers.

*/

DiscreteValidator<std::string> LogLevelValidator();

/** Initializes the logging system with settings from OptionsDB and starts capturing
 * LoggerCreatedSignal to add new loggers to OptionsDB on the fly. */
FO_COMMON_API void InitLoggingOptionsDBSystem();

// Configure a logger and lookup and/or register the \p name logger in OptionsDB.  Set the initial threshold.
FO_COMMON_API void RegisterLoggerWithOptionsDB(const NamedThreadedLogger& logger, const std::string& name);

FO_COMMON_API void UpdateLoggerThresholdsFromOptionsDB();

/** Return the option names, labels and levels for all executables from OptionsDB. */
FO_COMMON_API std::set<std::tuple<std::string, std::string, LogLevel>> LoggerExecutableOptionsLabelsAndLevels();

/** Return the option names, labels and levels for all sources/channels from OptionsDB. */
FO_COMMON_API std::set<std::tuple<std::string, std::string, LogLevel>> LoggerSourceOptionsLabelsAndLevels();

/** Sets the logger thresholds from a list of options, labels and thresholds. */
FO_COMMON_API void SetLoggerThresholds(const std::set<std::tuple<std::string, std::string, LogLevel>>&);

extern int g_indent;

/** A function that returns the correct amount of spacing for the current
  * indentation level during a dump. */
std::string DumpIndent();

#endif // _LoggerWithOptionsDB_h_
