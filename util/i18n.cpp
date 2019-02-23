#include "i18n.h"

#include "Directories.h"
#include "Logger.h"
#include "OptionsDB.h"
#include "StringTable.h"

#include <boost/locale.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include <mutex>

namespace {
    std::string GetDefaultStringTableFileName()
    { return PathToString(GetResourceDir() / "stringtables" / "en.txt"); }

    std::map<std::string, std::shared_ptr<const StringTable>>  stringtables;
    std::recursive_mutex                                       stringtable_access_mutex;
    bool                                                       stringtable_filename_init = false;

    /** Determines stringtable to use from users locale when stringtable filename is at default setting */
    void InitStringtableFileName() {
        // Only check on the first call
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

        std::string lang {};
        try {
            lang = std::use_facet<boost::locale::info>(GetLocale()).language();
        } catch(const std::bad_cast&) {
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

        boost::filesystem::path lang_filename{ lang + ".txt" };
        boost::filesystem::path stringtable_file{ GetResourceDir() / "stringtables" / lang_filename };

        if (IsExistingFile(stringtable_file))
            GetOptionsDB().Set("resource.stringtable.path", PathToString(stringtable_file));
        else
            WarnLogger() << "Stringtable file " << PathToString(stringtable_file)
                         << " not found, falling back to default";
    }

    std::string GetStringTableFileName() {
        std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
        InitStringtableFileName();

        std::string option_filename = GetOptionsDB().Get<std::string>("resource.stringtable.path");
        if (option_filename.empty())
            return GetDefaultStringTableFileName();
        else
            return option_filename;
    }

    const StringTable& GetStringTable(std::string stringtable_filename = "") {
        std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);

        // get option-configured stringtable if no filename specified
        if (stringtable_filename.empty())
            stringtable_filename = GetStringTableFileName();

        // ensure the default stringtable is loaded first
        auto default_stringtable_it = stringtables.find(GetDefaultStringTableFileName());
        if (default_stringtable_it == stringtables.end()) {
            auto table = std::make_shared<StringTable>(GetDefaultStringTableFileName());
            stringtables[GetDefaultStringTableFileName()] = table;
            default_stringtable_it = stringtables.find(GetDefaultStringTableFileName());
        }

        // attempt to find requested stringtable...
        auto it = stringtables.find(stringtable_filename);
        if (it != stringtables.end())
            return *(it->second);

        // if not already loaded, load, store, and return,
        // using default stringtable for fallback expansion lookups
        auto table = std::make_shared<StringTable>(stringtable_filename, default_stringtable_it->second);
        stringtables[stringtable_filename] = table;

        return *table;
    }

    const StringTable& GetDefaultStringTable()
    { return GetStringTable(GetDefaultStringTableFileName()); }
}

std::locale GetLocale(const std::string& name) {
    static bool locale_init { false };
    // Initialize backend and generator on first use, provide a log for current enivornment locale
    static auto locale_backend = boost::locale::localization_backend_manager::global();
    if (!locale_init)
        locale_backend.select("std");
    static boost::locale::generator locale_gen(locale_backend);
    if (!locale_init) {
        locale_gen.locale_cache_enabled(true);
        try {
            InfoLogger() << "Global locale: " << std::use_facet<boost::locale::info>(locale_gen("")).name();
        } catch (const std::runtime_error&) {
            ErrorLogger() << "Global locale: set to invalid locale, setting to C locale";
            std::locale::global(std::locale::classic());
        }
        locale_init = true;
    }

    std::locale retval;
    try {
        retval = locale_gen(name);
    } catch(const std::runtime_error&) {
        ErrorLogger() << "Requested locale \"" << name << "\" is not a valid locale for this operating system";
        return std::locale::classic();
    }

    TraceLogger() << "Requested " << (name.empty() ? "(default)" : name) << " locale"
                  << " returning " << std::use_facet<boost::locale::info>(retval).name();
    return retval;
}

void FlushLoadedStringTables() {
    std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
    stringtables.clear();
}

const std::string& UserString(const std::string& str) {
    std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
    if (GetStringTable().StringExists(str))
        return GetStringTable()[str];
    return GetDefaultStringTable()[str];
}

std::vector<std::string> UserStringList(const std::string& key) {
    std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
    std::vector<std::string> result;
    std::istringstream template_stream(UserString(key));
    std::string item;
    while (std::getline(template_stream, item))
        result.push_back(item);
    return result;
}

