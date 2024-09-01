#include "Logger.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
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
#include <boost/unordered_map.hpp>

#include "Directories.h"

#ifdef _MSC_VER
#  include <ctime>
#else
#  include <time.h>
#endif
#include <mutex>
#include <regex>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace attr = boost::log::attributes;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

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
}


// Provide a LogLevel input formatter for filtering
template<typename CharT, typename TraitsT>
inline std::basic_istream<CharT, TraitsT >& operator>>(std::basic_istream<CharT, TraitsT>& is, LogLevel& level)
{
    std::string tmp;
    is >> tmp; // to_string(level) ...?
    level = to_LogLevel(tmp);
    return is;
}

#if !defined(CONSTINIT_STRING)
#  if defined(__cpp_lib_constexpr_string) && ((!defined(__GNUC__) || (__GNUC__ > 11))) && ((!defined(_MSC_VER) || (_MSC_VER >= 1934)))
#    define CONSTINIT_STRING constinit
#  else
#    define CONSTINIT_STRING
#  endif
#endif

namespace {
    std::string& LocalUnnamedLoggerIdentifier() noexcept {
        // Create default logger name as a static function variable to avoid static initialization fiasco
        static CONSTINIT_STRING std::string unnamed_logger_identifier;
        return unnamed_logger_identifier;
    }

    const std::string& DisplayName(const std::string& channel_name) noexcept 
    { return (channel_name.empty() ? LocalUnnamedLoggerIdentifier() : channel_name); }

    boost::optional<LogLevel>& ForcedThreshold() noexcept {
        // Create forced threshold as a static function variable to avoid static initialization fiasco
        static boost::optional<LogLevel> forced_threshold = boost::none;
        return forced_threshold;
    }

    using LoggerTextFileSinkFrontend = boost::log::sinks::synchronous_sink<boost::log::sinks::text_file_backend>;

    using LoggerFileSinkFrontEndConfigurer = std::function<void(LoggerTextFileSinkFrontend& sink_frontend)>;

    boost::shared_ptr<LoggerTextFileSinkFrontend::sink_backend_type>& FileSinkBackend() noexcept {
        // Create the sink backend as a function local static variable to avoid the static
        // initilization fiasco.
        static boost::shared_ptr<LoggerTextFileSinkFrontend::sink_backend_type> m_sink_backend;
        return m_sink_backend;
    }

    /** Create a new file sink front end for \p file_sink_backend for \p channel_name and
        configure it with \p configure_front_end. */
    void ConfigureToFileSinkFrontEndCore(const boost::shared_ptr<LoggerTextFileSinkFrontend::sink_backend_type>& file_sink_backend,
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
        std::mutex m_mutex;
        boost::unordered_map<std::string, boost::shared_ptr<LoggerTextFileSinkFrontend>> m_names_to_front_ends;
        boost::unordered_map<std::string, LoggerFileSinkFrontEndConfigurer> m_names_to_front_end_configurers;
    public:

        void AddOrReplaceLoggerName(const std::string& channel_name,
                                    boost::shared_ptr<LoggerTextFileSinkFrontend> front_end = nullptr)
        {
            std::scoped_lock lock(m_mutex);

            // Remove the old front end if it is different.
            const auto& name_and_old_frontend = m_names_to_front_ends.find(channel_name);
            if (name_and_old_frontend != m_names_to_front_ends.end()) {

                if (front_end == name_and_old_frontend->second)
                    return;

                logging::core::get()->remove_sink(name_and_old_frontend->second);
                m_names_to_front_ends.erase(name_and_old_frontend);
            }

            m_names_to_front_ends.emplace(channel_name, front_end);

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
            std::scoped_lock lock(m_mutex);

            // Remove the old front end if it is different.
            m_names_to_front_end_configurers.erase(channel_name);
            m_names_to_front_end_configurers.emplace(channel_name, configure_front_end);
        }

        /** Configure front ends for any logger with stored configuration functions. */
        void ConfigureFrontEnds(const boost::shared_ptr<LoggerTextFileSinkFrontend::sink_backend_type>& file_sink_backend) {
            for (const auto& [name, conf]: m_names_to_front_end_configurers)
                ConfigureToFileSinkFrontEndCore(file_sink_backend, name, conf);
        }

        std::vector<std::string> LoggersNames() {
            std::scoped_lock lock(m_mutex);

            std::vector<std::string> retval;
            retval.reserve(m_names_to_front_ends.size());
            for (const auto& name_and_frontend : m_names_to_front_ends)
                retval.push_back(name_and_frontend.first);
            return retval;
        }

        void ShutdownFileSinks() {
            std::scoped_lock lock(m_mutex);

            for (const auto& name_and_frontend : m_names_to_front_ends)
                logging::core::get()->remove_sink(name_and_frontend.second);
        }

    };

    LoggersToSinkFrontEnds& GetLoggersToSinkFrontEnds() {
        // Create loggers_names_to_front_ends as a static function variable to avoid static initialization fiasco.
        static LoggersToSinkFrontEnds loggers_names_to_front_ends{};
        return loggers_names_to_front_ends;
    }

    void ConfigureToFileSinkFrontEndCore(const boost::shared_ptr<LoggerTextFileSinkFrontend::sink_backend_type>& file_sink_backend,
                                         const std::string& channel_name,
                                         const LoggerFileSinkFrontEndConfigurer& configure_front_end)
    {
        // Create a sink frontend for formatting.
        auto sink_frontend = boost::make_shared<LoggerTextFileSinkFrontend>(file_sink_backend);

        configure_front_end(*sink_frontend);

        // Replace any previous frontend for this channel
        GetLoggersToSinkFrontEnds().AddOrReplaceLoggerName(channel_name, std::move(sink_frontend));
    }
}

