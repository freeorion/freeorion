#include "Logger.h"

#include "OptionsDB.h"
#include "i18n.h"
#include "Version.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>

#include <ctime>
#include <regex>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace attr = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;


namespace {
    // Create the log logger for logging of logger and logging related events.
    // Manually created to prevent a recursive call during initialization.
    BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(                                \
        FO_GLOBAL_LOGGER_NAME(log), NamedThreadedLogger)                \
    {                                                                   \
        return NamedThreadedLogger(                                     \
            (boost::log::keywords::severity = LogLevel::debug),         \
            (boost::log::keywords::channel = "log"));                   \
    }


    // Compile time constant pointers to constant char arrays.
    constexpr const char* const log_level_names[] = {"debug", "info", "warn", "error"};

    constexpr LogLevel default_sink_level = LogLevel::debug;
    constexpr LogLevel default_source_level = LogLevel::info;

    DiscreteValidator<std::string> LogLevelValidator() {
        std::set<std::string> valid_levels = {"debug", "info", "warn", "error",
                                              "DEBUG", "INFO", "WARN", "ERROR",
                                              "0", "1", "2", "3" };
        auto validator = DiscreteValidator<std::string>(valid_levels);
        return validator;
    }

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
        if (!GetOptionsDB().OptionExists(full_option_name))
            GetOptionsDB().Add<std::string>(
                full_option_name, description, default_level, LogLevelValidator());

        // Return the threshold from the db.
        return to_LogLevel(GetOptionsDB().Get<std::string>(full_option_name));
    }
}



std::string to_string(const LogLevel level)
{ return log_level_names[static_cast<std::size_t>(level)]; }

LogLevel to_LogLevel(const std::string& text) {
    if (text == "error")    return LogLevel::error;
    if (text == "warn")     return LogLevel::warn;
    if (text == "info")     return LogLevel::info;
    if (text == "debug")    return LogLevel::debug;

    if (text == "ERROR")    return LogLevel::error;
    if (text == "WARN")     return LogLevel::warn;
    if (text == "INFO")     return LogLevel::info;
    if (text == "DEBUG")    return LogLevel::debug;

    if (text == "3")    return LogLevel::error;
    if (text == "2")    return LogLevel::warn;
    if (text == "1")    return LogLevel::info;
    if (text == "0")    return LogLevel::debug;

    WarnLogger(log) << "\"" << text <<"\" is not a valid log level. "
                    << "Valid levels are error, warn, info, and debug";

    return LogLevel::debug;
}

// Provide a LogLevel stream out formatter for streaming logs
template<typename CharT, typename TraitsT>
std::basic_ostream<CharT, TraitsT>& operator<<(
    std::basic_ostream<CharT, TraitsT>& os, const LogLevel& level)
{
    os << log_level_names[static_cast<std::size_t>(level)];
    return os;
}

// Provide a LogLevel input formatter for filtering
template<typename CharT, typename TraitsT>
inline std::basic_istream<CharT, TraitsT >& operator>>(
    std::basic_istream<CharT, TraitsT>& is, LogLevel& level)
{
    std::string tmp;
    is >> tmp;
    level = to_LogLevel(tmp);
    return is;
}

int g_indent = 0;

std::string DumpIndent()
{ return std::string(g_indent * 4, ' '); }

namespace {
    using TextFileSinkBackend  = sinks::text_file_backend;
    using TextFileSinkFrontend = sinks::synchronous_sink<TextFileSinkBackend>;

    // The backend should be accessible synchronously from any thread by any sink frontend
    boost::shared_ptr<TextFileSinkBackend>  f_file_sink_backend;

    std::string f_default_exec_logger_name;
}

const std::string& DefaultExecLoggerName()
{ return f_default_exec_logger_name; }

void InitLoggingSystem(const std::string& logFile, const std::string& _default_exec_logger_name) {
    f_default_exec_logger_name = _default_exec_logger_name;
    std::transform(f_default_exec_logger_name.begin(), f_default_exec_logger_name.end(), f_default_exec_logger_name.begin(),
                   [](const char c) { return std::tolower(c); });

    // Register LogLevel so that the formatters will be found.
    logging::register_simple_formatter_factory<LogLevel, char>("Severity");
    logging::register_simple_filter_factory<LogLevel>("Severity");

    // Create a sink backend that logs to a file
    f_file_sink_backend = boost::make_shared<TextFileSinkBackend>(
        keywords::file_name = logFile.c_str(),
        keywords::auto_flush = true
    );

    // Create the frontend for formatting default records.
    auto file_sink_frontend = boost::make_shared<TextFileSinkFrontend>(f_file_sink_backend);

    // Create the format
    file_sink_frontend->set_formatter(
        expr::stream
        << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
        << " [" << log_severity << "] "
        << f_default_exec_logger_name << " : " << log_src_filename << ":" << log_src_linenum << " : "
        << expr::message
    );

    // Set a filter to only use records from the default channel
    file_sink_frontend->set_filter(log_channel == "");

    logging::core::get()->add_sink(file_sink_frontend);

    // Add global attributes to all records
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());

    // Initialize the internal logger
    RegisterLoggerWithOptionsDB(FO_GLOBAL_LOGGER_NAME(log)::get(), "log");

    // Setup the OptionsDB options for the file sink.
    LogLevel options_db_log_threshold = AddLoggerToOptionsDB(
        exec_option_name_prefix + f_default_exec_logger_name);

    // Use the option to set the threshold of the default logger
    SetLoggerThreshold("", options_db_log_threshold);

    // Print setup message.
    auto date_time = std::time(nullptr);
    InfoLogger(log) << "Logger initialized at " << std::ctime(&date_time);
    InfoLogger() << FreeOrionVersionString();
}

