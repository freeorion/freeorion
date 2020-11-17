#include "AnimationNodeBlendTree.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AnimationNode.hpp"


namespace godot {


AnimationNodeBlendTree::___method_bindings AnimationNodeBlendTree::___mb = {};

void AnimationNodeBlendTree::___init_method_bindings() {
	___mb.mb__node_changed = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "_node_changed");
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "_tree_changed");
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "add_node");
	___mb.mb_connect_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "connect_node");
	___mb.mb_disconnect_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "disconnect_node");
	___mb.mb_get_graph_offset = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "get_graph_offset");
	___mb.mb_get_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "get_node");
	___mb.mb_get_node_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "get_node_position");
	___mb.mb_has_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "has_node");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "remove_node");
	___mb.mb_rename_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "rename_node");
	___mb.mb_set_graph_offset = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "set_graph_offset");
	___mb.mb_set_node_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendTree", "set_node_position");
}

AnimationNodeBlendTree *AnimationNodeBlendTree::_new()
{
	return (AnimationNodeBlendTree *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeBlendTree")());
}
void AnimationNodeBlendTree::_node_changed(const String node) {
	___godot_icall_void_String(___mb.mb__node_changed, (const Object *) this, node);
}

void AnimationNodeBlendTree::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void AnimationNodeBlendTree::add_node(const String name, const Ref<AnimationNode> node, const Vector2 position) {
	___godot_icall_void_String_Object_Vector2(___mb.mb_add_node, (const Object *) this, name, node.ptr(), position);
}

void AnimationNodeBlendTree::connect_node(const String input_node, const int64_t input_index, const String output_node) {
	___godot_icall_void_String_int_String(___mb.mb_connect_node, (const Object *) this, input_node, input_index, output_node);
}

void AnimationNodeBlendTree::disconnect_node(const String input_node, const int64_t input_index) {
	___godot_icall_void_String_int(___mb.mb_disconnect_node, (const Object *) this, input_node, input_index);
}

Vector2 AnimationNodeBlendTree::get_graph_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_graph_offset, (const Object *) this);
}

Ref<AnimationNode> AnimationNodeBlendTree::get_node(const String name) const {
	return Ref<AnimationNode>::__internal_constructor(___godot_icall_Object_String(___mb.mb_get_node, (const Object *) this, name));
}

Vector2 AnimationNodeBlendTree::get_node_position(const String name) const {
	return ___godot_icall_Vector2_String(___mb.mb_get_node_position, (const Object *) this, name);
}

bool AnimationNodeBlendTree::has_node(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_node, (const Object *) this, name);
}

void AnimationNodeBlendTree::remove_node(const String name) {
	___godot_icall_void_String(___mb.mb_remove_node, (const Object *) this, name);
}

void AnimationNodeBlendTree::rename_node(const String name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_node, (const Object *) this, name, new_name);
}

void AnimationNodeBlendTree::set_graph_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_graph_offset, (const Object *) this, offset);
}

void AnimationNodeBlendTree::set_node_position(const String name, const Vector2 position) {
	___godot_icall_void_String_Vector2(___mb.mb_set_node_position, (const Object *) this, name, position);
}

}