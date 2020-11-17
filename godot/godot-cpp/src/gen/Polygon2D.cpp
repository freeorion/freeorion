#include "Polygon2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


Polygon2D::___method_bindings Polygon2D::___mb = {};

void Polygon2D::___init_method_bindings() {
	___mb.mb__get_bones = godot::api->godot_method_bind_get_method("Polygon2D", "_get_bones");
	___mb.mb__set_bones = godot::api->godot_method_bind_get_method("Polygon2D", "_set_bones");
	___mb.mb__skeleton_bone_setup_changed = godot::api->godot_method_bind_get_method("Polygon2D", "_skeleton_bone_setup_changed");
	___mb.mb_add_bone = godot::api->godot_method_bind_get_method("Polygon2D", "add_bone");
	___mb.mb_clear_bones = godot::api->godot_method_bind_get_method("Polygon2D", "clear_bones");
	___mb.mb_erase_bone = godot::api->godot_method_bind_get_method("Polygon2D", "erase_bone");
	___mb.mb_get_antialiased = godot::api->godot_method_bind_get_method("Polygon2D", "get_antialiased");
	___mb.mb_get_bone_count = godot::api->godot_method_bind_get_method("Polygon2D", "get_bone_count");
	___mb.mb_get_bone_path = godot::api->godot_method_bind_get_method("Polygon2D", "get_bone_path");
	___mb.mb_get_bone_weights = godot::api->godot_method_bind_get_method("Polygon2D", "get_bone_weights");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("Polygon2D", "get_color");
	___mb.mb_get_internal_vertex_count = godot::api->godot_method_bind_get_method("Polygon2D", "get_internal_vertex_count");
	___mb.mb_get_invert = godot::api->godot_method_bind_get_method("Polygon2D", "get_invert");
	___mb.mb_get_invert_border = godot::api->godot_method_bind_get_method("Polygon2D", "get_invert_border");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("Polygon2D", "get_offset");
	___mb.mb_get_polygon = godot::api->godot_method_bind_get_method("Polygon2D", "get_polygon");
	___mb.mb_get_polygons = godot::api->godot_method_bind_get_method("Polygon2D", "get_polygons");
	___mb.mb_get_skeleton = godot::api->godot_method_bind_get_method("Polygon2D", "get_skeleton");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Polygon2D", "get_texture");
	___mb.mb_get_texture_offset = godot::api->godot_method_bind_get_method("Polygon2D", "get_texture_offset");
	___mb.mb_get_texture_rotation = godot::api->godot_method_bind_get_method("Polygon2D", "get_texture_rotation");
	___mb.mb_get_texture_rotation_degrees = godot::api->godot_method_bind_get_method("Polygon2D", "get_texture_rotation_degrees");
	___mb.mb_get_texture_scale = godot::api->godot_method_bind_get_method("Polygon2D", "get_texture_scale");
	___mb.mb_get_uv = godot::api->godot_method_bind_get_method("Polygon2D", "get_uv");
	___mb.mb_get_vertex_colors = godot::api->godot_method_bind_get_method("Polygon2D", "get_vertex_colors");
	___mb.mb_set_antialiased = godot::api->godot_method_bind_get_method("Polygon2D", "set_antialiased");
	___mb.mb_set_bone_path = godot::api->godot_method_bind_get_method("Polygon2D", "set_bone_path");
	___mb.mb_set_bone_weights = godot::api->godot_method_bind_get_method("Polygon2D", "set_bone_weights");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("Polygon2D", "set_color");
	___mb.mb_set_internal_vertex_count = godot::api->godot_method_bind_get_method("Polygon2D", "set_internal_vertex_count");
	___mb.mb_set_invert = godot::api->godot_method_bind_get_method("Polygon2D", "set_invert");
	___mb.mb_set_invert_border = godot::api->godot_method_bind_get_method("Polygon2D", "set_invert_border");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("Polygon2D", "set_offset");
	___mb.mb_set_polygon = godot::api->godot_method_bind_get_method("Polygon2D", "set_polygon");
	___mb.mb_set_polygons = godot::api->godot_method_bind_get_method("Polygon2D", "set_polygons");
	___mb.mb_set_skeleton = godot::api->godot_method_bind_get_method("Polygon2D", "set_skeleton");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Polygon2D", "set_texture");
	___mb.mb_set_texture_offset = godot::api->godot_method_bind_get_method("Polygon2D", "set_texture_offset");
	___mb.mb_set_texture_rotation = godot::api->godot_method_bind_get_method("Polygon2D", "set_texture_rotation");
	___mb.mb_set_texture_rotation_degrees = godot::api->godot_method_bind_get_method("Polygon2D", "set_texture_rotation_degrees");
	___mb.mb_set_texture_scale = godot::api->godot_method_bind_get_method("Polygon2D", "set_texture_scale");
	___mb.mb_set_uv = godot::api->godot_method_bind_get_method("Polygon2D", "set_uv");
	___mb.mb_set_vertex_colors = godot::api->godot_method_bind_get_method("Polygon2D", "set_vertex_colors");
}

