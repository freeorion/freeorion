#ifndef GODOT_CPP_BASEBUTTON_HPP
#define GODOT_CPP_BASEBUTTON_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "BaseButton.hpp"
#include "Control.hpp"

#include "Control.hpp"
namespace godot {

class InputEvent;
class ButtonGroup;
class ShortCut;

class BaseButton : public Control {
	struct ___method_bindings {
		godot_method_bind *mb__gui_input;
		godot_method_bind *mb__pressed;
		godot_method_bind *mb__toggled;
		godot_method_bind *mb__unhandled_input;
		godot_method_bind *mb_get_action_mode;
		godot_method_bind *mb_get_button_group;
		godot_method_bind *mb_get_button_mask;
		godot_method_bind *mb_get_draw_mode;
		godot_method_bind *mb_get_enabled_focus_mode;
		godot_method_bind *mb_get_shortcut;
		godot_method_bind *mb_is_disabled;
		godot_method_bind *mb_is_hovered;
		godot_method_bind *mb_is_keep_pressed_outside;
		godot_method_bind *mb_is_pressed;
		godot_method_bind *mb_is_shortcut_in_tooltip_enabled;
		godot_method_bind *mb_is_toggle_mode;
		godot_method_bind *mb_set_action_mode;
		godot_method_bind *mb_set_button_group;
		godot_method_bind *mb_set_button_mask;
		godot_method_bind *mb_set_disabled;
		godot_method_bind *mb_set_enabled_focus_mode;
		godot_method_bind *mb_set_keep_pressed_outside;
		godot_method_bind *mb_set_pressed;
		godot_method_bind *mb_set_shortcut;
		godot_method_bind *mb_set_shortcut_in_tooltip;
		godot_method_bind *mb_set_toggle_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "BaseButton"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum ActionMode {
		ACTION_MODE_BUTTON_PRESS = 0,
		ACTION_MODE_BUTTON_RELEASE = 1,
	};
	enum DrawMode {
		DRAW_NORMAL = 0,
		DRAW_PRESSED = 1,
		DRAW_HOVER = 2,
		DRAW_DISABLED = 3,
		DRAW_HOVER_PRESSED = 4,
	};

	// constants

	// methods
	void _gui_input(const Ref<InputEvent> arg0);
	void _pressed();
	void _toggled(const bool button_pressed);
	void _unhandled_input(const Ref<InputEvent> arg0);
	BaseButton::ActionMode get_action_mode() const;
	Ref<ButtonGroup> get_button_group() const;
	int64_t get_button_mask() const;
	BaseButton::DrawMode get_draw_mode() const;
	Control::FocusMode get_enabled_focus_mode() const;
	Ref<ShortCut> get_shortcut() const;
	bool is_disabled() const;
	bool is_hovered() const;
	bool is_keep_pressed_outside() const;
	bool is_pressed() const;
	bool is_shortcut_in_tooltip_enabled() const;
	bool is_toggle_mode() const;
	void set_action_mode(const int64_t mode);
	void set_button_group(const Ref<ButtonGroup> button_group);
	void set_button_mask(const int64_t mask);
	void set_disabled(const bool disabled);
	void set_enabled_focus_mode(const int64_t mode);
	void set_keep_pressed_outside(const bool enabled);
	void set_pressed(const bool pressed);
	void set_shortcut(const Ref<ShortCut> shortcut);
	void set_shortcut_in_tooltip(const bool enabled);
	void set_toggle_mode(const bool enabled);

};

}

#endif