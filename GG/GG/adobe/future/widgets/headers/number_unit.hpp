/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_NUMBER_UNIT_HPP
#define ADOBE_NUMBER_UNIT_HPP

/****************************************************************************************************/

#include <GG/adobe/algorithm/clamp.hpp>
#include <GG/adobe/name_fwd.hpp>
#include <GG/adobe/dictionary_fwd.hpp>

#include <string>

/*!
    \defgroup apl_widgets_number_unit Number Unit Descriptor Utilities
    \ingroup apl_widgets
*/

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_number_unit

    This struct is a decoration utility used by widgets that need to
    format number information for the user. As it stands the struct is
    to describe a single linear transformation of a number from its base
    unit to this unit. For instance, a cell in the property model may
    hold a number in units inches, and a unit_t decorator for a
    display_number_t would detail how to convert that value to units
    centimeters by means of a linear transformation. The
    display_number_t will select the most appropriate linear
    transformation of its set of possibilities when displaying the value
    from the property model.
*/

struct unit_t
{
    /*!
        Default constructor. Yay.
    */
    unit_t() :
                base_unit_id_m(0),
        decimal_places_m(0),    
                trailing_zeroes_m(false),
                format_m("#"),
        increment_m(1),
        scale_m_m(1),
        scale_b_m(0),
        min_value_m((std::numeric_limits<double>::min)()),
        max_value_m((std::numeric_limits<double>::max)())
    { }

    /// this is the name of the unit this linear transformation represents
    std::string name_m;
        /// this is the short name  of the unit this linear transformation represents (ie cm for centimeters) 
        std::string short_name_m;
        /// this is the name of the "base unit" group to which this linear transformation is to relate
        adobe::name_t base_unit_filter_m;
        /// this is the name of the "base unit" group to which this linear transformation is to relate
        adobe::name_t base_unit_m;
        // Unique id for base unit
        unsigned int base_unit_id_m;    /// this is the name of the "base unit" group to which this linear transformation is to relate

    /// this will be the formatting information for the result of the transformation instead of format_m
    unsigned int decimal_places_m;
        bool trailing_zeroes_m;

    /// legacy format printf-style string
        std::string format_m;   

    /// in some instances the user is able to increment the value. This specifies the amount (in this unit) that the user can increment the value.
    double increment_m;

    /// multiplier to go from this unit to the base unit using y = mx + b
    double scale_m_m;

    /// y-intersect to go from this unit to the base unit using y = mx + b
    double scale_b_m; 

    /// optional minimum value for this unit -- the value cannot go below this value when this unit represents it.
    double min_value_m;

    /// optional maximum value for this unit -- the value cannot go above this value when this unit represents it.
    double max_value_m;
};

/*!
    \ingroup apl_widgets_number_unit

    \param dict is the dictionary holding relevant unit information within.
    \param default_unit contains values for the resultant unit should they not be defined in \c dict.

    \return a complete unit_t as a result of the merging of the two parameters.

    \par Dictionary Values
    The values extracted from the dictionary are the following:
    <table>
    <tr>
        <th>key</th>
        <th>value type</th>
        <th>default value</th>
        <th>description</th>
    </tr>
    <tr>
        <td><code>name</code></td>
        <td><code>string</code></td>
        <td>""</td>
        <td>Name of this unit</td>
    </tr>
    <tr>
        <td><code>bind</code></td>
        <td><code>name</code></td>
        <td><i>empty</i></td>
        <td>Name of base unit to which this unit is bound</td>
    </tr>
    <tr>
        <td><code>format</code></td>
        <td><code>string</code></td>
        <td><code>"#.00"</code></td>
        <td>printf-style format for the widget when this unit is selected</td>
    </tr>
    <tr>
        <td><code>increment</code></td>
        <td><code>double</code></td>
        <td><code>1</code></td>
        <td>amount by which the property model value should be increemented when this unit is selected</td>
    </tr>
    <tr>
        <td><code>scale</code></td>
        <td><code>array of two doubles</code></td>
        <td><code>[ 1.0, 0.0 ]</code></td>
        <td>
            Scale factor for the property model's value to this specified unit.
            This allows you to represent a given cell (say, <code>\@width_inches</code>) in an arbitrary unit given a linear conversion. The linear formula is <code>y = a(x) + b</code>, so the default settings are simply <code>y = x</code>. As an example, if you cell is saving values in unit inches and you want to display your value in centimeters, the scale would be <code>[ 2.54, 0.0 ]</code>.
        </td>
    </tr>
    <tr>
        <td><code>min_value</code></td>
        <td><code>double</code></td>
        <td><i>N/A</i></td>
        <td>Optional. Minimum value allowed for this unit. Note that if you have a value that can be represented as multiple units, you have to specify a minimum value for each unit.</td>
    </tr>
    <tr>
        <td><code>max_value</code></td>
        <td><code>double</code></td>
        <td><i>N/A</i></td>
        <td>Optional. Maximum value allowed for this unit. Note that if you have a value that can be represented as multiple units, you have to specify a maximum value for each unit.</td>
    </tr>
*/
unit_t to_unit(const dictionary_t& dict,  const unit_t& default_unit = unit_t());

/*!
    \ingroup apl_widgets_number_unit

    Performs a linear transformation on a base value given a unit.

    \param base_value is the value to be converted to the scaled value
    \param unit is the unit by which the base value is to be transformed

    \return the scaled value according to the unit_t's linear transformation.
*/
inline double to_scaled_value(double base_value, const unit_t& unit)
{ return (unit.scale_m_m * base_value) + unit.scale_b_m; }

/*!
    \ingroup apl_widgets_number_unit

    Performs an inverse-linear transformation on a scaled value given a unit.

    \param scaled_value is the value to be converted to the base value
    \param unit is the unit by which the scaled value is to be transformed

    \return the base value according to the unit_t's linear transformation.
*/
inline double to_base_value(double scaled_value, const unit_t& unit)
{ return (scaled_value - unit.scale_b_m) / unit.scale_m_m; }

/*!
    \ingroup apl_widgets_number_unit

    Performs an inverse-linear transformation on a scaled value given a
    unit. It will also pin the value after the transformation has taken
    place to the unit_t's specified minimum and maximum values.

    \todo the min and max values should come from the unit_t instead of parameters.

    \param scaled_value is the value to be converted to the base value
    \param unit is the unit by which the scaled value is to be transformed
    \param min is the minimum value allowed for the result
    \param max is the maximumn value allowed for the result

    \return the pinned base value according to the unit_t's linear transformation.
*/
inline double to_pinned_base_value(double               scaled_value,
                                   const unit_t& unit,
                                   double               min,
                                   double               max)
{
    double base(to_base_value(scaled_value, unit));

    if (min == (std::numeric_limits<double>::min)())
        min = base;

    if (max == (std::numeric_limits<double>::max)())
        max = base;

    return adobe::clamp(base, min, max);
}

/****************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
