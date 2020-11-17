#include "InputEventScreenTouch.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventScreenTouch::___method_bindings InputEventScreenTouch::___mb = {};

void InputEventScreenTouch::___init_method_bindings() {
	___mb.mb_get_index = godot::api->godot_method_bind_get_method("InputEventScreenTouch", "get_index");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("InputEventScreenTouch", "get_position");
	___mb.mb_set_index = godot::api->godot_method_bind_get_method("InputEventScreenTouch", "set_index");
	___mb.mb_set_position = godot::api->godot_method_bind_get_method("InputEventScreenTouch", "set_position");
	___mb.mb_set_pressed = godot::api->godot_method_bind_get_method("InputEventScreenTouch", "set_pressed");
}

InputEventScreenTouch *InputEventScreenTouch::_new()
{
	return (InputEventScreenTouch *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventScreenTouch")());
}
int64_t InputEventScreenTouch::get_index() const {
	return ___godot_icall_int(___mb.mb_get_index, (const Object *) this);
}

Vector2 InputEventScreenTouch::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

void InputEventScreenTouch::set_index(const int64_t index) {
	___godot_icall_void_int(___mb.mb_set_index, (const Object *) this, index);
}

void InputEventScreenTouch::set_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_position, (const Object *) this, position);
}

void InputEventScreenTouch::set_pressed(const bool pressed) {
	___godot_icall_void_bool(___mb.mb_set_pressed, (const Object *) this, pressed);
}

}