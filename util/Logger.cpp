#include "Logger.h"

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

// TODO consider adding thread and process id as options

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

    std::stringstream InvalidLogLevelWarning(const std::string& text) {
        std::stringstream ss;
        ss << "\"" << text <<"\" is not a valid log level. "
           << "Valid levels are ";

        for (int ii = static_cast<int>(LogLevel::min); ii <= static_cast<int>(LogLevel::max); ++ii) {
            auto log_level = static_cast<LogLevel>(ii);
            auto name = to_string(log_level);

            // Add commas between names
            if (ii != static_cast<int>(LogLevel::min) && ii != static_cast<int>(LogLevel::max))
                ss << ", ";

            // Except before the last name
            if (ii != static_cast<int>(LogLevel::min) && ii == static_cast<int>(LogLevel::max))
                ss << " and ";

            ss << name;
        }

        ss << ".";
        return ss;
    }
}


std::string to_string(const LogLevel level)
{ return log_level_names[static_cast<std::size_t>(level)]; }


LogLevel to_LogLevel(const std::string& text) {

    // Use a static local variable so that during static initialization it
    // is initialized on first use in any compilation unit.
    static std::unordered_map<std::string, LogLevel> string_to_log_level = ValidNameToLogLevel();

    auto it = string_to_log_level.find(text);
    if (it != string_to_log_level.end())
        return it->second;

    WarnLogger(log) << InvalidLogLevelWarning(text).str();
    return LogLevel::debug;
}

