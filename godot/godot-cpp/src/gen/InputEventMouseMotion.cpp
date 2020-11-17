#include "InputEventMouseMotion.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventMouseMotion::___method_bindings InputEventMouseMotion::___mb = {};

void InputEventMouseMotion::___init_method_bindings() {
	___mb.mb_get_pressure = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "get_pressure");
	___mb.mb_get_relative = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "get_relative");
	___mb.mb_get_speed = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "get_speed");
	___mb.mb_get_tilt = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "get_tilt");
	___mb.mb_set_pressure = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "set_pressure");
	___mb.mb_set_relative = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "set_relative");
	___mb.mb_set_speed = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "set_speed");
	___mb.mb_set_tilt = godot::api->godot_method_bind_get_method("InputEventMouseMotion", "set_tilt");
}

InputEventMouseMotion *InputEventMouseMotion::_new()
{
	return (InputEventMouseMotion *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventMouseMotion")());
}
real_t InputEventMouseMotion::get_pressure() const {
	return ___godot_icall_float(___mb.mb_get_pressure, (const Object *) this);
}

Vector2 InputEventMouseMotion::get_relative() const {
	return ___godot_icall_Vector2(___mb.mb_get_relative, (const Object *) this);
}

Vector2 InputEventMouseMotion::get_speed() const {
	return ___godot_icall_Vector2(___mb.mb_get_speed, (const Object *) this);
}

Vector2 InputEventMouseMotion::get_tilt() const {
	return ___godot_icall_Vector2(___mb.mb_get_tilt, (const Object *) this);
}

void InputEventMouseMotion::set_pressure(const real_t pressure) {
	___godot_icall_void_float(___mb.mb_set_pressure, (const Object *) this, pressure);
}

void InputEventMouseMotion::set_relative(const Vector2 relative) {
	___godot_icall_void_Vector2(___mb.mb_set_relative, (const Object *) this, relative);
}

void InputEventMouseMotion::set_speed(const Vector2 speed) {
	___godot_icall_void_Vector2(___mb.mb_set_speed, (const Object *) this, speed);
}

void InputEventMouseMotion::set_tilt(const Vector2 tilt) {
	___godot_icall_void_Vector2(___mb.mb_set_tilt, (const Object *) this, tilt);
}

}