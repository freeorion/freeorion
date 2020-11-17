#include "Input.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "InputEvent.hpp"
#include "Resource.hpp"


namespace godot {


Input *Input::_singleton = NULL;


Input::Input() {
	_owner = godot::api->godot_global_get_singleton((char *) "Input");
}


Input::___method_bindings Input::___mb = {};

void Input::___init_method_bindings() {
	___mb.mb_action_press = godot::api->godot_method_bind_get_method("Input", "action_press");
	___mb.mb_action_release = godot::api->godot_method_bind_get_method("Input", "action_release");
	___mb.mb_add_joy_mapping = godot::api->godot_method_bind_get_method("Input", "add_joy_mapping");
	___mb.mb_get_accelerometer = godot::api->godot_method_bind_get_method("Input", "get_accelerometer");
	___mb.mb_get_action_strength = godot::api->godot_method_bind_get_method("Input", "get_action_strength");
	___mb.mb_get_connected_joypads = godot::api->godot_method_bind_get_method("Input", "get_connected_joypads");
	___mb.mb_get_current_cursor_shape = godot::api->godot_method_bind_get_method("Input", "get_current_cursor_shape");
	___mb.mb_get_gravity = godot::api->godot_method_bind_get_method("Input", "get_gravity");
	___mb.mb_get_gyroscope = godot::api->godot_method_bind_get_method("Input", "get_gyroscope");
	___mb.mb_get_joy_axis = godot::api->godot_method_bind_get_method("Input", "get_joy_axis");
	___mb.mb_get_joy_axis_index_from_string = godot::api->godot_method_bind_get_method("Input", "get_joy_axis_index_from_string");
	___mb.mb_get_joy_axis_string = godot::api->godot_method_bind_get_method("Input", "get_joy_axis_string");
	___mb.mb_get_joy_button_index_from_string = godot::api->godot_method_bind_get_method("Input", "get_joy_button_index_from_string");
	___mb.mb_get_joy_button_string = godot::api->godot_method_bind_get_method("Input", "get_joy_button_string");
	___mb.mb_get_joy_guid = godot::api->godot_method_bind_get_method("Input", "get_joy_guid");
	___mb.mb_get_joy_name = godot::api->godot_method_bind_get_method("Input", "get_joy_name");
	___mb.mb_get_joy_vibration_duration = godot::api->godot_method_bind_get_method("Input", "get_joy_vibration_duration");
	___mb.mb_get_joy_vibration_strength = godot::api->godot_method_bind_get_method("Input", "get_joy_vibration_strength");
	___mb.mb_get_last_mouse_speed = godot::api->godot_method_bind_get_method("Input", "get_last_mouse_speed");
	___mb.mb_get_magnetometer = godot::api->godot_method_bind_get_method("Input", "get_magnetometer");
	___mb.mb_get_mouse_button_mask = godot::api->godot_method_bind_get_method("Input", "get_mouse_button_mask");
	___mb.mb_get_mouse_mode = godot::api->godot_method_bind_get_method("Input", "get_mouse_mode");
	___mb.mb_is_action_just_pressed = godot::api->godot_method_bind_get_method("Input", "is_action_just_pressed");
	___mb.mb_is_action_just_released = godot::api->godot_method_bind_get_method("Input", "is_action_just_released");
	___mb.mb_is_action_pressed = godot::api->godot_method_bind_get_method("Input", "is_action_pressed");
	___mb.mb_is_joy_button_pressed = godot::api->godot_method_bind_get_method("Input", "is_joy_button_pressed");
	___mb.mb_is_joy_known = godot::api->godot_method_bind_get_method("Input", "is_joy_known");
	___mb.mb_is_key_pressed = godot::api->godot_method_bind_get_method("Input", "is_key_pressed");
	___mb.mb_is_mouse_button_pressed = godot::api->godot_method_bind_get_method("Input", "is_mouse_button_pressed");
	___mb.mb_joy_connection_changed = godot::api->godot_method_bind_get_method("Input", "joy_connection_changed");
	___mb.mb_parse_input_event = godot::api->godot_method_bind_get_method("Input", "parse_input_event");
	___mb.mb_remove_joy_mapping = godot::api->godot_method_bind_get_method("Input", "remove_joy_mapping");
	___mb.mb_set_custom_mouse_cursor = godot::api->godot_method_bind_get_method("Input", "set_custom_mouse_cursor");
	___mb.mb_set_default_cursor_shape = godot::api->godot_method_bind_get_method("Input", "set_default_cursor_shape");
	___mb.mb_set_mouse_mode = godot::api->godot_method_bind_get_method("Input", "set_mouse_mode");
	___mb.mb_set_use_accumulated_input = godot::api->godot_method_bind_get_method("Input", "set_use_accumulated_input");
	___mb.mb_start_joy_vibration = godot::api->godot_method_bind_get_method("Input", "start_joy_vibration");
	___mb.mb_stop_joy_vibration = godot::api->godot_method_bind_get_method("Input", "stop_joy_vibration");
	___mb.mb_vibrate_handheld = godot::api->godot_method_bind_get_method("Input", "vibrate_handheld");
	___mb.mb_warp_mouse_position = godot::api->godot_method_bind_get_method("Input", "warp_mouse_position");
}

void Input::action_press(const String action, const real_t strength) {
	___godot_icall_void_String_float(___mb.mb_action_press, (const Object *) this, action, strength);
}

void Input::action_release(const String action) {
	___godot_icall_void_String(___mb.mb_action_release, (const Object *) this, action);
}

void Input::add_joy_mapping(const String mapping, const bool update_existing) {
	___godot_icall_void_String_bool(___mb.mb_add_joy_mapping, (const Object *) this, mapping, update_existing);
}

Vector3 Input::get_accelerometer() const {
	return ___godot_icall_Vector3(___mb.mb_get_accelerometer, (const Object *) this);
}

real_t Input::get_action_strength(const String action) const {
	return ___godot_icall_float_String(___mb.mb_get_action_strength, (const Object *) this, action);
}

Array Input::get_connected_joypads() {
	return ___godot_icall_Array(___mb.mb_get_connected_joypads, (const Object *) this);
}

Input::CursorShape Input::get_current_cursor_shape() const {
	return (Input::CursorShape) ___godot_icall_int(___mb.mb_get_current_cursor_shape, (const Object *) this);
}

Vector3 Input::get_gravity() const {
	return ___godot_icall_Vector3(___mb.mb_get_gravity, (const Object *) this);
}

Vector3 Input::get_gyroscope() const {
	return ___godot_icall_Vector3(___mb.mb_get_gyroscope, (const Object *) this);
}

real_t Input::get_joy_axis(const int64_t device, const int64_t axis) const {
	return ___godot_icall_float_int_int(___mb.mb_get_joy_axis, (const Object *) this, device, axis);
}

int64_t Input::get_joy_axis_index_from_string(const String axis) {
	return ___godot_icall_int_String(___mb.mb_get_joy_axis_index_from_string, (const Object *) this, axis);
}

String Input::get_joy_axis_string(const int64_t axis_index) {
	return ___godot_icall_String_int(___mb.mb_get_joy_axis_string, (const Object *) this, axis_index);
}

int64_t Input::get_joy_button_index_from_string(const String button) {
	return ___godot_icall_int_String(___mb.mb_get_joy_button_index_from_string, (const Object *) this, button);
}

String Input::get_joy_button_string(const int64_t button_index) {
	return ___godot_icall_String_int(___mb.mb_get_joy_button_string, (const Object *) this, button_index);
}

String Input::get_joy_guid(const int64_t device) const {
	return ___godot_icall_String_int(___mb.mb_get_joy_guid, (const Object *) this, device);
}

String Input::get_joy_name(const int64_t device) {
	return ___godot_icall_String_int(___mb.mb_get_joy_name, (const Object *) this, device);
}

real_t Input::get_joy_vibration_duration(const int64_t device) {
	return ___godot_icall_float_int(___mb.mb_get_joy_vibration_duration, (const Object *) this, device);
}

Vector2 Input::get_joy_vibration_strength(const int64_t device) {
	return ___godot_icall_Vector2_int(___mb.mb_get_joy_vibration_strength, (const Object *) this, device);
}

Vector2 Input::get_last_mouse_speed() const {
	return ___godot_icall_Vector2(___mb.mb_get_last_mouse_speed, (const Object *) this);
}

Vector3 Input::get_magnetometer() const {
	return ___godot_icall_Vector3(___mb.mb_get_magnetometer, (const Object *) this);
}

int64_t Input::get_mouse_button_mask() const {
	return ___godot_icall_int(___mb.mb_get_mouse_button_mask, (const Object *) this);
}

Input::MouseMode Input::get_mouse_mode() const {
	return (Input::MouseMode) ___godot_icall_int(___mb.mb_get_mouse_mode, (const Object *) this);
}

bool Input::is_action_just_pressed(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_is_action_just_pressed, (const Object *) this, action);
}

