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

#include <ctime>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace attr = boost::log::attributes;
namespace keywords = boost::log::keywords;


namespace {

    // internal_logger is intentionally omitted from all converters.  It is only used internally.
    // Hence the name.
#define InternalLogger(name) FO_LOGGER(name, LogLevel::internal_logger)

    // Compile time constant pointers to constant char arrays.
    constexpr const char* const log_level_names[] = {"debug", "info ", "warn ", "error", " log "};

    DiscreteValidator<std::string> LogLevelValidator() {
        std::set<std::string> valid_levels = {"debug", "info", "warn", "error",
                                              "DEBUG", "INFO", "WARN", "ERROR",
                                              "0", "1", "2", "3" };
        auto validator = DiscreteValidator<std::string>(valid_levels);
        return validator;
    }

}

std::string to_string(const LogLevel level)
{ return log_level_names[static_cast<std::size_t>(level)]; }

LogLevel to_LogLevel(const std::string& text) {
    if (text == "error")    return LogLevel::error;
    if (text == "warn")     return LogLevel::warn;
    if (text == "info")     return LogLevel::info;
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

void InitLoggingSystem(const std::string& logFile, const std::string& _root_logger_name) {
    std::string root_logger_name = _root_logger_name;
    std::transform(root_logger_name.begin(), root_logger_name.end(), root_logger_name.begin(),
                   [](const char c) { return std::tolower(c); });

    // Register LogLevel so that the formatters will be found.
    logging::register_simple_formatter_factory<LogLevel, char>("Severity");
    logging::register_simple_filter_factory<LogLevel>("Severity");

    logging::add_file_log(
        keywords::file_name = logFile.c_str(),
        keywords::auto_flush = true,
        keywords::format = expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%H:%M:%S.%f")
            << " [" << log_severity << "] "
            << root_logger_name << " : " << log_src_filename << ":" << log_src_linenum << " : "
            << expr::message
    );

    logging::core::get()->set_filter(log_severity >= LogLevel::debug);
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock());

    auto date_time = std::time(nullptr);
    DebugLogger() << "Logger initialized at " << std::ctime(&date_time);
    InfoLogger() << FreeOrionVersionString();

    LogLevel options_db_log_priority = to_LogLevel(GetOptionsDB().Get<std::string>("log-level"));
    SetLogFileSinkPriority(options_db_log_priority);

    // Setup the OptionsDB options for the file sink.
    const std::string sink_option_name = "logging.sinks." + root_logger_name;
    GetOptionsDB().Add<std::string>(
        sink_option_name, UserStringNop("OPTIONS_DB_LOGGER_FILE_SINK_LEVEL"),
        "info", LogLevelValidator());

    // Setup the OptionsDB options for this default source.
    const std::string source_option_name = "logging.sources." + root_logger_name;
    GetOptionsDB().Add<std::string>(
        source_option_name, UserStringNop("OPTIONS_DB_LOGGER_SOURCE_LEVEL"),
        "debug", LogLevelValidator());

}

void SetLogFileSinkPriority(LogLevel priority) {
    logging::core::get()->set_filter(log_severity >= priority);
}

/** Sets the \p priority of \p source.  \p source == "" is the default logger.*/
FO_COMMON_API void SetLoggerSourcePriority(const std::string & source, LogLevel priority) {
    //FIXME
}

void RegisterLoggerWithOptionsDB(const std::string& name) {
    if (name.empty())
        return;

    // Setup the OptionsDB options for this source.
    const std::string option_name = "logging.sources." + name;
    GetOptionsDB().Add<std::string>(
        option_name, UserStringNop("OPTIONS_DB_LOGGER_SOURCE_LEVEL"),
        "info", LogLevelValidator());
}