void ApplyConfigurationToFileSinkFrontEnd(const std::string& channel_name,
                                          const LoggerFileSinkFrontEndConfigurer& configure_front_end)
{
    if (const auto& file_sink_backend = FileSinkBackend()) {
        ConfigureToFileSinkFrontEndCore(file_sink_backend, channel_name, configure_front_end);
    } else {
        // If the file sink backend has not been configured store the name so
        // that a frontend can be added later.
        GetLoggersToSinkFrontEnds().StoreConfigurerWithLoggerName(channel_name, configure_front_end);
    }
}

const std::string& DefaultExecLoggerName() noexcept
{ return LocalUnnamedLoggerIdentifier(); }

std::vector<std::string> CreatedLoggersNames()
{ return GetLoggersToSinkFrontEnds().LoggersNames(); }

BOOST_LOG_ATTRIBUTE_KEYWORD(log_severity, "Severity", LogLevel);
BOOST_LOG_ATTRIBUTE_KEYWORD(log_channel, "Channel", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(log_src_filename, "SrcFilename", std::string);
BOOST_LOG_ATTRIBUTE_KEYWORD(log_src_linenum, "SrcLinenum", int);
BOOST_LOG_ATTRIBUTE_KEYWORD(thread_id, "ThreadID", boost::log::attributes::current_thread_id::value_type);

namespace {
    std::mutex severity_filter_mutex; /// guards severity_filter
    expr::channel_severity_filter_actor<std::string, LogLevel> severity_filter =
        expr::channel_severity_filter(log_channel, log_severity);

    void SetLoggerThresholdCore(const std::string& source, LogLevel threshold) {
        std::scoped_lock lock(severity_filter_mutex);

        auto used_threshold = ForcedThreshold() ? *ForcedThreshold() : threshold;
        severity_filter[source] = used_threshold;
        logging::core::get()->set_filter(severity_filter);
    }

    void ConfigureFileSinkFrontEnd(LoggerTextFileSinkFrontend& sink_frontend,
                                   const std::string& channel_name)
    {
        // Create the format
        sink_frontend.set_formatter(
            expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
            << " {" << thread_id << "}"
            << " [" << log_severity << "] "
            << DisplayName(channel_name)
            << " : " << log_src_filename << ":" << log_src_linenum << " : "
            << expr::message
        );

        // Set a filter to only format this channel
        sink_frontend.set_filter(log_channel == channel_name);
    }
}

void SetLoggerThreshold(const std::string& source, LogLevel threshold) {
    SetLoggerThresholdCore(source, threshold);
    InfoLogger(log) << "Setting \"" << source << "\" logger threshold to \"" << threshold << "\".";
}

void InitLoggingSystem(const std::string& log_file, std::string_view _unnamed_logger_identifier) {
    auto& unnamed_logger_identifier = LocalUnnamedLoggerIdentifier();
    unnamed_logger_identifier = _unnamed_logger_identifier;
    std::transform(unnamed_logger_identifier.begin(), unnamed_logger_identifier.end(),
                   unnamed_logger_identifier.begin(), // in-place replacement
                   [](const char c) { return std::tolower(c); });

    // Register LogLevel so that the formatters will be found.
    logging::register_simple_formatter_factory<LogLevel, char>("Severity");
    logging::register_simple_filter_factory<LogLevel>("Severity");

    // Create a sink backend that logs to a file
    auto& file_sink_backend = FileSinkBackend();
    file_sink_backend = boost::make_shared<LoggerTextFileSinkFrontend::sink_backend_type>(
        keywords::file_name = FilenameToPath(log_file),
        keywords::auto_flush = true
    );

    // Create the frontend for formatting default records.
    ApplyConfigurationToFileSinkFrontEnd("",
        [](LoggerTextFileSinkFrontend& sink_frontend)
        { return ConfigureFileSinkFrontEnd(sink_frontend, ""); });

    // Add global attributes to all records
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());
    logging::core::get()->add_global_attribute("ThreadID", attr::current_thread_id());

    SetLoggerThresholdCore("", default_log_level_threshold);

    // Initialize the logging system's logger
    ConfigureLogger(FO_GLOBAL_LOGGER_NAME(log)::get(), "log");

    // Create sink front ends for all previously created loggers.
    GetLoggersToSinkFrontEnds().ConfigureFrontEnds(file_sink_backend);

    {
        // Log setup message with timestamp. Doing all the following because it
        // correctly handles time zones with daylight saving time adjustment for
        // me, but the simpler-to-use boost equivalents for formatting time are
        // off by 1 hour. Also can't use just std::datetime because it is not
        // always implemented thread-safe.
        auto date_time = std::time(nullptr);
        std::tm temp_tm;
        #ifdef _MSC_VER
            localtime_s(&temp_tm, &date_time);
        #else
            localtime_r(&date_time, &temp_tm);
        #endif

        char time_as_string_buf[100] = {};
        std::strftime(time_as_string_buf, sizeof(time_as_string_buf), "%c", &temp_tm);
        InfoLogger(log) << "Logger initialized at " << time_as_string_buf;
    }
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

void OverrideAllLoggersThresholds(boost::optional<LogLevel> threshold) {
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

    ApplyConfigurationToFileSinkFrontEnd(name,
        [name](LoggerTextFileSinkFrontend& sink_frontend)
        { return ConfigureFileSinkFrontEnd(sink_frontend, name); });

    // Store as static to initialize once.
    static bool dummy = InitializeLoggerCreatedSignal();
    (void)dummy; // Hide unused variable warning

    LoggerCreatedSignal(name);
}

