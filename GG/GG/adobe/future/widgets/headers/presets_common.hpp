/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_PRESETS_COMMON_HPP
#define ADOBE_WIDGET_PRESETS_COMMON_HPP

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

namespace implementation {

/****************************************************************************************************/

void reload_preset_widget(presets_t& control);

void append_user_preset(presets_t& control, const dictionary_t& snapshot);
void delete_user_preset(presets_t& control);

void save_user_preset_set(presets_t& control);
void append_user_preset_set(presets_t& control);

// found in the os-specific '.../preferences/top_level_folder/domain/name.presets" file
array_t load_user_preset_set(presets_t& control);

// found in the same location that the other resources for the preset widget are found
array_t load_default_preset_set(presets_t& control);

// actual implementation of this should be in platform-specific space. It is the path
// to the platform-designated preferences folder, then into the folder specified by
// the domain of the preset.
boost::filesystem::path preset_directory(const presets_t& control);

// fetches strings out of the localization set (if available)
// this is *different* than the global localization function. It is used to fetch *keys*
// to be sent to the global localization proc at runtime so the proc can get the correct
// localized value back. This should only be used for private, static strings that the
// client could not otherwise modify (e.g., the button names in the subdialogs for the
// preset widget)
std::string localization_value(const dictionary_t& set, name_t key, const std::string& default_string);

inline std::string localization_value(const presets_t& control, name_t key, const std::string& default_string)
{ return localization_value(control.localization_set_m, key, default_string); }

/****************************************************************************************************/

} // namespace implementation

/****************************************************************************************************/

// The following list are for the dictionary containing the localized strings for the built-in
// dialogs withing the preset widget.
extern aggregate_name_t key_preset_add_dialog_group_name;
extern aggregate_name_t key_preset_add_dialog_message;
extern aggregate_name_t key_preset_add_dialog_name;
extern aggregate_name_t key_preset_add_subdialog_cancel_button_alt_text;
extern aggregate_name_t key_preset_add_subdialog_default_preset_name;
extern aggregate_name_t key_preset_add_subdialog_ok_button_alt_text;
extern aggregate_name_t key_preset_category_popup_alt_text;
extern aggregate_name_t key_preset_category_popup_name;
extern aggregate_name_t key_preset_custom_category_name;
extern aggregate_name_t key_preset_delete_dialog_message;
extern aggregate_name_t key_preset_delete_dialog_name;
extern aggregate_name_t key_preset_delete_subdialog_cancel_button_alt_text;
extern aggregate_name_t key_preset_delete_subdialog_ok_button_alt_text;
extern aggregate_name_t key_preset_file_extension;
extern aggregate_name_t key_preset_menu_item_add_preset;
extern aggregate_name_t key_preset_menu_item_append_preset;
extern aggregate_name_t key_preset_menu_item_delete_preset;
extern aggregate_name_t key_preset_preset_popup_alt_text;
extern aggregate_name_t key_preset_preset_popup_name;
extern aggregate_name_t key_preset_subdialog_cancel_button_name;
extern aggregate_name_t key_preset_subdialog_ok_button_name;
extern aggregate_name_t key_preset_top_folder_name;
extern aggregate_name_t key_preset_user_preset_list_empty_warning;
extern aggregate_name_t key_preset_user_presets_category_name;

// The following are the names given to the structural components of the presets in memory.
extern aggregate_name_t key_preset_value; // The dictionary of property values for a single preset
extern aggregate_name_t key_preset_name;  // The names of the categories and the presets themselves
extern aggregate_name_t key_preset_items; // The set of presets listed for a single category

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif

/****************************************************************************************************/
