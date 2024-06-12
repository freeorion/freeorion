#include "i18n.h"

#include "Directories.h"
#include "Logger.h"
#include "OptionsDB.h"
#include "StringTable.h"

#include <boost/locale.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <atomic>

// define needed on Windows due to conflict with windows.h and std::min and std::max
#ifndef NOMINMAX
#  define NOMINMAX
#endif
// define needed in GCC
#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif
#if defined(_MSC_VER) && _MSC_VER >= 1930
struct IUnknown; // Workaround for "combaseapi.h(229,21): error C2760: syntax error: 'identifier' was unexpected here; expected 'type specifier'"
#endif

#include <boost/stacktrace.hpp>

namespace {
    std::map<std::string, std::shared_ptr<StringTable>> stringtables;
    std::shared_mutex                                   stringtable_access_mutex;
    std::atomic<bool>                                   stringtable_filename_init;
    std::mutex                                          stringtable_filename_init_mutex;
    StringTable                                         error_stringtable;
    std::shared_mutex                                   error_stringtable_access_mutex;
    constexpr std::string_view                          ERROR_STRING = "ERROR: ";


    std::string StackTrace() {
        static std::atomic<int> string_error_lookup_count = 0;
        if (string_error_lookup_count++ > 3)
            return "";
        std::stringstream ss;
        ss << "stacktrace:\n" << boost::stacktrace::stacktrace();
        return ss.str();
    }


    std::string operator+(const std::string_view sv, const std::string& s) {
        std::string retval;
        retval.reserve(sv.size() + s.size());
        retval.append(sv);
        retval.append(s);
        return retval;
    }

    std::string operator+(const std::string_view sv1, const std::string_view sv2) {
        std::string retval;
        retval.reserve(sv1.size() + sv2.size());
        retval.append(sv1);
        retval.append(sv2);
        return retval;
    }

    std::string operator+(const std::string_view sv, const char* c) {
        std::string retval;
        retval.reserve(sv.size() + std::strlen(c));
        retval.append(sv);
        retval.append(c);
        return retval;
    }


    // fallback stringtable to look up key in if entry is not found in currently configured stringtable
    boost::filesystem::path DevDefaultEnglishStringtablePath()
    { return GetResourceDir() / "stringtables/en.txt"; }

    // filename to use as default value for stringtable filename option.
    // based on the system's locale. not necessarily the same as the
    // "dev default" (english) stringtable filename for fallback lookup
    // includes "<resource-dir>/stringtables/" directory part of path
    boost::filesystem::path GetDefaultStringTableFileName() {
        std::string lang;

#if defined(FREEORION_ANDROID)
        lang = GetAndroidLang();
#else
        // early return when unable to get locale language string
        try {
            lang = std::use_facet<boost::locale::info>(GetLocale()).language();
        } catch(const std::bad_cast&) {
            ErrorLogger() << "Bad locale cast when setting default language";
        }
#endif

        boost::algorithm::to_lower(lang);

        // handle failed locale lookup or C locale
        if (lang.empty() || lang == "c" || lang == "posix") {
            WarnLogger() << "Lanuage not detected from locale: \"" << lang << "\"; falling back to default en";
            lang = "en";
        } else {
            DebugLogger() << "Detected locale language: " << lang;
        }

        boost::filesystem::path lang_filename{ lang + ".txt" };
        boost::filesystem::path default_stringtable_path{ GetResourceDir() / "stringtables" / lang_filename };

        // default to english if locale-derived filename not present
        if (!IsExistingFile(default_stringtable_path)) {
            WarnLogger() << "Detected language file not present: " << PathToString(default_stringtable_path) << "  Reverting to en.txt";
            default_stringtable_path = DevDefaultEnglishStringtablePath();
        }

        if (!IsExistingFile(default_stringtable_path))
            ErrorLogger() << "Default english stringtable file also not present !!!: " << PathToString(default_stringtable_path);

        DebugLogger() << "GetDefaultStringTableFileName returning: " << PathToString(default_stringtable_path);
        return default_stringtable_path;
    }

