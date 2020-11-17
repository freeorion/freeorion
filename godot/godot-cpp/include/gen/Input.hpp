#ifndef GODOT_CPP_INPUT_HPP
#define GODOT_CPP_INPUT_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Input.hpp"

#include "Object.hpp"
namespace godot {

class InputEvent;
class Resource;

class Input : public Object {
	static Input *_singleton;

	Input();

	struct ___method_bindings {
		godot_method_bind *mb_action_press;
		godot_method_bind *mb_action_release;
		godot_method_bind *mb_add_joy_mapping;
		godot_method_bind *mb_get_accelerometer;
		godot_method_bind *mb_get_action_strength;
		godot_method_bind *mb_get_connected_joypads;
		godot_method_bind *mb_get_current_cursor_shape;
		godot_method_bind *mb_get_gravity;
		godot_method_bind *mb_get_gyroscope;
		godot_method_bind *mb_get_joy_axis;
		godot_method_bind *mb_get_joy_axis_index_from_string;
		godot_method_bind *mb_get_joy_axis_string;
		godot_method_bind *mb_get_joy_button_index_from_string;
		godot_method_bind *mb_get_joy_button_string;
		godot_method_bind *mb_get_joy_guid;
		godot_method_bind *mb_get_joy_name;
		godot_method_bind *mb_get_joy_vibration_duration;
		godot_method_bind *mb_get_joy_vibration_strength;
		godot_method_bind *mb_get_last_mouse_speed;
		godot_method_bind *mb_get_magnetometer;
		godot_method_bind *mb_get_mouse_button_mask;
		godot_method_bind *mb_get_mouse_mode;
		godot_method_bind *mb_is_action_just_pressed;
		godot_method_bind *mb_is_action_just_released;
		godot_method_bind *mb_is_action_pressed;
		godot_method_bind *mb_is_joy_button_pressed;
		godot_method_bind *mb_is_joy_known;
		godot_method_bind *mb_is_key_pressed;
		godot_method_bind *mb_is_mouse_button_pressed;
		godot_method_bind *mb_joy_connection_changed;
		godot_method_bind *mb_parse_input_event;
		godot_method_bind *mb_remove_joy_mapping;
		godot_method_bind *mb_set_custom_mouse_cursor;
		godot_method_bind *mb_set_default_cursor_shape;
		godot_method_bind *mb_set_mouse_mode;
		godot_method_bind *mb_set_use_accumulated_input;
		godot_method_bind *mb_start_joy_vibration;
		godot_method_bind *mb_stop_joy_vibration;
		godot_method_bind *mb_vibrate_handheld;
		godot_method_bind *mb_warp_mouse_position;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline Input *get_singleton()
	{
		if (!Input::_singleton) {
			Input::_singleton = new Input;
		}
		return Input::_singleton;
	}

	static inline const char *___get_class_name() { return (const char *) "Input"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum MouseMode {
		MOUSE_MODE_VISIBLE = 0,
		MOUSE_MODE_HIDDEN = 1,
		MOUSE_MODE_CAPTURED = 2,
		MOUSE_MODE_CONFINED = 3,
	};
	enum CursorShape {
		CURSOR_ARROW = 0,
		CURSOR_IBEAM = 1,
		CURSOR_POINTING_HAND = 2,
		CURSOR_CROSS = 3,
		CURSOR_WAIT = 4,
		CURSOR_BUSY = 5,
		CURSOR_DRAG = 6,
		CURSOR_CAN_DROP = 7,
		CURSOR_FORBIDDEN = 8,
		CURSOR_VSIZE = 9,
		CURSOR_HSIZE = 10,
		CURSOR_BDIAGSIZE = 11,
		CURSOR_FDIAGSIZE = 12,
		CURSOR_MOVE = 13,
		CURSOR_VSPLIT = 14,
		CURSOR_HSPLIT = 15,
		CURSOR_HELP = 16,
	};

	// constants

	// methods
	void action_press(const String action, const real_t strength = 1);
	void action_release(const String action);
	void add_joy_mapping(const String mapping, const bool update_existing = false);
	Vector3 get_accelerometer() const;
	real_t get_action_strength(const String action) const;
	Array get_connected_joypads();
	Input::CursorShape get_current_cursor_shape() const;
	Vector3 get_gravity() const;
	Vector3 get_gyroscope() const;
	real_t get_joy_axis(const int64_t device, const int64_t axis) const;
	int64_t get_joy_axis_index_from_string(const String axis);
	String get_joy_axis_string(const int64_t axis_index);
	int64_t get_joy_button_index_from_string(const String button);
	String get_joy_button_string(const int64_t button_index);
	String get_joy_guid(const int64_t device) const;
	String get_joy_name(const int64_t device);
	real_t get_joy_vibration_duration(const int64_t device);
	Vector2 get_joy_vibration_strength(const int64_t device);
	Vector2 get_last_mouse_speed() const;
	Vector3 get_magnetometer() const;
	int64_t get_mouse_button_mask() const;
	Input::MouseMode get_mouse_mode() const;
	bool is_action_just_pressed(const String action) const;
	bool is_action_just_released(const String action) const;
	bool is_action_pressed(const String action) const;
	bool is_joy_button_pressed(const int64_t device, const int64_t button) const;
	bool is_joy_known(const int64_t device);
	bool is_key_pressed(const int64_t scancode) const;
	bool is_mouse_button_pressed(const int64_t button) const;
	void joy_connection_changed(const int64_t device, const bool connected, const String name, const String guid);
	void parse_input_event(const Ref<InputEvent> event);
	void remove_joy_mapping(const String guid);
	void set_custom_mouse_cursor(const Ref<Resource> image, const int64_t shape = 0, const Vector2 hotspot = Vector2(0, 0));
	void set_default_cursor_shape(const int64_t shape = 0);
	void set_mouse_mode(const int64_t mode);
	void set_use_accumulated_input(const bool enable);
	void start_joy_vibration(const int64_t device, const real_t weak_magnitude, const real_t strong_magnitude, const real_t duration = 0);
	void stop_joy_vibration(const int64_t device);
	void vibrate_handheld(const int64_t duration_ms = 500);
	void warp_mouse_position(const Vector2 to);

};

}

#endif