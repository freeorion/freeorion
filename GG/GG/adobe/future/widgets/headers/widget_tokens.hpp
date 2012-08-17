/*
    Copyright 2005-2007 Adobe Systems Incorporated
    Distributed under the MIT License (see accompanying file LICENSE_1_0_0.txt
    or a copy at http://stlab.adobe.com/licenses.html)
*/

/****************************************************************************************************/

#ifndef ADOBE_WIDGET_TOKENS_HPP
#define ADOBE_WIDGET_TOKENS_HPP

/****************************************************************************************************/

#include <GG/adobe/name_fwd.hpp>

/****************************************************************************************************/

namespace adobe {

/****************************************************************************************************/

extern aggregate_name_t key_action;
extern aggregate_name_t key_alt_text;
extern aggregate_name_t key_bind;
extern aggregate_name_t key_bind_additional;
extern aggregate_name_t key_bind_controller;
extern aggregate_name_t key_bind_group;
extern aggregate_name_t key_bind_output;
extern aggregate_name_t key_bind_units;
extern aggregate_name_t key_bind_view;
extern aggregate_name_t key_cancel;
extern aggregate_name_t key_characters;
extern aggregate_name_t key_class;
extern aggregate_name_t key_contributing;
extern aggregate_name_t key_count;
extern aggregate_name_t key_custom_item_name;
extern aggregate_name_t key_default;
extern aggregate_name_t key_digits;
extern aggregate_name_t key_display_disable;
extern aggregate_name_t key_domain;
extern aggregate_name_t key_first;
extern aggregate_name_t key_focus;
extern aggregate_name_t key_format;
extern aggregate_name_t key_decimal_places;
extern aggregate_name_t key_trailing_zeroes;
extern aggregate_name_t key_grow;
extern aggregate_name_t key_identifier;
extern aggregate_name_t key_image;
extern aggregate_name_t key_image_on;
extern aggregate_name_t key_image_off;
extern aggregate_name_t key_image_disabled;
extern aggregate_name_t key_increment;
extern aggregate_name_t key_interval_count;
extern aggregate_name_t key_is_indeterminate;
extern aggregate_name_t key_is_relevance;
extern aggregate_name_t key_items;
extern aggregate_name_t key_last;
extern aggregate_name_t key_lines;
extern aggregate_name_t key_localization_set;
extern aggregate_name_t key_max_characters;
extern aggregate_name_t key_max_digits;
extern aggregate_name_t key_max_value;
extern aggregate_name_t key_metal;
extern aggregate_name_t key_min_max_filter;
extern aggregate_name_t key_min_value;
extern aggregate_name_t key_monospaced;
extern aggregate_name_t key_name;
extern aggregate_name_t key_offset_contents;
extern aggregate_name_t key_orientation;
extern aggregate_name_t key_password;
extern aggregate_name_t key_popup_bind;
extern aggregate_name_t key_popup_placement;
extern aggregate_name_t key_popup_value;
extern aggregate_name_t key_scale;
extern aggregate_name_t key_scrollable;
extern aggregate_name_t key_short_name; 
extern aggregate_name_t key_size;
extern aggregate_name_t key_slider_point;
extern aggregate_name_t key_slider_ticks;
extern aggregate_name_t key_target;
extern aggregate_name_t key_touch;
extern aggregate_name_t key_touched;
extern aggregate_name_t key_units;
extern aggregate_name_t key_value;
extern aggregate_name_t key_value_off;
extern aggregate_name_t key_value_on;
extern aggregate_name_t key_wrap;
extern aggregate_name_t key_text_horizontal;
extern aggregate_name_t key_text_vertical;

extern aggregate_name_t key_move;
extern aggregate_name_t key_on_top;
extern aggregate_name_t key_modal;
extern aggregate_name_t key_step;
extern aggregate_name_t key_allow_edits;

extern aggregate_name_t  key_modifier_set;
extern aggregate_name_t  key_modifier_option;
extern aggregate_name_t  key_modifier_command;
extern aggregate_name_t  key_modifier_control;
extern aggregate_name_t  key_modifier_shift;

// REVISIT (sparent) : deprecated modifiers.
extern aggregate_name_t key_modifiers;
extern aggregate_name_t key_modifiers_cmd;
extern aggregate_name_t key_modifiers_ctl;
extern aggregate_name_t key_modifiers_ctlcmd;
extern aggregate_name_t key_modifiers_default;
extern aggregate_name_t key_modifiers_opt;
extern aggregate_name_t key_modifiers_optcmd;
extern aggregate_name_t key_modifiers_optctl;
extern aggregate_name_t key_modifiers_optctlcmd;
// end deprecated

extern aggregate_name_t name_row;
extern aggregate_name_t name_column;
extern aggregate_name_t name_overlay;
extern aggregate_name_t name_reveal;
extern aggregate_name_t name_preset;
extern aggregate_name_t name_preview;
extern aggregate_name_t name_static_text;
extern aggregate_name_t name_control_button;

extern aggregate_name_t name_button;
extern aggregate_name_t name_checkbox;
extern aggregate_name_t name_dialog;
extern aggregate_name_t name_display_number;
extern aggregate_name_t name_edit_number;
extern aggregate_name_t name_edit_text;
extern aggregate_name_t name_group;
extern aggregate_name_t name_image;
extern aggregate_name_t name_label;
extern aggregate_name_t name_link;
extern aggregate_name_t name_list;
extern aggregate_name_t name_message;
extern aggregate_name_t name_optional;
extern aggregate_name_t name_palette;
extern aggregate_name_t name_panel;
extern aggregate_name_t name_popup_cluster;
extern aggregate_name_t name_popup;
extern aggregate_name_t name_progress_bar;
extern aggregate_name_t name_radio_button;
extern aggregate_name_t name_separator;
extern aggregate_name_t name_size_group;
extern aggregate_name_t name_slider;
extern aggregate_name_t name_tab_group;
extern aggregate_name_t name_toggle;

extern aggregate_name_t name_radio_button_group;
extern aggregate_name_t name_menu_bar;
extern aggregate_name_t name_int_spin;
extern aggregate_name_t name_double_spin;

/****************************************************************************************************/

} // namespace adobe

/****************************************************************************************************/

#endif