    // sets the stringtable filename option default value.
    // also checks the option-set stringtable path, and if it is blank or the
    // specified file doesn't exist, tries to reinterpret the option value as
    // a path in the standard location, or reverts to the default stringtable
    // location if other attempts fail.
    void InitStringtableFileName() {
        if (stringtable_filename_init)
            return; // already initialized
        std::scoped_lock filename_init_lock{stringtable_filename_init_mutex};
        if (stringtable_filename_init)
            return; // already initialized while getting the lock...

        // set option default value based on system locale
        auto default_stringtable_path = GetDefaultStringTableFileName();
        auto default_stringtable_path_string = PathToString(default_stringtable_path);
        GetOptionsDB().SetDefault("resource.stringtable.path", default_stringtable_path_string);

        // get option-configured stringtable path. may be the default empty
        // string (set by call to:   db.Add<std::string>("resource.stringtable.path" ...
        // or this may have been overridden from one of the config XML files or from
        // a command line argument.
        std::string option_path = GetOptionsDB().Get<std::string>("resource.stringtable.path");
        boost::filesystem::path stringtable_path = FilenameToPath(option_path);

        // verify that option-derived stringtable file exists, with fallbacks
        DebugLogger() << "Stringtable option path: " << option_path;

        if (option_path.empty()) {
            DebugLogger() << "Stringtable option path not specified yet, using default: " << default_stringtable_path_string;
            GetOptionsDB().Set("resource.stringtable.path", default_stringtable_path_string);
            stringtable_filename_init = true;
            return;
        }

        bool set_option = false;

        if (!IsExistingFile(stringtable_path)) {
            set_option = true;
            // try interpreting path as a filename located in the stringtables directory
            stringtable_path = GetResourceDir() / "stringtables" / FilenameToPath(option_path);
        }
        if (!IsExistingFile(stringtable_path)) {
            set_option = true;
            // try interpreting path as directory and filename in resources directory
            stringtable_path = GetResourceDir() / FilenameToPath(option_path);
        }
        if (!IsExistingFile(stringtable_path)) {
            set_option = true;
            // fall back to default option value
            ErrorLogger() << "Stringtable option path file is missing: " << PathToString(stringtable_path);
            DebugLogger() << "Resetting to default: " << default_stringtable_path_string;
            stringtable_path = std::move(default_stringtable_path);
        }

        if (set_option)
            GetOptionsDB().Set("resource.stringtable.path", PathToString(stringtable_path));

        stringtable_filename_init = true;
    }

    // get currently set stringtable filename option value, or the default value
    // if the currenty value is empty
    std::string GetStringTableFileName() {
        InitStringtableFileName();

        std::string option_path = GetOptionsDB().Get<std::string>("resource.stringtable.path");
        if (option_path.empty())
            return GetOptionsDB().GetDefault<std::string>("resource.stringtable.path");
        else
            return option_path;
    }

    template <typename SS>
    std::shared_ptr<StringTable> GetOrCreateStringTable(
        SS&& filename, std::shared_lock<std::shared_mutex>& access_lock,
        std::shared_ptr<const StringTable> fallback = nullptr)
    {
        if (auto it = stringtables.find(filename); it != stringtables.end())
            return it->second;

        auto retval{std::make_shared<StringTable>(filename, std::move(fallback))};

        access_lock.unlock();
        try {
            std::unique_lock mutation_lock(stringtable_access_mutex);
            stringtables.emplace(std::forward<SS>(filename), retval);
        } catch (...) {
            ErrorLogger() << "Unable to emplace new stringtable";
            access_lock.lock();
            return nullptr;
        }

        access_lock.lock();
        return retval;
    }

