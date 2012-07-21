/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_VALUE_RANGE_FORMAT_HPP
#define ADOBE_VALUE_RANGE_FORMAT_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/cmath.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/name.hpp>

#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*
    REVISIT (sparent) : This is a simple filter for handling sliders. The general notion of a
    filter should describe a set of valid values (potentially infinite - such as all positive
    numbers).

    This should be considered a starting point for such a classification.
*/

class value_range_format_t
{
public:
    value_range_format_t() :
        first_m (0.0),
        last_m (0.0),
        interval_count_m (0)
    { }

    void set(const dictionary_t& parameters)
    {
        get_value(parameters, key_first, first_m);
        get_value(parameters, key_last, last_m);

        interval_count_m = static_cast<std::size_t>(last_m - first_m);

        get_value(parameters, key_interval_count, interval_count_m);
    }

    std::size_t size() const
    { return interval_count_m; }

    const any_regular_t& at(std::size_t index) const
    {
        double result(double(index) / double(interval_count_m) * (last_m - first_m) + first_m);

        result = (std::min)((std::max)(first_m, result), last_m);

        result_m.assign(result);

        return result_m;
    }

    std::size_t find(const any_regular_t& source) const
    {
        double result = adobe::round(double(interval_count_m) * (source.cast<double>() - first_m) / (last_m - first_m));

        return static_cast<std::size_t>(std::min<double>(std::max<double>(0, result), interval_count_m));
    }

    friend bool operator==(const value_range_format_t& lhs, const value_range_format_t& rhs)
    {
        return lhs.first_m == rhs.first_m &&
               lhs.last_m == rhs.last_m &&
               lhs.interval_count_m == rhs.interval_count_m &&
               lhs.result_m == rhs.result_m;
    }

private:
    double                       first_m;
    double                       last_m;
    std::size_t                  interval_count_m;
    mutable any_regular_t result_m;
};

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
