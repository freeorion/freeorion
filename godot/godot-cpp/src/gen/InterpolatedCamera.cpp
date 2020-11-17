#include "InterpolatedCamera.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Object.hpp"


namespace godot {


InterpolatedCamera::___method_bindings InterpolatedCamera::___mb = {};

void InterpolatedCamera::___init_method_bindings() {
	___mb.mb_get_speed = godot::api->godot_method_bind_get_method("InterpolatedCamera", "get_speed");
	___mb.mb_get_target_path = godot::api->godot_method_bind_get_method("InterpolatedCamera", "get_target_path");
	___mb.mb_is_interpolation_enabled = godot::api->godot_method_bind_get_method("InterpolatedCamera", "is_interpolation_enabled");
	___mb.mb_set_interpolation_enabled = godot::api->godot_method_bind_get_method("InterpolatedCamera", "set_interpolation_enabled");
	___mb.mb_set_speed = godot::api->godot_method_bind_get_method("InterpolatedCamera", "set_speed");
	___mb.mb_set_target = godot::api->godot_method_bind_get_method("InterpolatedCamera", "set_target");
	___mb.mb_set_target_path = godot::api->godot_method_bind_get_method("InterpolatedCamera", "set_target_path");
}

InterpolatedCamera *InterpolatedCamera::_new()
{
	return (InterpolatedCamera *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"InterpolatedCamera")());
}
real_t InterpolatedCamera::get_speed() const {
	return ___godot_icall_float(___mb.mb_get_speed, (const Object *) this);
}

NodePath InterpolatedCamera::get_target_path() const {
	return ___godot_icall_NodePath(___mb.mb_get_target_path, (const Object *) this);
}

bool InterpolatedCamera::is_interpolation_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_interpolation_enabled, (const Object *) this);
}

void InterpolatedCamera::set_interpolation_enabled(const bool target_path) {
	___godot_icall_void_bool(___mb.mb_set_interpolation_enabled, (const Object *) this, target_path);
}

void InterpolatedCamera::set_speed(const real_t speed) {
	___godot_icall_void_float(___mb.mb_set_speed, (const Object *) this, speed);
}

void InterpolatedCamera::set_target(const Object *target) {
	___godot_icall_void_Object(___mb.mb_set_target, (const Object *) this, target);
}

void InterpolatedCamera::set_target_path(const NodePath target_path) {
	___godot_icall_void_NodePath(___mb.mb_set_target_path, (const Object *) this, target_path);
}

}