    StringTable& GetStringTable(const std::string& stringtable_filename,
                                std::shared_lock<std::shared_mutex>& access_lock)
    {
        if (!access_lock)
            ErrorLogger() << "GetStringTable passed unlocked shared_lock";

        if (auto it = stringtables.find(stringtable_filename); it != stringtables.end())
            return *it->second;

        // ensure the default stringtable is loaded first
        InitStringtableFileName();
        auto default_stringtable_filename{GetOptionsDB().GetDefault<std::string>("resource.stringtable.path")};

        if (default_stringtable_filename == stringtable_filename) {
            if (auto default_table{GetOrCreateStringTable(default_stringtable_filename, access_lock)})
                return *default_table;
            throw std::runtime_error("couldn't get default stringtable!");
        }

        auto default_table{GetOrCreateStringTable(std::move(default_stringtable_filename), access_lock)};

        if (auto table{GetOrCreateStringTable(stringtable_filename, access_lock, default_table)})
            return *table;
        else if (default_table)
            return *default_table;
        else
            throw std::runtime_error("couldn't get stringtable or default stringtable!");
    }

    std::shared_mutex path_LUT_mutex;
    std::map<boost::filesystem::path, std::string> path_to_string_LUT;

    StringTable& GetStringTable(const boost::filesystem::path& stringtable_path,
                                std::shared_lock<std::shared_mutex>& access_lock)
    {
        {
            std::shared_lock path_LUT_read_lock{path_LUT_mutex};
            auto path_it = path_to_string_LUT.find(stringtable_path);
            if (path_it != path_to_string_LUT.end())
                return GetStringTable(path_it->second, access_lock);
        }

        {
            std::unique_lock path_LUT_write_lock{path_LUT_mutex};
            const auto& string_of_path = path_to_string_LUT.emplace(stringtable_path,
                                                                    PathToString(stringtable_path)).first->second;
            return GetStringTable(string_of_path, access_lock);
        }
    }

    StringTable& GetStringTable(std::shared_lock<std::shared_mutex>& access_lock)
    { return GetStringTable(GetStringTableFileName(), access_lock); }

    StringTable& GetDevDefaultStringTable(std::shared_lock<std::shared_mutex>& access_lock)
    { return GetStringTable(DevDefaultEnglishStringtablePath(), access_lock); }
}

#if !defined(FREEORION_ANDROID)
const std::locale& GetLocale(std::string_view name) {
    thread_local auto retval = [name_str{std::string{name}}]() -> std::locale {
        static auto locale_backend = boost::locale::localization_backend_manager::global();
        locale_backend.select("std");
        static boost::locale::generator locale_gen(locale_backend);
        locale_gen.locale_cache_enabled(true);
        try {
            auto retval2 = locale_gen.generate(name_str);
            std::use_facet<boost::locale::info>(retval2);
            return retval2;
        } catch (...) {
            return std::locale::classic();
        }
    }();
    return retval;
}
#endif

void FlushLoadedStringTables() {
    std::unique_lock mutation_lock(stringtable_access_mutex);
    stringtables.clear();
}

AllStringsResultT& AllStringtableEntries(bool default_table) {
    std::shared_lock stringtable_lock(stringtable_access_mutex);
    if (default_table)
        return GetDevDefaultStringTable(stringtable_lock).AllStrings();
    else
        return GetStringTable(stringtable_lock).AllStrings();
}

const std::string& UserString(const std::string& str) {
    {
        std::shared_lock stringtable_lock(stringtable_access_mutex);
        const auto& [string_found, string_value] = GetStringTable(stringtable_lock).CheckGet(str);
        if (string_found)
            return string_value;

        const auto& [default_string_found, default_string_value] =
            GetDevDefaultStringTable(stringtable_lock).CheckGet(str);
        if (default_string_found)
            return default_string_value;
    }

    {
        std::shared_lock error_read_lock(error_stringtable_access_mutex);
        const auto& [error_string_found, error_string_value] = error_stringtable.CheckGet(str);
        if (error_string_found)
            return error_string_value;
    }

    ErrorLogger() << "Missing string: " << str;
    DebugLogger() << StackTrace();

    {
        std::unique_lock error_mutation_lock(error_stringtable_access_mutex);
        auto error_string{ERROR_STRING + str};
        return error_stringtable.Add(str, std::move(error_string));
    }
}

