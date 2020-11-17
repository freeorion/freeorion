#include "SoftBody.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"


namespace godot {


SoftBody::___method_bindings SoftBody::___mb = {};

void SoftBody::___init_method_bindings() {
	___mb.mb__draw_soft_mesh = godot::api->godot_method_bind_get_method("SoftBody", "_draw_soft_mesh");
	___mb.mb_add_collision_exception_with = godot::api->godot_method_bind_get_method("SoftBody", "add_collision_exception_with");
	___mb.mb_get_areaAngular_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "get_areaAngular_stiffness");
	___mb.mb_get_collision_exceptions = godot::api->godot_method_bind_get_method("SoftBody", "get_collision_exceptions");
	___mb.mb_get_collision_layer = godot::api->godot_method_bind_get_method("SoftBody", "get_collision_layer");
	___mb.mb_get_collision_layer_bit = godot::api->godot_method_bind_get_method("SoftBody", "get_collision_layer_bit");
	___mb.mb_get_collision_mask = godot::api->godot_method_bind_get_method("SoftBody", "get_collision_mask");
	___mb.mb_get_collision_mask_bit = godot::api->godot_method_bind_get_method("SoftBody", "get_collision_mask_bit");
	___mb.mb_get_damping_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "get_damping_coefficient");
	___mb.mb_get_drag_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "get_drag_coefficient");
	___mb.mb_get_linear_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "get_linear_stiffness");
	___mb.mb_get_parent_collision_ignore = godot::api->godot_method_bind_get_method("SoftBody", "get_parent_collision_ignore");
	___mb.mb_get_pose_matching_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "get_pose_matching_coefficient");
	___mb.mb_get_pressure_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "get_pressure_coefficient");
	___mb.mb_get_simulation_precision = godot::api->godot_method_bind_get_method("SoftBody", "get_simulation_precision");
	___mb.mb_get_total_mass = godot::api->godot_method_bind_get_method("SoftBody", "get_total_mass");
	___mb.mb_get_volume_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "get_volume_stiffness");
	___mb.mb_is_ray_pickable = godot::api->godot_method_bind_get_method("SoftBody", "is_ray_pickable");
	___mb.mb_remove_collision_exception_with = godot::api->godot_method_bind_get_method("SoftBody", "remove_collision_exception_with");
	___mb.mb_set_areaAngular_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "set_areaAngular_stiffness");
	___mb.mb_set_collision_layer = godot::api->godot_method_bind_get_method("SoftBody", "set_collision_layer");
	___mb.mb_set_collision_layer_bit = godot::api->godot_method_bind_get_method("SoftBody", "set_collision_layer_bit");
	___mb.mb_set_collision_mask = godot::api->godot_method_bind_get_method("SoftBody", "set_collision_mask");
	___mb.mb_set_collision_mask_bit = godot::api->godot_method_bind_get_method("SoftBody", "set_collision_mask_bit");
	___mb.mb_set_damping_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "set_damping_coefficient");
	___mb.mb_set_drag_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "set_drag_coefficient");
	___mb.mb_set_linear_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "set_linear_stiffness");
	___mb.mb_set_parent_collision_ignore = godot::api->godot_method_bind_get_method("SoftBody", "set_parent_collision_ignore");
	___mb.mb_set_pose_matching_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "set_pose_matching_coefficient");
	___mb.mb_set_pressure_coefficient = godot::api->godot_method_bind_get_method("SoftBody", "set_pressure_coefficient");
	___mb.mb_set_ray_pickable = godot::api->godot_method_bind_get_method("SoftBody", "set_ray_pickable");
	___mb.mb_set_simulation_precision = godot::api->godot_method_bind_get_method("SoftBody", "set_simulation_precision");
	___mb.mb_set_total_mass = godot::api->godot_method_bind_get_method("SoftBody", "set_total_mass");
	___mb.mb_set_volume_stiffness = godot::api->godot_method_bind_get_method("SoftBody", "set_volume_stiffness");
}

SoftBody *SoftBody::_new()
{
	return (SoftBody *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SoftBody")());
}
void SoftBody::_draw_soft_mesh() {
	___godot_icall_void(___mb.mb__draw_soft_mesh, (const Object *) this);
}

void SoftBody::add_collision_exception_with(const Node *body) {
	___godot_icall_void_Object(___mb.mb_add_collision_exception_with, (const Object *) this, body);
}

real_t SoftBody::get_areaAngular_stiffness() {
	return ___godot_icall_float(___mb.mb_get_areaAngular_stiffness, (const Object *) this);
}

Array SoftBody::get_collision_exceptions() {
	return ___godot_icall_Array(___mb.mb_get_collision_exceptions, (const Object *) this);
}

int64_t SoftBody::get_collision_layer() const {
	return ___godot_icall_int(___mb.mb_get_collision_layer, (const Object *) this);
}

bool SoftBody::get_collision_layer_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_layer_bit, (const Object *) this, bit);
}

