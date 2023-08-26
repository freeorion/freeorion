#include "LoggerWithOptionsDB.h"

#include "OptionsDB.h"
#include "i18n.h"

#include <functional>
#include <regex>

// TODO consider adding thread and process id as options

namespace {
    // Create the log logger for logging of logger and logging related events.
    // Manually created to prevent a recursive call during initialization.
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(                                    \
        FO_GLOBAL_LOGGER_NAME(log), NamedThreadedLogger)                    \
    {                                                                       \
        return NamedThreadedLogger(                                         \
            (boost::log::keywords::severity = default_log_level_threshold), \
            (boost::log::keywords::channel = "log"));                       \
    }

    constexpr LogLevel default_sink_level = default_log_level_threshold;
    constexpr LogLevel default_source_level = default_log_level_threshold;

    constexpr auto exec_option_name_prefix = "logging.execs.";
    constexpr auto source_option_name_prefix = "logging.sources.";

    std::regex exec_name_regex("(?:logging\\.execs\\.)(\\S+)");
    std::regex source_name_regex("(?:logging\\.sources\\.)(\\S+)");

    /** Add the log threshold for a logger with \p full_option_name to OptionsDB and return the
        threshold read from OptionsDB. */
    LogLevel AddLoggerToOptionsDB(const std::string& full_option_name) {
        // Determine the type of logger, executable default or channel.
        std::smatch ematch;
        std::regex_search(full_option_name, ematch, exec_name_regex);
        bool is_an_exec_root_logger = !ematch.empty();

        std::smatch cmatch;
        std::regex_search(full_option_name, cmatch, source_name_regex);
        bool is_a_channel_logger = !cmatch.empty();

        if (!is_an_exec_root_logger && !is_a_channel_logger)
            WarnLogger(log) << "Adding a logger to OptionsDB with an unknown prefix. " << full_option_name;

        // Find the appropriate defaults.
        auto default_level = to_string(default_source_level);
        auto description = UserStringNop("OPTIONS_DB_LOGGER_SOURCE_LEVEL");

        if (is_an_exec_root_logger) {
            default_level = to_string(default_sink_level);
            description =  UserStringNop("OPTIONS_DB_LOGGER_FILE_SINK_LEVEL");
        }

        // Create the new option if necessary.
        if (!GetOptionsDB().OptionExists(full_option_name)) {
            GetOptionsDB().Add<std::string>(full_option_name, std::string{description},
                                            std::move(default_level), LogLevelValidator());
        }

        // Return the threshold from the db.
        return to_LogLevel(GetOptionsDB().Get<std::string>(full_option_name));
    }
}

std::unique_ptr<DiscreteValidator<std::string>> LogLevelValidator() {
    std::vector<std::string> valid_labels;
    valid_labels.reserve(LoggerDetails::valid_names_and_levels.size());
    std::transform(LoggerDetails::valid_names_and_levels.begin(),
                   LoggerDetails::valid_names_and_levels.end(),
                   std::back_inserter(valid_labels),
                   [](auto name_level) { return std::string{name_level.first}; });
    return std::make_unique<DiscreteValidator<std::string>>(std::move(valid_labels));
}

void InitLoggingOptionsDBSystem() {
    // Initialize the logging system's logger
    RegisterLoggerWithOptionsDB("log");

    // Setup the OptionsDB options for the file sink.
    LogLevel options_db_log_threshold = AddLoggerToOptionsDB(
        exec_option_name_prefix + DefaultExecLoggerName());

    // Use the option to set the threshold of the default logger
    SetLoggerThreshold("", options_db_log_threshold);

    // Link the logger created signal to the OptionsDB registration
    LoggerCreatedSignal.connect([](const std::string& logger_name){ RegisterLoggerWithOptionsDB(logger_name); });

    // Register all loggers created during static initialization
    for (const auto& name: CreatedLoggersNames())
        RegisterLoggerWithOptionsDB(name);

    InfoLogger(log) << "Initialized OptionsDB logging configuration.";
}

