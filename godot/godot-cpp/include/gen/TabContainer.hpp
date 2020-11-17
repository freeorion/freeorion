#ifndef GODOT_CPP_TABCONTAINER_HPP
#define GODOT_CPP_TABCONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "TabContainer.hpp"

#include "Container.hpp"
namespace godot {

class InputEvent;
class Control;
class Popup;
class Texture;
class Node;

class TabContainer : public Container {
	struct ___method_bindings {
		godot_method_bind *mb__child_renamed_callback;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__on_mouse_exited;
		godot_method_bind *mb__on_theme_changed;
		godot_method_bind *mb__update_current_tab;
		godot_method_bind *mb_are_tabs_visible;
		godot_method_bind *mb_get_current_tab;
		godot_method_bind *mb_get_current_tab_control;
		godot_method_bind *mb_get_drag_to_rearrange_enabled;
		godot_method_bind *mb_get_popup;
		godot_method_bind *mb_get_previous_tab;
		godot_method_bind *mb_get_tab_align;
		godot_method_bind *mb_get_tab_control;
		godot_method_bind *mb_get_tab_count;
		godot_method_bind *mb_get_tab_disabled;
		godot_method_bind *mb_get_tab_icon;
		godot_method_bind *mb_get_tab_title;
		godot_method_bind *mb_get_tabs_rearrange_group;
		godot_method_bind *mb_get_use_hidden_tabs_for_min_size;
		godot_method_bind *mb_set_current_tab;
		godot_method_bind *mb_set_drag_to_rearrange_enabled;
		godot_method_bind *mb_set_popup;
		godot_method_bind *mb_set_tab_align;
		godot_method_bind *mb_set_tab_disabled;
		godot_method_bind *mb_set_tab_icon;
		godot_method_bind *mb_set_tab_title;
		godot_method_bind *mb_set_tabs_rearrange_group;
		godot_method_bind *mb_set_tabs_visible;
		godot_method_bind *mb_set_use_hidden_tabs_for_min_size;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "TabContainer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TabAlign {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
	};

	// constants


	static TabContainer *_new();

	// methods
	void _child_renamed_callback();
	void _gui_input(const Ref<InputEvent> arg0);
	void _on_mouse_exited();
	void _on_theme_changed();
	void _update_current_tab();
	bool are_tabs_visible() const;
	int64_t get_current_tab() const;
	Control *get_current_tab_control() const;
	bool get_drag_to_rearrange_enabled() const;
	Popup *get_popup() const;
	int64_t get_previous_tab() const;
	TabContainer::TabAlign get_tab_align() const;
	Control *get_tab_control(const int64_t idx) const;
	int64_t get_tab_count() const;
	bool get_tab_disabled(const int64_t tab_idx) const;
	Ref<Texture> get_tab_icon(const int64_t tab_idx) const;
	String get_tab_title(const int64_t tab_idx) const;
	int64_t get_tabs_rearrange_group() const;
	bool get_use_hidden_tabs_for_min_size() const;
	void set_current_tab(const int64_t tab_idx);
	void set_drag_to_rearrange_enabled(const bool enabled);
	void set_popup(const Node *popup);
	void set_tab_align(const int64_t align);
	void set_tab_disabled(const int64_t tab_idx, const bool disabled);
	void set_tab_icon(const int64_t tab_idx, const Ref<Texture> icon);
	void set_tab_title(const int64_t tab_idx, const String title);
	void set_tabs_rearrange_group(const int64_t group_id);
	void set_tabs_visible(const bool visible);
	void set_use_hidden_tabs_for_min_size(const bool enabled);

};

}

#endif