int64_t SoftBody::get_collision_mask() const {
	return ___godot_icall_int(___mb.mb_get_collision_mask, (const Object *) this);
}

bool SoftBody::get_collision_mask_bit(const int64_t bit) const {
	return ___godot_icall_bool_int(___mb.mb_get_collision_mask_bit, (const Object *) this, bit);
}

real_t SoftBody::get_damping_coefficient() {
	return ___godot_icall_float(___mb.mb_get_damping_coefficient, (const Object *) this);
}

real_t SoftBody::get_drag_coefficient() {
	return ___godot_icall_float(___mb.mb_get_drag_coefficient, (const Object *) this);
}

real_t SoftBody::get_linear_stiffness() {
	return ___godot_icall_float(___mb.mb_get_linear_stiffness, (const Object *) this);
}

NodePath SoftBody::get_parent_collision_ignore() const {
	return ___godot_icall_NodePath(___mb.mb_get_parent_collision_ignore, (const Object *) this);
}

real_t SoftBody::get_pose_matching_coefficient() {
	return ___godot_icall_float(___mb.mb_get_pose_matching_coefficient, (const Object *) this);
}

real_t SoftBody::get_pressure_coefficient() {
	return ___godot_icall_float(___mb.mb_get_pressure_coefficient, (const Object *) this);
}

int64_t SoftBody::get_simulation_precision() {
	return ___godot_icall_int(___mb.mb_get_simulation_precision, (const Object *) this);
}

real_t SoftBody::get_total_mass() {
	return ___godot_icall_float(___mb.mb_get_total_mass, (const Object *) this);
}

real_t SoftBody::get_volume_stiffness() {
	return ___godot_icall_float(___mb.mb_get_volume_stiffness, (const Object *) this);
}

bool SoftBody::is_ray_pickable() const {
	return ___godot_icall_bool(___mb.mb_is_ray_pickable, (const Object *) this);
}

void SoftBody::remove_collision_exception_with(const Node *body) {
	___godot_icall_void_Object(___mb.mb_remove_collision_exception_with, (const Object *) this, body);
}

void SoftBody::set_areaAngular_stiffness(const real_t areaAngular_stiffness) {
	___godot_icall_void_float(___mb.mb_set_areaAngular_stiffness, (const Object *) this, areaAngular_stiffness);
}

void SoftBody::set_collision_layer(const int64_t collision_layer) {
	___godot_icall_void_int(___mb.mb_set_collision_layer, (const Object *) this, collision_layer);
}

void SoftBody::set_collision_layer_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_layer_bit, (const Object *) this, bit, value);
}

void SoftBody::set_collision_mask(const int64_t collision_mask) {
	___godot_icall_void_int(___mb.mb_set_collision_mask, (const Object *) this, collision_mask);
}

void SoftBody::set_collision_mask_bit(const int64_t bit, const bool value) {
	___godot_icall_void_int_bool(___mb.mb_set_collision_mask_bit, (const Object *) this, bit, value);
}

void SoftBody::set_damping_coefficient(const real_t damping_coefficient) {
	___godot_icall_void_float(___mb.mb_set_damping_coefficient, (const Object *) this, damping_coefficient);
}

void SoftBody::set_drag_coefficient(const real_t drag_coefficient) {
	___godot_icall_void_float(___mb.mb_set_drag_coefficient, (const Object *) this, drag_coefficient);
}

void SoftBody::set_linear_stiffness(const real_t linear_stiffness) {
	___godot_icall_void_float(___mb.mb_set_linear_stiffness, (const Object *) this, linear_stiffness);
}

void SoftBody::set_parent_collision_ignore(const NodePath parent_collision_ignore) {
	___godot_icall_void_NodePath(___mb.mb_set_parent_collision_ignore, (const Object *) this, parent_collision_ignore);
}

void SoftBody::set_pose_matching_coefficient(const real_t pose_matching_coefficient) {
	___godot_icall_void_float(___mb.mb_set_pose_matching_coefficient, (const Object *) this, pose_matching_coefficient);
}

void SoftBody::set_pressure_coefficient(const real_t pressure_coefficient) {
	___godot_icall_void_float(___mb.mb_set_pressure_coefficient, (const Object *) this, pressure_coefficient);
}

void SoftBody::set_ray_pickable(const bool ray_pickable) {
	___godot_icall_void_bool(___mb.mb_set_ray_pickable, (const Object *) this, ray_pickable);
}

void SoftBody::set_simulation_precision(const int64_t simulation_precision) {
	___godot_icall_void_int(___mb.mb_set_simulation_precision, (const Object *) this, simulation_precision);
}

void SoftBody::set_total_mass(const real_t mass) {
	___godot_icall_void_float(___mb.mb_set_total_mass, (const Object *) this, mass);
}

void SoftBody::set_volume_stiffness(const real_t volume_stiffness) {
	___godot_icall_void_float(___mb.mb_set_volume_stiffness, (const Object *) this, volume_stiffness);
}

}