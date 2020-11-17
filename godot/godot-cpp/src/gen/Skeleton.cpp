#include "Skeleton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "SkinReference.hpp"
#include "Skin.hpp"


namespace godot {


Skeleton::___method_bindings Skeleton::___mb = {};

void Skeleton::___init_method_bindings() {
	___mb.mb_add_bone = godot::api->godot_method_bind_get_method("Skeleton", "add_bone");
	___mb.mb_bind_child_node_to_bone = godot::api->godot_method_bind_get_method("Skeleton", "bind_child_node_to_bone");
	___mb.mb_clear_bones = godot::api->godot_method_bind_get_method("Skeleton", "clear_bones");
	___mb.mb_find_bone = godot::api->godot_method_bind_get_method("Skeleton", "find_bone");
	___mb.mb_get_bone_count = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_count");
	___mb.mb_get_bone_custom_pose = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_custom_pose");
	___mb.mb_get_bone_global_pose = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_global_pose");
	___mb.mb_get_bone_name = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_name");
	___mb.mb_get_bone_parent = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_parent");
	___mb.mb_get_bone_pose = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_pose");
	___mb.mb_get_bone_rest = godot::api->godot_method_bind_get_method("Skeleton", "get_bone_rest");
	___mb.mb_get_bound_child_nodes_to_bone = godot::api->godot_method_bind_get_method("Skeleton", "get_bound_child_nodes_to_bone");
	___mb.mb_is_bone_rest_disabled = godot::api->godot_method_bind_get_method("Skeleton", "is_bone_rest_disabled");
	___mb.mb_localize_rests = godot::api->godot_method_bind_get_method("Skeleton", "localize_rests");
	___mb.mb_physical_bones_add_collision_exception = godot::api->godot_method_bind_get_method("Skeleton", "physical_bones_add_collision_exception");
	___mb.mb_physical_bones_remove_collision_exception = godot::api->godot_method_bind_get_method("Skeleton", "physical_bones_remove_collision_exception");
	___mb.mb_physical_bones_start_simulation = godot::api->godot_method_bind_get_method("Skeleton", "physical_bones_start_simulation");
	___mb.mb_physical_bones_stop_simulation = godot::api->godot_method_bind_get_method("Skeleton", "physical_bones_stop_simulation");
	___mb.mb_register_skin = godot::api->godot_method_bind_get_method("Skeleton", "register_skin");
	___mb.mb_set_bone_custom_pose = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_custom_pose");
	___mb.mb_set_bone_disable_rest = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_disable_rest");
	___mb.mb_set_bone_global_pose_override = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_global_pose_override");
	___mb.mb_set_bone_parent = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_parent");
	___mb.mb_set_bone_pose = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_pose");
	___mb.mb_set_bone_rest = godot::api->godot_method_bind_get_method("Skeleton", "set_bone_rest");
	___mb.mb_unbind_child_node_from_bone = godot::api->godot_method_bind_get_method("Skeleton", "unbind_child_node_from_bone");
	___mb.mb_unparent_bone_and_rest = godot::api->godot_method_bind_get_method("Skeleton", "unparent_bone_and_rest");
}

Skeleton *Skeleton::_new()
{
	return (Skeleton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Skeleton")());
}
void Skeleton::add_bone(const String name) {
	___godot_icall_void_String(___mb.mb_add_bone, (const Object *) this, name);
}

void Skeleton::bind_child_node_to_bone(const int64_t bone_idx, const Node *node) {
	___godot_icall_void_int_Object(___mb.mb_bind_child_node_to_bone, (const Object *) this, bone_idx, node);
}

void Skeleton::clear_bones() {
	___godot_icall_void(___mb.mb_clear_bones, (const Object *) this);
}

int64_t Skeleton::find_bone(const String name) const {
	return ___godot_icall_int_String(___mb.mb_find_bone, (const Object *) this, name);
}

int64_t Skeleton::get_bone_count() const {
	return ___godot_icall_int(___mb.mb_get_bone_count, (const Object *) this);
}

Transform Skeleton::get_bone_custom_pose(const int64_t bone_idx) const {
	return ___godot_icall_Transform_int(___mb.mb_get_bone_custom_pose, (const Object *) this, bone_idx);
}

Transform Skeleton::get_bone_global_pose(const int64_t bone_idx) const {
	return ___godot_icall_Transform_int(___mb.mb_get_bone_global_pose, (const Object *) this, bone_idx);
}

String Skeleton::get_bone_name(const int64_t bone_idx) const {
	return ___godot_icall_String_int(___mb.mb_get_bone_name, (const Object *) this, bone_idx);
}

int64_t Skeleton::get_bone_parent(const int64_t bone_idx) const {
	return ___godot_icall_int_int(___mb.mb_get_bone_parent, (const Object *) this, bone_idx);
}

Transform Skeleton::get_bone_pose(const int64_t bone_idx) const {
	return ___godot_icall_Transform_int(___mb.mb_get_bone_pose, (const Object *) this, bone_idx);
}

Transform Skeleton::get_bone_rest(const int64_t bone_idx) const {
	return ___godot_icall_Transform_int(___mb.mb_get_bone_rest, (const Object *) this, bone_idx);
}

Array Skeleton::get_bound_child_nodes_to_bone(const int64_t bone_idx) const {
	return ___godot_icall_Array_int(___mb.mb_get_bound_child_nodes_to_bone, (const Object *) this, bone_idx);
}

bool Skeleton::is_bone_rest_disabled(const int64_t bone_idx) const {
	return ___godot_icall_bool_int(___mb.mb_is_bone_rest_disabled, (const Object *) this, bone_idx);
}

void Skeleton::localize_rests() {
	___godot_icall_void(___mb.mb_localize_rests, (const Object *) this);
}

void Skeleton::physical_bones_add_collision_exception(const RID exception) {
	___godot_icall_void_RID(___mb.mb_physical_bones_add_collision_exception, (const Object *) this, exception);
}

void Skeleton::physical_bones_remove_collision_exception(const RID exception) {
	___godot_icall_void_RID(___mb.mb_physical_bones_remove_collision_exception, (const Object *) this, exception);
}

void Skeleton::physical_bones_start_simulation(const Array bones) {
	___godot_icall_void_Array(___mb.mb_physical_bones_start_simulation, (const Object *) this, bones);
}

void Skeleton::physical_bones_stop_simulation() {
	___godot_icall_void(___mb.mb_physical_bones_stop_simulation, (const Object *) this);
}

Ref<SkinReference> Skeleton::register_skin(const Ref<Skin> skin) {
	return Ref<SkinReference>::__internal_constructor(___godot_icall_Object_Object(___mb.mb_register_skin, (const Object *) this, skin.ptr()));
}

void Skeleton::set_bone_custom_pose(const int64_t bone_idx, const Transform custom_pose) {
	___godot_icall_void_int_Transform(___mb.mb_set_bone_custom_pose, (const Object *) this, bone_idx, custom_pose);
}

void Skeleton::set_bone_disable_rest(const int64_t bone_idx, const bool disable) {
	___godot_icall_void_int_bool(___mb.mb_set_bone_disable_rest, (const Object *) this, bone_idx, disable);
}

void Skeleton::set_bone_global_pose_override(const int64_t bone_idx, const Transform pose, const real_t amount, const bool persistent) {
	___godot_icall_void_int_Transform_float_bool(___mb.mb_set_bone_global_pose_override, (const Object *) this, bone_idx, pose, amount, persistent);
}

void Skeleton::set_bone_parent(const int64_t bone_idx, const int64_t parent_idx) {
	___godot_icall_void_int_int(___mb.mb_set_bone_parent, (const Object *) this, bone_idx, parent_idx);
}

void Skeleton::set_bone_pose(const int64_t bone_idx, const Transform pose) {
	___godot_icall_void_int_Transform(___mb.mb_set_bone_pose, (const Object *) this, bone_idx, pose);
}

void Skeleton::set_bone_rest(const int64_t bone_idx, const Transform rest) {
	___godot_icall_void_int_Transform(___mb.mb_set_bone_rest, (const Object *) this, bone_idx, rest);
}

void Skeleton::unbind_child_node_from_bone(const int64_t bone_idx, const Node *node) {
	___godot_icall_void_int_Object(___mb.mb_unbind_child_node_from_bone, (const Object *) this, bone_idx, node);
}

void Skeleton::unparent_bone_and_rest(const int64_t bone_idx) {
	___godot_icall_void_int(___mb.mb_unparent_bone_and_rest, (const Object *) this, bone_idx);
}

}