void RegisterLoggerWithOptionsDB(const std::string& logger_name, const bool is_exec_logger) {
    if (logger_name.empty())
        return;

    // Setup the OptionsDB options for this source.
    LogLevel options_db_log_threshold = AddLoggerToOptionsDB(
        (is_exec_logger ? exec_option_name_prefix : source_option_name_prefix) + logger_name);

    // Use the option.
    SetLoggerThreshold(logger_name, options_db_log_threshold);

    DebugLogger(log) << "Configure log source \"" << logger_name << "\" from optionsDB "
                     << "using threshold " << to_string(options_db_log_threshold);
}


void ChangeLoggerThresholdInOptionsDB(const std::string& full_option, LogLevel option_value) {
    // Determine the logger name by checing which prefix matches
    std::smatch match;
    std::regex_search(full_option, match, exec_name_regex);
    if (match.empty())
        std::regex_search(full_option, match, source_name_regex);

    if (match.empty()) {
        ErrorLogger(log) << "Trying to set logger threshold for " << full_option
                         << ", but the prefix is not recognized.";
        return;
    }

    const auto& option_name = match[1];

    std::vector<std::tuple<std::string, std::string, LogLevel>> one_value{
        std::make_tuple(full_option, option_name, option_value)};
    SetLoggerThresholds(one_value);
}

namespace {
    /** Returns the list of full option names, logger names and thresholds for loggers in
        OptionsDB will \p prefix using \p prefix_regex.*/
    auto LoggerOptionsLabelsAndLevels(const std::string prefix, const std::regex& prefix_regex) {
        // Get a list of all of the potential loggers
        auto loggers = GetOptionsDB().FindOptions(prefix, true);

        std::vector<std::tuple<std::string, std::string, LogLevel>> retval;
        retval.reserve(loggers.size());

        for (const auto option : loggers) {
            std::string full_option{option};

            // Find the option name
            std::smatch match;
            std::regex_search(full_option, match, prefix_regex);
            if (match.empty()) {
                ErrorLogger(log) << "Unable to find a logger name from option name \"" << full_option << "\"";
                continue;
            }
            const auto& option_name = match[1];

            // Add it to options db and get the option value
            const auto option_value = AddLoggerToOptionsDB(full_option);

            // Add to return value
            retval.emplace_back(std::move(full_option), option_name, option_value);
        }

        return retval;
    }
}

/** Return the option names, labels and levels for the requested types from OptionsDB. */
std::vector<std::tuple<std::string, std::string, LogLevel>> LoggerOptionsLabelsAndLevels(const LoggerTypes types) {
    switch (types) {
    case LoggerTypes::exec:
        // Only the per executable loggers
        return LoggerOptionsLabelsAndLevels(exec_option_name_prefix, exec_name_regex);
    case LoggerTypes::named:
        // Only the standard source loggers
        return LoggerOptionsLabelsAndLevels(source_option_name_prefix, source_name_regex);
    default: {
        // Combine both types of loggers into one set
        auto exec_loggers = LoggerOptionsLabelsAndLevels(exec_option_name_prefix, exec_name_regex);
        auto source_loggers = LoggerOptionsLabelsAndLevels(source_option_name_prefix, source_name_regex);
        source_loggers.insert(source_loggers.end(),
                              std::make_move_iterator(exec_loggers.begin()),
                              std::make_move_iterator(exec_loggers.end()));
        return source_loggers;
    }
    }
}

/** Sets the logger thresholds from a list of options, labels and thresholds. */
void SetLoggerThresholds(const std::vector<std::tuple<std::string, std::string, LogLevel>>& fulloption_name_and_levels) {
    for (const auto& [full_option, name, value] : fulloption_name_and_levels) {

        // Update the option in OptionsDB if it already exists.
        if (GetOptionsDB().OptionExists(full_option))
            GetOptionsDB().Set(full_option, to_string(value));

        // Set the logger threshold.
        std::smatch match;
        std::regex_search(full_option, match, exec_name_regex);
        bool is_my_root_logger = (!match.empty() && match[1] == DefaultExecLoggerName());

        SetLoggerThreshold((is_my_root_logger ? "" : name), value);
    }
}
