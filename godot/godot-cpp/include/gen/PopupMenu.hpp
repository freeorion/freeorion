#ifndef GODOT_CPP_POPUPMENU_HPP
#define GODOT_CPP_POPUPMENU_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Popup.hpp"
namespace godot {

class InputEvent;
class ShortCut;
class Texture;

class PopupMenu : public Popup {
	struct ___method_bindings {
		godot_method_bind *mb__get_items;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__set_items;
		godot_method_bind *mb__submenu_timeout;
		godot_method_bind *mb_add_check_item;
		godot_method_bind *mb_add_check_shortcut;
		godot_method_bind *mb_add_icon_check_item;
		godot_method_bind *mb_add_icon_check_shortcut;
		godot_method_bind *mb_add_icon_item;
		godot_method_bind *mb_add_icon_radio_check_item;
		godot_method_bind *mb_add_icon_radio_check_shortcut;
		godot_method_bind *mb_add_icon_shortcut;
		godot_method_bind *mb_add_item;
		godot_method_bind *mb_add_multistate_item;
		godot_method_bind *mb_add_radio_check_item;
		godot_method_bind *mb_add_radio_check_shortcut;
		godot_method_bind *mb_add_separator;
		godot_method_bind *mb_add_shortcut;
		godot_method_bind *mb_add_submenu_item;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_get_allow_search;
		godot_method_bind *mb_get_item_accelerator;
		godot_method_bind *mb_get_item_count;
		godot_method_bind *mb_get_item_icon;
		godot_method_bind *mb_get_item_id;
		godot_method_bind *mb_get_item_index;
		godot_method_bind *mb_get_item_metadata;
		godot_method_bind *mb_get_item_shortcut;
		godot_method_bind *mb_get_item_submenu;
		godot_method_bind *mb_get_item_text;
		godot_method_bind *mb_get_item_tooltip;
		godot_method_bind *mb_get_submenu_popup_delay;
		godot_method_bind *mb_is_hide_on_checkable_item_selection;
		godot_method_bind *mb_is_hide_on_item_selection;
		godot_method_bind *mb_is_hide_on_state_item_selection;
		godot_method_bind *mb_is_hide_on_window_lose_focus;
		godot_method_bind *mb_is_item_checkable;
		godot_method_bind *mb_is_item_checked;
		godot_method_bind *mb_is_item_disabled;
		godot_method_bind *mb_is_item_radio_checkable;
		godot_method_bind *mb_is_item_separator;
		godot_method_bind *mb_is_item_shortcut_disabled;
		godot_method_bind *mb_remove_item;
		godot_method_bind *mb_set_allow_search;
		godot_method_bind *mb_set_hide_on_checkable_item_selection;
		godot_method_bind *mb_set_hide_on_item_selection;
		godot_method_bind *mb_set_hide_on_state_item_selection;
		godot_method_bind *mb_set_hide_on_window_lose_focus;
		godot_method_bind *mb_set_item_accelerator;
		godot_method_bind *mb_set_item_as_checkable;
		godot_method_bind *mb_set_item_as_radio_checkable;
		godot_method_bind *mb_set_item_as_separator;
		godot_method_bind *mb_set_item_checked;
		godot_method_bind *mb_set_item_disabled;
		godot_method_bind *mb_set_item_icon;
		godot_method_bind *mb_set_item_id;
		godot_method_bind *mb_set_item_metadata;
		godot_method_bind *mb_set_item_multistate;
		godot_method_bind *mb_set_item_shortcut;
		godot_method_bind *mb_set_item_shortcut_disabled;
		godot_method_bind *mb_set_item_submenu;
		godot_method_bind *mb_set_item_text;
		godot_method_bind *mb_set_item_tooltip;
		godot_method_bind *mb_set_submenu_popup_delay;
		godot_method_bind *mb_toggle_item_checked;
		godot_method_bind *mb_toggle_item_multistate;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "PopupMenu"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static PopupMenu *_new();

	// methods
	Array _get_items() const;
	void _gui_input(const Ref<InputEvent> arg0);
	void _set_items(const Array arg0);
	void _submenu_timeout();
	void add_check_item(const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_check_shortcut(const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_icon_check_item(const Ref<Texture> texture, const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_icon_check_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_icon_item(const Ref<Texture> texture, const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_icon_radio_check_item(const Ref<Texture> texture, const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_icon_radio_check_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_icon_shortcut(const Ref<Texture> texture, const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_item(const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_multistate_item(const String label, const int64_t max_states, const int64_t default_state = 0, const int64_t id = -1, const int64_t accel = 0);
	void add_radio_check_item(const String label, const int64_t id = -1, const int64_t accel = 0);
	void add_radio_check_shortcut(const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_separator(const String label = "");
	void add_shortcut(const Ref<ShortCut> shortcut, const int64_t id = -1, const bool global = false);
	void add_submenu_item(const String label, const String submenu, const int64_t id = -1);
	void clear();
	bool get_allow_search() const;
	int64_t get_item_accelerator(const int64_t idx) const;
	int64_t get_item_count() const;
	Ref<Texture> get_item_icon(const int64_t idx) const;
	int64_t get_item_id(const int64_t idx) const;
	int64_t get_item_index(const int64_t id) const;
	Variant get_item_metadata(const int64_t idx) const;
	Ref<ShortCut> get_item_shortcut(const int64_t idx) const;
	String get_item_submenu(const int64_t idx) const;
	String get_item_text(const int64_t idx) const;
	String get_item_tooltip(const int64_t idx) const;
	real_t get_submenu_popup_delay() const;
	bool is_hide_on_checkable_item_selection() const;
	bool is_hide_on_item_selection() const;
	bool is_hide_on_state_item_selection() const;
	bool is_hide_on_window_lose_focus() const;
	bool is_item_checkable(const int64_t idx) const;
	bool is_item_checked(const int64_t idx) const;
	bool is_item_disabled(const int64_t idx) const;
	bool is_item_radio_checkable(const int64_t idx) const;
	bool is_item_separator(const int64_t idx) const;
	bool is_item_shortcut_disabled(const int64_t idx) const;
	void remove_item(const int64_t idx);
	void set_allow_search(const bool allow);
	void set_hide_on_checkable_item_selection(const bool enable);
	void set_hide_on_item_selection(const bool enable);
	void set_hide_on_state_item_selection(const bool enable);
	void set_hide_on_window_lose_focus(const bool enable);
	void set_item_accelerator(const int64_t idx, const int64_t accel);
	void set_item_as_checkable(const int64_t idx, const bool enable);
	void set_item_as_radio_checkable(const int64_t idx, const bool enable);
	void set_item_as_separator(const int64_t idx, const bool enable);
	void set_item_checked(const int64_t idx, const bool checked);
	void set_item_disabled(const int64_t idx, const bool disabled);
	void set_item_icon(const int64_t idx, const Ref<Texture> icon);
	void set_item_id(const int64_t idx, const int64_t id);
	void set_item_metadata(const int64_t idx, const Variant metadata);
	void set_item_multistate(const int64_t idx, const int64_t state);
	void set_item_shortcut(const int64_t idx, const Ref<ShortCut> shortcut, const bool global = false);
	void set_item_shortcut_disabled(const int64_t idx, const bool disabled);
	void set_item_submenu(const int64_t idx, const String submenu);
	void set_item_text(const int64_t idx, const String text);
	void set_item_tooltip(const int64_t idx, const String tooltip);
	void set_submenu_popup_delay(const real_t seconds);
	void toggle_item_checked(const int64_t idx);
	void toggle_item_multistate(const int64_t idx);

};

}

#endif