/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/platform_number_formatter_data.hpp>

#include <GG/adobe/future/locale.hpp>
#include <GG/adobe/future/number_formatter.hpp>
#include <GG/adobe/name.hpp>
#include <GG/adobe/string.hpp>

#include <sstream>

/****************************************************************************************************/

namespace {

/****************************************************************************************************/

void replace_all(std::string& haystack, const std::string& needle, const std::string& new_needle)
{
    if (needle == std::string() || needle == new_needle)
        return;

    std::string::size_type pos(0);

    while (true)
    {
        pos = haystack.find(needle, pos);

        if (pos == std::string::npos)
            break;

        haystack.replace(pos, needle.size(), new_needle);

        pos += new_needle.size();
    }
}

/****************************************************************************************************/

std::string pull_thousands_separator(const std::string& src)
{
    std::string result(src);
    std::string thousands_separator;

    get_value(adobe::current_locale(), adobe::key_locale_thousands_separator, thousands_separator);

    replace_all(result, thousands_separator, std::string());

    return result;
}

/****************************************************************************************************/

std::locale& current_iostream_locale()
{
    static std::locale locale_s;

    return locale_s;    
}

/****************************************************************************************************/

template <typename T>
std::string number_format(const std::string& format, const T& value)
{
    std::stringstream      stream;
    std::locale            current_locale(current_iostream_locale());
    std::string            current_decimal;
    std::string::size_type point(0);

    get_value(adobe::current_locale(), adobe::key_locale_decimal_point, current_decimal);

    point = format.find(current_decimal);

    if (point == std::string::npos)
    {
        stream.unsetf(std::ios_base::showpoint);

        stream.precision(0);
    }
    else
    {
        std::streamsize pres(static_cast<std::streamsize>(format.size() - point - 1));

        stream.precision(pres);
    }

    stream.imbue(current_locale);

    stream.setf(std::ios_base::fixed);

    stream << value;

    return pull_thousands_separator(stream.str());
}

/****************************************************************************************************/

template <typename T>
T number_parse(const std::string& /*format*/, const std::string& str)
{
    std::stringstream stream(pull_thousands_separator(str));
    T                 result = T();

    stream.imbue(current_iostream_locale());

    stream >> result;

    return result;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

void number_formatter_platform_data_t::initialize()
{ }

/****************************************************************************************************/

void number_formatter_platform_data_t::set_format(const std::string& format)
{
    format_m = format;
}

/****************************************************************************************************/

std::string number_formatter_platform_data_t::get_format() const
{
    return format_m;
}

/****************************************************************************************************/

std::string number_formatter_platform_data_t::format(const any_regular_t& x)
{
    if (false) { }
#if 0
    else if (x.type() == typeid(char))      return number_format<char>(format_m, x.cast<char>());
    else if (x.type() == typeid(short))     return number_format<short>(format_m, x.cast<short>());
    else if (x.type() == typeid(int))       return number_format<int>(format_m, x.cast<int>());
    else if (x.type() == typeid(long))      return number_format<long>(format_m, x.cast<long>());
    else if (x.type() == typeid(long long)) return number_format<long long>(format_m, x.cast<long long>());
    else if (x.type() == typeid(float))     return number_format<float>(format_m, x.cast<float>());
#endif
    else if (x.type_info() == type_info<double>())    return number_format<double>(format_m, x.cast<double>());
    else return std::string("formatter_format_number error");

    return std::string();
}

/****************************************************************************************************/

any_regular_t number_formatter_platform_data_t::parse(const std::string& str, any_regular_t the_type)
{
        /* REVISIT (sparent) : We don't actually store all these types currently. */
    if (false) { }
#if 0
    else if (the_type.type() == typeid(char))      return any_regular_t(number_parse<char>(format_m, str));
    else if (the_type.type() == typeid(short))     return any_regular_t(number_parse<short>(format_m, str));
    else if (the_type.type() == typeid(int))       return any_regular_t(number_parse<int>(format_m, str));
    else if (the_type.type() == typeid(long))      return any_regular_t(number_parse<long>(format_m, str));
    else if (the_type.type() == typeid(long long)) return any_regular_t(number_parse<long long>(format_m, str));
    else if (the_type.type() == typeid(float))     return any_regular_t(number_parse<float>(format_m, str));
#endif
    else if (the_type.type_info() == type_info<double>())    return any_regular_t(number_parse<double>(format_m, str));
    else return any_regular_t(std::string("formatter_format_number error"));

    return any_regular_t();
}

/****************************************************************************************************/

void number_formatter_platform_data_t::monitor_locale(const dictionary_t& locale_info)
{
    std::string locale_identifier;

    get_value(locale_info, key_locale_identifier, locale_identifier);

    current_iostream_locale() = std::locale(locale_identifier.c_str());
}

/****************************************************************************************************/

#if 0
#pragma mark -
#endif

/****************************************************************************************************/

bool completely_valid_number_string_given_current_locale(const std::string& value)
{
    std::string thousands_separator;

    get_value(adobe::current_locale(), adobe::key_locale_thousands_separator, thousands_separator);

    if (value.find(thousands_separator) != std::string::npos)
        return false;

    static const double minimum((std::numeric_limits<double>::min)());
    std::stringstream   stream(value);
    double              number((std::numeric_limits<double>::min)());

    stream.imbue(current_iostream_locale());

    stream >> number;

    return number != minimum && stream.eof();
}

/****************************************************************************************************/

#if 0
#pragma mark -
#endif

/****************************************************************************************************/

void number_formatter_t::set_format(const std::string& format)
{ data_m.set_format(format); }

/****************************************************************************************************/

std::string number_formatter_t::get_format() const
{ return data_m.get_format(); }

/****************************************************************************************************/

template <>
std::string number_formatter_t::format<any_regular_t>(const any_regular_t& x)
{ return data_m.format(x); }

/****************************************************************************************************/

template <>
any_regular_t number_formatter_t::parse<any_regular_t>(const std::string& str, any_regular_t dummy)
{ return data_m.parse(str, dummy); }

/****************************************************************************************************/

void number_formatter_t::monitor_locale(const dictionary_t& locale_data)
{ return data_m.monitor_locale(locale_data); }

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
