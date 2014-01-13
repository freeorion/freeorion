// -*- C++ -*-
#ifndef _I18N_h_
#define _I18N_h_

#include <string>
#include <list>

#include <boost/format.hpp>

#include "Export.h"

/** Returns a language-specific string for the key-string \a str */
FO_COMMON_API const std::string& UserString(const std::string& str);

/** Returns a language-specific list of strings for the key-string \a str_list */
FO_COMMON_API void UserStringList(const std::string& str_list, std::list<std::string>& strings);

/** Clears all loaded strings, so that subsequent UserString lookups will cause
  * the stringtable(s) to be reloaded. */
FO_COMMON_API void FlushLoadedStringTables();

/** Placeholder for non local translations, evaluates to a non operation */
#define UserStringNop(key) key

/** Returns the language of the StringTable currently in use */
const std::string& Language();

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
FO_COMMON_API boost::format FlexibleFormat(const std::string& string_to_format);

/** Returns the stringified form of \a n as a roman number.  "Only" defined for 1 <= n <= 3999, as we can't display the
    symbol for 5000. */
FO_COMMON_API std::string RomanNumber(unsigned int n);

#endif // _I18N_h_