bool Input::is_action_just_released(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_is_action_just_released, (const Object *) this, action);
}

bool Input::is_action_pressed(const String action) const {
	return ___godot_icall_bool_String(___mb.mb_is_action_pressed, (const Object *) this, action);
}

bool Input::is_joy_button_pressed(const int64_t device, const int64_t button) const {
	return ___godot_icall_bool_int_int(___mb.mb_is_joy_button_pressed, (const Object *) this, device, button);
}

bool Input::is_joy_known(const int64_t device) {
	return ___godot_icall_bool_int(___mb.mb_is_joy_known, (const Object *) this, device);
}

bool Input::is_key_pressed(const int64_t scancode) const {
	return ___godot_icall_bool_int(___mb.mb_is_key_pressed, (const Object *) this, scancode);
}

bool Input::is_mouse_button_pressed(const int64_t button) const {
	return ___godot_icall_bool_int(___mb.mb_is_mouse_button_pressed, (const Object *) this, button);
}

void Input::joy_connection_changed(const int64_t device, const bool connected, const String name, const String guid) {
	___godot_icall_void_int_bool_String_String(___mb.mb_joy_connection_changed, (const Object *) this, device, connected, name, guid);
}

void Input::parse_input_event(const Ref<InputEvent> event) {
	___godot_icall_void_Object(___mb.mb_parse_input_event, (const Object *) this, event.ptr());
}

