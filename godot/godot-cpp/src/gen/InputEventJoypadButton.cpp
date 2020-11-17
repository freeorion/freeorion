#include "InputEventJoypadButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventJoypadButton::___method_bindings InputEventJoypadButton::___mb = {};

void InputEventJoypadButton::___init_method_bindings() {
	___mb.mb_get_button_index = godot::api->godot_method_bind_get_method("InputEventJoypadButton", "get_button_index");
	___mb.mb_get_pressure = godot::api->godot_method_bind_get_method("InputEventJoypadButton", "get_pressure");
	___mb.mb_set_button_index = godot::api->godot_method_bind_get_method("InputEventJoypadButton", "set_button_index");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("InputEventJoypadButton", "set_pressed");
	___mb.mb_set_pressure = godot::api->godot_method_bind_get_method("InputEventJoypadButton", "set_pressure");
}

InputEventJoypadButton *InputEventJoypadButton::_new()
{
	return (InputEventJoypadButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventJoypadButton")());
}
int64_t InputEventJoypadButton::get_button_index() const {
	return ___godot_icall_int(___mb.mb_get_button_index, (const Object *) this);
}

real_t InputEventJoypadButton::get_pressure() const {
	return ___godot_icall_float(___mb.mb_get_pressure, (const Object *) this);
}

void InputEventJoypadButton::set_button_index(const int64_t button_index) {
	___godot_icall_void_int(___mb.mb_set_button_index, (const Object *) this, button_index);
}

void InputEventJoypadButton::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

void InputEventJoypadButton::set_pressure(const real_t pressure) {
	___godot_icall_void_float(___mb.mb_set_pressure, (const Object *) this, pressure);
}

}