#include "InputEventScreenDrag.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventScreenDrag::___method_bindings InputEventScreenDrag::___mb = {};

void InputEventScreenDrag::___init_method_bindings() {
	___mb.mb_get_index = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "get_index");
	___mb.mb_get_position = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "get_position");
	___mb.mb_get_relative = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "get_relative");
	___mb.mb_get_speed = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "get_speed");
	___mb.mb_set_index = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "set_index");
	___mb.mb_set_position = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "set_position");
	___mb.mb_set_relative = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "set_relative");
	___mb.mb_set_speed = godot::api->godot_method_bind_get_method("InputEventScreenDrag", "set_speed");
}

InputEventScreenDrag *InputEventScreenDrag::_new()
{
	return (InputEventScreenDrag *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventScreenDrag")());
}
int64_t InputEventScreenDrag::get_index() const {
	return ___godot_icall_int(___mb.mb_get_index, (const Object *) this);
}

Vector2 InputEventScreenDrag::get_position() const {
	return ___godot_icall_Vector2(___mb.mb_get_position, (const Object *) this);
}

Vector2 InputEventScreenDrag::get_relative() const {
	return ___godot_icall_Vector2(___mb.mb_get_relative, (const Object *) this);
}

Vector2 InputEventScreenDrag::get_speed() const {
	return ___godot_icall_Vector2(___mb.mb_get_speed, (const Object *) this);
}

void InputEventScreenDrag::set_index(const int64_t index) {
	___godot_icall_void_int(___mb.mb_set_index, (const Object *) this, index);
}

void InputEventScreenDrag::set_position(const Vector2 position) {
	___godot_icall_void_Vector2(___mb.mb_set_position, (const Object *) this, position);
}

void InputEventScreenDrag::set_relative(const Vector2 relative) {
	___godot_icall_void_Vector2(___mb.mb_set_relative, (const Object *) this, relative);
}

void InputEventScreenDrag::set_speed(const Vector2 speed) {
	___godot_icall_void_Vector2(___mb.mb_set_speed, (const Object *) this, speed);
}

}