void Input::remove_joy_mapping(const String guid) {
	___godot_icall_void_String(___mb.mb_remove_joy_mapping, (const Object *) this, guid);
}

void Input::set_custom_mouse_cursor(const Ref<Resource> image, const int64_t shape, const Vector2 hotspot) {
	___godot_icall_void_Object_int_Vector2(___mb.mb_set_custom_mouse_cursor, (const Object *) this, image.ptr(), shape, hotspot);
}

void Input::set_default_cursor_shape(const int64_t shape) {
	___godot_icall_void_int(___mb.mb_set_default_cursor_shape, (const Object *) this, shape);
}

void Input::set_mouse_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mouse_mode, (const Object *) this, mode);
}

void Input::set_use_accumulated_input(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_accumulated_input, (const Object *) this, enable);
}

void Input::start_joy_vibration(const int64_t device, const real_t weak_magnitude, const real_t strong_magnitude, const real_t duration) {
	___godot_icall_void_int_float_float_float(___mb.mb_start_joy_vibration, (const Object *) this, device, weak_magnitude, strong_magnitude, duration);
}

void Input::stop_joy_vibration(const int64_t device) {
	___godot_icall_void_int(___mb.mb_stop_joy_vibration, (const Object *) this, device);
}

void Input::vibrate_handheld(const int64_t duration_ms) {
	___godot_icall_void_int(___mb.mb_vibrate_handheld, (const Object *) this, duration_ms);
}

void Input::warp_mouse_position(const Vector2 to) {
	___godot_icall_void_Vector2(___mb.mb_warp_mouse_position, (const Object *) this, to);
}

}