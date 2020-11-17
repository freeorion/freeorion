#include "AnimationTree.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "AnimationNode.hpp"


namespace godot {


AnimationTree::___method_bindings AnimationTree::___mb = {};

void AnimationTree::___init_method_bindings() {
	___mb.mb__clear_caches = godot::api->godot_method_bind_get_method("AnimationTree", "_clear_caches");
	___mb.mb__node_removed = godot::api->godot_method_bind_get_method("AnimationTree", "_node_removed");
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("AnimationTree", "_tree_changed");
	___mb.mb__update_properties = godot::api->godot_method_bind_get_method("AnimationTree", "_update_properties");
	___mb.mb_advance = godot::api->godot_method_bind_get_method("AnimationTree", "advance");
	___mb.mb_get_animation_player = godot::api->godot_method_bind_get_method("AnimationTree", "get_animation_player");
	___mb.mb_get_process_mode = godot::api->godot_method_bind_get_method("AnimationTree", "get_process_mode");
	___mb.mb_get_root_motion_track = godot::api->godot_method_bind_get_method("AnimationTree", "get_root_motion_track");
	___mb.mb_get_root_motion_transform = godot::api->godot_method_bind_get_method("AnimationTree", "get_root_motion_transform");
	___mb.mb_get_tree_root = godot::api->godot_method_bind_get_method("AnimationTree", "get_tree_root");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("AnimationTree", "is_active");
	___mb.mb_rename_parameter = godot::api->godot_method_bind_get_method("AnimationTree", "rename_parameter");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("AnimationTree", "set_active");
	___mb.mb_set_animation_player = godot::api->godot_method_bind_get_method("AnimationTree", "set_animation_player");
	___mb.mb_set_process_mode = godot::api->godot_method_bind_get_method("AnimationTree", "set_process_mode");
	___mb.mb_set_root_motion_track = godot::api->godot_method_bind_get_method("AnimationTree", "set_root_motion_track");
	___mb.mb_set_tree_root = godot::api->godot_method_bind_get_method("AnimationTree", "set_tree_root");
}

AnimationTree *AnimationTree::_new()
{
	return (AnimationTree *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationTree")());
}
void AnimationTree::_clear_caches() {
	___godot_icall_void(___mb.mb__clear_caches, (const Object *) this);
}

void AnimationTree::_node_removed(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__node_removed, (const Object *) this, arg0);
}

void AnimationTree::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void AnimationTree::_update_properties() {
	___godot_icall_void(___mb.mb__update_properties, (const Object *) this);
}

void AnimationTree::advance(const real_t delta) {
	___godot_icall_void_float(___mb.mb_advance, (const Object *) this, delta);
}

NodePath AnimationTree::get_animation_player() const {
	return ___godot_icall_NodePath(___mb.mb_get_animation_player, (const Object *) this);
}

AnimationTree::AnimationProcessMode AnimationTree::get_process_mode() const {
	return (AnimationTree::AnimationProcessMode) ___godot_icall_int(___mb.mb_get_process_mode, (const Object *) this);
}

NodePath AnimationTree::get_root_motion_track() const {
	return ___godot_icall_NodePath(___mb.mb_get_root_motion_track, (const Object *) this);
}

Transform AnimationTree::get_root_motion_transform() const {
	return ___godot_icall_Transform(___mb.mb_get_root_motion_transform, (const Object *) this);
}

Ref<AnimationNode> AnimationTree::get_tree_root() const {
	return Ref<AnimationNode>::__internal_constructor(___godot_icall_Object(___mb.mb_get_tree_root, (const Object *) this));
}

bool AnimationTree::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

void AnimationTree::rename_parameter(const String old_name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_parameter, (const Object *) this, old_name, new_name);
}

void AnimationTree::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

void AnimationTree::set_animation_player(const NodePath root) {
	___godot_icall_void_NodePath(___mb.mb_set_animation_player, (const Object *) this, root);
}

void AnimationTree::set_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_process_mode, (const Object *) this, mode);
}

void AnimationTree::set_root_motion_track(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_root_motion_track, (const Object *) this, path);
}

void AnimationTree::set_tree_root(const Ref<AnimationNode> root) {
	___godot_icall_void_Object(___mb.mb_set_tree_root, (const Object *) this, root.ptr());
}

}