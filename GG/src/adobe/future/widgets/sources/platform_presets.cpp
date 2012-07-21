/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/platform_presets.hpp>
#include <GG/adobe/future/widgets/headers/presets_common.hpp>
#include <GG/adobe/future/widgets/headers/display.hpp>

#include <GG/adobe/name.hpp>
#include <GG/adobe/future/resources.hpp>
#include <GG/adobe/future/widgets/headers/widget_utils.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include <GG/Button.h>
#include <GG/GUI.h>
#include <GG/StyleFactory.h>


/****************************************************************************************************/

namespace {

/****************************************************************************************************/

GG::SubTexture normal_image()
{
    static boost::shared_ptr<GG::Texture> texture_s;
    if (!texture_s)
        texture_s = GG::GUI::GetGUI()->GetTexture("preset_button_unpressed.png");
    return GG::SubTexture(texture_s, GG::X0, GG::Y0, texture_s->Width(), texture_s->Height());
}

/****************************************************************************************************/

GG::SubTexture clicked_image()
{
    static boost::shared_ptr<GG::Texture> texture_s;
    if (!texture_s)
        texture_s = GG::GUI::GetGUI()->GetTexture("preset_button_pressed.png");
    return GG::SubTexture(texture_s, GG::X0, GG::Y0, texture_s->Width(), texture_s->Height());
}

/****************************************************************************************************/

GG::SubTexture disabled_image()
{
    static boost::shared_ptr<GG::Texture> texture_s;
    if (!texture_s)
        texture_s = GG::GUI::GetGUI()->GetTexture("preset_button_disabled.png");
    return GG::SubTexture(texture_s, GG::X0, GG::Y0, texture_s->Width(), texture_s->Height());
}

/****************************************************************************************************/

void reset_textures(adobe::presets_t& preset)
{
    if (!preset.control_m->Disabled()) {
        preset.control_m->SetUnpressedGraphic(normal_image());
        preset.control_m->SetPressedGraphic(clicked_image());
        preset.control_m->SetRolloverGraphic(normal_image());
    } else {
        preset.control_m->SetUnpressedGraphic(disabled_image());
        preset.control_m->SetPressedGraphic(disabled_image());
        preset.control_m->SetRolloverGraphic(disabled_image());
    }
}

/****************************************************************************************************/

void presets_button_clicked(adobe::presets_t& preset)
{
    preset.selected_m = true;

    adobe::name_t append_presets(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_append_preset, "Append Preset...").c_str());
    adobe::name_t add_preset(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_add_preset, "Add Preset...").c_str());
    adobe::name_t delete_preset(adobe::implementation::localization_value(preset, adobe::key_preset_menu_item_delete_preset, "Delete Preset...").c_str());
#ifndef NDEBUG
    adobe::static_name_t resave_presets("(debug) Re-save Presets To File");
#endif

    adobe::name_t options[] =
    {
        append_presets,
        add_preset,
        delete_preset,
#ifndef NDEBUG
        resave_presets
#endif
    };
    std::size_t   num_options(sizeof(options) / sizeof(options[0]));
    adobe::name_t choice;

    if (adobe::context_menu(GG::Pt(preset.control_m->UpperLeft().x + 13,
                                   preset.control_m->UpperLeft().y + 10),
                            options, options + num_options,
                            choice)) {
        if (choice == add_preset && preset.snapshot_proc_m)
            adobe::implementation::append_user_preset(preset, preset.snapshot_proc_m());
        else if (choice == append_presets)
            adobe::implementation::append_user_preset_set(preset);
        else if (choice == delete_preset)
            adobe::implementation::delete_user_preset(preset);
#ifndef NDEBUG
        else if (choice == resave_presets)
            adobe::implementation::save_user_preset_set(preset);
#endif
    }

    preset.selected_m = false;
}

/****************************************************************************************************/

} // namespace

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

