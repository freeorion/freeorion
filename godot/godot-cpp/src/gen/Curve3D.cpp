#include "Curve3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Curve3D::___method_bindings Curve3D::___mb = {};

void Curve3D::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("Curve3D", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("Curve3D", "_set_data");
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("Curve3D", "add_point");
	___mb.mb_clear_points = godot::api->godot_method_bind_get_method("Curve3D", "clear_points");
	___mb.mb_get_bake_interval = godot::api->godot_method_bind_get_method("Curve3D", "get_bake_interval");
	___mb.mb_get_baked_length = godot::api->godot_method_bind_get_method("Curve3D", "get_baked_length");
	___mb.mb_get_baked_points = godot::api->godot_method_bind_get_method("Curve3D", "get_baked_points");
	___mb.mb_get_baked_tilts = godot::api->godot_method_bind_get_method("Curve3D", "get_baked_tilts");
	___mb.mb_get_baked_up_vectors = godot::api->godot_method_bind_get_method("Curve3D", "get_baked_up_vectors");
	___mb.mb_get_closest_offset = godot::api->godot_method_bind_get_method("Curve3D", "get_closest_offset");
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("Curve3D", "get_closest_point");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("Curve3D", "get_point_count");
	___mb.mb_get_point_in = godot::api->godot_method_bind_get_method("Curve3D", "get_point_in");
	___mb.mb_get_point_out = godot::api->godot_method_bind_get_method("Curve3D", "get_point_out");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("Curve3D", "get_point_position");
	___mb.mb_get_point_tilt = godot::api->godot_method_bind_get_method("Curve3D", "get_point_tilt");
	___mb.mb_interpolate = godot::api->godot_method_bind_get_method("Curve3D", "interpolate");
	___mb.mb_interpolate_baked = godot::api->godot_method_bind_get_method("Curve3D", "interpolate_baked");
	___mb.mb_interpolate_baked_up_vector = godot::api->godot_method_bind_get_method("Curve3D", "interpolate_baked_up_vector");
	___mb.mb_interpolatef = godot::api->godot_method_bind_get_method("Curve3D", "interpolatef");
	___mb.mb_is_up_vector_enabled = godot::api->godot_method_bind_get_method("Curve3D", "is_up_vector_enabled");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("Curve3D", "remove_point");
	___mb.mb_set_bake_interval = godot::api->godot_method_bind_get_method("Curve3D", "set_bake_interval");
	___mb.mb_set_point_in = godot::api->godot_method_bind_get_method("Curve3D", "set_point_in");
	___mb.mb_set_point_out = godot::api->godot_method_bind_get_method("Curve3D", "set_point_out");
	___mb.mb_set_point_position = godot::api->godot_method_bind_get_method("Curve3D", "set_point_position");
	___mb.mb_set_point_tilt = godot::api->godot_method_bind_get_method("Curve3D", "set_point_tilt");
	___mb.mb_set_up_vector_enabled = godot::api->godot_method_bind_get_method("Curve3D", "set_up_vector_enabled");
	___mb.mb_tessellate = godot::api->godot_method_bind_get_method("Curve3D", "tessellate");
}

Curve3D *Curve3D::_new()
{
	return (Curve3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Curve3D")());
}
Dictionary Curve3D::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void Curve3D::_set_data(const Dictionary arg0) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, arg0);
}

void Curve3D::add_point(const Vector3 position, const Vector3 in, const Vector3 out, const int64_t at_position) {
	___godot_icall_void_Vector3_Vector3_Vector3_int(___mb.mb_add_point, (const Object *) this, position, in, out, at_position);
}

void Curve3D::clear_points() {
	___godot_icall_void(___mb.mb_clear_points, (const Object *) this);
}

real_t Curve3D::get_bake_interval() const {
	return ___godot_icall_float(___mb.mb_get_bake_interval, (const Object *) this);
}

real_t Curve3D::get_baked_length() const {
	return ___godot_icall_float(___mb.mb_get_baked_length, (const Object *) this);
}

PoolVector3Array Curve3D::get_baked_points() const {
	return ___godot_icall_PoolVector3Array(___mb.mb_get_baked_points, (const Object *) this);
}

PoolRealArray Curve3D::get_baked_tilts() const {
	return ___godot_icall_PoolRealArray(___mb.mb_get_baked_tilts, (const Object *) this);
}

PoolVector3Array Curve3D::get_baked_up_vectors() const {
	return ___godot_icall_PoolVector3Array(___mb.mb_get_baked_up_vectors, (const Object *) this);
}

real_t Curve3D::get_closest_offset(const Vector3 to_point) const {
	return ___godot_icall_float_Vector3(___mb.mb_get_closest_offset, (const Object *) this, to_point);
}

Vector3 Curve3D::get_closest_point(const Vector3 to_point) const {
	return ___godot_icall_Vector3_Vector3(___mb.mb_get_closest_point, (const Object *) this, to_point);
}

int64_t Curve3D::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

Vector3 Curve3D::get_point_in(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_point_in, (const Object *) this, idx);
}

Vector3 Curve3D::get_point_out(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_point_out, (const Object *) this, idx);
}

Vector3 Curve3D::get_point_position(const int64_t idx) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_point_position, (const Object *) this, idx);
}

real_t Curve3D::get_point_tilt(const int64_t idx) const {
	return ___godot_icall_float_int(___mb.mb_get_point_tilt, (const Object *) this, idx);
}

Vector3 Curve3D::interpolate(const int64_t idx, const real_t t) const {
	return ___godot_icall_Vector3_int_float(___mb.mb_interpolate, (const Object *) this, idx, t);
}

Vector3 Curve3D::interpolate_baked(const real_t offset, const bool cubic) const {
	return ___godot_icall_Vector3_float_bool(___mb.mb_interpolate_baked, (const Object *) this, offset, cubic);
}

Vector3 Curve3D::interpolate_baked_up_vector(const real_t offset, const bool apply_tilt) const {
	return ___godot_icall_Vector3_float_bool(___mb.mb_interpolate_baked_up_vector, (const Object *) this, offset, apply_tilt);
}

Vector3 Curve3D::interpolatef(const real_t fofs) const {
	return ___godot_icall_Vector3_float(___mb.mb_interpolatef, (const Object *) this, fofs);
}

bool Curve3D::is_up_vector_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_up_vector_enabled, (const Object *) this);
}

void Curve3D::remove_point(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, idx);
}

void Curve3D::set_bake_interval(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_bake_interval, (const Object *) this, distance);
}

void Curve3D::set_point_in(const int64_t idx, const Vector3 position) {
	___godot_icall_void_int_Vector3(___mb.mb_set_point_in, (const Object *) this, idx, position);
}

void Curve3D::set_point_out(const int64_t idx, const Vector3 position) {
	___godot_icall_void_int_Vector3(___mb.mb_set_point_out, (const Object *) this, idx, position);
}

void Curve3D::set_point_position(const int64_t idx, const Vector3 position) {
	___godot_icall_void_int_Vector3(___mb.mb_set_point_position, (const Object *) this, idx, position);
}

void Curve3D::set_point_tilt(const int64_t idx, const real_t tilt) {
	___godot_icall_void_int_float(___mb.mb_set_point_tilt, (const Object *) this, idx, tilt);
}

void Curve3D::set_up_vector_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_up_vector_enabled, (const Object *) this, enable);
}

PoolVector3Array Curve3D::tessellate(const int64_t max_stages, const real_t tolerance_degrees) const {
	return ___godot_icall_PoolVector3Array_int_float(___mb.mb_tessellate, (const Object *) this, max_stages, tolerance_degrees);
}

}