/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#include <GG/adobe/future/widgets/headers/widget_tokens.hpp>
#include <GG/adobe/name.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

aggregate_name_t  key_action              = { "action" };
aggregate_name_t  key_alt_text            = { "alt" };
aggregate_name_t  key_bind                = { "bind" };
aggregate_name_t  key_bind_additional     = { "bind_additional" };
aggregate_name_t  key_bind_controller     = { "bind_controller" };
aggregate_name_t  key_bind_group          = { "bind_group" };
aggregate_name_t  key_bind_output         = { "bind_output" };
aggregate_name_t  key_bind_units          = { "bind_units" };
aggregate_name_t  key_bind_view           = { "bind_view" };
aggregate_name_t  key_cancel              = { "cancel" };
aggregate_name_t  key_characters          = { "characters" };
aggregate_name_t  key_class               = { "class" };
aggregate_name_t  key_contributing        = { "contributing" };
aggregate_name_t  key_count               = { "count" };
aggregate_name_t  key_custom_item_name    = { "custom_item_name" };
aggregate_name_t  key_decimal_places      = { "decimal_places" };
aggregate_name_t  key_default             = { "default" };
aggregate_name_t  key_digits              = { "digits" };
aggregate_name_t  key_display_disable     = { "display_disable" };
aggregate_name_t  key_domain              = { "domain" };
aggregate_name_t  key_first               = { "first" };
aggregate_name_t  key_focus               = { "focus" };
aggregate_name_t  key_format              = { "format" };
aggregate_name_t  key_grow                = { "grow" };
aggregate_name_t  key_identifier          = { "identifier" };
aggregate_name_t  key_image               = { "image" };
aggregate_name_t  key_image_on            = { "image_on" };
aggregate_name_t  key_image_off           = { "image_off" };
aggregate_name_t  key_image_disabled      = { "image_disabled" };
aggregate_name_t  key_increment           = { "increment" };
aggregate_name_t  key_interval_count      = { "interval_count" };
aggregate_name_t  key_is_indeterminate    = { "is_indeterminate" };
aggregate_name_t  key_is_relevance        = { "is_relevance" };
aggregate_name_t  key_items               = { "items" };
aggregate_name_t  key_last                = { "last" };
aggregate_name_t  key_lines               = { "lines" };
aggregate_name_t  key_localization_set    = { "localization_set" };
aggregate_name_t  key_max_characters      = { "max_characters" };
aggregate_name_t  key_max_digits          = { "max_digits" };
aggregate_name_t  key_max_value           = { "max_value" };
aggregate_name_t  key_metal               = { "metal" };
aggregate_name_t  key_min_max_filter      = { "min_max_filter" };
aggregate_name_t  key_min_value           = { "min_value" };
aggregate_name_t  key_monospaced          = { "monospaced" };
aggregate_name_t  key_name                = { "name" };
aggregate_name_t  key_offset_contents     = { "offset_contents" };
aggregate_name_t  key_orientation         = { "orientation" };
aggregate_name_t  key_password            = { "password" };
aggregate_name_t  key_popup_bind          = { "popup_bind" };
aggregate_name_t  key_popup_placement     = { "popup_placement" };
aggregate_name_t  key_popup_value         = { "popup_value" };
aggregate_name_t  key_scale               = { "scale" };
aggregate_name_t  key_scrollable          = { "scrollable" };
aggregate_name_t  key_short_name          = { "short_name" };
aggregate_name_t  key_size                = { "size" };
aggregate_name_t  key_slider_point        = { "slider_point" };
aggregate_name_t  key_slider_ticks        = { "slider_ticks" };
aggregate_name_t  key_target              = { "target" };
aggregate_name_t  key_touch               = { "touch" };
aggregate_name_t  key_touched             = { "touched" };
aggregate_name_t  key_trailing_zeroes     = { "trailing_zeroes" };
aggregate_name_t  key_units               = { "units" };
aggregate_name_t  key_value               = { "value" };
aggregate_name_t  key_value_off           = { "value_off" };
aggregate_name_t  key_value_on            = { "value_on" };
aggregate_name_t  key_wrap                = { "wrap" };
aggregate_name_t  key_text_horizontal     = { "text_horizontal" };
aggregate_name_t  key_text_vertical       = { "text_vertical" };

