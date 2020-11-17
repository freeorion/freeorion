#include "AStar.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AStar::___method_bindings AStar::___mb = {};

void AStar::___init_method_bindings() {
	___mb.mb__compute_cost = godot::api->godot_method_bind_get_method("AStar", "_compute_cost");
	___mb.mb__estimate_cost = godot::api->godot_method_bind_get_method("AStar", "_estimate_cost");
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("AStar", "add_point");
	___mb.mb_are_points_connected = godot::api->godot_method_bind_get_method("AStar", "are_points_connected");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("AStar", "clear");
	___mb.mb_connect_points = godot::api->godot_method_bind_get_method("AStar", "connect_points");
	___mb.mb_disconnect_points = godot::api->godot_method_bind_get_method("AStar", "disconnect_points");
	___mb.mb_get_available_point_id = godot::api->godot_method_bind_get_method("AStar", "get_available_point_id");
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("AStar", "get_closest_point");
	___mb.mb_get_closest_position_in_segment = godot::api->godot_method_bind_get_method("AStar", "get_closest_position_in_segment");
	___mb.mb_get_id_path = godot::api->godot_method_bind_get_method("AStar", "get_id_path");
	___mb.mb_get_point_capacity = godot::api->godot_method_bind_get_method("AStar", "get_point_capacity");
	___mb.mb_get_point_connections = godot::api->godot_method_bind_get_method("AStar", "get_point_connections");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("AStar", "get_point_count");
	___mb.mb_get_point_path = godot::api->godot_method_bind_get_method("AStar", "get_point_path");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("AStar", "get_point_position");
	___mb.mb_get_point_weight_scale = godot::api->godot_method_bind_get_method("AStar", "get_point_weight_scale");
	___mb.mb_get_points = godot::api->godot_method_bind_get_method("AStar", "get_points");
	___mb.mb_has_point = godot::api->godot_method_bind_get_method("AStar", "has_point");
	___mb.mb_is_point_disabled = godot::api->godot_method_bind_get_method("AStar", "is_point_disabled");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("AStar", "remove_point");
	___mb.mb_reserve_space = godot::api->godot_method_bind_get_method("AStar", "reserve_space");
	___mb.mb_set_point_disabled = godot::api->godot_method_bind_get_method("AStar", "set_point_disabled");
	___mb.mb_set_point_position = godot::api->godot_method_bind_get_method("AStar", "set_point_position");
	___mb.mb_set_point_weight_scale = godot::api->godot_method_bind_get_method("AStar", "set_point_weight_scale");
}

AStar *AStar::_new()
{
	return (AStar *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AStar")());
}
real_t AStar::_compute_cost(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_float_int_int(___mb.mb__compute_cost, (const Object *) this, from_id, to_id);
}

real_t AStar::_estimate_cost(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_float_int_int(___mb.mb__estimate_cost, (const Object *) this, from_id, to_id);
}

void AStar::add_point(const int64_t id, const Vector3 position, const real_t weight_scale) {
	___godot_icall_void_int_Vector3_float(___mb.mb_add_point, (const Object *) this, id, position, weight_scale);
}

bool AStar::are_points_connected(const int64_t id, const int64_t to_id, const bool bidirectional) const {
	return ___godot_icall_bool_int_int_bool(___mb.mb_are_points_connected, (const Object *) this, id, to_id, bidirectional);
}

void AStar::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void AStar::connect_points(const int64_t id, const int64_t to_id, const bool bidirectional) {
	___godot_icall_void_int_int_bool(___mb.mb_connect_points, (const Object *) this, id, to_id, bidirectional);
}

void AStar::disconnect_points(const int64_t id, const int64_t to_id, const bool bidirectional) {
	___godot_icall_void_int_int_bool(___mb.mb_disconnect_points, (const Object *) this, id, to_id, bidirectional);
}

int64_t AStar::get_available_point_id() const {
	return ___godot_icall_int(___mb.mb_get_available_point_id, (const Object *) this);
}

int64_t AStar::get_closest_point(const Vector3 to_position, const bool include_disabled) const {
	return ___godot_icall_int_Vector3_bool(___mb.mb_get_closest_point, (const Object *) this, to_position, include_disabled);
}

Vector3 AStar::get_closest_position_in_segment(const Vector3 to_position) const {
	return ___godot_icall_Vector3_Vector3(___mb.mb_get_closest_position_in_segment, (const Object *) this, to_position);
}

PoolIntArray AStar::get_id_path(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_PoolIntArray_int_int(___mb.mb_get_id_path, (const Object *) this, from_id, to_id);
}

int64_t AStar::get_point_capacity() const {
	return ___godot_icall_int(___mb.mb_get_point_capacity, (const Object *) this);
}

PoolIntArray AStar::get_point_connections(const int64_t id) {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_point_connections, (const Object *) this, id);
}

int64_t AStar::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

PoolVector3Array AStar::get_point_path(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_PoolVector3Array_int_int(___mb.mb_get_point_path, (const Object *) this, from_id, to_id);
}

Vector3 AStar::get_point_position(const int64_t id) const {
	return ___godot_icall_Vector3_int(___mb.mb_get_point_position, (const Object *) this, id);
}

real_t AStar::get_point_weight_scale(const int64_t id) const {
	return ___godot_icall_float_int(___mb.mb_get_point_weight_scale, (const Object *) this, id);
}

Array AStar::get_points() {
	return ___godot_icall_Array(___mb.mb_get_points, (const Object *) this);
}

bool AStar::has_point(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_has_point, (const Object *) this, id);
}

bool AStar::is_point_disabled(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_is_point_disabled, (const Object *) this, id);
}

void AStar::remove_point(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, id);
}

void AStar::reserve_space(const int64_t num_nodes) {
	___godot_icall_void_int(___mb.mb_reserve_space, (const Object *) this, num_nodes);
}

void AStar::set_point_disabled(const int64_t id, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_point_disabled, (const Object *) this, id, disabled);
}

void AStar::set_point_position(const int64_t id, const Vector3 position) {
	___godot_icall_void_int_Vector3(___mb.mb_set_point_position, (const Object *) this, id, position);
}

void AStar::set_point_weight_scale(const int64_t id, const real_t weight_scale) {
	___godot_icall_void_int_float(___mb.mb_set_point_weight_scale, (const Object *) this, id, weight_scale);
}

}