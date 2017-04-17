#include "Logger.h"

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
#include <boost/optional.hpp>

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
    constexpr const char* const log_level_names[] = {"trace", "debug", "info", "warn", "error"};
}


std::string to_string(const LogLevel level)
{ return log_level_names[static_cast<std::size_t>(level)]; }

LogLevel to_LogLevel(const std::string& text) {
    if (text == "error")    return LogLevel::error;
    if (text == "warn")     return LogLevel::warn;
    if (text == "info")     return LogLevel::info;
    if (text == "debug")    return LogLevel::debug;
    if (text == "trace")    return LogLevel::trace;

    if (text == "ERROR")    return LogLevel::error;
    if (text == "WARN")     return LogLevel::warn;
    if (text == "INFO")     return LogLevel::info;
    if (text == "DEBUG")    return LogLevel::debug;
    if (text == "TRACE")    return LogLevel::trace;

    if (text == "4")    return LogLevel::error;
    if (text == "3")    return LogLevel::warn;
    if (text == "2")    return LogLevel::info;
    if (text == "1")    return LogLevel::debug;
    if (text == "0")    return LogLevel::trace;

    WarnLogger(log) << "\"" << text <<"\" is not a valid log level. "
                    << "Valid levels are error, warn, info, debug and trace";

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

    boost::shared_ptr<TextFileSinkBackend>& GetSinkBackend() {
        // Create the sink backend as a function local static variable to avoid the static
        // initilization fiasco.
        static boost::shared_ptr<TextFileSinkBackend> m_sink_backend;
        return m_sink_backend;
    }

    void CreateFileSinkFrontEnd(const std::string& name) {
        auto& file_sink_backend = GetSinkBackend();

        // Return if the file sink backed has not been configured
        if (!file_sink_backend)
            return;

        // Create a sink frontend for formatting.
        auto sink_frontend = boost::make_shared<TextFileSinkFrontend>(file_sink_backend);

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
    }
}

LoggerCreatedSignalType LoggerCreatedSignal;

namespace {
    std::string& LocalDefaultExecLoggerName() {
        // Create default logger name as a static function variable to avoid static initialization fiasco
        static std::string default_exec_logger_name;
        return default_exec_logger_name;
    }

    boost::optional<LogLevel>& ForcedThreshold() {
        // Create forced threshold as a static function variable to avoid static initialization fiasco
        static boost::optional<LogLevel> forced_threshold = boost::none;
        return forced_threshold;
    }

}

const std::string& DefaultExecLoggerName()
{ return LocalDefaultExecLoggerName(); }

void InitLoggingSystem(const std::string& logFile, const std::string& _default_exec_logger_name) {
    auto& default_exec_logger_name = LocalDefaultExecLoggerName();
    default_exec_logger_name = _default_exec_logger_name;
    std::transform(default_exec_logger_name.begin(), default_exec_logger_name.end(), default_exec_logger_name.begin(),
                   [](const char c) { return std::tolower(c); });

    // Register LogLevel so that the formatters will be found.
    logging::register_simple_formatter_factory<LogLevel, char>("Severity");
    logging::register_simple_filter_factory<LogLevel>("Severity");

    // Create a sink backend that logs to a file
    auto& file_sink_backend = GetSinkBackend();
    file_sink_backend = boost::make_shared<TextFileSinkBackend>(
        keywords::file_name = logFile.c_str(),
        keywords::auto_flush = true
    );

    // Create the frontend for formatting default records.
    auto file_sink_frontend = boost::make_shared<TextFileSinkFrontend>(file_sink_backend);

    // Create the format
    file_sink_frontend->set_formatter(
        expr::stream
        << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
        << " [" << log_severity << "] "
        << default_exec_logger_name << " : " << log_src_filename << ":" << log_src_linenum << " : "
        << expr::message
    );

    // Set a filter to only use records from the default channel
    file_sink_frontend->set_filter(log_channel == "");

    logging::core::get()->add_sink(file_sink_frontend);

    // Add global attributes to all records
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());

    SetLoggerThreshold("", max_LogLevel);

    // Initialize the internal logger
    ConfigureLogger(FO_GLOBAL_LOGGER_NAME(log)::get(), "log");

    // Print setup message.
    auto date_time = std::time(nullptr);
    InfoLogger(log) << "Logger initialized at " << std::ctime(&date_time);
    InfoLogger() << FreeOrionVersionString();
}

void OverrideLoggerThresholds(const LogLevel threshold) {
    InfoLogger(log) << "Forcing all logger threshold to be " << to_string(threshold);
    ForcedThreshold() = threshold;
}

void ConfigureLogger(NamedThreadedLogger& logger, const std::string& name) {
    if (name.empty())
        return;

    CreateFileSinkFrontEnd(name);

    SetLoggerThreshold(name, min_LogLevel);

    LoggerCreatedSignal(logger, name);

    InfoLogger(log) << "Added log source \"" << name << "\".";
}

namespace {
    // Create a minimum severity table filter
    auto f_min_channel_severity = expr::channel_severity_filter(log_channel, log_severity);
}

void SetLoggerThreshold(const std::string& source, LogLevel threshold) {
    auto used_threshold = ForcedThreshold() ? *ForcedThreshold() : threshold;
    logging::core::get()->reset_filter();
    f_min_channel_severity[source] = used_threshold;
    logging::core::get()->set_filter(f_min_channel_severity);

    InfoLogger(log) << "Setting \"" << (source.empty() ? "default" : source)
                    << "\" logger threshold to \"" << used_threshold << "\".";
}
