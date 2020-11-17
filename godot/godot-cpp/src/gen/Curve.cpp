#include "Curve.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Curve::___method_bindings Curve::___mb = {};

void Curve::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("Curve", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("Curve", "_set_data");
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("Curve", "add_point");
	___mb.mb_bake = godot::api->godot_method_bind_get_method("Curve", "bake");
	___mb.mb_clean_dupes = godot::api->godot_method_bind_get_method("Curve", "clean_dupes");
	___mb.mb_clear_points = godot::api->godot_method_bind_get_method("Curve", "clear_points");
	___mb.mb_get_bake_resolution = godot::api->godot_method_bind_get_method("Curve", "get_bake_resolution");
	___mb.mb_get_max_value = godot::api->godot_method_bind_get_method("Curve", "get_max_value");
	___mb.mb_get_min_value = godot::api->godot_method_bind_get_method("Curve", "get_min_value");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("Curve", "get_point_count");
	___mb.mb_get_point_left_mode = godot::api->godot_method_bind_get_method("Curve", "get_point_left_mode");
	___mb.mb_get_point_left_tangent = godot::api->godot_method_bind_get_method("Curve", "get_point_left_tangent");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("Curve", "get_point_position");
	___mb.mb_get_point_right_mode = godot::api->godot_method_bind_get_method("Curve", "get_point_right_mode");
	___mb.mb_get_point_right_tangent = godot::api->godot_method_bind_get_method("Curve", "get_point_right_tangent");
	___mb.mb_interpolate = godot::api->godot_method_bind_get_method("Curve", "interpolate");
	___mb.mb_interpolate_baked = godot::api->godot_method_bind_get_method("Curve", "interpolate_baked");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("Curve", "remove_point");
	___mb.mb_set_bake_resolution = godot::api->godot_method_bind_get_method("Curve", "set_bake_resolution");
	___mb.mb_set_max_value = godot::api->godot_method_bind_get_method("Curve", "set_max_value");
	___mb.mb_set_min_value = godot::api->godot_method_bind_get_method("Curve", "set_min_value");
	___mb.mb_set_point_left_mode = godot::api->godot_method_bind_get_method("Curve", "set_point_left_mode");
	___mb.mb_set_point_left_tangent = godot::api->godot_method_bind_get_method("Curve", "set_point_left_tangent");
	___mb.mb_set_point_offset = godot::api->godot_method_bind_get_method("Curve", "set_point_offset");
	___mb.mb_set_point_right_mode = godot::api->godot_method_bind_get_method("Curve", "set_point_right_mode");
	___mb.mb_set_point_right_tangent = godot::api->godot_method_bind_get_method("Curve", "set_point_right_tangent");
	___mb.mb_set_point_value = godot::api->godot_method_bind_get_method("Curve", "set_point_value");
}

Curve *Curve::_new()
{
	return (Curve *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Curve")());
}
Array Curve::_get_data() const {
	return ___godot_icall_Array(___mb.mb__get_data, (const Object *) this);
}

void Curve::_set_data(const Array data) {
	___godot_icall_void_Array(___mb.mb__set_data, (const Object *) this, data);
}

int64_t Curve::add_point(const Vector2 position, const real_t left_tangent, const real_t right_tangent, const int64_t left_mode, const int64_t right_mode) {
	return ___godot_icall_int_Vector2_float_float_int_int(___mb.mb_add_point, (const Object *) this, position, left_tangent, right_tangent, left_mode, right_mode);
}

void Curve::bake() {
	___godot_icall_void(___mb.mb_bake, (const Object *) this);
}

void Curve::clean_dupes() {
	___godot_icall_void(___mb.mb_clean_dupes, (const Object *) this);
}

void Curve::clear_points() {
	___godot_icall_void(___mb.mb_clear_points, (const Object *) this);
}

int64_t Curve::get_bake_resolution() const {
	return ___godot_icall_int(___mb.mb_get_bake_resolution, (const Object *) this);
}

real_t Curve::get_max_value() const {
	return ___godot_icall_float(___mb.mb_get_max_value, (const Object *) this);
}

real_t Curve::get_min_value() const {
	return ___godot_icall_float(___mb.mb_get_min_value, (const Object *) this);
}

int64_t Curve::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

Curve::TangentMode Curve::get_point_left_mode(const int64_t index) const {
	return (Curve::TangentMode) ___godot_icall_int_int(___mb.mb_get_point_left_mode, (const Object *) this, index);
}

real_t Curve::get_point_left_tangent(const int64_t index) const {
	return ___godot_icall_float_int(___mb.mb_get_point_left_tangent, (const Object *) this, index);
}

Vector2 Curve::get_point_position(const int64_t index) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_position, (const Object *) this, index);
}

Curve::TangentMode Curve::get_point_right_mode(const int64_t index) const {
	return (Curve::TangentMode) ___godot_icall_int_int(___mb.mb_get_point_right_mode, (const Object *) this, index);
}

real_t Curve::get_point_right_tangent(const int64_t index) const {
	return ___godot_icall_float_int(___mb.mb_get_point_right_tangent, (const Object *) this, index);
}

real_t Curve::interpolate(const real_t offset) const {
	return ___godot_icall_float_float(___mb.mb_interpolate, (const Object *) this, offset);
}

real_t Curve::interpolate_baked(const real_t offset) {
	return ___godot_icall_float_float(___mb.mb_interpolate_baked, (const Object *) this, offset);
}

void Curve::remove_point(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, index);
}

void Curve::set_bake_resolution(const int64_t resolution) {
	___godot_icall_void_int(___mb.mb_set_bake_resolution, (const Object *) this, resolution);
}

void Curve::set_max_value(const real_t max) {
	___godot_icall_void_float(___mb.mb_set_max_value, (const Object *) this, max);
}

void Curve::set_min_value(const real_t min) {
	___godot_icall_void_float(___mb.mb_set_min_value, (const Object *) this, min);
}

void Curve::set_point_left_mode(const int64_t index, const int64_t mode) {
	___godot_icall_void_int_int(___mb.mb_set_point_left_mode, (const Object *) this, index, mode);
}

void Curve::set_point_left_tangent(const int64_t index, const real_t tangent) {
	___godot_icall_void_int_float(___mb.mb_set_point_left_tangent, (const Object *) this, index, tangent);
}

int64_t Curve::set_point_offset(const int64_t index, const real_t offset) {
	return ___godot_icall_int_int_float(___mb.mb_set_point_offset, (const Object *) this, index, offset);
}

void Curve::set_point_right_mode(const int64_t index, const int64_t mode) {
	___godot_icall_void_int_int(___mb.mb_set_point_right_mode, (const Object *) this, index, mode);
}

void Curve::set_point_right_tangent(const int64_t index, const real_t tangent) {
	___godot_icall_void_int_float(___mb.mb_set_point_right_tangent, (const Object *) this, index, tangent);
}

void Curve::set_point_value(const int64_t index, const real_t y) {
	___godot_icall_void_int_float(___mb.mb_set_point_value, (const Object *) this, index, y);
}

}