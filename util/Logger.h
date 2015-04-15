// -*- C++ -*-
#ifndef _Logger_h_
#define _Logger_h_

#include <boost/log/trivial.hpp>

#include "Export.h"


/** Initializes the logging system. Log to the given file.
 * If the file already exists it will be deleted. */
FO_COMMON_API void InitLogger(const std::string& logFile, const std::string& pattern);

/** Accessors for the App's logger */
FO_COMMON_API void SetLoggerPriority(int priority);

#define TraceLogger()\
    BOOST_LOG_TRIVIAL(trace)

#define DebugLogger()\
    BOOST_LOG_TRIVIAL(debug)

#define ErrorLogger()\
    BOOST_LOG_TRIVIAL(error)

#define FatalLogger()\
    BOOST_LOG_TRIVIAL(fatal)

extern int g_indent;

/** A function that returns the correct amount of spacing for the current
  * indentation level during a dump. */
std::string DumpIndent();

/** Returns the integer priority level that should be passed for a given priority name string. */
FO_COMMON_API int PriorityValue(const std::string& name);

#endif // _Logger_h_
