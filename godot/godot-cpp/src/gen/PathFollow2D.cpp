#include "PathFollow2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PathFollow2D::___method_bindings PathFollow2D::___mb = {};

void PathFollow2D::___init_method_bindings() {
	___mb.mb_get_cubic_interpolation = godot::api->godot_method_bind_get_method("PathFollow2D", "get_cubic_interpolation");
	___mb.mb_get_h_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "get_h_offset");
	___mb.mb_get_lookahead = godot::api->godot_method_bind_get_method("PathFollow2D", "get_lookahead");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "get_offset");
	___mb.mb_get_unit_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "get_unit_offset");
	___mb.mb_get_v_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "get_v_offset");
	___mb.mb_has_loop = godot::api->godot_method_bind_get_method("PathFollow2D", "has_loop");
	___mb.mb_is_rotating = godot::api->godot_method_bind_get_method("PathFollow2D", "is_rotating");
	___mb.mb_set_cubic_interpolation = godot::api->godot_method_bind_get_method("PathFollow2D", "set_cubic_interpolation");
	___mb.mb_set_h_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "set_h_offset");
	___mb.mb_set_lookahead = godot::api->godot_method_bind_get_method("PathFollow2D", "set_lookahead");
	___mb.mb_set_loop = godot::api->godot_method_bind_get_method("PathFollow2D", "set_loop");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "set_offset");
	___mb.mb_set_rotate = godot::api->godot_method_bind_get_method("PathFollow2D", "set_rotate");
	___mb.mb_set_unit_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "set_unit_offset");
	___mb.mb_set_v_offset = godot::api->godot_method_bind_get_method("PathFollow2D", "set_v_offset");
}

PathFollow2D *PathFollow2D::_new()
{
	return (PathFollow2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PathFollow2D")());
}
bool PathFollow2D::get_cubic_interpolation() const {
	return ___godot_icall_bool(___mb.mb_get_cubic_interpolation, (const Object *) this);
}

real_t PathFollow2D::get_h_offset() const {
	return ___godot_icall_float(___mb.mb_get_h_offset, (const Object *) this);
}

real_t PathFollow2D::get_lookahead() const {
	return ___godot_icall_float(___mb.mb_get_lookahead, (const Object *) this);
}

real_t PathFollow2D::get_offset() const {
	return ___godot_icall_float(___mb.mb_get_offset, (const Object *) this);
}

real_t PathFollow2D::get_unit_offset() const {
	return ___godot_icall_float(___mb.mb_get_unit_offset, (const Object *) this);
}

real_t PathFollow2D::get_v_offset() const {
	return ___godot_icall_float(___mb.mb_get_v_offset, (const Object *) this);
}

bool PathFollow2D::has_loop() const {
	return ___godot_icall_bool(___mb.mb_has_loop, (const Object *) this);
}

bool PathFollow2D::is_rotating() const {
	return ___godot_icall_bool(___mb.mb_is_rotating, (const Object *) this);
}

void PathFollow2D::set_cubic_interpolation(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_cubic_interpolation, (const Object *) this, enable);
}

void PathFollow2D::set_h_offset(const real_t h_offset) {
	___godot_icall_void_float(___mb.mb_set_h_offset, (const Object *) this, h_offset);
}

void PathFollow2D::set_lookahead(const real_t lookahead) {
	___godot_icall_void_float(___mb.mb_set_lookahead, (const Object *) this, lookahead);
}

void PathFollow2D::set_loop(const bool loop) {
	___godot_icall_void_bool(___mb.mb_set_loop, (const Object *) this, loop);
}

void PathFollow2D::set_offset(const real_t offset) {
	___godot_icall_void_float(___mb.mb_set_offset, (const Object *) this, offset);
}

void PathFollow2D::set_rotate(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_rotate, (const Object *) this, enable);
}

void PathFollow2D::set_unit_offset(const real_t unit_offset) {
	___godot_icall_void_float(___mb.mb_set_unit_offset, (const Object *) this, unit_offset);
}

void PathFollow2D::set_v_offset(const real_t v_offset) {
	___godot_icall_void_float(___mb.mb_set_v_offset, (const Object *) this, v_offset);
}

}