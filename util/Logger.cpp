#include "Logger.h"

#include "Version.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sinks/frontend_requirements.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>

#include <boost/optional.hpp>

#include <ctime>
#include <mutex>
#include <regex>
#include <unordered_map>

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
    if (text == to_string(LogLevel::error))    return LogLevel::error;
    if (text == to_string(LogLevel::warn))     return LogLevel::warn;
    if (text == to_string(LogLevel::info))     return LogLevel::info;
    if (text == to_string(LogLevel::debug))    return LogLevel::debug;
    if (text == to_string(LogLevel::trace))    return LogLevel::trace;

    if (text == "4")    return LogLevel::error;
    if (text == "3")    return LogLevel::warn;
    if (text == "2")    return LogLevel::info;
    if (text == "1")    return LogLevel::debug;
    if (text == "0")    return LogLevel::trace;

    // Allow mixed case.
    std::string mixed_case = text;
    std::transform(mixed_case.begin(), mixed_case.end(), mixed_case.begin(),
                   [](const char c) { return std::tolower(c); });

    if (mixed_case == to_string(LogLevel::error))    return LogLevel::error;
    if (mixed_case == to_string(LogLevel::warn))     return LogLevel::warn;
    if (mixed_case == to_string(LogLevel::info))     return LogLevel::info;
    if (mixed_case == to_string(LogLevel::debug))    return LogLevel::debug;
    if (mixed_case == to_string(LogLevel::trace))    return LogLevel::trace;

    WarnLogger(log) << "\"" << text <<"\" is not a valid log level. "
                    << "Valid levels are " << to_string(LogLevel::trace)
                    << ", " << to_string(LogLevel::debug)
                    << ", " << to_string(LogLevel::info)
                    << ", " << to_string(LogLevel::warn)
                    << " and " << to_string(LogLevel::error);

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
    std::string& LocalUnnamedLoggerIdentifier() {
        // Create default logger name as a static function variable to avoid static initialization fiasco
        static std::string unnamed_logger_identifier;
        return unnamed_logger_identifier;
    }

    boost::optional<LogLevel>& ForcedThreshold() {
        // Create forced threshold as a static function variable to avoid static initialization fiasco
        static boost::optional<LogLevel> forced_threshold = boost::none;
        return forced_threshold;
    }

    using TextFileSinkBackend  = sinks::text_file_backend;
    using TextFileSinkFrontend = sinks::synchronous_sink<TextFileSinkBackend>;

    boost::shared_ptr<TextFileSinkBackend>& GetSinkBackend() {
        // Create the sink backend as a function local static variable to avoid the static
        // initilization fiasco.
        static boost::shared_ptr<TextFileSinkBackend> m_sink_backend;
        return m_sink_backend;
    }

    /** LoggersToSinkFrontEnds is used to track all of the global named loggers.  It maps the
        names to the sink front ends.

        It is used to:
          - bind their sink front ends to the file logger backend
          - to provide a complete list of loggers names (i.e. to OptionsDB)
    */
    class LoggersToSinkFrontEnds {
        /// m_mutex serializes access from different threads
        std::mutex m_mutex = {};
        std::unordered_map<std::string, boost::shared_ptr<TextFileSinkFrontend>> m_names_to_front_ends = {};

        public:

        void AddOrReplaceLoggerName(const std::string& channel_name,
                                    boost::shared_ptr<TextFileSinkFrontend> front_end = nullptr)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            // Remove the old front end if it is different.
            const auto& name_and_old_frontend = m_names_to_front_ends.find(channel_name);
            if (name_and_old_frontend != m_names_to_front_ends.end()) {

                if (front_end == name_and_old_frontend->second)
                    return;

                logging::core::get()->remove_sink(name_and_old_frontend->second);
                m_names_to_front_ends.erase(name_and_old_frontend);
            }

            m_names_to_front_ends.insert({channel_name, front_end});

            // Add the new frontend if it is non null.
            if (!front_end)
                return;

            logging::core::get()->add_sink(front_end);

            InfoLogger(log) << "Added logger named \"" << channel_name << "\"";
        }

        std::vector<std::string> LoggersNames() {
            std::lock_guard<std::mutex> lock(m_mutex);

            std::vector<std::string> retval;
            for (const auto& name_and_frontend : m_names_to_front_ends)
                retval.push_back(name_and_frontend.first);
            return retval;
        }

    };

    LoggersToSinkFrontEnds& GetLoggersToSinkFrontEnds() {
        // Create loggers_names_to_front_ends as a static function variable to avoid static initialization fiasco.
        static LoggersToSinkFrontEnds loggers_names_to_front_ends{};
        return loggers_names_to_front_ends;
    }

    void CreateFileSinkFrontEnd(const std::string& channel_name) {
        auto& file_sink_backend = GetSinkBackend();

        // Return if the file sink backed has not been configured
        if (!file_sink_backend)
            return;

        // Create a sink frontend for formatting.
        boost::shared_ptr<TextFileSinkFrontend> sink_frontend
            = boost::make_shared<TextFileSinkFrontend>(file_sink_backend);

        auto display_name = channel_name.empty() ? LocalUnnamedLoggerIdentifier() : channel_name;
        // Create the format
        sink_frontend->set_formatter(
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
            << " [" << log_severity << "] "
            << display_name
            << " : " << log_src_filename << ":" << log_src_linenum << " : "
            << expr::message
        );

        // Set a filter to only format this channel
        sink_frontend->set_filter(log_channel == channel_name);

        // Replace any previous frontend for this channel
        GetLoggersToSinkFrontEnds().AddOrReplaceLoggerName(channel_name, sink_frontend);
    }
}