bool UserStringExists(const std::string& str) {
    std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
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

const std::string& Language() {
    std::lock_guard<std::recursive_mutex> stringtable_lock(stringtable_access_mutex);
    return GetStringTable().Language();
}

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

    double RoundMagnitude(double mag, int digits) {
        // power of 10 of highest valued digit in number
        // = 2 (100's)   for 234.4
        // = 4 (10000's) for 45324
        int pow10 = static_cast<int>(floor(log10(mag)));
        //std::cout << "magnitude power of 10: " << pow10 << std::endl;


        // round number to fit in requested number of digits
        // shift number by power of 10 so that ones digit is the lowest-value digit
        // that will be retained in final number
        // eg. 45324 with 3 digits -> pow10 = 4, digits = 3
        //     want to shift 3 to the 1's digits position, so need 4 - 3 + 1 shift = 2
        double rounding_factor = pow(10.0, static_cast<double>(pow10 - digits + 1));
        // shift, round, shift back. this leaves 0 after the lowest retained digit
        mag = mag / rounding_factor;
        mag = round(mag);
        mag *= rounding_factor;
        //std::cout << "magnitude after initial rounding to " << digits << " digits: " << mag << std::endl;

        // rounding may have changed the power of 10 of the number
        // eg. 9999 with 3 digits, shifted to 999.9, rounded to 1000,
        // shifted back to 10000, now the power of 10 is 5 instead of 4.
        // so, redo this calculation
        pow10 = static_cast<int>(floor(log10(mag)));
        rounding_factor = pow(10.0, static_cast<double>(pow10 - digits + 1));
        mag = mag / rounding_factor;
        mag = round(mag);
        mag *= rounding_factor;
        //std::cout << "magnitude after second rounding to " << digits << " digits: " << mag << std::endl;

        return mag;
    }

    int TestRounding() {
        for (double num : {-0.001, -0.02, 0.0001,
                           84.5, 90.9, 99.9,
                           9999.9, 245.1, 2451.1, 12451.1, 1124451.1})
        {
            std::cout << "n: " << num
                      << "  r2: " << DoubleToString(num, 2, false)
                      << "  r3: " << DoubleToString(num, 3, false)
                      << "  r5: " << DoubleToString(num, 5, false) << std::endl;
        }
        exit(0);
    }
    //int dummy = TestRounding();
}

std::string DoubleToString(double val, int digits, bool always_show_sign) {
    std::string text; // = ""

    // minimum digits is 2. Fewer than this and things can't be sensibly displayed.
    // eg. 300 with 2 digits is 0.3k. With 1 digits, it would be unrepresentable.
    digits = std::max(digits, 2);

    // default result for sentinel value
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return UserString("UNKNOWN_VALUE_SYMBOL");

    double mag = std::abs(val);

    // early termination if magnitude is 0
    if (mag == 0.0 || RoundMagnitude(mag, digits + 1) == 0.0) {
        std::string format = "%1." + std::to_string(digits - 1) + "f";
        text += (boost::format(format) % mag).str();
        return text;
    }

    // prepend signs if neccessary
    int effective_sign = EffectiveSign(val);
    if (effective_sign == -1)
        text += "-";
    else if (always_show_sign)
        text += "+";

    if (mag > LARGE_UI_DISPLAY_VALUE)
        mag = LARGE_UI_DISPLAY_VALUE;

    // if value is effectively 0, avoid unnecessary later processing
    if (effective_sign == 0) {
        text = "0.0";
        for (int n = 2; n < digits; ++n)
            text += "0";  // fill in 0's to required number of digits
        return text;
    }

    //std::cout << std::endl << "DoubleToString val: " << val << " digits: " << digits << std::endl;
    const double initial_mag = mag;

    // round magnitude to appropriate precision for requested digits
    mag = RoundMagnitude(initial_mag, digits);
    int pow10 = static_cast<int>(floor(log10(mag)));


    // determine base unit for number: the next lower power of 10^3 from the
    // number (inclusive)
    int pow10_digits_above_pow1000 = 0;
    if (pow10 >= 0)
        pow10_digits_above_pow1000 = pow10 % 3;
    else
        pow10_digits_above_pow1000 = (pow10 % 3) + 3;   // +3 ensures positive result of mod
    int unit_pow10 = pow10 - pow10_digits_above_pow1000;

    if (digits == 2 && pow10_digits_above_pow1000 == 2) {
        digits = 3;

        // rounding to 2 digits when 3 digits must be shown to display the
        // number will cause apparent rounding issues.
        // re-do rounding for 3 digits of precision
        mag = RoundMagnitude(initial_mag, digits);
        pow10 = static_cast<int>(floor(log10(mag)));

        if (pow10 >= 0)
            pow10_digits_above_pow1000 = pow10 % 3;
        else
            pow10_digits_above_pow1000 = (pow10 % 3) + 3;   // +3 ensures positive result of mod
        unit_pow10 = pow10 - pow10_digits_above_pow1000;
    }


    // special limit: currently don't use any base unit powers below 0 (1's digit)
    if (unit_pow10 < 0)
        unit_pow10 = 0;

    int lowest_digit_pow10 = pow10 - digits + 1;

    //std::cout << "unit power of 10: " << unit_pow10
    //          << "  pow10 digits above pow1000: " << pow10_digits_above_pow1000
    //          << "  lowest_digit_pow10: " << lowest_digit_pow10
    //          << std::endl;

    // fraction digits:
    int fraction_digits = std::max(0, std::min(digits - 1, unit_pow10 - lowest_digit_pow10));
    //std::cout << "fraction_digits: " << fraction_digits << std::endl;


    // scale number by unit power of 10
    // eg. if mag = 45324 and unit_pow10 = 3, get mag = 45.324
    mag /= pow(10.0, static_cast<double>(unit_pow10));


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
        text += "T";        // Tera
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

