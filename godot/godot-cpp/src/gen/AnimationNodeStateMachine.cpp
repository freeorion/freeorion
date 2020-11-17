#include "AnimationNodeStateMachine.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AnimationNode.hpp"
#include "AnimationNodeStateMachineTransition.hpp"


namespace godot {


AnimationNodeStateMachine::___method_bindings AnimationNodeStateMachine::___mb = {};

void AnimationNodeStateMachine::___init_method_bindings() {
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "_tree_changed");
	___mb.mb_add_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "add_node");
	___mb.mb_add_transition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "add_transition");
	___mb.mb_get_end_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_end_node");
	___mb.mb_get_graph_offset = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_graph_offset");
	___mb.mb_get_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_node");
	___mb.mb_get_node_name = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_node_name");
	___mb.mb_get_node_position = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_node_position");
	___mb.mb_get_start_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_start_node");
	___mb.mb_get_transition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_transition");
	___mb.mb_get_transition_count = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_transition_count");
	___mb.mb_get_transition_from = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_transition_from");
	___mb.mb_get_transition_to = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "get_transition_to");
	___mb.mb_has_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "has_node");
	___mb.mb_has_transition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "has_transition");
	___mb.mb_remove_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "remove_node");
	___mb.mb_remove_transition = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "remove_transition");
	___mb.mb_remove_transition_by_index = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "remove_transition_by_index");
	___mb.mb_rename_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "rename_node");
	___mb.mb_set_end_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "set_end_node");
	___mb.mb_set_graph_offset = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "set_graph_offset");
	___mb.mb_set_node_position = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "set_node_position");
	___mb.mb_set_start_node = godot::api->godot_method_bind_get_method("AnimationNodeStateMachine", "set_start_node");
}

AnimationNodeStateMachine *AnimationNodeStateMachine::_new()
{
	return (AnimationNodeStateMachine *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeStateMachine")());
}
void AnimationNodeStateMachine::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void AnimationNodeStateMachine::add_node(const String name, const Ref<AnimationNode> node, const Vector2 position) {
	___godot_icall_void_String_Object_Vector2(___mb.mb_add_node, (const Object *) this, name, node.ptr(), position);
}

void AnimationNodeStateMachine::add_transition(const String from, const String to, const Ref<AnimationNodeStateMachineTransition> transition) {
	___godot_icall_void_String_String_Object(___mb.mb_add_transition, (const Object *) this, from, to, transition.ptr());
}

String AnimationNodeStateMachine::get_end_node() const {
	return ___godot_icall_String(___mb.mb_get_end_node, (const Object *) this);
}

Vector2 AnimationNodeStateMachine::get_graph_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_graph_offset, (const Object *) this);
}

Ref<AnimationNode> AnimationNodeStateMachine::get_node(const String name) const {
	return Ref<AnimationNode>::__internal_constructor(___godot_icall_Object_String(___mb.mb_get_node, (const Object *) this, name));
}

String AnimationNodeStateMachine::get_node_name(const Ref<AnimationNode> node) const {
	return ___godot_icall_String_Object(___mb.mb_get_node_name, (const Object *) this, node.ptr());
}

Vector2 AnimationNodeStateMachine::get_node_position(const String name) const {
	return ___godot_icall_Vector2_String(___mb.mb_get_node_position, (const Object *) this, name);
}

String AnimationNodeStateMachine::get_start_node() const {
	return ___godot_icall_String(___mb.mb_get_start_node, (const Object *) this);
}

Ref<AnimationNodeStateMachineTransition> AnimationNodeStateMachine::get_transition(const int64_t idx) const {
	return Ref<AnimationNodeStateMachineTransition>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_transition, (const Object *) this, idx));
}

int64_t AnimationNodeStateMachine::get_transition_count() const {
	return ___godot_icall_int(___mb.mb_get_transition_count, (const Object *) this);
}

String AnimationNodeStateMachine::get_transition_from(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_transition_from, (const Object *) this, idx);
}

String AnimationNodeStateMachine::get_transition_to(const int64_t idx) const {
	return ___godot_icall_String_int(___mb.mb_get_transition_to, (const Object *) this, idx);
}

bool AnimationNodeStateMachine::has_node(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_node, (const Object *) this, name);
}

bool AnimationNodeStateMachine::has_transition(const String from, const String to) const {
	return ___godot_icall_bool_String_String(___mb.mb_has_transition, (const Object *) this, from, to);
}

void AnimationNodeStateMachine::remove_node(const String name) {
	___godot_icall_void_String(___mb.mb_remove_node, (const Object *) this, name);
}

void AnimationNodeStateMachine::remove_transition(const String from, const String to) {
	___godot_icall_void_String_String(___mb.mb_remove_transition, (const Object *) this, from, to);
}

void AnimationNodeStateMachine::remove_transition_by_index(const int64_t idx) {
	___godot_icall_void_int(___mb.mb_remove_transition_by_index, (const Object *) this, idx);
}

void AnimationNodeStateMachine::rename_node(const String name, const String new_name) {
	___godot_icall_void_String_String(___mb.mb_rename_node, (const Object *) this, name, new_name);
}

void AnimationNodeStateMachine::set_end_node(const String name) {
	___godot_icall_void_String(___mb.mb_set_end_node, (const Object *) this, name);
}

void AnimationNodeStateMachine::set_graph_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_graph_offset, (const Object *) this, offset);
}

void AnimationNodeStateMachine::set_node_position(const String name, const Vector2 position) {
	___godot_icall_void_String_Vector2(___mb.mb_set_node_position, (const Object *) this, name, position);
}

void AnimationNodeStateMachine::set_start_node(const String name) {
	___godot_icall_void_String(___mb.mb_set_start_node, (const Object *) this, name);
}

}