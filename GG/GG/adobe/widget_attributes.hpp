/*
    Copyright 2006-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/*************************************************************************************************/

#ifndef ADOBE_WIDGET_ATTRIBUTES_HPP
#define ADOBE_WIDGET_ATTRIBUTES_HPP

#include <GG/adobe/config.hpp>

#include <GG/adobe/enum_ops.hpp>

#include <climits>

/*************************************************************************************************/

namespace adobe {

/*************************************************************************************************/

/*!
\defgroup widget_attributes Widget Attributes
\ingroup layout_library
*/

/*!
\ingroup widget_attributes

\brief Semantic theme settings to apply to a widget

\note
    The theme sizes (<code>theme_large_s</code> through <code>theme_mini_s</code>)\
    are mutually exclusive. A theme's adornments can be manipulated via logical
    operations.
*/

enum theme_t
{
    /// No theme
    theme_none_s                =   0,

    /// For dialogs & windows (atypical)
    theme_large_s               =   1,
    /// For dialogs & windows
    theme_normal_s              =   2,
    /// For palettes
    theme_small_s               =   3,
    /// For palettes (atypical)
    theme_mini_s                =   4,

    /// Adornment for displaying a label
    theme_adornment_label_s     =   1L << 29,
    /// Adornment for displaying a number
    theme_adornment_number_s    =   1L << 30,
    /// Adornment for monospacing widget text
    theme_adornment_mono_s      =   1L << 31,

    /// Theme mask to obtain widget size
    theme_mask_s                =   theme_large_s   |
                                    theme_normal_s  |
                                    theme_small_s   |
                                    theme_mini_s,

    /// Theme mask to obtain widget adornments
    theme_adornment_mask_s      =   theme_adornment_mono_s      |
                                    theme_adornment_number_s    |
                                    theme_adornment_label_s,

    /// Default theme
    theme_default_s             =   theme_normal_s
};

/*************************************************************************************************/

ADOBE_DEFINE_BITSET_OPS(theme_t)

/*************************************************************************************************/

/*!
\ingroup widget_attributes

\brief Standard keyboard state modifiers

\note
    These are typically used by controls to specify alternate
    namings when various modifiers are held down to denote different
    behaviors
*/

enum modifiers_t
{
    /// No modifiers
    modifiers_none_s                = 0,

    /// Left-shift key (if applicable)
    modifiers_left_shift_s          = 1 << 0,

    /// Right-shift key (if applicable)
    modifiers_right_shift_s         = 1 << 1,

    /// Left-option (or alt) key (if applicable)
    modifiers_left_option_s         = 1 << 2,

    /// Right-option (or alt) key (if applicable)
    modifiers_right_option_s        = 1 << 3,

    /// Left-control key (if applicable)
    modifiers_left_control_s        = 1 << 4,

    /// Right-control key (if applicable)
    modifiers_right_control_s       = 1 << 5,

    /// Caps lock key
    modifiers_caps_lock_s           = 1 << 6,


    /// Any command key
    modifiers_any_command_s         = 1 << 7,

    /// Any shift key
    modifiers_any_shift_s           = modifiers_left_shift_s | modifiers_right_shift_s,

    /// Any option (or alt) key (if applicable)
    modifiers_any_option_s          = modifiers_left_option_s | modifiers_right_option_s,

    /// Any control key (if applicable)
    modifiers_any_control_s         = modifiers_left_control_s | modifiers_right_control_s,
    
    modifiers_all_s                 = UINT_MAX
};

/*************************************************************************************************/

ADOBE_DEFINE_BITSET_OPS(modifiers_t)

/*************************************************************************************************/

} // namepspace adobe

/*************************************************************************************************/

#endif

/*************************************************************************************************/
