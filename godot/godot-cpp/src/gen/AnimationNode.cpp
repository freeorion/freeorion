#include "AnimationNode.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AnimationNode.hpp"
#include "Object.hpp"


namespace godot {


AnimationNode::___method_bindings AnimationNode::___mb = {};

void AnimationNode::___init_method_bindings() {
	___mb.mb__get_filters = godot::api->godot_method_bind_get_method("AnimationNode", "_get_filters");
	___mb.mb__set_filters = godot::api->godot_method_bind_get_method("AnimationNode", "_set_filters");
	___mb.mb_add_input = godot::api->godot_method_bind_get_method("AnimationNode", "add_input");
	___mb.mb_blend_animation = godot::api->godot_method_bind_get_method("AnimationNode", "blend_animation");
	___mb.mb_blend_input = godot::api->godot_method_bind_get_method("AnimationNode", "blend_input");
	___mb.mb_blend_node = godot::api->godot_method_bind_get_method("AnimationNode", "blend_node");
	___mb.mb_get_caption = godot::api->godot_method_bind_get_method("AnimationNode", "get_caption");
	___mb.mb_get_child_by_name = godot::api->godot_method_bind_get_method("AnimationNode", "get_child_by_name");
	___mb.mb_get_child_nodes = godot::api->godot_method_bind_get_method("AnimationNode", "get_child_nodes");
	___mb.mb_get_input_count = godot::api->godot_method_bind_get_method("AnimationNode", "get_input_count");
	___mb.mb_get_input_name = godot::api->godot_method_bind_get_method("AnimationNode", "get_input_name");
	___mb.mb_get_parameter = godot::api->godot_method_bind_get_method("AnimationNode", "get_parameter");
	___mb.mb_get_parameter_default_value = godot::api->godot_method_bind_get_method("AnimationNode", "get_parameter_default_value");
	___mb.mb_get_parameter_list = godot::api->godot_method_bind_get_method("AnimationNode", "get_parameter_list");
	___mb.mb_has_filter = godot::api->godot_method_bind_get_method("AnimationNode", "has_filter");
	___mb.mb_is_filter_enabled = godot::api->godot_method_bind_get_method("AnimationNode", "is_filter_enabled");
	___mb.mb_is_path_filtered = godot::api->godot_method_bind_get_method("AnimationNode", "is_path_filtered");
	___mb.mb_process = godot::api->godot_method_bind_get_method("AnimationNode", "process");
	___mb.mb_remove_input = godot::api->godot_method_bind_get_method("AnimationNode", "remove_input");
	___mb.mb_set_filter_enabled = godot::api->godot_method_bind_get_method("AnimationNode", "set_filter_enabled");
	___mb.mb_set_filter_path = godot::api->godot_method_bind_get_method("AnimationNode", "set_filter_path");
	___mb.mb_set_parameter = godot::api->godot_method_bind_get_method("AnimationNode", "set_parameter");
}

AnimationNode *AnimationNode::_new()
{
	return (AnimationNode *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNode")());
}
Array AnimationNode::_get_filters() const {
	return ___godot_icall_Array(___mb.mb__get_filters, (const Object *) this);
}

void AnimationNode::_set_filters(const Array filters) {
	___godot_icall_void_Array(___mb.mb__set_filters, (const Object *) this, filters);
}

void AnimationNode::add_input(const String name) {
	___godot_icall_void_String(___mb.mb_add_input, (const Object *) this, name);
}

void AnimationNode::blend_animation(const String animation, const real_t time, const real_t delta, const bool seeked, const real_t blend) {
	___godot_icall_void_String_float_float_bool_float(___mb.mb_blend_animation, (const Object *) this, animation, time, delta, seeked, blend);
}

real_t AnimationNode::blend_input(const int64_t input_index, const real_t time, const bool seek, const real_t blend, const int64_t filter, const bool optimize) {
	return ___godot_icall_float_int_float_bool_float_int_bool(___mb.mb_blend_input, (const Object *) this, input_index, time, seek, blend, filter, optimize);
}

real_t AnimationNode::blend_node(const String name, const Ref<AnimationNode> node, const real_t time, const bool seek, const real_t blend, const int64_t filter, const bool optimize) {
	return ___godot_icall_float_String_Object_float_bool_float_int_bool(___mb.mb_blend_node, (const Object *) this, name, node.ptr(), time, seek, blend, filter, optimize);
}

String AnimationNode::get_caption() {
	return ___godot_icall_String(___mb.mb_get_caption, (const Object *) this);
}

Object *AnimationNode::get_child_by_name(const String name) {
	return (Object *) ___godot_icall_Object_String(___mb.mb_get_child_by_name, (const Object *) this, name);
}

Dictionary AnimationNode::get_child_nodes() {
	return ___godot_icall_Dictionary(___mb.mb_get_child_nodes, (const Object *) this);
}

int64_t AnimationNode::get_input_count() const {
	return ___godot_icall_int(___mb.mb_get_input_count, (const Object *) this);
}

String AnimationNode::get_input_name(const int64_t input) {
	return ___godot_icall_String_int(___mb.mb_get_input_name, (const Object *) this, input);
}

Variant AnimationNode::get_parameter(const String name) const {
	return ___godot_icall_Variant_String(___mb.mb_get_parameter, (const Object *) this, name);
}

Variant AnimationNode::get_parameter_default_value(const String name) {
	return ___godot_icall_Variant_String(___mb.mb_get_parameter_default_value, (const Object *) this, name);
}

Array AnimationNode::get_parameter_list() {
	return ___godot_icall_Array(___mb.mb_get_parameter_list, (const Object *) this);
}

String AnimationNode::has_filter() {
	return ___godot_icall_String(___mb.mb_has_filter, (const Object *) this);
}

bool AnimationNode::is_filter_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_filter_enabled, (const Object *) this);
}

bool AnimationNode::is_path_filtered(const NodePath path) const {
	return ___godot_icall_bool_NodePath(___mb.mb_is_path_filtered, (const Object *) this, path);
}

void AnimationNode::process(const real_t time, const bool seek) {
	___godot_icall_void_float_bool(___mb.mb_process, (const Object *) this, time, seek);
}

void AnimationNode::remove_input(const int64_t index) {
	___godot_icall_void_int(___mb.mb_remove_input, (const Object *) this, index);
}

void AnimationNode::set_filter_enabled(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_filter_enabled, (const Object *) this, enable);
}

void AnimationNode::set_filter_path(const NodePath path, const bool enable) {
	___godot_icall_void_NodePath_bool(___mb.mb_set_filter_path, (const Object *) this, path, enable);
}

void AnimationNode::set_parameter(const String name, const Variant value) {
	___godot_icall_void_String_Variant(___mb.mb_set_parameter, (const Object *) this, name, value);
}

}