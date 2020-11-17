#include "AnimationNodeBlendSpace1D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AnimationRootNode.hpp"


namespace godot {


AnimationNodeBlendSpace1D::___method_bindings AnimationNodeBlendSpace1D::___mb = {};

void AnimationNodeBlendSpace1D::___init_method_bindings() {
	___mb.mb__add_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "_add_blend_point");
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "_tree_changed");
	___mb.mb_add_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "add_blend_point");
	___mb.mb_get_blend_point_count = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_blend_point_count");
	___mb.mb_get_blend_point_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_blend_point_node");
	___mb.mb_get_blend_point_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_blend_point_position");
	___mb.mb_get_max_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_max_space");
	___mb.mb_get_min_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_min_space");
	___mb.mb_get_snap = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_snap");
	___mb.mb_get_value_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "get_value_label");
	___mb.mb_remove_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "remove_blend_point");
	___mb.mb_set_blend_point_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_blend_point_node");
	___mb.mb_set_blend_point_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_blend_point_position");
	___mb.mb_set_max_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_max_space");
	___mb.mb_set_min_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_min_space");
	___mb.mb_set_snap = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_snap");
	___mb.mb_set_value_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace1D", "set_value_label");
}

AnimationNodeBlendSpace1D *AnimationNodeBlendSpace1D::_new()
{
	return (AnimationNodeBlendSpace1D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeBlendSpace1D")());
}
void AnimationNodeBlendSpace1D::_add_blend_point(const int64_t index, const Ref<AnimationRootNode> node) {
	___godot_icall_void_int_Object(___mb.mb__add_blend_point, (const Object *) this, index, node.ptr());
}

void AnimationNodeBlendSpace1D::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void AnimationNodeBlendSpace1D::add_blend_point(const Ref<AnimationRootNode> node, const real_t pos, const int64_t at_index) {
	___godot_icall_void_Object_float_int(___mb.mb_add_blend_point, (const Object *) this, node.ptr(), pos, at_index);
}

int64_t AnimationNodeBlendSpace1D::get_blend_point_count() const {
	return ___godot_icall_int(___mb.mb_get_blend_point_count, (const Object *) this);
}

Ref<AnimationRootNode> AnimationNodeBlendSpace1D::get_blend_point_node(const int64_t point) const {
	return Ref<AnimationRootNode>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_blend_point_node, (const Object *) this, point));
}

real_t AnimationNodeBlendSpace1D::get_blend_point_position(const int64_t point) const {
	return ___godot_icall_float_int(___mb.mb_get_blend_point_position, (const Object *) this, point);
}

real_t AnimationNodeBlendSpace1D::get_max_space() const {
	return ___godot_icall_float(___mb.mb_get_max_space, (const Object *) this);
}

real_t AnimationNodeBlendSpace1D::get_min_space() const {
	return ___godot_icall_float(___mb.mb_get_min_space, (const Object *) this);
}

real_t AnimationNodeBlendSpace1D::get_snap() const {
	return ___godot_icall_float(___mb.mb_get_snap, (const Object *) this);
}

String AnimationNodeBlendSpace1D::get_value_label() const {
	return ___godot_icall_String(___mb.mb_get_value_label, (const Object *) this);
}

void AnimationNodeBlendSpace1D::remove_blend_point(const int64_t point) {
	___godot_icall_void_int(___mb.mb_remove_blend_point, (const Object *) this, point);
}

void AnimationNodeBlendSpace1D::set_blend_point_node(const int64_t point, const Ref<AnimationRootNode> node) {
	___godot_icall_void_int_Object(___mb.mb_set_blend_point_node, (const Object *) this, point, node.ptr());
}

void AnimationNodeBlendSpace1D::set_blend_point_position(const int64_t point, const real_t pos) {
	___godot_icall_void_int_float(___mb.mb_set_blend_point_position, (const Object *) this, point, pos);
}

void AnimationNodeBlendSpace1D::set_max_space(const real_t max_space) {
	___godot_icall_void_float(___mb.mb_set_max_space, (const Object *) this, max_space);
}

void AnimationNodeBlendSpace1D::set_min_space(const real_t min_space) {
	___godot_icall_void_float(___mb.mb_set_min_space, (const Object *) this, min_space);
}

void AnimationNodeBlendSpace1D::set_snap(const real_t snap) {
	___godot_icall_void_float(___mb.mb_set_snap, (const Object *) this, snap);
}

void AnimationNodeBlendSpace1D::set_value_label(const String text) {
	___godot_icall_void_String(___mb.mb_set_value_label, (const Object *) this, text);
}

}