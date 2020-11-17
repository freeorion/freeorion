#ifndef GODOT_CPP_TABS_HPP
#define GODOT_CPP_TABS_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Tabs.hpp"

#include "Control.hpp"
namespace godot {

class InputEvent;
class Texture;

class Tabs : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__on_mouse_exited;
		godot_method_bind *mb__update_hover;
		godot_method_bind *mb_add_tab;
		godot_method_bind *mb_ensure_tab_visible;
		godot_method_bind *mb_get_current_tab;
		godot_method_bind *mb_get_drag_to_rearrange_enabled;
		godot_method_bind *mb_get_offset_buttons_visible;
		godot_method_bind *mb_get_scrolling_enabled;
		godot_method_bind *mb_get_select_with_rmb;
		godot_method_bind *mb_get_tab_align;
		godot_method_bind *mb_get_tab_close_display_policy;
		godot_method_bind *mb_get_tab_count;
		godot_method_bind *mb_get_tab_disabled;
		godot_method_bind *mb_get_tab_icon;
		godot_method_bind *mb_get_tab_offset;
		godot_method_bind *mb_get_tab_rect;
		godot_method_bind *mb_get_tab_title;
		godot_method_bind *mb_get_tabs_rearrange_group;
		godot_method_bind *mb_move_tab;
		godot_method_bind *mb_remove_tab;
		godot_method_bind *mb_set_current_tab;
		godot_method_bind *mb_set_drag_to_rearrange_enabled;
		godot_method_bind *mb_set_scrolling_enabled;
		godot_method_bind *mb_set_select_with_rmb;
		godot_method_bind *mb_set_tab_align;
		godot_method_bind *mb_set_tab_close_display_policy;
		godot_method_bind *mb_set_tab_disabled;
		godot_method_bind *mb_set_tab_icon;
		godot_method_bind *mb_set_tab_title;
		godot_method_bind *mb_set_tabs_rearrange_group;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Tabs"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum CloseButtonDisplayPolicy {
		CLOSE_BUTTON_SHOW_NEVER = 0,
		CLOSE_BUTTON_SHOW_ACTIVE_ONLY = 1,
		CLOSE_BUTTON_SHOW_ALWAYS = 2,
		CLOSE_BUTTON_MAX = 3,
	};
	enum TabAlign {
		ALIGN_LEFT = 0,
		ALIGN_CENTER = 1,
		ALIGN_RIGHT = 2,
		ALIGN_MAX = 3,
	};

	// constants


	static Tabs *_new();

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	void _on_mouse_exited();
	void _update_hover();
	void add_tab(const String title = "", const Ref<Texture> icon = nullptr);
	void ensure_tab_visible(const int64_t idx);
	int64_t get_current_tab() const;
	bool get_drag_to_rearrange_enabled() const;
	bool get_offset_buttons_visible() const;
	bool get_scrolling_enabled() const;
	bool get_select_with_rmb() const;
	Tabs::TabAlign get_tab_align() const;
	Tabs::CloseButtonDisplayPolicy get_tab_close_display_policy() const;
	int64_t get_tab_count() const;
	bool get_tab_disabled(const int64_t tab_idx) const;
	Ref<Texture> get_tab_icon(const int64_t tab_idx) const;
	int64_t get_tab_offset() const;
	Rect2 get_tab_rect(const int64_t tab_idx) const;
	String get_tab_title(const int64_t tab_idx) const;
	int64_t get_tabs_rearrange_group() const;
	void move_tab(const int64_t from, const int64_t to);
	void remove_tab(const int64_t tab_idx);
	void set_current_tab(const int64_t tab_idx);
	void set_drag_to_rearrange_enabled(const bool enabled);
	void set_scrolling_enabled(const bool enabled);
	void set_select_with_rmb(const bool enabled);
	void set_tab_align(const int64_t align);
	void set_tab_close_display_policy(const int64_t policy);
	void set_tab_disabled(const int64_t tab_idx, const bool disabled);
	void set_tab_icon(const int64_t tab_idx, const Ref<Texture> icon);
	void set_tab_title(const int64_t tab_idx, const String title);
	void set_tabs_rearrange_group(const int64_t group_id);

};

}

#endif