Polygon2D *Polygon2D::_new()
{
	return (Polygon2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Polygon2D")());
}
Array Polygon2D::_get_bones() const {
	return ___godot_icall_Array(___mb.mb__get_bones, (const Object *) this);
}

void Polygon2D::_set_bones(const Array bones) {
	___godot_icall_void_Array(___mb.mb__set_bones, (const Object *) this, bones);
}

void Polygon2D::_skeleton_bone_setup_changed() {
	___godot_icall_void(___mb.mb__skeleton_bone_setup_changed, (const Object *) this);
}

void Polygon2D::add_bone(const NodePath path, const PoolRealArray weights) {
	___godot_icall_void_NodePath_PoolRealArray(___mb.mb_add_bone, (const Object *) this, path, weights);
}

void Polygon2D::clear_bones() {
	___godot_icall_void(___mb.mb_clear_bones, (const Object *) this);
}

void Polygon2D::erase_bone(const int64_t index) {
	___godot_icall_void_int(___mb.mb_erase_bone, (const Object *) this, index);
}

bool Polygon2D::get_antialiased() const {
	return ___godot_icall_bool(___mb.mb_get_antialiased, (const Object *) this);
}

int64_t Polygon2D::get_bone_count() const {
	return ___godot_icall_int(___mb.mb_get_bone_count, (const Object *) this);
}

NodePath Polygon2D::get_bone_path(const int64_t index) const {
	return ___godot_icall_NodePath_int(___mb.mb_get_bone_path, (const Object *) this, index);
}

PoolRealArray Polygon2D::get_bone_weights(const int64_t index) const {
	return ___godot_icall_PoolRealArray_int(___mb.mb_get_bone_weights, (const Object *) this, index);
}

Color Polygon2D::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

int64_t Polygon2D::get_internal_vertex_count() const {
	return ___godot_icall_int(___mb.mb_get_internal_vertex_count, (const Object *) this);
}

bool Polygon2D::get_invert() const {
	return ___godot_icall_bool(___mb.mb_get_invert, (const Object *) this);
}

real_t Polygon2D::get_invert_border() const {
	return ___godot_icall_float(___mb.mb_get_invert_border, (const Object *) this);
}

Vector2 Polygon2D::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

PoolVector2Array Polygon2D::get_polygon() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_polygon, (const Object *) this);
}

Array Polygon2D::get_polygons() const {
	return ___godot_icall_Array(___mb.mb_get_polygons, (const Object *) this);
}

NodePath Polygon2D::get_skeleton() const {
	return ___godot_icall_NodePath(___mb.mb_get_skeleton, (const Object *) this);
}

Ref<Texture> Polygon2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

Vector2 Polygon2D::get_texture_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_texture_offset, (const Object *) this);
}

real_t Polygon2D::get_texture_rotation() const {
	return ___godot_icall_float(___mb.mb_get_texture_rotation, (const Object *) this);
}

real_t Polygon2D::get_texture_rotation_degrees() const {
	return ___godot_icall_float(___mb.mb_get_texture_rotation_degrees, (const Object *) this);
}

Vector2 Polygon2D::get_texture_scale() const {
	return ___godot_icall_Vector2(___mb.mb_get_texture_scale, (const Object *) this);
}

PoolVector2Array Polygon2D::get_uv() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_uv, (const Object *) this);
}

PoolColorArray Polygon2D::get_vertex_colors() const {
	return ___godot_icall_PoolColorArray(___mb.mb_get_vertex_colors, (const Object *) this);
}

void Polygon2D::set_antialiased(const bool antialiased) {
	___godot_icall_void_bool(___mb.mb_set_antialiased, (const Object *) this, antialiased);
}

void Polygon2D::set_bone_path(const int64_t index, const NodePath path) {
	___godot_icall_void_int_NodePath(___mb.mb_set_bone_path, (const Object *) this, index, path);
}

void Polygon2D::set_bone_weights(const int64_t index, const PoolRealArray weights) {
	___godot_icall_void_int_PoolRealArray(___mb.mb_set_bone_weights, (const Object *) this, index, weights);
}

void Polygon2D::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void Polygon2D::set_internal_vertex_count(const int64_t internal_vertex_count) {
	___godot_icall_void_int(___mb.mb_set_internal_vertex_count, (const Object *) this, internal_vertex_count);
}

void Polygon2D::set_invert(const bool invert) {
	___godot_icall_void_bool(___mb.mb_set_invert, (const Object *) this, invert);
}

void Polygon2D::set_invert_border(const real_t invert_border) {
	___godot_icall_void_float(___mb.mb_set_invert_border, (const Object *) this, invert_border);
}

void Polygon2D::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void Polygon2D::set_polygon(const PoolVector2Array polygon) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_polygon, (const Object *) this, polygon);
}

void Polygon2D::set_polygons(const Array polygons) {
	___godot_icall_void_Array(___mb.mb_set_polygons, (const Object *) this, polygons);
}

void Polygon2D::set_skeleton(const NodePath skeleton) {
	___godot_icall_void_NodePath(___mb.mb_set_skeleton, (const Object *) this, skeleton);
}

void Polygon2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Polygon2D::set_texture_offset(const Vector2 texture_offset) {
	___godot_icall_void_Vector2(___mb.mb_set_texture_offset, (const Object *) this, texture_offset);
}

void Polygon2D::set_texture_rotation(const real_t texture_rotation) {
	___godot_icall_void_float(___mb.mb_set_texture_rotation, (const Object *) this, texture_rotation);
}

void Polygon2D::set_texture_rotation_degrees(const real_t texture_rotation) {
	___godot_icall_void_float(___mb.mb_set_texture_rotation_degrees, (const Object *) this, texture_rotation);
}

void Polygon2D::set_texture_scale(const Vector2 texture_scale) {
	___godot_icall_void_Vector2(___mb.mb_set_texture_scale, (const Object *) this, texture_scale);
}

void Polygon2D::set_uv(const PoolVector2Array uv) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_uv, (const Object *) this, uv);
}

void Polygon2D::set_vertex_colors(const PoolColorArray vertex_colors) {
	___godot_icall_void_PoolColorArray(___mb.mb_set_vertex_colors, (const Object *) this, vertex_colors);
}

}