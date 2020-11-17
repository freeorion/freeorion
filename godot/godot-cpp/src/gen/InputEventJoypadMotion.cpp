#include "InputEventJoypadMotion.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


InputEventJoypadMotion::___method_bindings InputEventJoypadMotion::___mb = {};

void InputEventJoypadMotion::___init_method_bindings() {
	___mb.mb_get_axis = godot::api->godot_method_bind_get_method("InputEventJoypadMotion", "get_axis");
	___mb.mb_get_axis_value = godot::api->godot_method_bind_get_method("InputEventJoypadMotion", "get_axis_value");
	___mb.mb_set_axis = godot::api->godot_method_bind_get_method("InputEventJoypadMotion", "set_axis");
	___mb.mb_set_axis_value = godot::api->godot_method_bind_get_method("InputEventJoypadMotion", "set_axis_value");
}

InputEventJoypadMotion *InputEventJoypadMotion::_new()
{
	return (InputEventJoypadMotion *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InputEventJoypadMotion")());
}
int64_t InputEventJoypadMotion::get_axis() const {
	return ___godot_icall_int(___mb.mb_get_axis, (const Object *) this);
}

real_t InputEventJoypadMotion::get_axis_value() const {
	return ___godot_icall_float(___mb.mb_get_axis_value, (const Object *) this);
}

void InputEventJoypadMotion::set_axis(const int64_t axis) {
	___godot_icall_void_int(___mb.mb_set_axis, (const Object *) this, axis);
}

void InputEventJoypadMotion::set_axis_value(const real_t axis_value) {
	___godot_icall_void_float(___mb.mb_set_axis_value, (const Object *) this, axis_value);
}

}