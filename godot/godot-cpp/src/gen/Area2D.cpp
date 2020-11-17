#include "Area2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


Area2D::___method_bindings Area2D::___mb = {};

void Area2D::___init_method_bindings() {
	___mb.mb__area_enter_tree = godot::api->godot_method_bind_get_method("Area2D", "_area_enter_tree");
	___mb.mb__area_exit_tree = godot::api->godot_method_bind_get_method("Area2D", "_area_exit_tree");
	___mb.mb__area_inout = godot::api->godot_method_bind_get_method("Area2D", "_area_inout");
	___mb.mb__body_enter_tree = godot::api->godot_method_bind_get_method("Area2D", "_body_enter_tree");
	___mb.mb__body_exit_tree = godot::api->godot_method_bind_get_method("Area2D", "_body_exit_tree");
	___mb.mb__body_inout = godot::api->godot_method_bind_get_method("Area2D", "_body_inout");
	___mb.mb_get_angular_damp = godot::api->godot_method_bind_get_method("Area2D", "get_angular_damp");
	___mb.mb_get_audio_bus_name = godot::api->godot_method_bind_get_method("Area2D", "get_audio_bus_name");
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("Area2D", "get_collision_layer");
	___mb.mb_get_collision_layer_bit = godot::api->godot_method_bind_get_method("Area2D", "get_collision_layer_bit");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("Area2D", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("Area2D", "get_collision_mask_bit");
	___mb.mb_get_gravity = godot::api->godot_method_bind_get_method("Area2D", "get_gravity");
	___mb.mb_get_gravity_distance_scale = godot::api->godot_method_bind_get_method("Area2D", "get_gravity_distance_scale");
	___mb.mb_get_gravity_vector = godot::api->godot_method_bind_get_method("Area2D", "get_gravity_vector");
	___mb.mb_get_linear_damp = godot::api->godot_method_bind_get_method("Area2D", "get_linear_damp");
	___mb.mb_get_overlapping_areas = godot::api->godot_method_bind_get_method("Area2D", "get_overlapping_areas");
	___mb.mb_get_overlapping_bodies = godot::api->godot_method_bind_get_method("Area2D", "get_overlapping_bodies");
	___mb.mb_get_priority = godot::api->godot_method_bind_get_method("Area2D", "get_priority");
	___mb.mb_get_space_override_mode = godot::api->godot_method_bind_get_method("Area2D", "get_space_override_mode");
	___mb.mb_is_gravity_a_point = godot::api->godot_method_bind_get_method("Area2D", "is_gravity_a_point");
	___mb.mb_is_monitorable = godot::api->godot_method_bind_get_method("Area2D", "is_monitorable");
	___mb.mb_is_monitoring = godot::api->godot_method_bind_get_method("Area2D", "is_monitoring");
	___mb.mb_is_overriding_audio_bus = godot::api->godot_method_bind_get_method("Area2D", "is_overriding_audio_bus");
	___mb.mb_overlaps_area = godot::api->godot_method_bind_get_method("Area2D", "overlaps_area");
	___mb.mb_overlaps_body = godot::api->godot_method_bind_get_method("Area2D", "overlaps_body");
	___mb.mb_set_angular_damp = godot::api->godot_method_bind_get_method("Area2D", "set_angular_damp");
	___mb.mb_set_audio_bus_name = godot::api->godot_method_bind_get_method("Area2D", "set_audio_bus_name");
	___mb.mb_set_audio_bus_override = godot::api->godot_method_bind_get_method("Area2D", "set_audio_bus_override");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("Area2D", "set_collision_layer");
	___mb.mb_set_collision_layer_bit = godot::api->godot_method_bind_get_method("Area2D", "set_collision_layer_bit");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("Area2D", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("Area2D", "set_collision_mask_bit");
	___mb.mb_set_gravity = godot::api->godot_method_bind_get_method("Area2D", "set_gravity");
	___mb.mb_set_gravity_distance_scale = godot::api->godot_method_bind_get_method("Area2D", "set_gravity_distance_scale");
	___mb.mb_set_gravity_is_point = godot::api->godot_method_bind_get_method("Area2D", "set_gravity_is_point");
	___mb.mb_set_gravity_vector = godot::api->godot_method_bind_get_method("Area2D", "set_gravity_vector");
	___mb.mb_set_linear_damp = godot::api->godot_method_bind_get_method("Area2D", "set_linear_damp");
	___mb.mb_set_monitorable = godot::api->godot_method_bind_get_method("Area2D", "set_monitorable");
	___mb.mb_set_monitoring = godot::api->godot_method_bind_get_method("Area2D", "set_monitoring");
	___mb.mb_set_priority = godot::api->godot_method_bind_get_method("Area2D", "set_priority");
	___mb.mb_set_space_override_mode = godot::api->godot_method_bind_get_method("Area2D", "set_space_override_mode");
}

Area2D *Area2D::_new()
{
	return (Area2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Area2D")());
}
void Area2D::_area_enter_tree(const int64_t id) {
	___godot_icall_void_int(___mb.mb__area_enter_tree, (const Object *) this, id);
}

void Area2D::_area_exit_tree(const int64_t id) {
	___godot_icall_void_int(___mb.mb__area_exit_tree, (const Object *) this, id);
}

void Area2D::_area_inout(const int64_t arg0, const RID arg1, const int64_t arg2, const int64_t arg3, const int64_t arg4) {
	___godot_icall_void_int_RID_int_int_int(___mb.mb__area_inout, (const Object *) this, arg0, arg1, arg2, arg3, arg4);
}

void Area2D::_body_enter_tree(const int64_t id) {
	___godot_icall_void_int(___mb.mb__body_enter_tree, (const Object *) this, id);
}

void Area2D::_body_exit_tree(const int64_t id) {
	___godot_icall_void_int(___mb.mb__body_exit_tree, (const Object *) this, id);
}

void Area2D::_body_inout(const int64_t arg0, const RID arg1, const int64_t arg2, const int64_t arg3, const int64_t arg4) {
	___godot_icall_void_int_RID_int_int_int(___mb.mb__body_inout, (const Object *) this, arg0, arg1, arg2, arg3, arg4);
}

real_t Area2D::get_angular_damp() const {
	return ___godot_icall_float(___mb.mb_get_angular_damp, (const Object *) this);
}

String Area2D::get_audio_bus_name() const {
	return ___godot_icall_String(___mb.mb_get_audio_bus_name, (const Object *) this);
}

int64_t Area2D::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

bool Area2D::get_collision_layer_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_layer_bit, (const Object *) this, bit);
}

int64_t Area2D::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool Area2D::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

real_t Area2D::get_gravity() const {
	return ___godot_icall_float(___mb.mb_get_gravity, (const Object *) this);
}

real_t Area2D::get_gravity_distance_scale() const {
	return ___godot_icall_float(___mb.mb_get_gravity_distance_scale, (const Object *) this);
}

Vector2 Area2D::get_gravity_vector() const {
	return ___godot_icall_Vector2(___mb.mb_get_gravity_vector, (const Object *) this);
}

real_t Area2D::get_linear_damp() const {
	return ___godot_icall_float(___mb.mb_get_linear_damp, (const Object *) this);
}

Array Area2D::get_overlapping_areas() const {
	return ___godot_icall_Array(___mb.mb_get_overlapping_areas, (const Object *) this);
}

Array Area2D::get_overlapping_bodies() const {
	return ___godot_icall_Array(___mb.mb_get_overlapping_bodies, (const Object *) this);
}

real_t Area2D::get_priority() const {
	return ___godot_icall_float(___mb.mb_get_priority, (const Object *) this);
}

Area2D::SpaceOverride Area2D::get_space_override_mode() const {
	return (Area2D::SpaceOverride) ___godot_icall_int(___mb.mb_get_space_override_mode, (const Object *) this);
}

bool Area2D::is_gravity_a_point() const {
	return ___godot_icall_bool(___mb.mb_is_gravity_a_point, (const Object *) this);
}

bool Area2D::is_monitorable() const {
	return ___godot_icall_bool(___mb.mb_is_monitorable, (const Object *) this);
}

bool Area2D::is_monitoring() const {
	return ___godot_icall_bool(___mb.mb_is_monitoring, (const Object *) this);
}

bool Area2D::is_overriding_audio_bus() const {
	return ___godot_icall_bool(___mb.mb_is_overriding_audio_bus, (const Object *) this);
}

bool Area2D::overlaps_area(const Node *area) const {
	return ___godot_icall_bool_Object(___mb.mb_overlaps_area, (const Object *) this, area);
}

bool Area2D::overlaps_body(const Node *body) const {
	return ___godot_icall_bool_Object(___mb.mb_overlaps_body, (const Object *) this, body);
}

void Area2D::set_angular_damp(const real_t angular_damp) {
	___godot_icall_void_float(___mb.mb_set_angular_damp, (const Object *) this, angular_damp);
}

void Area2D::set_audio_bus_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_audio_bus_name, (const Object *) this, name);
}

