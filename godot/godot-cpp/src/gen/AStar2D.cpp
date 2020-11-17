#include "AStar2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AStar2D::___method_bindings AStar2D::___mb = {};

void AStar2D::___init_method_bindings() {
	___mb.mb_add_point = godot::api->godot_method_bind_get_method("AStar2D", "add_point");
	___mb.mb_are_points_connected = godot::api->godot_method_bind_get_method("AStar2D", "are_points_connected");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("AStar2D", "clear");
	___mb.mb_connect_points = godot::api->godot_method_bind_get_method("AStar2D", "connect_points");
	___mb.mb_disconnect_points = godot::api->godot_method_bind_get_method("AStar2D", "disconnect_points");
	___mb.mb_get_available_point_id = godot::api->godot_method_bind_get_method("AStar2D", "get_available_point_id");
	___mb.mb_get_closest_point = godot::api->godot_method_bind_get_method("AStar2D", "get_closest_point");
	___mb.mb_get_closest_position_in_segment = godot::api->godot_method_bind_get_method("AStar2D", "get_closest_position_in_segment");
	___mb.mb_get_id_path = godot::api->godot_method_bind_get_method("AStar2D", "get_id_path");
	___mb.mb_get_point_capacity = godot::api->godot_method_bind_get_method("AStar2D", "get_point_capacity");
	___mb.mb_get_point_connections = godot::api->godot_method_bind_get_method("AStar2D", "get_point_connections");
	___mb.mb_get_point_count = godot::api->godot_method_bind_get_method("AStar2D", "get_point_count");
	___mb.mb_get_point_path = godot::api->godot_method_bind_get_method("AStar2D", "get_point_path");
	___mb.mb_get_point_position = godot::api->godot_method_bind_get_method("AStar2D", "get_point_position");
	___mb.mb_get_point_weight_scale = godot::api->godot_method_bind_get_method("AStar2D", "get_point_weight_scale");
	___mb.mb_get_points = godot::api->godot_method_bind_get_method("AStar2D", "get_points");
	___mb.mb_has_point = godot::api->godot_method_bind_get_method("AStar2D", "has_point");
	___mb.mb_is_point_disabled = godot::api->godot_method_bind_get_method("AStar2D", "is_point_disabled");
	___mb.mb_remove_point = godot::api->godot_method_bind_get_method("AStar2D", "remove_point");
	___mb.mb_reserve_space = godot::api->godot_method_bind_get_method("AStar2D", "reserve_space");
	___mb.mb_set_point_disabled = godot::api->godot_method_bind_get_method("AStar2D", "set_point_disabled");
	___mb.mb_set_point_position = godot::api->godot_method_bind_get_method("AStar2D", "set_point_position");
	___mb.mb_set_point_weight_scale = godot::api->godot_method_bind_get_method("AStar2D", "set_point_weight_scale");
}

AStar2D *AStar2D::_new()
{
	return (AStar2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AStar2D")());
}
void AStar2D::add_point(const int64_t id, const Vector2 position, const real_t weight_scale) {
	___godot_icall_void_int_Vector2_float(___mb.mb_add_point, (const Object *) this, id, position, weight_scale);
}

bool AStar2D::are_points_connected(const int64_t id, const int64_t to_id) const {
	return ___godot_icall_bool_int_int(___mb.mb_are_points_connected, (const Object *) this, id, to_id);
}

void AStar2D::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void AStar2D::connect_points(const int64_t id, const int64_t to_id, const bool bidirectional) {
	___godot_icall_void_int_int_bool(___mb.mb_connect_points, (const Object *) this, id, to_id, bidirectional);
}

void AStar2D::disconnect_points(const int64_t id, const int64_t to_id) {
	___godot_icall_void_int_int(___mb.mb_disconnect_points, (const Object *) this, id, to_id);
}

int64_t AStar2D::get_available_point_id() const {
	return ___godot_icall_int(___mb.mb_get_available_point_id, (const Object *) this);
}

int64_t AStar2D::get_closest_point(const Vector2 to_position, const bool include_disabled) const {
	return ___godot_icall_int_Vector2_bool(___mb.mb_get_closest_point, (const Object *) this, to_position, include_disabled);
}

Vector2 AStar2D::get_closest_position_in_segment(const Vector2 to_position) const {
	return ___godot_icall_Vector2_Vector2(___mb.mb_get_closest_position_in_segment, (const Object *) this, to_position);
}

PoolIntArray AStar2D::get_id_path(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_PoolIntArray_int_int(___mb.mb_get_id_path, (const Object *) this, from_id, to_id);
}

int64_t AStar2D::get_point_capacity() const {
	return ___godot_icall_int(___mb.mb_get_point_capacity, (const Object *) this);
}

PoolIntArray AStar2D::get_point_connections(const int64_t id) {
	return ___godot_icall_PoolIntArray_int(___mb.mb_get_point_connections, (const Object *) this, id);
}

int64_t AStar2D::get_point_count() const {
	return ___godot_icall_int(___mb.mb_get_point_count, (const Object *) this);
}

PoolVector2Array AStar2D::get_point_path(const int64_t from_id, const int64_t to_id) {
	return ___godot_icall_PoolVector2Array_int_int(___mb.mb_get_point_path, (const Object *) this, from_id, to_id);
}

Vector2 AStar2D::get_point_position(const int64_t id) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_point_position, (const Object *) this, id);
}

real_t AStar2D::get_point_weight_scale(const int64_t id) const {
	return ___godot_icall_float_int(___mb.mb_get_point_weight_scale, (const Object *) this, id);
}

Array AStar2D::get_points() {
	return ___godot_icall_Array(___mb.mb_get_points, (const Object *) this);
}

bool AStar2D::has_point(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_has_point, (const Object *) this, id);
}

bool AStar2D::is_point_disabled(const int64_t id) const {
	return ___godot_icall_bool_int(___mb.mb_is_point_disabled, (const Object *) this, id);
}

void AStar2D::remove_point(const int64_t id) {
	___godot_icall_void_int(___mb.mb_remove_point, (const Object *) this, id);
}

void AStar2D::reserve_space(const int64_t num_nodes) {
	___godot_icall_void_int(___mb.mb_reserve_space, (const Object *) this, num_nodes);
}

void AStar2D::set_point_disabled(const int64_t id, const bool disabled) {
	___godot_icall_void_int_bool(___mb.mb_set_point_disabled, (const Object *) this, id, disabled);
}

void AStar2D::set_point_position(const int64_t id, const Vector2 position) {
	___godot_icall_void_int_Vector2(___mb.mb_set_point_position, (const Object *) this, id, position);
}

void AStar2D::set_point_weight_scale(const int64_t id, const real_t weight_scale) {
	___godot_icall_void_int_float(___mb.mb_set_point_weight_scale, (const Object *) this, id, weight_scale);
}

}