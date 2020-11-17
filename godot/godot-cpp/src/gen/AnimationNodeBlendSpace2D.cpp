#include "AnimationNodeBlendSpace2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "AnimationRootNode.hpp"


namespace godot {


AnimationNodeBlendSpace2D::___method_bindings AnimationNodeBlendSpace2D::___mb = {};

void AnimationNodeBlendSpace2D::___init_method_bindings() {
	___mb.mb__add_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "_add_blend_point");
	___mb.mb__get_triangles = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "_get_triangles");
	___mb.mb__set_triangles = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "_set_triangles");
	___mb.mb__tree_changed = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "_tree_changed");
	___mb.mb__update_triangles = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "_update_triangles");
	___mb.mb_add_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "add_blend_point");
	___mb.mb_add_triangle = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "add_triangle");
	___mb.mb_get_auto_triangles = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_auto_triangles");
	___mb.mb_get_blend_mode = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_blend_mode");
	___mb.mb_get_blend_point_count = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_blend_point_count");
	___mb.mb_get_blend_point_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_blend_point_node");
	___mb.mb_get_blend_point_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_blend_point_position");
	___mb.mb_get_max_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_max_space");
	___mb.mb_get_min_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_min_space");
	___mb.mb_get_snap = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_snap");
	___mb.mb_get_triangle_count = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_triangle_count");
	___mb.mb_get_triangle_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_triangle_point");
	___mb.mb_get_x_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_x_label");
	___mb.mb_get_y_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "get_y_label");
	___mb.mb_remove_blend_point = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "remove_blend_point");
	___mb.mb_remove_triangle = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "remove_triangle");
	___mb.mb_set_auto_triangles = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_auto_triangles");
	___mb.mb_set_blend_mode = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_blend_mode");
	___mb.mb_set_blend_point_node = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_blend_point_node");
	___mb.mb_set_blend_point_position = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_blend_point_position");
	___mb.mb_set_max_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_max_space");
	___mb.mb_set_min_space = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_min_space");
	___mb.mb_set_snap = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_snap");
	___mb.mb_set_x_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_x_label");
	___mb.mb_set_y_label = godot::api->godot_method_bind_get_method("AnimationNodeBlendSpace2D", "set_y_label");
}

AnimationNodeBlendSpace2D *AnimationNodeBlendSpace2D::_new()
{
	return (AnimationNodeBlendSpace2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeBlendSpace2D")());
}
void AnimationNodeBlendSpace2D::_add_blend_point(const int64_t index, const Ref<AnimationRootNode> node) {
	___godot_icall_void_int_Object(___mb.mb__add_blend_point, (const Object *) this, index, node.ptr());
}

PoolIntArray AnimationNodeBlendSpace2D::_get_triangles() const {
	return ___godot_icall_PoolIntArray(___mb.mb__get_triangles, (const Object *) this);
}

void AnimationNodeBlendSpace2D::_set_triangles(const PoolIntArray triangles) {
	___godot_icall_void_PoolIntArray(___mb.mb__set_triangles, (const Object *) this, triangles);
}

void AnimationNodeBlendSpace2D::_tree_changed() {
	___godot_icall_void(___mb.mb__tree_changed, (const Object *) this);
}

void AnimationNodeBlendSpace2D::_update_triangles() {
	___godot_icall_void(___mb.mb__update_triangles, (const Object *) this);
}

void AnimationNodeBlendSpace2D::add_blend_point(const Ref<AnimationRootNode> node, const Vector2 pos, const int64_t at_index) {
	___godot_icall_void_Object_Vector2_int(___mb.mb_add_blend_point, (const Object *) this, node.ptr(), pos, at_index);
}

void AnimationNodeBlendSpace2D::add_triangle(const int64_t x, const int64_t y, const int64_t z, const int64_t at_index) {
	___godot_icall_void_int_int_int_int(___mb.mb_add_triangle, (const Object *) this, x, y, z, at_index);
}

bool AnimationNodeBlendSpace2D::get_auto_triangles() const {
	return ___godot_icall_bool(___mb.mb_get_auto_triangles, (const Object *) this);
}

AnimationNodeBlendSpace2D::BlendMode AnimationNodeBlendSpace2D::get_blend_mode() const {
	return (AnimationNodeBlendSpace2D::BlendMode) ___godot_icall_int(___mb.mb_get_blend_mode, (const Object *) this);
}

int64_t AnimationNodeBlendSpace2D::get_blend_point_count() const {
	return ___godot_icall_int(___mb.mb_get_blend_point_count, (const Object *) this);
}

Ref<AnimationRootNode> AnimationNodeBlendSpace2D::get_blend_point_node(const int64_t point) const {
	return Ref<AnimationRootNode>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_blend_point_node, (const Object *) this, point));
}