const std::string& DefaultExecLoggerName()
{ return LocalUnnamedLoggerIdentifier(); }

std::vector<std::string> CreatedLoggersNames()
{ return GetLoggersToSinkFrontEnds().LoggersNames(); }

namespace {
    // Create a minimum severity table filter
    auto f_min_channel_severity = expr::channel_severity_filter(log_channel, log_severity);

    // Set the logger threshold and return the logger name and threshold used.
    std::pair<std::string, LogLevel> SetLoggerThresholdCore(const std::string& source, LogLevel threshold) {
        auto used_threshold = ForcedThreshold() ? *ForcedThreshold() : threshold;
        logging::core::get()->reset_filter();
        f_min_channel_severity[source] = used_threshold;
        logging::core::get()->set_filter(f_min_channel_severity);

        auto& logger_name = (source.empty() ? LocalUnnamedLoggerIdentifier() : source);
        return {logger_name, used_threshold};
    }
}

void SetLoggerThreshold(const std::string& source, LogLevel threshold) {
    const auto& name_and_threshold = SetLoggerThresholdCore(source, threshold);

    InfoLogger(log) << "Setting \"" << name_and_threshold.first
                    << "\" logger threshold to \"" << name_and_threshold.second << "\".";
}

void InitLoggingSystem(const std::string& log_file, const std::string& _unnamed_logger_identifier) {
    auto& unnamed_logger_identifier = LocalUnnamedLoggerIdentifier();
    unnamed_logger_identifier = _unnamed_logger_identifier;
    std::transform(unnamed_logger_identifier.begin(), unnamed_logger_identifier.end(), unnamed_logger_identifier.begin(),
                   [](const char c) { return std::tolower(c); });

    // Register LogLevel so that the formatters will be found.
    logging::register_simple_formatter_factory<LogLevel, char>("Severity");
    logging::register_simple_filter_factory<LogLevel>("Severity");

    // Create a sink backend that logs to a file
    auto& file_sink_backend = GetSinkBackend();
    file_sink_backend = boost::make_shared<TextFileSinkBackend>(
        keywords::file_name = log_file.c_str(),
        keywords::auto_flush = true
    );

    // Create the frontend for formatting default records.
    CreateFileSinkFrontEnd("");

    // Add global attributes to all records
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());

    SetLoggerThresholdCore("", default_LogLevel);

    // Initialize the internal logger
    ConfigureLogger(FO_GLOBAL_LOGGER_NAME(log)::get(), "log");

    // Create sink front ends for all previously created loggers.
    for(const auto& name : CreatedLoggersNames())
        CreateFileSinkFrontEnd(name);

    // Print setup message.
    auto date_time = std::time(nullptr);
    InfoLogger(log) << "Logger initialized at " << std::ctime(&date_time);
    InfoLogger() << FreeOrionVersionString();
}

void OverrideAllLoggersThresholds(const boost::optional<LogLevel>& threshold) {
    if (threshold)
        InfoLogger(log) << "Overriding the thresholds of all loggers to be " << to_string(*threshold);
    else
        InfoLogger(log) << "Removing override of loggers' thresholds.  Thresholds may now be changed to any value.";

    ForcedThreshold() = threshold;

    if (!threshold)
        return;

    SetLoggerThreshold("", *threshold);

    for (const auto& name : GetLoggersToSinkFrontEnds().LoggersNames())
        SetLoggerThreshold(name, *threshold);
}

LoggerCreatedSignalType LoggerCreatedSignal;

void ConfigureLogger(NamedThreadedLogger& logger, const std::string& name) {
    // Note: Do not log in this function.  If a logger is used during
    // static initialization it will cause boost::log to recursively call
    // its internal global_locker_storage mutex and lock up.
    SetLoggerThresholdCore(name, default_LogLevel);

    if (name.empty())
        return;

    CreateFileSinkFrontEnd(name);

    LoggerCreatedSignal(name);
}
