#ifndef _Logger_h_
#define _Logger_h_

#include <boost/log/trivial.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>

#include "Export.h"


/** Initializes the logging system. Log to the given file.
 * If the file already exists it will be deleted. */
FO_COMMON_API void InitLogger(const std::string& logFile, const std::string& pattern);

/** Accessors for the App's logger */
FO_COMMON_API void SetLoggerPriority(int priority);

BOOST_LOG_ATTRIBUTE_KEYWORD(log_src_filename, "SrcFilename", std::string);
BOOST_LOG_ATTRIBUTE_KEYWORD(log_src_linenum, "SrcLinenum", int);

#define __BASE_FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define FO_LOGGER(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(),\
        (::boost::log::keywords::severity = ::boost::log::trivial::lvl)) <<\
        ::boost::log::add_value("SrcFilename", __BASE_FILENAME__) <<\
        ::boost::log::add_value("SrcLinenum", __LINE__)

#define TraceLogger()\
    FO_LOGGER(trace)

#define DebugLogger()\
    FO_LOGGER(debug)

#define ErrorLogger()\
    FO_LOGGER(error)

#define FatalLogger()\
    FO_LOGGER(fatal)

extern int g_indent;

/** A function that returns the correct amount of spacing for the current
  * indentation level during a dump. */
std::string DumpIndent();

/** Returns the integer priority level that should be passed for a given priority name string. */
FO_COMMON_API int PriorityValue(const std::string& name);

#endif // _Logger_h_
