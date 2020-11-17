#include "InputEventMouseButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventMouseButton::___method_bindings InputEventMouseButton::___mb = {};

void InputEventMouseButton::___init_method_bindings() {
	___mb.mb_get_button_index = godot::api->godot_method_bind_get_method("InputEventMouseButton", "get_button_index");
	___mb.mb_get_factor = godot::api->godot_method_bind_get_method("InputEventMouseButton", "get_factor");
	___mb.mb_is_doubleclick = godot::api->godot_method_bind_get_method("InputEventMouseButton", "is_doubleclick");
	___mb.mb_set_button_index = godot::api->godot_method_bind_get_method("InputEventMouseButton", "set_button_index");
	___mb.mb_set_doubleclick = godot::api->godot_method_bind_get_method("InputEventMouseButton", "set_doubleclick");
	___mb.mb_set_factor = godot::api->godot_method_bind_get_method("InputEventMouseButton", "set_factor");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("InputEventMouseButton", "set_pressed");
}

InputEventMouseButton *InputEventMouseButton::_new()
{
	return (InputEventMouseButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventMouseButton")());
}
int64_t InputEventMouseButton::get_button_index() const {
	return ___godot_icall_int(___mb.mb_get_button_index, (const Object *) this);
}

real_t InputEventMouseButton::get_factor() {
	return ___godot_icall_float(___mb.mb_get_factor, (const Object *) this);
}

bool InputEventMouseButton::is_doubleclick() const {
	return ___godot_icall_bool(___mb.mb_is_doubleclick, (const Object *) this);
}

void InputEventMouseButton::set_button_index(const int64_t button_index) {
	___godot_icall_void_int(___mb.mb_set_button_index, (const Object *) this, button_index);
}

void InputEventMouseButton::set_doubleclick(const bool doubleclick) {
	___godot_icall_void_bool(___mb.mb_set_doubleclick, (const Object *) this, doubleclick);
}

void InputEventMouseButton::set_factor(const real_t factor) {
	___godot_icall_void_float(___mb.mb_set_factor, (const Object *) this, factor);
}

void InputEventMouseButton::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

}