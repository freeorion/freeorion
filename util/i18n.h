// -*- C++ -*-
#ifndef _I18N_h_
#define _I18N_h_

#include <string>

#include <boost/format.hpp>

/** Returns a language-specific string for the key-string \a str */
const std::string& UserString(const std::string& str);

/** Placeholder for non local translations, evaluates to a non operation */
#define UserStringNop(key) key

/** Returns the language of the StringTable currently in use */
const std::string& Language();

/** Wraps boost::format such that it won't crash if passed the wrong number of arguments */
boost::format FlexibleFormat(const std::string& string_to_format);

/** Returns the stringified form of \a n as a roman number.  "Only" defined for 1 <= n <= 3999, as we can't display the
    symbol for 5000. */
std::string RomanNumber(unsigned int n);

#endif // _I18N_h_