aggregate_name_t key_move                 = { "move" };
aggregate_name_t key_on_top               = { "on_top" };
aggregate_name_t key_modal                = { "modal" };
aggregate_name_t key_step                 = { "step" };
aggregate_name_t key_allow_edits          = { "allow_edits" };

// New modifer names:
aggregate_name_t  key_modifier_set        = { "modifier_set" };
aggregate_name_t  key_modifier_option     = { "option" };
aggregate_name_t  key_modifier_command    = { "command" };
aggregate_name_t  key_modifier_control    = { "control" };
aggregate_name_t  key_modifier_shift      = { "shift" };

// REVISIT = { sparent } : deprecated modifer names.
aggregate_name_t  key_modifiers           = { "modifiers" };
aggregate_name_t  key_modifiers_cmd       = { "cmd" };
aggregate_name_t  key_modifiers_ctl       = { "ctl" };
aggregate_name_t  key_modifiers_ctlcmd    = { "ctlcmd" };
aggregate_name_t  key_modifiers_default   = { "default" };
aggregate_name_t  key_modifiers_opt       = { "opt" };
aggregate_name_t  key_modifiers_optcmd    = { "optcmd" };
aggregate_name_t  key_modifiers_optctl    = { "optctl" };
aggregate_name_t  key_modifiers_optctlcmd = { "optctlcmd" };
// end deprecated modifer names

aggregate_name_t name_row                 = { "row" };
aggregate_name_t name_column              = { "column" };
aggregate_name_t name_overlay             = { "overlay" };
aggregate_name_t name_reveal              = { "reveal" };
aggregate_name_t name_preset              = { "preset" };
aggregate_name_t name_preview             = { "preview" };
aggregate_name_t name_static_text         = { "static_text" };
aggregate_name_t name_control_button      = { "control_button" };

aggregate_name_t  name_button             = { "button" };
aggregate_name_t  name_checkbox           = { "checkbox" };
aggregate_name_t  name_dialog             = { "dialog" };
aggregate_name_t  name_display_number     = { "display_number" };
aggregate_name_t  name_edit_number        = { "edit_number" };
aggregate_name_t  name_edit_text          = { "edit_text" };
aggregate_name_t  name_group              = { "group" };
aggregate_name_t  name_image              = { "image" };
aggregate_name_t  name_label              = { "label" };
aggregate_name_t  name_link               = { "link" };
aggregate_name_t  name_list               = { "list" };
aggregate_name_t  name_message            = { "message" };
aggregate_name_t  name_optional           = { "optional" };
aggregate_name_t  name_palette            = { "palette" };
aggregate_name_t  name_panel              = { "panel" };
aggregate_name_t  name_popup              = { "popup" };
aggregate_name_t  name_popup_cluster      = { "popup_cluster" };
aggregate_name_t  name_progress_bar       = { "progress_bar" };
aggregate_name_t  name_radio_button       = { "radio_button" };
aggregate_name_t  name_separator          = { "separator" };
aggregate_name_t  name_size_group         = { "size_group" };
aggregate_name_t  name_slider             = { "slider" };
aggregate_name_t  name_tab_group          = { "tab_group" };
aggregate_name_t  name_toggle             = { "toggle" };

aggregate_name_t  name_radio_button_group = { "radio_button_group" };
aggregate_name_t  name_menu_bar           = { "menu_bar" };
aggregate_name_t  name_int_spin           = { "int_spin" };
aggregate_name_t  name_double_spin        = { "double_spin" };

/****************************************************************************************************/

} //namespace adobe

/****************************************************************************************************/