std::unordered_map<std::string, LogLevel> ValidNameToLogLevel() {
    std::unordered_map<std::string, LogLevel> retval{};

    for (int ii = static_cast<int>(LogLevel::min); ii <= static_cast<int>(LogLevel::max); ++ii) {
        auto log_level = static_cast<LogLevel>(ii);

        //Insert the number
        retval.emplace(std::to_string(ii), log_level);

        // Insert the lower case
        auto name = to_string(log_level);
        retval.emplace(name, log_level);

        // Insert the upper case
        std::transform(name.begin(), name.end(), name.begin(),
                       [](const char c) { return std::toupper(c); });
        retval.emplace(name, log_level);
    }
    return retval;
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

namespace {
    std::string& LocalUnnamedLoggerIdentifier() {
        // Create default logger name as a static function variable to avoid static initialization fiasco
        static std::string unnamed_logger_identifier;
        return unnamed_logger_identifier;
    }

    const std::string& DisplayName(const std::string& channel_name)
    { return (channel_name.empty() ? LocalUnnamedLoggerIdentifier() : channel_name); }

    boost::optional<LogLevel>& ForcedThreshold() {
        // Create forced threshold as a static function variable to avoid static initialization fiasco
        static boost::optional<LogLevel> forced_threshold = boost::none;
        return forced_threshold;
    }

    using TextFileSinkBackend  = sinks::text_file_backend;
    using TextFileSinkFrontend = sinks::synchronous_sink<TextFileSinkBackend>;

    boost::shared_ptr<TextFileSinkBackend>& FileSinkBackend() {
        // Create the sink backend as a function local static variable to avoid the static
        // initilization fiasco.
        static boost::shared_ptr<TextFileSinkBackend> m_sink_backend;
        return m_sink_backend;
    }

    /** Create a new file sink front end for \p file_sink_backend for \p channel_name and
        configure it with \p configure_front_end. */
    void ConfigureToFileSinkFrontEndCore(const boost::shared_ptr<TextFileSinkBackend>& file_sink_backend,
                                         const std::string& channel_name,
                                         const LoggerFileSinkFrontEndConfigurer& configure_front_end);

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
        std::unordered_map<std::string, LoggerFileSinkFrontEndConfigurer> m_names_to_front_end_configurers = {};
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

            InfoLogger(log) << "Added logger named \"" << DisplayName(channel_name) << "\"";
        }

        /** Store a configuration function \p configure_front_end to be applied later. */
        void StoreConfigurerWithLoggerName(const std::string& channel_name,
                                           const LoggerFileSinkFrontEndConfigurer& configure_front_end)
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            // Remove the old front end if it is different.
            m_names_to_front_end_configurers.erase(channel_name);
            m_names_to_front_end_configurers.insert({channel_name, configure_front_end});
        }

        /** Configure front ends for any logger with stored configuration functions. */
        void ConfigureFrontEnds(const boost::shared_ptr<TextFileSinkBackend>& file_sink_backend) {
            for (const auto& name_and_conf: m_names_to_front_end_configurers)
                ConfigureToFileSinkFrontEndCore(file_sink_backend, name_and_conf.first, name_and_conf.second);
        }

        std::vector<std::string> LoggersNames() {
            std::lock_guard<std::mutex> lock(m_mutex);

            std::vector<std::string> retval;
            for (const auto& name_and_frontend : m_names_to_front_ends)
                retval.push_back(name_and_frontend.first);
            return retval;
        }

        void ShutdownFileSinks() {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (const auto& name_and_frontend : m_names_to_front_ends)
                logging::core::get()->remove_sink(name_and_frontend.second);
        }

    };

    LoggersToSinkFrontEnds& GetLoggersToSinkFrontEnds() {
        // Create loggers_names_to_front_ends as a static function variable to avoid static initialization fiasco.
        static LoggersToSinkFrontEnds loggers_names_to_front_ends{};
        return loggers_names_to_front_ends;
    }

    void ConfigureFileSinkFrontEnd(TextFileSinkFrontend& sink_frontend, const std::string& channel_name) {
        // Create the format
        sink_frontend.set_formatter(
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
            << " [" << log_severity << "] "
            << DisplayName(channel_name)
            << " : " << log_src_filename << ":" << log_src_linenum << " : "
            << expr::message
        );

        // Set a filter to only format this channel
        sink_frontend.set_filter(log_channel == channel_name);
    }

    void ConfigureToFileSinkFrontEndCore(const boost::shared_ptr<TextFileSinkBackend>& file_sink_backend,
                                         const std::string& channel_name,
                                         const LoggerFileSinkFrontEndConfigurer& configure_front_end)
    {
        // Create a sink frontend for formatting.
        auto sink_frontend = boost::make_shared<TextFileSinkFrontend>(file_sink_backend);

        configure_front_end(*sink_frontend);

        // Replace any previous frontend for this channel
        GetLoggersToSinkFrontEnds().AddOrReplaceLoggerName(channel_name, sink_frontend);
    }
}

void ApplyConfigurationToFileSinkFrontEnd(const std::string& channel_name,
                                          const LoggerFileSinkFrontEndConfigurer& configure_front_end)
{
    auto& file_sink_backend = FileSinkBackend();

    // If the file sink backend has not been configured store the name so
    // that a frontend can be added later.
    if (!file_sink_backend) {
        GetLoggersToSinkFrontEnds().StoreConfigurerWithLoggerName(channel_name, configure_front_end);
        return;
    }

    ConfigureToFileSinkFrontEndCore(file_sink_backend, channel_name, configure_front_end);
}

const std::string& DefaultExecLoggerName()
{ return LocalUnnamedLoggerIdentifier(); }

std::vector<std::string> CreatedLoggersNames()
{ return GetLoggersToSinkFrontEnds().LoggersNames(); }

namespace {

    /** LoggerThresholdSetter sets the threshold of a logger */
    class LoggerThresholdSetter {
        /// m_mutex serializes access from different threads
        std::mutex m_mutex = {};

        // Create a minimum severity table filter
        expr::channel_severity_filter_actor<std::string, LogLevel>
        m_min_channel_severity = expr::channel_severity_filter(log_channel, log_severity);