const std::string& UserString(const std::string_view str) {
    {
        std::shared_lock stringtable_lock(stringtable_access_mutex);
        const auto& [string_found, string_value] = GetStringTable(stringtable_lock).CheckGet(str);
        if (string_found)
            return string_value;

        const auto& [default_string_found, default_string_value] =
            GetDevDefaultStringTable(stringtable_lock).CheckGet(str);
        if (default_string_found)
            return default_string_value;
    }

    {
        std::shared_lock error_read_lock(error_stringtable_access_mutex);
        const auto& [error_string_found, error_string_value] = error_stringtable.CheckGet(str);
        if (error_string_found)
            return error_string_value;
    }

    ErrorLogger() << "Missing string: " << str;
    DebugLogger() << StackTrace();

    {
        std::unique_lock error_mutation_lock(error_stringtable_access_mutex);
        auto error_string{ERROR_STRING + str};
        return error_stringtable.Add(std::string{str}, std::move(error_string));
    }
}

const std::string& UserString(const char* str) {
    {
        std::shared_lock stringtable_lock(stringtable_access_mutex);
        const auto& [string_found, string_value] = GetStringTable(stringtable_lock).CheckGet(str);
        if (string_found)
            return string_value;

        const auto& [default_string_found, default_string_value] =
            GetDevDefaultStringTable(stringtable_lock).CheckGet(str);
        if (default_string_found)
            return default_string_value;
    }

    {
        std::shared_lock error_read_lock(error_stringtable_access_mutex);
        const auto& [error_string_found, error_string_value] = error_stringtable.CheckGet(str);
        if (error_string_found)
            return error_string_value;
    }

    ErrorLogger() << "Missing string: " << str;
    DebugLogger() << StackTrace();

    {
        std::unique_lock error_mutation_lock(error_stringtable_access_mutex);
        auto error_string{ERROR_STRING + str};
        return error_stringtable.Add(std::string{str}, std::move(error_string));
    }
}

std::vector<std::string> UserStringList(const std::string& key) {
    std::vector<std::string> result;
    result.reserve(20); // rough guesstimate
    std::istringstream template_stream(UserString(key));
    // split big string into newline-separated substrings strings
    std::string item;
    while (std::getline(template_stream, item))
        result.push_back(item);
    return result;
}

bool UserStringExists(const std::string& str) {
    std::shared_lock stringtable_lock(stringtable_access_mutex);
    return GetStringTable(stringtable_lock).StringExists(str) ||
           GetDevDefaultStringTable(stringtable_lock).StringExists(str);
}

bool UserStringExists(const std::string_view str) {
    std::shared_lock stringtable_lock(stringtable_access_mutex);
    return GetStringTable(stringtable_lock).StringExists(str) ||
           GetDevDefaultStringTable(stringtable_lock).StringExists(str);
}

bool UserStringExists(const char* str) {
    std::shared_lock stringtable_lock(stringtable_access_mutex);
    return GetStringTable(stringtable_lock).StringExists(str) ||
           GetDevDefaultStringTable(stringtable_lock).StringExists(str);
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
    std::shared_lock stringtable_lock(stringtable_access_mutex);
    return GetStringTable(stringtable_lock).Language();
}

