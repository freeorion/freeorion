#include "SkeletonIK.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Skeleton.hpp"


namespace godot {


SkeletonIK::___method_bindings SkeletonIK::___mb = {};

void SkeletonIK::___init_method_bindings() {
	___mb.mb_get_interpolation = godot::api->godot_method_bind_get_method("SkeletonIK", "get_interpolation");
	___mb.mb_get_magnet_position = godot::api->godot_method_bind_get_method("SkeletonIK", "get_magnet_position");
	___mb.mb_get_max_iterations = godot::api->godot_method_bind_get_method("SkeletonIK", "get_max_iterations");
	___mb.mb_get_min_distance = godot::api->godot_method_bind_get_method("SkeletonIK", "get_min_distance");
	___mb.mb_get_parent_skeleton = godot::api->godot_method_bind_get_method("SkeletonIK", "get_parent_skeleton");
	___mb.mb_get_root_bone = godot::api->godot_method_bind_get_method("SkeletonIK", "get_root_bone");
	___mb.mb_get_target_node = godot::api->godot_method_bind_get_method("SkeletonIK", "get_target_node");
	___mb.mb_get_target_transform = godot::api->godot_method_bind_get_method("SkeletonIK", "get_target_transform");
	___mb.mb_get_tip_bone = godot::api->godot_method_bind_get_method("SkeletonIK", "get_tip_bone");
	___mb.mb_is_override_tip_basis = godot::api->godot_method_bind_get_method("SkeletonIK", "is_override_tip_basis");
	___mb.mb_is_running = godot::api->godot_method_bind_get_method("SkeletonIK", "is_running");
	___mb.mb_is_using_magnet = godot::api->godot_method_bind_get_method("SkeletonIK", "is_using_magnet");
	___mb.mb_set_interpolation = godot::api->godot_method_bind_get_method("SkeletonIK", "set_interpolation");
	___mb.mb_set_magnet_position = godot::api->godot_method_bind_get_method("SkeletonIK", "set_magnet_position");
	___mb.mb_set_max_iterations = godot::api->godot_method_bind_get_method("SkeletonIK", "set_max_iterations");
	___mb.mb_set_min_distance = godot::api->godot_method_bind_get_method("SkeletonIK", "set_min_distance");
	___mb.mb_set_override_tip_basis = godot::api->godot_method_bind_get_method("SkeletonIK", "set_override_tip_basis");
	___mb.mb_set_root_bone = godot::api->godot_method_bind_get_method("SkeletonIK", "set_root_bone");
	___mb.mb_set_target_node = godot::api->godot_method_bind_get_method("SkeletonIK", "set_target_node");
	___mb.mb_set_target_transform = godot::api->godot_method_bind_get_method("SkeletonIK", "set_target_transform");
	___mb.mb_set_tip_bone = godot::api->godot_method_bind_get_method("SkeletonIK", "set_tip_bone");
	___mb.mb_set_use_magnet = godot::api->godot_method_bind_get_method("SkeletonIK", "set_use_magnet");
	___mb.mb_start = godot::api->godot_method_bind_get_method("SkeletonIK", "start");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("SkeletonIK", "stop");
}

SkeletonIK *SkeletonIK::_new()
{
	return (SkeletonIK *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SkeletonIK")());
}
real_t SkeletonIK::get_interpolation() const {
	return ___godot_icall_float(___mb.mb_get_interpolation, (const Object *) this);
}

Vector3 SkeletonIK::get_magnet_position() const {
	return ___godot_icall_Vector3(___mb.mb_get_magnet_position, (const Object *) this);
}

int64_t SkeletonIK::get_max_iterations() const {
	return ___godot_icall_int(___mb.mb_get_max_iterations, (const Object *) this);
}

real_t SkeletonIK::get_min_distance() const {
	return ___godot_icall_float(___mb.mb_get_min_distance, (const Object *) this);
}

Skeleton *SkeletonIK::get_parent_skeleton() const {
	return (Skeleton *) ___godot_icall_Object(___mb.mb_get_parent_skeleton, (const Object *) this);
}

String SkeletonIK::get_root_bone() const {
	return ___godot_icall_String(___mb.mb_get_root_bone, (const Object *) this);
}

NodePath SkeletonIK::get_target_node() {
	return ___godot_icall_NodePath(___mb.mb_get_target_node, (const Object *) this);
}

Transform SkeletonIK::get_target_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_target_transform, (const Object *) this);
}

String SkeletonIK::get_tip_bone() const {
	return ___godot_icall_String(___mb.mb_get_tip_bone, (const Object *) this);
}

bool SkeletonIK::is_override_tip_basis() const {
	return ___godot_icall_bool(___mb.mb_is_override_tip_basis, (const Object *) this);
}

bool SkeletonIK::is_running() {
	return ___godot_icall_bool(___mb.mb_is_running, (const Object *) this);
}

bool SkeletonIK::is_using_magnet() const {
	return ___godot_icall_bool(___mb.mb_is_using_magnet, (const Object *) this);
}

void SkeletonIK::set_interpolation(const real_t interpolation) {
	___godot_icall_void_float(___mb.mb_set_interpolation, (const Object *) this, interpolation);
}

void SkeletonIK::set_magnet_position(const Vector3 local_position) {
	___godot_icall_void_Vector3(___mb.mb_set_magnet_position, (const Object *) this, local_position);
}

void SkeletonIK::set_max_iterations(const int64_t iterations) {
	___godot_icall_void_int(___mb.mb_set_max_iterations, (const Object *) this, iterations);
}

void SkeletonIK::set_min_distance(const real_t min_distance) {
	___godot_icall_void_float(___mb.mb_set_min_distance, (const Object *) this, min_distance);
}

void SkeletonIK::set_override_tip_basis(const bool override) {
	___godot_icall_void_bool(___mb.mb_set_override_tip_basis, (const Object *) this, override);
}

void SkeletonIK::set_root_bone(const String root_bone) {
	___godot_icall_void_String(___mb.mb_set_root_bone, (const Object *) this, root_bone);
}

void SkeletonIK::set_target_node(const NodePath node) {
	___godot_icall_void_NodePath(___mb.mb_set_target_node, (const Object *) this, node);
}

void SkeletonIK::set_target_transform(const Transform target) {
	___godot_icall_void_Transform(___mb.mb_set_target_transform, (const Object *) this, target);
}

void SkeletonIK::set_tip_bone(const String tip_bone) {
	___godot_icall_void_String(___mb.mb_set_tip_bone, (const Object *) this, tip_bone);
}

void SkeletonIK::set_use_magnet(const bool use) {
	___godot_icall_void_bool(___mb.mb_set_use_magnet, (const Object *) this, use);
}

void SkeletonIK::start(const bool one_time) {
	___godot_icall_void_bool(___mb.mb_start, (const Object *) this, one_time);
}

void SkeletonIK::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}