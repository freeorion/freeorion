// -*- C++ -*-
#ifndef _Logger_h_
#define _Logger_h_

#include <log4cpp/Category.hh>

/** Initializes the logging system. Log to the given file.
 * If the file already exists it will be deleted. */
void InitLogger(const std::string& logFile, const std::string& pattern);

/** Accessor for the App's logger */
log4cpp::Category& Logger();

/** Returns the integer priority level that should be passed to log4cpp for a given priority name string. */
int PriorityValue(const std::string& name);

#endif // _Logger_h_
