/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_BUTTON_HELPER_HPP
#define ADOBE_BUTTON_HELPER_HPP

/****************************************************************************************************/

#include <GG/adobe/config.hpp>

#include <GG/adobe/any_regular.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>

#include <vector>
#include <string>


/*!
    \defgroup apl_widgets_button_helper Button Widget Helpers
    \ingroup apl_widgets

    The following are a collection of algorithms and data structures
    that assist in the creation of a button_t widget.
*/

/*!
    \defgroup apl_widgets_image_button_helper Image Button Widget Helpers
    \ingroup apl_widgets

    The following are a collection of algorithms and data structures
    that assist in the creation of a image_button_t widget. Many of
    the algorithms specified in the \ref apl_widgets_button_helper
    section of the documentation can also be used for image_button_t.
*/

namespace GG {
    class Texture;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    The button_hit_proc_t is the function signature required for a
    client callback proc that wants to receive notification from a
    button widget that a latch has been fired.
*/

typedef boost::function<void (const any_regular_t&, const dictionary_t&)> button_hit_proc_t;

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    A button_t widget is a collection of latches, each with its own
    unique information. Users select which latch they want the button to
    currently represent by holding down a series of modifier keys (alt,
    ctrl, etc.) When the user clicks on a button, the current latch is
    said to fire, at which point the related callback proc is called
    with the retained state information for this latch.

    This struct is what is used to convey all the necessary information
    to the button_t constructor for a single latch state. An array of
    these button descriptors are passed to the button during
    construction time.
*/
struct button_state_descriptor_t
{
    /*!
        Default constructor.
    */
    button_state_descriptor_t() : 
        modifier_set_m(modifiers_none_s)
    { }

    /*! The name of this button state */
    std::string name_m;

    /*! Additional help text for the widget when the user pauses over it */
    std::string alt_text_m;

    /*! The modifier state required to select this button state */
    modifiers_t modifier_set_m;

    /*! The notification proc to be called when this state is fired. */
    button_hit_proc_t hit_proc_m;

    /*! (used internally) The retained state of the property model for this button state */
    any_regular_t value_m;

    /*! (used internally) The retained contributing state information for this button state */
    dictionary_t contributing_m;

    button_state_descriptor_t& operator=(const button_state_descriptor_t& x) 
    { /*REVISIT MOVE*/ 
        name_m = x.name_m; 
        alt_text_m = x.alt_text_m; 
        modifier_set_m = x.modifier_set_m; 
        hit_proc_m = x.hit_proc_m; 
        value_m = x.value_m; 
        contributing_m = x.contributing_m; 
        return *this;
    }

};

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    This is simply a vector of button state descriptors. It can be used
    to ease in the construction of a button_t widget.
*/
typedef std::vector<button_state_descriptor_t> button_state_set_t;

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_image_button_helper
*/
struct image_button_state_descriptor_t
{
    typedef boost::shared_ptr<GG::Texture> image_type;

    /*!
        Default constructor.
    */
    image_button_state_descriptor_t() : 
        modifier_set_m(modifiers_none_s)
    { }

    /*! The primary image of this button state */
    image_type image_m;

    /*! The disabled image of this button state */
    image_type disabled_image_m;

    /*! Image to be shown when the user hovers over the button with the cursor */
    image_type hover_image_m;

    /*! Image to be used by the cursor when the user hovers over the button with it */
    image_type cursor_image_m;

    /*! Additional help text for the widget when the user pauses over it */
    std::string alt_text_m;

    /*! The modifier state required to select this button state */
    modifiers_t modifier_set_m;

    /*! The notification proc to be called when this state is fired. */
    button_hit_proc_t hit_proc_m;

    /*! (used internally) The retained state of the property model for this button state */
    any_regular_t value_m;

    /*! (used internally) The retained contributing state information for this button state */
    dictionary_t contributing_m;
};

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_image_button_helper

    This is simply a vector of image button state descriptors. It can be used
    to ease in the construction of an image_button_t widget.
*/
typedef std::vector<image_button_state_descriptor_t> image_button_state_set_t;

/****************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    \param set is the set of button state candidates.
    \param modifier_mask is the related mask to be used for this button_t
    \param modifiers is the bitflag of modifier keys currently held down by the user

    \return Given a modifier mask and modifier key set, will return the
    relevant button state from a set of possible states.
*/
template <typename ButtonStateRange>
typename boost::range_iterator<ButtonStateRange>::type
button_modifier_state(ButtonStateRange& range,
                      modifiers_t       modifier_mask,
                      modifiers_t       modifiers)
{
    typedef typename boost::range_iterator<ButtonStateRange>::type iterator;

    modifiers &= modifier_mask;

    iterator first(boost::begin(range));
    iterator last(boost::end(range));

    for (; first != last; ++first)
        if ((first->modifier_set_m & modifiers) == modifiers)
            break;

    return first;
}

/*!
    \ingroup apl_widgets_button_helper

    \param set is the set of button state candidates.

    \return Given a set of state candidates, will return the
    representative state for when the user has no modifier keys
    depressed.
*/
template <typename ButtonStateRange>
typename boost::range_iterator<ButtonStateRange>::type
button_default_state(ButtonStateRange& range)
{
    typename boost::range_iterator<ButtonStateRange>::type
        result(button_modifier_state(range, modifiers_none_s, modifiers_none_s));

    if (result == boost::end(range))
        throw std::runtime_error("No default state specified for button");

    return result;
}

/*!
    \ingroup apl_widgets_button_helper

    (deprecated)

    \param name is the name of the modifier keys depressed

    \return a modifier bitflag value representing the modifier keys depressed.
*/
modifiers_t name_to_modifer(name_t name);

/*!
    \ingroup apl_widgets_button_helper

    \param modifier_set the modifier key(s). Expected to be either
                        a singleton name_t or an array_t of name_t.

    \return the specified modifier keys as represented by the modifiers_t enumeration.
*/
modifiers_t value_to_modifier(const any_regular_t& modifier_set);

/*************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    This struct is a utility struct that gives a button_t widget the
    ability to model a \ref concept_view for the property model. Because
    a button_t is a set of latches, this struct acts as a shim between
    the property model and the series of potential latches for any given
    button.

    This struct is commonly used during the factory construction of the
    button to bind it to a property model.

    \model_of
        - \ref concept_view
*/
template <typename Control, typename UserData>
struct display_compositor_t
{
    /*! The model type for this view */
    typedef any_regular_t model_type;

    /*!
        \param control is the button to which this struct will apply
        \param user_data is a client-specified type for user information
    */
    display_compositor_t(Control& control, UserData user_data) :
        control_m(control), user_data_m(user_data)
    { }

    /*!
        \param to_value the value to be rerouted to button_t::set for the relevant button
    */
    void display(const any_regular_t& to_value)
    {
        control_m.set(user_data_m, to_value);
    }

#ifndef ADOBE_NO_DOCUMENTATION
    friend bool operator==(const display_compositor_t& lhs, const display_compositor_t& rhs)
    {
        return lhs.control_m == rhs.control_m && lhs.user_data_m == rhs.user_data_m;
    }

    Control& control_m;
    UserData user_data_m;
#endif
};

/*************************************************************************************************/

/*!
    \ingroup apl_widgets_button_helper

    A utility proc to create a display_compositor_t for a given button state

    \param control is the button to which this struct will apply
    \param index specifies which of all the button states this compositor will represent
*/
template <typename Control, typename UserData>
display_compositor_t<Control, UserData>* make_display_compositor(Control& control, UserData index)
{
    return new display_compositor_t<Control, UserData>(control, index);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
