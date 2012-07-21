/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_PRESETS_HPP
#define ADOBE_WIDGET_PRESETS_HPP

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_popup.hpp>

#include <GG/adobe/array.hpp>
#include <GG/adobe/dictionary.hpp>
#include <GG/adobe/widget_attributes.hpp>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>


namespace GG {
    class Button;
}

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

struct presets_t;

/****************************************************************************************************/

typedef boost::function<dictionary_t ()> model_snapshot_proc_t;

/****************************************************************************************************/

/*!
    \ingroup ui_core

    \brief UI core presets widget

    The semantics of a preset widget is a widget that can save and restore "snapshots" of the state
    of the current model. These snapshots are given a name and can be recalled and imposed upon a
    model at any time, allowing for the user of the model to preserve, well, presets.
*/

struct presets_t : boost::noncopyable
{
    typedef dictionary_t            model_type;
    typedef popup_t::setter_type    setter_type;

    /*!
        \param name             Preset file name (xyz becomes xyz.preset)
        \param path             Path to presets files (usually ~/.[the name of the application] or %APP_DATA%/[the name of the application])
        \param alt_text         Additional help text for the widget when the user pauses over it
        \param bind_set         Array of name/string pairs that area values which will be saved/restored by presets
        \param localization_set Set of named arguments that contain the basic string keys for
                                localization of widget-private resources (e.g., the keys used
                                in the add/delete subdialogs to the preset widget).
        \param theme            Theme for the widget

        \note
            If the presets is set to a value other than <code>true_value</code> or
            <code>false_value</code>, the presets gets a 'dash' (undefined) state.
    */
    presets_t(const std::string&         name,
              const std::string&         path,
              const std::string&         alt_text,
              const array_t&             bind_set,
              const dictionary_t&        localization_set,
              theme_t                    theme);


    /*!
        \param result The ideal extent values for this widget given its current attributes
    */
    void measure(extents_t& result);

    /*!
        \param place_data Position information to apply to the widget
    */
    void place(const place_data_t& place_data);

    /*!
        \param value Preset settings to which we need to be set
    */
    void display(const model_type& value);

    /*!
        \param value Set of additional preset data to be added to the preset listings
    */
    void display_additional_preset_set(const array_t& value);

    /*!
        \param proc  Procedure called to imbue the model with preset data
    */
    void monitor(const setter_type& proc)
    { proc_m = proc; }

    /*!
        \param proc  Procedure called when a model snapshot is needed
    */
    void snapshot_callback(const model_snapshot_proc_t& proc)
    { snapshot_proc_m = proc; }

    GG::Button*           control_m;
    dictionary_t          localization_set_m;
    popup_t               category_popup_m; // the category popup
    popup_t               popup_m;          // the actual preset popup
    theme_t               theme_m;
    array_t               bind_set_m; // the set of parameters to be stored in the preset
    std::string           name_m;
    std::string           path_m;
    std::string           alt_text_m;
    bool                  selected_m;
    long                  popup_height_m;
    model_snapshot_proc_t snapshot_proc_m;
    dictionary_t          last_m;
    bool                  type_2_debounce_m;
    setter_type           proc_m;
    bool                  custom_m;

    // these store the various preset sources as arrays
    array_t               dynamic_preset_set_m;
    array_t               default_preset_set_m;
    array_t               user_preset_set_m;
    array_t               composite_m;

    void do_imbue(const popup_t::model_type& value);

private:
    void display_custom();
};

/****************************************************************************************************/

/*!
    \param make_enabled Specifies whether or not to show this widget enabled
*/

void enable(presets_t& value, bool make_enabled);

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
