#include "Curve2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Curve2D::___method_bindings Curve2D::___mb = {};

void Curve2D::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("Curve2D", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("Curve2D", "_set_data");
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("Curve2D", "add_point");
	___mb.mb_clear_points = godot::api->godot_method_bind_get_method("Curve2D", "clear_points");
	___mb.mb_get_bake_interval = godot::api->godot_method_bind_get_method("Curve2D", "get_bake_interval");
	___mb.mb_get_baked_length = godot::api->godot_method_bind_get_method("Curve2D", "get_baked_length");
	___mb.mb_get_baked_points = godot::api->godot_method_bind_get_method("Curve2D", "get_baked_points");
	___mb.mb_get_closest_offset = godot::api->godot_method_bind_get_method("Curve2D", "get_closest_offset");
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("Curve2D", "get_closest_point");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("Curve2D", "get_point_count");
	___mb.mb_get_point_in = godot::api->godot_method_bind_get_method("Curve2D", "get_point_in");
	___mb.mb_get_point_out = godot::api->godot_method_bind_get_method("Curve2D", "get_point_out");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("Curve2D", "get_point_position");
	___mb.mb_interpolate = godot::api->godot_method_bind_get_method("Curve2D", "interpolate");
	___mb.mb_interpolate_baked = godot::api->godot_method_bind_get_method("Curve2D", "interpolate_baked");
	___mb.mb_interpolatef = godot::api->godot_method_bind_get_method("Curve2D", "interpolatef");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("Curve2D", "remove_point");
	___mb.mb_set_bake_interval = godot::api->godot_method_bind_get_method("Curve2D", "set_bake_interval");
	___mb.mb_set_point_in = godot::api->godot_method_bind_get_method("Curve2D", "set_point_in");
	___mb.mb_set_point_out = godot::api->godot_method_bind_get_method("Curve2D", "set_point_out");
	___mb.mb_set_point_position = godot::api->godot_method_bind_get_method("Curve2D", "set_point_position");
	___mb.mb_tessellate = godot::api->godot_method_bind_get_method("Curve2D", "tessellate");
}

Curve2D *Curve2D::_new()
{
	return (Curve2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Curve2D")());
}
Dictionary Curve2D::_get_data() const {
	return ___godot_icall_Dictionary(___mb.mb__get_data, (const Object *) this);
}

void Curve2D::_set_data(const Dictionary arg0) {
	___godot_icall_void_Dictionary(___mb.mb__set_data, (const Object *) this, arg0);
}

void Curve2D::add_point(const Vector2 position, const Vector2 in, const Vector2 out, const int64_t at_position) {
	___godot_icall_void_Vector2_Vector2_Vector2_int(___mb.mb_add_point, (const Object *) this, position, in, out, at_position);
}

void Curve2D::clear_points() {
	___godot_icall_void(___mb.mb_clear_points, (const Object *) this);
}

real_t Curve2D::get_bake_interval() const {
	return ___godot_icall_float(___mb.mb_get_bake_interval, (const Object *) this);
}

real_t Curve2D::get_baked_length() const {
	return ___godot_icall_float(___mb.mb_get_baked_length, (const Object *) this);
}

PoolVector2Array Curve2D::get_baked_points() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_baked_points, (const Object *) this);
}

real_t Curve2D::get_closest_offset(const Vector2 to_point) const {
	return ___godot_icall_float_Vector2(___mb.mb_get_closest_offset, (const Object *) this, to_point);
}

Vector2 Curve2D::get_closest_point(const Vector2 to_point) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_get_closest_point, (const Object *) this, to_point);
}

int64_t Curve2D::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

Vector2 Curve2D::get_point_in(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_in, (const Object *) this, idx);
}

Vector2 Curve2D::get_point_out(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_out, (const Object *) this, idx);
}

Vector2 Curve2D::get_point_position(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_position, (const Object *) this, idx);
}

Vector2 Curve2D::interpolate(const int64_t idx, const real_t t) const {
	return ___godot_icall_Vector2_int_float(___mb.mb_interpolate, (const Object *) this, idx, t);
}

Vector2 Curve2D::interpolate_baked(const real_t offset, const bool cubic) const {
	return ___godot_icall_Vector2_float_bool(___mb.mb_interpolate_baked, (const Object *) this, offset, cubic);
}

Vector2 Curve2D::interpolatef(const real_t fofs) const {
	return ___godot_icall_Vector2_float(___mb.mb_interpolatef, (const Object *) this, fofs);
}

void Curve2D::remove_point(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, idx);
}

void Curve2D::set_bake_interval(const real_t distance) {
	___godot_icall_void_float(___mb.mb_set_bake_interval, (const Object *) this, distance);
}

void Curve2D::set_point_in(const int64_t idx, const Vector2 position) {
	___godot_icall_void_int_Vector2(___mb.mb_set_point_in, (const Object *) this, idx, position);
}

void Curve2D::set_point_out(const int64_t idx, const Vector2 position) {
	___godot_icall_void_int_Vector2(___mb.mb_set_point_out, (const Object *) this, idx, position);
}

void Curve2D::set_point_position(const int64_t idx, const Vector2 position) {
	___godot_icall_void_int_Vector2(___mb.mb_set_point_position, (const Object *) this, idx, position);
}

PoolVector2Array Curve2D::tessellate(const int64_t max_stages, const real_t tolerance_degrees) const {
	return ___godot_icall_PoolVector2Array_int_float(___mb.mb_tessellate, (const Object *) this, max_stages, tolerance_degrees);
}

}