std::string RomanNumber(unsigned int n) {
    //letter pattern (N) and the associated values (V)
    static constexpr std::array N = {  "M",  "CM",  "D",  "CD",  "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
    static constexpr std::array V = { 1000u, 900u, 500u,  400u, 100u,  90u, 50u,  40u, 10u,   9u,  5u,   4u,  1u };
    if (n == 0) return ""; // the romans didn't know there is a zero, read a book about history of the zero if you want to know more
                           // Roman numbers are written using patterns, you chosse the highest pattern lower that the number
                           // write it down, and substract it's value until you reach zero.

    // safety check to avoid very long loops
    if (n > 10000)
        return "!";

    // start with the highest pattern and reduce the size every time it doesn't fit
    std::string retval;
    unsigned int remainder = n; // remainder of the number to be written
    int i = 0;                  // pattern index
    while (remainder > 0) {
        // check if number is larger than the actual pattern value
        if (remainder >= V[i]) {
            retval += N[i]; // write pattern down
            remainder -= V[i]; // reduce number
        } else {
            i++; // go to next pattern
        }
    }
    return retval;
}

namespace {
    constexpr double SMALL_UI_DISPLAY_VALUE = 1.0e-6;
    constexpr double LARGE_UI_DISPLAY_VALUE = 9.99999999e+9;
    constexpr double UNKNOWN_UI_DISPLAY_VALUE = std::numeric_limits<double>::infinity();

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

    [[nodiscard]] constexpr std::string_view UnitPostFix(int unit_pow10) noexcept {
        // append base scale SI prefix (as postfix)
        switch (unit_pow10) {
        case -15: return "f"; break;        // femto
        case -12: return "p"; break;        // pico
        case -9:  return "n"; break;        // nano
        case -6:  return "\xC2\xB5"; break; // micro / µ in UTF-8
        case -3:  return "m"; break;        // milli
        case 3:   return "k"; break;        // kilo
        case 6:   return "M"; break;        // Mega
        case 9:   return "G"; break;        // Giga
        case 12:  return "T"; break;        // Tera
        default:  return {};  break;
        }
    }
}

std::string DoubleToString(double val, int digits, bool always_show_sign) {
    // minimum digits is 2. Fewer than this and things can't be sensibly displayed.
    // eg. 300 with 2 digits is 0.3k. With 1 digits, it would be unrepresentable.
    digits = std::max(digits, 2);

    // default result for sentinel value
    if (val == UNKNOWN_UI_DISPLAY_VALUE)
        return UserString("UNKNOWN_VALUE_SYMBOL");

    double mag = std::abs(val);

    // early termination if magnitude is 0
    if (mag == 0.0 || RoundMagnitude(mag, digits + 1) == 0.0) {
        std::string format = "%1." + std::to_string(digits - 1) + "f"; // TODO: avoid extra string here?
        return (boost::format(format) % mag).str();
    }

    std::string text;
    text.reserve(static_cast<std::size_t>(digits)+3u);

    // prepend signs if neccessary
    int effective_sign = EffectiveSign(val);
    if (effective_sign == -1)
        text += "-";
    else if (always_show_sign)
        text += "+";

    // if value is effectively 0, avoid unnecessary later processing
    if (effective_sign == 0) {
        text = "0.0";
        for (int n = 2; n < digits; ++n)
            text += "0";  // fill in 0's to required number of digits
        return text;
    }

    if (mag > LARGE_UI_DISPLAY_VALUE)
        mag = LARGE_UI_DISPLAY_VALUE;

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

    const int lowest_digit_pow10 = pow10 - digits + 1;

    //std::cout << "unit power of 10: " << unit_pow10
    //          << "  pow10 digits above pow1000: " << pow10_digits_above_pow1000
    //          << "  lowest_digit_pow10: " << lowest_digit_pow10
    //          << std::endl;

    // fraction digits:
    const int fraction_digits = std::max(0, std::min(digits - 1, unit_pow10 - lowest_digit_pow10));
    //std::cout << "fraction_digits: " << fraction_digits << std::endl;


    // scale number by unit power of 10
    // eg. if mag = 45324 and unit_pow10 = 3, get mag = 45.324
    mag /= pow(10.0, static_cast<double>(unit_pow10));


    const std::string format{"%" + std::to_string(digits) + "." + std::to_string(fraction_digits) + "f"};
    text += (boost::format(format) % mag).str();
    text.append(UnitPostFix(unit_pow10));

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