Vector2 AnimationNodeBlendSpace2D::get_blend_point_position(const int64_t point) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_blend_point_position, (const Object *) this, point);
}

Vector2 AnimationNodeBlendSpace2D::get_max_space() const {
	return ___godot_icall_Vector2(___mb.mb_get_max_space, (const Object *) this);
}

Vector2 AnimationNodeBlendSpace2D::get_min_space() const {
	return ___godot_icall_Vector2(___mb.mb_get_min_space, (const Object *) this);
}

Vector2 AnimationNodeBlendSpace2D::get_snap() const {
	return ___godot_icall_Vector2(___mb.mb_get_snap, (const Object *) this);
}

int64_t AnimationNodeBlendSpace2D::get_triangle_count() const {
	return ___godot_icall_int(___mb.mb_get_triangle_count, (const Object *) this);
}

int64_t AnimationNodeBlendSpace2D::get_triangle_point(const int64_t triangle, const int64_t point) {
	return ___godot_icall_int_int_int(___mb.mb_get_triangle_point, (const Object *) this, triangle, point);
}

String AnimationNodeBlendSpace2D::get_x_label() const {
	return ___godot_icall_String(___mb.mb_get_x_label, (const Object *) this);
}

String AnimationNodeBlendSpace2D::get_y_label() const {
	return ___godot_icall_String(___mb.mb_get_y_label, (const Object *) this);
}

void AnimationNodeBlendSpace2D::remove_blend_point(const int64_t point) {
	___godot_icall_void_int(___mb.mb_remove_blend_point, (const Object *) this, point);
}

void AnimationNodeBlendSpace2D::remove_triangle(const int64_t triangle) {
	___godot_icall_void_int(___mb.mb_remove_triangle, (const Object *) this, triangle);
}

void AnimationNodeBlendSpace2D::set_auto_triangles(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_auto_triangles, (const Object *) this, enable);
}

void AnimationNodeBlendSpace2D::set_blend_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_blend_mode, (const Object *) this, mode);
}

void AnimationNodeBlendSpace2D::set_blend_point_node(const int64_t point, const Ref<AnimationRootNode> node) {
	___godot_icall_void_int_Object(___mb.mb_set_blend_point_node, (const Object *) this, point, node.ptr());
}

void AnimationNodeBlendSpace2D::set_blend_point_position(const int64_t point, const Vector2 pos) {
	___godot_icall_void_int_Vector2(___mb.mb_set_blend_point_position, (const Object *) this, point, pos);
}

void AnimationNodeBlendSpace2D::set_max_space(const Vector2 max_space) {
	___godot_icall_void_Vector2(___mb.mb_set_max_space, (const Object *) this, max_space);
}

void AnimationNodeBlendSpace2D::set_min_space(const Vector2 min_space) {
	___godot_icall_void_Vector2(___mb.mb_set_min_space, (const Object *) this, min_space);
}

void AnimationNodeBlendSpace2D::set_snap(const Vector2 snap) {
	___godot_icall_void_Vector2(___mb.mb_set_snap, (const Object *) this, snap);
}

void AnimationNodeBlendSpace2D::set_x_label(const String text) {
	___godot_icall_void_String(___mb.mb_set_x_label, (const Object *) this, text);
}

void AnimationNodeBlendSpace2D::set_y_label(const String text) {
	___godot_icall_void_String(___mb.mb_set_y_label, (const Object *) this, text);
}

}