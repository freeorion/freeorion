#ifndef GODOT_CPP_SCROLLCONTAINER_HPP
#define GODOT_CPP_SCROLLCONTAINER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Container.hpp"
namespace godot {

class Control;
class InputEvent;
class HScrollBar;
class VScrollBar;

class ScrollContainer : public Container {
	struct ___method_bindings {
		godot_method_bind *mb__ensure_focused_visible;
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__scroll_moved;
		godot_method_bind *mb__update_scrollbar_position;
		godot_method_bind *mb_get_deadzone;
		godot_method_bind *mb_get_h_scroll;
		godot_method_bind *mb_get_h_scrollbar;
		godot_method_bind *mb_get_v_scroll;
		godot_method_bind *mb_get_v_scrollbar;
		godot_method_bind *mb_is_following_focus;
		godot_method_bind *mb_is_h_scroll_enabled;
		godot_method_bind *mb_is_v_scroll_enabled;
		godot_method_bind *mb_set_deadzone;
		godot_method_bind *mb_set_enable_h_scroll;
		godot_method_bind *mb_set_enable_v_scroll;
		godot_method_bind *mb_set_follow_focus;
		godot_method_bind *mb_set_h_scroll;
		godot_method_bind *mb_set_v_scroll;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "ScrollContainer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static ScrollContainer *_new();

	// methods
	void _ensure_focused_visible(const Control *arg0);
	void _gui_input(const Ref<InputEvent> arg0);
	void _scroll_moved(const real_t arg0);
	void _update_scrollbar_position();
	int64_t get_deadzone() const;
	int64_t get_h_scroll() const;
	HScrollBar *get_h_scrollbar();
	int64_t get_v_scroll() const;
	VScrollBar *get_v_scrollbar();
	bool is_following_focus() const;
	bool is_h_scroll_enabled() const;
	bool is_v_scroll_enabled() const;
	void set_deadzone(const int64_t deadzone);
	void set_enable_h_scroll(const bool enable);
	void set_enable_v_scroll(const bool enable);
	void set_follow_focus(const bool enabled);
	void set_h_scroll(const int64_t value);
	void set_v_scroll(const int64_t value);

};

}

#endif