void RegisterLoggerWithOptionsDB(NamedThreadedLogger& logger, const std::string& name) {
    if (name.empty())
        return;

    // Create a sink frontend for formatting.
    auto sink_frontend = boost::make_shared<TextFileSinkFrontend>(f_file_sink_backend);

    // Create the format
    sink_frontend->set_formatter(
        expr::stream
        << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
        << " [" << log_severity << "] "
        << name
        << " : " << log_src_filename << ":" << log_src_linenum << " : "
        << expr::message
    );

    // Set a filter to only format this channel
    sink_frontend->set_filter(log_channel == name);

    logging::core::get()->add_sink(sink_frontend);

    // Setup the OptionsDB options for this source.
    LogLevel options_db_log_threshold = AddLoggerToOptionsDB(
        source_option_name_prefix + name);

    // Use the option.
    SetLoggerThreshold(name, options_db_log_threshold);

    InfoLogger(log) << "Added log source \"" << name << "\".";
}

namespace {
    // Create a minimum severity table filter
    auto f_min_channel_severity = expr::channel_severity_filter(log_channel, log_severity);
}

void SetLoggerThreshold(const std::string& source, LogLevel threshold) {
    InfoLogger(log) << "Setting \"" << (source.empty() ? "default" : source)
                     << "\" logger threshold to \"" << threshold << "\".";

    logging::core::get()->reset_filter();
    f_min_channel_severity[source] = threshold;
    logging::core::get()->set_filter(f_min_channel_severity);
}


void UpdateLoggerThresholdsFromOptionsDB() {
    SetLoggerThresholds(LoggerExecutableOptionsLabelsAndLevels());
    SetLoggerThresholds(LoggerSourceOptionsLabelsAndLevels());
}

namespace {

    /** Returns the list of full option names, logger names and thresholds for loggers in
        OptionsDB will \p prefix using \p prefix_regex.*/
    std::set<std::tuple<std::string, std::string, LogLevel>> LoggerOptionsLabelsAndLevels(
        const std::string prefix, const std::regex& prefix_regex) {
        // Get a list of all of the potential loggers
        std::set<std::string> loggers;
        GetOptionsDB().FindOptions(loggers, prefix, true);

        std::set<std::tuple<std::string, std::string, LogLevel>> retval;
        for (const auto& full_option : loggers) {
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
            retval.insert(std::make_tuple(full_option, option_name, option_value));

            DebugLogger(log) << "Found " << full_option << ",  name = " << option_name << " value = " << option_value;
        }

        return retval;
    }
}

/** Return the option names, labels and levels for all executables from OptionsDB. */
std::set<std::tuple<std::string, std::string, LogLevel>> LoggerExecutableOptionsLabelsAndLevels()
{ return LoggerOptionsLabelsAndLevels(exec_option_name_prefix, exec_name_regex); }

/** Return the option names, labels and levels for all sources/channels from OptionsDB. */
std::set<std::tuple<std::string, std::string, LogLevel>> LoggerSourceOptionsLabelsAndLevels()
{ return LoggerOptionsLabelsAndLevels(source_option_name_prefix, source_name_regex); }

/** Sets the logger thresholds from a list of options, labels and thresholds. */
void SetLoggerThresholds(const std::set<std::tuple<std::string, std::string, LogLevel>>& fulloption_name_and_levels) {
    for (const auto& fulloption_name_and_level : fulloption_name_and_levels) {
        const auto& full_option = std::get<0>(fulloption_name_and_level);
        const auto& name = std::get<1>(fulloption_name_and_level);
        const auto& value = std::get<2>(fulloption_name_and_level);


        // Update the option in OptionsDB if it already exists.
        if (GetOptionsDB().OptionExists(full_option))
            GetOptionsDB().Set(full_option, to_string(value));

        // Set the logger threshold.

        std::smatch match;
        std::regex_search(full_option, match, exec_name_regex);
        bool is_my_root_logger = (!match.empty() && match[1] == f_default_exec_logger_name);

        SetLoggerThreshold((is_my_root_logger ? "" : name), value);
    }
}