presets_t::presets_t(const std::string&         name,
                     const std::string&         path,
                     const std::string&         alt_text,
                     const array_t&             bind_set,
                     const dictionary_t&        localization_set,
                     theme_t                    theme) :
    control_m(0),
    localization_set_m(localization_set),
    category_popup_m(implementation::localization_value(localization_set, 
                        key_preset_category_popup_name, "Category:"), 
                     implementation::localization_value(localization_set, 
                        key_preset_category_popup_alt_text, 
                        "Select a category of presets for this dialog"), 
                     implementation::localization_value(localization_set, 
                        key_preset_custom_category_name, "Custom"), 
                     0, 0, theme),
    popup_m(implementation::localization_value(localization_set, 
                key_preset_preset_popup_name, "Preset:"), 
            implementation::localization_value(localization_set, 
                key_preset_preset_popup_alt_text, 
                "Select a preset for settings in this dialog"), 
            implementation::localization_value(localization_set, 
                key_preset_custom_category_name, "Custom"), 
            0, 0, theme),
    theme_m(theme),
    bind_set_m(bind_set),
    name_m(name),
    path_m(path),
    alt_text_m(alt_text),
    selected_m(false),
    type_2_debounce_m(false),
    custom_m(false)
{ }

/****************************************************************************************************/

void presets_t::measure(extents_t& result)
{
    assert(control_m);

    popup_m.measure(result);

    result.width() += 4 + 26; // gap + icon
    result.height() = (std::max)(result.height(), 21L);

    extents_t cat_result;

    category_popup_m.measure(cat_result);

    result.width() = (std::max)(result.width(), cat_result.width());

    // REVISIT (fbrereto) : This presumes the popups are of the same height.

    popup_height_m = cat_result.height();

    result.height() += popup_height_m + 4;

    if (!result.horizontal().guide_set_m.empty() &&
        !cat_result.horizontal().guide_set_m.empty()) {
        result.horizontal().guide_set_m[0] =
            (std::max)(result.horizontal().guide_set_m[0],
                       cat_result.horizontal().guide_set_m[0]);
    }
}

/****************************************************************************************************/

void presets_t::place(const place_data_t& place_data)
{
    assert(control_m);

    place_data_t category_place(place_data);
    place_data_t popup_place(place_data);
    place_data_t icon_place(place_data);

    // set up the top (category) popup
    width(category_place) -= 4 + 26;
    height(category_place) = popup_height_m;

    // set up the bottom popup
    top(popup_place) = bottom(place_data) - popup_height_m;
    width(popup_place) -= 4 + 26;
    height(popup_place) = popup_height_m;

    // set up the icon
    width(icon_place) = 26;
    height(icon_place) = 21;
    left(icon_place) += width(popup_place) + 4;

    popup_m.place(popup_place);
    category_popup_m.place(category_place);

    implementation::set_control_bounds(control_m, icon_place);
}

/****************************************************************************************************/             

// REVISIT: MM--we need to replace the display_t mechanism with concepts/any_*/
//              container idiom for event and drawing system.

template <> platform_display_type insert<presets_t>(display_t&             display,
                                                    platform_display_type& parent,
                                                    presets_t&             element)
{
    assert(!element.control_m);

    insert(display, parent, element.category_popup_m);

    insert(display, parent, element.popup_m);

    GG::X width(26);
    GG::Y height(21);
    boost::shared_ptr<GG::StyleFactory> style = GG::GUI::GetGUI()->GetStyleFactory();
    element.control_m = style->NewButton(GG::X0, GG::Y0, width, height,
                                         "", style->DefaultFont(), GG::CLR_GRAY);
    reset_textures(element);

    GG::Connect(element.control_m->ClickedSignal,
                boost::bind(&presets_button_clicked, boost::ref(element)));

    element.popup_m.monitor(boost::bind(&presets_t::do_imbue, boost::ref(element), _1));

    if (!element.alt_text_m.empty())
        adobe::implementation::set_control_alt_text(element.control_m, element.alt_text_m);

    return display.insert(parent, element.control_m);
}

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

boost::filesystem::path preset_directory(const presets_t& control)
{
    boost::filesystem::path result(control.path_m);
    boost::filesystem::create_directory(result);
    return result;
}

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

void enable(presets_t& control, bool enable)
{
    assert(control.control_m);
    control.control_m->Disable(!enable);
    reset_textures(control);
}

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/
