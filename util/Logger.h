// -*- C++ -*-
#ifndef _Logger_h_
#define _Logger_h_

#include <log4cpp/Category.hh>

#include "Export.h"

/** Initializes the logging system. Log to the given file.
 * If the file already exists it will be deleted. */
FO_COMMON_API void InitLogger(const std::string& logFile, const std::string& pattern);

/** Accessor for the App's logger */
FO_COMMON_API log4cpp::Category& Logger();

extern int g_indent;

/** A function that returns the correct amount of spacing for the current
  * indentation level during a dump. */
std::string DumpIndent();

/** Returns the integer priority level that should be passed to log4cpp for a given priority name string. */
FO_COMMON_API int PriorityValue(const std::string& name);

#endif // _Logger_h_
