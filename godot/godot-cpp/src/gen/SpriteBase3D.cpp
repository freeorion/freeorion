#include "SpriteBase3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "TriangleMesh.hpp"


namespace godot {


SpriteBase3D::___method_bindings SpriteBase3D::___mb = {};

void SpriteBase3D::___init_method_bindings() {
	___mb.mb__im_update = godot::api->godot_method_bind_get_method("SpriteBase3D", "_im_update");
	___mb.mb__queue_update = godot::api->godot_method_bind_get_method("SpriteBase3D", "_queue_update");
	___mb.mb_generate_triangle_mesh = godot::api->godot_method_bind_get_method("SpriteBase3D", "generate_triangle_mesh");
	___mb.mb_get_alpha_cut_mode = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_alpha_cut_mode");
	___mb.mb_get_axis = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_axis");
	___mb.mb_get_billboard_mode = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_billboard_mode");
	___mb.mb_get_draw_flag = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_draw_flag");
	___mb.mb_get_item_rect = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_item_rect");
	___mb.mb_get_modulate = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_modulate");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_offset");
	___mb.mb_get_opacity = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_opacity");
	___mb.mb_get_pixel_size = godot::api->godot_method_bind_get_method("SpriteBase3D", "get_pixel_size");
	___mb.mb_is_centered = godot::api->godot_method_bind_get_method("SpriteBase3D", "is_centered");
	___mb.mb_is_flipped_h = godot::api->godot_method_bind_get_method("SpriteBase3D", "is_flipped_h");
	___mb.mb_is_flipped_v = godot::api->godot_method_bind_get_method("SpriteBase3D", "is_flipped_v");
	___mb.mb_set_alpha_cut_mode = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_alpha_cut_mode");
	___mb.mb_set_axis = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_axis");
	___mb.mb_set_billboard_mode = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_billboard_mode");
	___mb.mb_set_centered = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_centered");
	___mb.mb_set_draw_flag = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_draw_flag");
	___mb.mb_set_flip_h = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_flip_h");
	___mb.mb_set_flip_v = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_flip_v");
	___mb.mb_set_modulate = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_modulate");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_offset");
	___mb.mb_set_opacity = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_opacity");
	___mb.mb_set_pixel_size = godot::api->godot_method_bind_get_method("SpriteBase3D", "set_pixel_size");
}

void SpriteBase3D::_im_update() {
	___godot_icall_void(___mb.mb__im_update, (const Object *) this);
}

void SpriteBase3D::_queue_update() {
	___godot_icall_void(___mb.mb__queue_update, (const Object *) this);
}

Ref<TriangleMesh> SpriteBase3D::generate_triangle_mesh() const {
	return Ref<TriangleMesh>::__internal_constructor(___godot_icall_Object(___mb.mb_generate_triangle_mesh, (const Object *) this));
}

SpriteBase3D::AlphaCutMode SpriteBase3D::get_alpha_cut_mode() const {
	return (SpriteBase3D::AlphaCutMode) ___godot_icall_int(___mb.mb_get_alpha_cut_mode, (const Object *) this);
}

Vector3::Axis SpriteBase3D::get_axis() const {
	return (Vector3::Axis) ___godot_icall_int(___mb.mb_get_axis, (const Object *) this);
}

SpatialMaterial::BillboardMode SpriteBase3D::get_billboard_mode() const {
	return (SpatialMaterial::BillboardMode) ___godot_icall_int(___mb.mb_get_billboard_mode, (const Object *) this);
}

bool SpriteBase3D::get_draw_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_draw_flag, (const Object *) this, flag);
}

Rect2 SpriteBase3D::get_item_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_item_rect, (const Object *) this);
}

Color SpriteBase3D::get_modulate() const {
	return ___godot_icall_Color(___mb.mb_get_modulate, (const Object *) this);
}

Vector2 SpriteBase3D::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

real_t SpriteBase3D::get_opacity() const {
	return ___godot_icall_float(___mb.mb_get_opacity, (const Object *) this);
}

real_t SpriteBase3D::get_pixel_size() const {
	return ___godot_icall_float(___mb.mb_get_pixel_size, (const Object *) this);
}

bool SpriteBase3D::is_centered() const {
	return ___godot_icall_bool(___mb.mb_is_centered, (const Object *) this);
}

bool SpriteBase3D::is_flipped_h() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_h, (const Object *) this);
}

bool SpriteBase3D::is_flipped_v() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_v, (const Object *) this);
}

void SpriteBase3D::set_alpha_cut_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_alpha_cut_mode, (const Object *) this, mode);
}

void SpriteBase3D::set_axis(const int64_t axis) {
	___godot_icall_void_int(___mb.mb_set_axis, (const Object *) this, axis);
}

void SpriteBase3D::set_billboard_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_billboard_mode, (const Object *) this, mode);
}

void SpriteBase3D::set_centered(const bool centered) {
	___godot_icall_void_bool(___mb.mb_set_centered, (const Object *) this, centered);
}

void SpriteBase3D::set_draw_flag(const int64_t flag, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_set_draw_flag, (const Object *) this, flag, enabled);
}

void SpriteBase3D::set_flip_h(const bool flip_h) {
	___godot_icall_void_bool(___mb.mb_set_flip_h, (const Object *) this, flip_h);
}

void SpriteBase3D::set_flip_v(const bool flip_v) {
	___godot_icall_void_bool(___mb.mb_set_flip_v, (const Object *) this, flip_v);
}

void SpriteBase3D::set_modulate(const Color modulate) {
	___godot_icall_void_Color(___mb.mb_set_modulate, (const Object *) this, modulate);
}

void SpriteBase3D::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void SpriteBase3D::set_opacity(const real_t opacity) {
	___godot_icall_void_float(___mb.mb_set_opacity, (const Object *) this, opacity);
}

void SpriteBase3D::set_pixel_size(const real_t pixel_size) {
	___godot_icall_void_float(___mb.mb_set_pixel_size, (const Object *) this, pixel_size);
}

}