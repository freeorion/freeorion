#include "PathFollow.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PathFollow::___method_bindings PathFollow::___mb = {};

void PathFollow::___init_method_bindings() {
	___mb.mb_get_cubic_interpolation = godot::api->godot_method_bind_get_method("PathFollow", "get_cubic_interpolation");
	___mb.mb_get_h_offset = godot::api->godot_method_bind_get_method("PathFollow", "get_h_offset");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("PathFollow", "get_offset");
	___mb.mb_get_rotation_mode = godot::api->godot_method_bind_get_method("PathFollow", "get_rotation_mode");
	___mb.mb_get_unit_offset = godot::api->godot_method_bind_get_method("PathFollow", "get_unit_offset");
	___mb.mb_get_v_offset = godot::api->godot_method_bind_get_method("PathFollow", "get_v_offset");
	___mb.mb_has_loop = godot::api->godot_method_bind_get_method("PathFollow", "has_loop");
	___mb.mb_set_cubic_interpolation = godot::api->godot_method_bind_get_method("PathFollow", "set_cubic_interpolation");
	___mb.mb_set_h_offset = godot::api->godot_method_bind_get_method("PathFollow", "set_h_offset");
	___mb.mb_set_loop = godot::api->godot_method_bind_get_method("PathFollow", "set_loop");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("PathFollow", "set_offset");
	___mb.mb_set_rotation_mode = godot::api->godot_method_bind_get_method("PathFollow", "set_rotation_mode");
	___mb.mb_set_unit_offset = godot::api->godot_method_bind_get_method("PathFollow", "set_unit_offset");
	___mb.mb_set_v_offset = godot::api->godot_method_bind_get_method("PathFollow", "set_v_offset");
}

PathFollow *PathFollow::_new()
{
	return (PathFollow *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PathFollow")());
}
bool PathFollow::get_cubic_interpolation() const {
	return ___godot_icall_bool(___mb.mb_get_cubic_interpolation, (const Object *) this);
}

real_t PathFollow::get_h_offset() const {
	return ___godot_icall_float(___mb.mb_get_h_offset, (const Object *) this);
}

real_t PathFollow::get_offset() const {
	return ___godot_icall_float(___mb.mb_get_offset, (const Object *) this);
}

PathFollow::RotationMode PathFollow::get_rotation_mode() const {
	return (PathFollow::RotationMode) ___godot_icall_int(___mb.mb_get_rotation_mode, (const Object *) this);
}

real_t PathFollow::get_unit_offset() const {
	return ___godot_icall_float(___mb.mb_get_unit_offset, (const Object *) this);
}

real_t PathFollow::get_v_offset() const {
	return ___godot_icall_float(___mb.mb_get_v_offset, (const Object *) this);
}

bool PathFollow::has_loop() const {
	return ___godot_icall_bool(___mb.mb_has_loop, (const Object *) this);
}

void PathFollow::set_cubic_interpolation(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_cubic_interpolation, (const Object *) this, enable);
}

void PathFollow::set_h_offset(const real_t h_offset) {
	___godot_icall_void_float(___mb.mb_set_h_offset, (const Object *) this, h_offset);
}

void PathFollow::set_loop(const bool loop) {
	___godot_icall_void_bool(___mb.mb_set_loop, (const Object *) this, loop);
}

void PathFollow::set_offset(const real_t offset) {
	___godot_icall_void_float(___mb.mb_set_offset, (const Object *) this, offset);
}

void PathFollow::set_rotation_mode(const int64_t rotation_mode) {
	___godot_icall_void_int(___mb.mb_set_rotation_mode, (const Object *) this, rotation_mode);
}

void PathFollow::set_unit_offset(const real_t unit_offset) {
	___godot_icall_void_float(___mb.mb_set_unit_offset, (const Object *) this, unit_offset);
}

void PathFollow::set_v_offset(const real_t v_offset) {
	___godot_icall_void_float(___mb.mb_set_v_offset, (const Object *) this, v_offset);
}

}