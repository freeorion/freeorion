#include "i18n.h"

#include "Directories.h"
#include "Logger.h"
#include "OptionsDB.h"
#include "StringTable.h"

#include <boost/locale/generator.hpp>
#include <boost/locale/info.hpp>
#include <boost/locale/conversion.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace {
    std::string GetDefaultStringTableFileName()
    { return PathToString(GetResourceDir() / "stringtables" / "en.txt"); }


    /** Determines stringtable to use from users locale when stringtable filename is at default setting */
    void InitStringtableFileName() {
        // Only check on the first call
        static bool stringtable_filename_init { false };
        if (stringtable_filename_init)
            return;

        stringtable_filename_init = true;

        bool was_specified = false;
        if (!GetOptionsDB().IsDefaultValue("resource.stringtable.path"))
            was_specified = true;

        // Set the english stingtable as the default option
        GetOptionsDB().SetDefault("resource.stringtable.path", PathToString(GetResourceDir() / "stringtables/en.txt"));
        if (was_specified) {
            DebugLogger() << "Detected language: Previously specified " << GetOptionsDB().Get<std::string>("resource.stringtable.path");
            return;
        }

        boost::locale::generator gen;
        std::locale user_locale { gen("") };
        std::string lang {};
        try {
            lang = std::use_facet<boost::locale::info>(user_locale).language();
        } catch(std::bad_cast) {
            WarnLogger() << "Detected language: Bad cast, falling back to default";
            return;
        }

        boost::algorithm::to_lower(lang);

        // early return when determined language is empty or C locale
        if (lang.empty() || lang == "c" || lang == "posix") {
            WarnLogger() << "Detected lanuage: Not detected, falling back to default";
            return;
        }

        DebugLogger() << "Detected language: " << lang;

        boost::filesystem::path lang_filename { lang + ".txt" };
        boost::filesystem::path stringtable_file { GetResourceDir() / "stringtables" / lang_filename };

        if (IsExistingFile(stringtable_file))
            GetOptionsDB().Set("resource.stringtable.path", PathToString(stringtable_file));
        else
            WarnLogger() << "Stringtable file " << PathToString(stringtable_file)
                         << " not found, falling back to default";
    }

    std::string GetStringTableFileName() {
        InitStringtableFileName();

        std::string option_filename = GetOptionsDB().Get<std::string>("resource.stringtable.path");
        if (option_filename.empty())
            return GetDefaultStringTableFileName();
        else
            return option_filename;
    }

    std::map<std::string, const StringTable_*> stringtables;

    const StringTable_& GetStringTable(std::string stringtable_filename = "") {
        // get option-configured stringtable if no filename specified
        if (stringtable_filename.empty())
            stringtable_filename = GetStringTableFileName();

        // ensure the default stringtable is loaded first
        auto default_stringtable_it = stringtables.find(GetDefaultStringTableFileName());
        if (default_stringtable_it == stringtables.end()) {
            auto table = new StringTable_(GetDefaultStringTableFileName());
            stringtables[GetDefaultStringTableFileName()] = table;
            default_stringtable_it = stringtables.find(GetDefaultStringTableFileName());
        }

        // attempt to find requested stringtable...
        auto it = stringtables.find(stringtable_filename);
        if (it != stringtables.end())
            return *(it->second);

        // if not already loaded, load, store, and return,
        // using default stringtable for fallback expansion lookups
        auto table = new StringTable_(stringtable_filename, default_stringtable_it->second);
        stringtables[stringtable_filename] = table;

        return *table;
    }

    const StringTable_& GetDefaultStringTable()
    { return GetStringTable(GetDefaultStringTableFileName()); }
}

void FlushLoadedStringTables()
{ stringtables.clear(); }

const std::string& UserString(const std::string& str) {
    if (GetStringTable().StringExists(str))
        return GetStringTable().String(str);
    return GetDefaultStringTable().String(str);
}

std::vector<std::string> UserStringList(const std::string& key) {
    std::vector<std::string> result;
    std::istringstream template_stream(UserString(key));
    std::string item;
    while (std::getline(template_stream, item))
        result.push_back(item);
    return result;
}

bool UserStringExists(const std::string& str) {
    if (GetStringTable().StringExists(str))
        return true;
    return GetDefaultStringTable().StringExists(str);
}

boost::format FlexibleFormat(const std::string &string_to_format) {
    try {
        boost::format retval(string_to_format);
        retval.exceptions(boost::io::no_error_bits);
        return retval;
    } catch (const std::exception& e) {
        ErrorLogger() << "FlexibleFormat caught exception when formatting: " << e.what();
    }
    boost::format retval(UserString("ERROR"));
    retval.exceptions(boost::io::no_error_bits);
    return retval;
}

const std::string& Language()
{ return GetStringTable().Language(); }