        public:
        // Set the logger threshold and return the logger name and threshold used.
        std::pair<std::string, LogLevel> SetThreshold(const std::string& source, LogLevel threshold) {
            std::lock_guard<std::mutex> lock(m_mutex);

            auto used_threshold = ForcedThreshold() ? *ForcedThreshold() : threshold;
            logging::core::get()->reset_filter();
            m_min_channel_severity[source] = used_threshold;
            logging::core::get()->set_filter(m_min_channel_severity);

            return {DisplayName(source), used_threshold};
        }

    };

    // Set the logger threshold and return the logger name and threshold used.
    std::pair<std::string, LogLevel> SetLoggerThresholdCore(const std::string& source, LogLevel threshold) {
        // Create logger_threshold_setter as a static variable to avoid the static initialization fiasco.
        static LoggerThresholdSetter logger_threshold_setter{};

        return logger_threshold_setter.SetThreshold(source, threshold);
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
    auto& file_sink_backend = FileSinkBackend();
    file_sink_backend = boost::make_shared<TextFileSinkBackend>(
        keywords::file_name = log_file.c_str(),
        keywords::auto_flush = true
    );

    // Create the frontend for formatting default records.
    ApplyConfigurationToFileSinkFrontEnd("", std::bind(ConfigureFileSinkFrontEnd, std::placeholders::_1, ""));

    // Add global attributes to all records
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());

    SetLoggerThresholdCore("", default_log_level_threshold);

    // Initialize the logging system's logger
    ConfigureLogger(FO_GLOBAL_LOGGER_NAME(log)::get(), "log");

    // Create sink front ends for all previously created loggers.
    GetLoggersToSinkFrontEnds().ConfigureFrontEnds(file_sink_backend);

    // Print setup message.
    auto date_time = std::time(nullptr);
    InfoLogger(log) << "Logger initialized at " << std::ctime(&date_time);
}

void ShutdownLoggingSystemFileSink() {
    // The file sink may not be safe to use during static deinitialization, because of the
    // following bug:

    // http://www.boost.org/doc/libs/1_64_0/libs/log/doc/html/log/rationale/why_crash_on_term.html
    // https://svn.boost.org/trac/boost/ticket/8642
    // https://svn.boost.org/trac/boost/ticket/9119

    // When either ticket is fixed the ShutdownLoggingSystem() function can be removed.

    GetLoggersToSinkFrontEnds().ShutdownFileSinks();
}

void OverrideAllLoggersThresholds(const boost::optional<LogLevel>& threshold) {
    if (threshold)
        InfoLogger(log) << "Overriding the thresholds of all loggers to be " << to_string(*threshold);
    else
        InfoLogger(log) << "Removing override of loggers' thresholds.  Thresholds may now be changed to any value.";

    ForcedThreshold() = threshold;

    if (!threshold)
        return;

    SetLoggerThresholdCore("", *threshold);

    for (const auto& name : GetLoggersToSinkFrontEnds().LoggersNames())
        SetLoggerThresholdCore(name, *threshold);
}

LoggerCreatedSignalType LoggerCreatedSignal;

namespace {
    // Initialize LoggerCreatedSignal.  During static initialization another
    // compilation unit might call ConfigureLogger() before Logger.cpp has been
    // initialized.
    bool InitializeLoggerCreatedSignal() {
        LoggerCreatedSignal = LoggerCreatedSignalType();
        return true;
    };
}

void ConfigureLogger(NamedThreadedLogger& logger, const std::string& name) {
    // Note: Do not log in this function.  If a logger is used during
    // static initialization it will cause boost::log to recursively call
    // its internal global_locker_storage mutex and lock up.
    SetLoggerThresholdCore(name, default_log_level_threshold);

    if (name.empty())
        return;

    ApplyConfigurationToFileSinkFrontEnd(name, std::bind(ConfigureFileSinkFrontEnd, std::placeholders::_1, name));

    // Store as static to initialize once.
    static bool dummy = InitializeLoggerCreatedSignal();
    (void)dummy; // Hide unused variable warning

    LoggerCreatedSignal(name);
}

