#include "Logger.h"

#include "../util/Version.h"
#include "../util/OptionsDB.h"

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/formatter_parser.hpp>
#include <boost/log/utility/setup/filter_parser.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace expr = boost::log::expressions;
namespace attr = boost::log::attributes;
namespace keywords = boost::log::keywords;


namespace {
    logging::trivial::severity_level IntToSeverity(int num) {
        switch (num) {
        case 5:   return logging::trivial::fatal;   break;
        case 4:   return logging::trivial::error;   break;
        case 3:   return logging::trivial::warning; break;
        case 2:   return logging::trivial::info;    break;
        case 1:   return logging::trivial::debug;   break;
        default:  return logging::trivial::trace;
        }
    }

    int StringToSeverityInt(const std::string& text) {
        if (text == "FATAL")    return 5;
        if (text == "ERROR")    return 4;
        if (text == "WARN")     return 3;
        if (text == "INFO")     return 2;
        if (text == "DEBUG")    return 1;
        if (text == "TRACE")    return 0;
        return 0;
    }
}


int g_indent = 0;

std::string DumpIndent()
{ return std::string(g_indent * 4, ' '); }

void InitLogger(const std::string& logFile, const std::string& pattern) {
    logging::add_file_log(
        keywords::file_name = logFile.c_str(),
        keywords::auto_flush = true,
        keywords::format = expr::stream
            << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
            << " [" << expr::attr<logging::trivial::severity_level>("Severity") << "] "
            << pattern << " : " << expr::message
        );

    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::trace);
    logging::core::get()->add_global_attribute("TimeStamp", attr::local_clock()); 

    DebugLogger() << "Logger initialized";
    DebugLogger() << FreeOrionVersionString();

    int options_db_log_priority = PriorityValue(GetOptionsDB().Get<std::string>("log-level"));
    SetLoggerPriority(options_db_log_priority);
}

void SetLoggerPriority(int priority) {
    logging::trivial::severity_level i_severity = IntToSeverity(priority);
    logging::core::get()->set_filter(logging::trivial::severity >= i_severity);
}

int PriorityValue(const std::string& name)
{ return StringToSeverityInt(name); }