std::string RomanNumber(unsigned int n) {
    //letter pattern (N) and the associated values (V)
    static const std::string  N[] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};
    static const unsigned int V[] = {1000,  900, 500,  400, 100,   90,  50,   40,  10,    9,   5,    4,   1};
    unsigned int remainder = n; //remainder of the number to be written
    int i = 0;                  //pattern index
    std::string retval = "";;
    if (n == 0) return "";      //the romans didn't know there is a zero, read a book about history of the zero if you want to know more
                                //Roman numbers are written using patterns, you chosse the highest pattern lower that the number
                                //write it down, and substract it's value until you reach zero.

    // safety check to avoid very long loops
    if (n > 10000)
        return "!";

    //we start with the highest pattern and reduce the size every time it doesn't fit
    while (remainder > 0) {
        //check if number is larger than the actual pattern value
        if (remainder >= V[i]) {
            //write pattern down
            retval += N[i];
            //reduce number
            remainder -= V[i];
        } else {
            //we need the next pattern
            i++;
        }
    }
    return retval;
}

namespace {
    const double SMALL_UI_DISPLAY_VALUE = 1.0e-6;
    const double LARGE_UI_DISPLAY_VALUE = 9.99999999e+9;
    const double UNKNOWN_UI_DISPLAY_VALUE = std::numeric_limits<double>::infinity();
}

std::string DoubleToString(double val, int digits, bool always_show_sign) {
    std::string text; // = ""

    // minimum digits is 2.  If digits was 1, then 30 couldn't be displayed,
    // as 0.1k is too much and 9 is too small and just 30 is 2 digits
    digits = std::max(digits, 2);

    // default result for sentinel value
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return UserString("UNKNOWN_VALUE_SYMBOL");

    double mag = std::abs(val);

    // early termination if magnitude is 0
    if (mag == 0.0) {
        std::string format = "%1." + std::to_string(digits - 1) + "f";
        text += (boost::format(format) % mag).str();
        return text;
    }

    // prepend signs if neccessary
    int effective_sign = EffectiveSign(val);
    if (effective_sign == -1) {
        text += "-";
    } else {
        if (always_show_sign) text += "+";
    }

    if (mag > LARGE_UI_DISPLAY_VALUE) mag = LARGE_UI_DISPLAY_VALUE;

    // if value is effectively 0, avoid unnecessary later processing
    if (effective_sign == 0) {
        text = "0.0";
        for (int n = 2; n < digits; ++n)
            text += "0";  // fill in 0's to required number of digits
        return text;
    }


    //std::cout << std::endl << "DoubleToString val: " << val << " digits: " << digits << std::endl;

    // power of 10 of highest valued digit in number
    int pow10 = static_cast<int>(floor(log10(mag))); // = 2 (100's) for 234.4,  = 4 (10000's) for 45324
    //std::cout << "magnitude power of 10: " << pow10 << std::endl;

    // determine base unit for number: the next lower power of 10^3 from the number (inclusive)
    int pow10_digits_above_pow1000 = 0;
    if (pow10 >= 0)
        pow10_digits_above_pow1000 = pow10 % 3;
    else
        pow10_digits_above_pow1000 = (pow10 % 3) + 3;   // +3 ensures positive result of mod
    int unit_pow10 = pow10 - pow10_digits_above_pow1000;
    if (unit_pow10 < 0)
        unit_pow10 = 0;
    //std::cout << "unit power of 10: " << unit_pow10 << std::endl;

    // if not enough digits to include most significant digit and next lower
    // power of 10, add extra digits. this still uses less space than using the
    // next higher power of 10 and adding a 0. out front.  for example, 240 with
    // 2 digits is better shown as "240" than "0.24k"
    digits = std::max(digits, pow10_digits_above_pow1000 + 1);
    //std::cout << "adjusted digits: " << digits << std::endl;

    int lowest_digit_pow10 = pow10 - digits + 1;
    //std::cout << "lowest_digit_pow10: " << lowest_digit_pow10 << std::endl;

    // fraction digits:
    int fraction_digits = std::max(0, std::min(digits - 1, unit_pow10 - lowest_digit_pow10));
    //std::cout << "fraction_digits: " << fraction_digits << std::endl;


    /* round number down at lowest digit to be displayed, to prevent lexical_cast from rounding up
       in cases like 0.998k with 2 digits -> 1.00k  instead of  0.99k  (as it should be) */
    double rounding_factor = pow(10.0, static_cast<double>(pow10 - digits + 1));
    mag /= rounding_factor;
    mag = floor(mag);
    mag *= rounding_factor;

    // scale number by unit power of 10
    mag /= pow(10.0, static_cast<double>(unit_pow10));  // if mag = 45324 and unitPow = 3, get mag = 45.324


    std::string format;
    format += "%" + std::to_string(digits) + "." +
                    std::to_string(fraction_digits) + "f";
    text += (boost::format(format) % mag).str();

    // append base scale SI prefix (as postfix)
    switch (unit_pow10) {
    case -15:
        text += "f";        // femto
        break;
    case -12:
        text += "p";        // pico
        break;
    case -9:
        text += "n";        // nano
        break;
    case -6:
        text += "\xC2\xB5"; // micro.  mu in UTF-8
        break;
    case -3:
        text += "m";        // milli
        break;
    case 3:
        text += "k";        // kilo
        break;
    case 6:
        text += "M";        // Mega
        break;
    case 9:
        text += "G";        // Giga
        break;
    case 12:
        text += "T";        // Terra
        break;
    default:
        break;
    }
    return text;
}

int EffectiveSign(double val) {
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return 0;

    if (std::abs(val) >= SMALL_UI_DISPLAY_VALUE) {
        if (val >= 0)
            return 1;
        else
            return -1;
    } else {
        return 0;
    }
}