void Area2D::set_audio_bus_override(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_audio_bus_override, (const Object *) this, enable);
}

void Area2D::set_collision_layer(const int64_t collision_layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, collision_layer);
}

void Area2D::set_collision_layer_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_layer_bit, (const Object *) this, bit, value);
}

void Area2D::set_collision_mask(const int64_t collision_mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, collision_mask);
}

void Area2D::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void Area2D::set_gravity(const real_t gravity) {
	___godot_icall_void_float(___mb.mb_set_gravity, (const Object *) this, gravity);
}

void Area2D::set_gravity_distance_scale(const real_t distance_scale) {
	___godot_icall_void_float(___mb.mb_set_gravity_distance_scale, (const Object *) this, distance_scale);
}

void Area2D::set_gravity_is_point(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_gravity_is_point, (const Object *) this, enable);
}

void Area2D::set_gravity_vector(const Vector2 vector) {
	___godot_icall_void_Vector2(___mb.mb_set_gravity_vector, (const Object *) this, vector);
}

void Area2D::set_linear_damp(const real_t linear_damp) {
	___godot_icall_void_float(___mb.mb_set_linear_damp, (const Object *) this, linear_damp);
}

void Area2D::set_monitorable(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_monitorable, (const Object *) this, enable);
}

void Area2D::set_monitoring(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_monitoring, (const Object *) this, enable);
}

void Area2D::set_priority(const real_t priority) {
	___godot_icall_void_float(___mb.mb_set_priority, (const Object *) this, priority);
}

void Area2D::set_space_override_mode(const int64_t space_override_mode) {
	___godot_icall_void_int(___mb.mb_set_space_override_mode, (const Object *) this, space_override_mode);
}

}