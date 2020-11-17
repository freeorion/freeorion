#include "Sprite.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


Sprite::___method_bindings Sprite::___mb = {};

void Sprite::___init_method_bindings() {
	___mb.mb__texture_changed = godot::api->godot_method_bind_get_method("Sprite", "_texture_changed");
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("Sprite", "get_frame");
	___mb.mb_get_frame_coords = godot::api->godot_method_bind_get_method("Sprite", "get_frame_coords");
	___mb.mb_get_hframes = godot::api->godot_method_bind_get_method("Sprite", "get_hframes");
	___mb.mb_get_normal_map = godot::api->godot_method_bind_get_method("Sprite", "get_normal_map");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("Sprite", "get_offset");
	___mb.mb_get_rect = godot::api->godot_method_bind_get_method("Sprite", "get_rect");
	___mb.mb_get_region_rect = godot::api->godot_method_bind_get_method("Sprite", "get_region_rect");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Sprite", "get_texture");
	___mb.mb_get_vframes = godot::api->godot_method_bind_get_method("Sprite", "get_vframes");
	___mb.mb_is_centered = godot::api->godot_method_bind_get_method("Sprite", "is_centered");
	___mb.mb_is_flipped_h = godot::api->godot_method_bind_get_method("Sprite", "is_flipped_h");
	___mb.mb_is_flipped_v = godot::api->godot_method_bind_get_method("Sprite", "is_flipped_v");
	___mb.mb_is_pixel_opaque = godot::api->godot_method_bind_get_method("Sprite", "is_pixel_opaque");
	___mb.mb_is_region = godot::api->godot_method_bind_get_method("Sprite", "is_region");
	___mb.mb_is_region_filter_clip_enabled = godot::api->godot_method_bind_get_method("Sprite", "is_region_filter_clip_enabled");
	___mb.mb_set_centered = godot::api->godot_method_bind_get_method("Sprite", "set_centered");
	___mb.mb_set_flip_h = godot::api->godot_method_bind_get_method("Sprite", "set_flip_h");
	___mb.mb_set_flip_v = godot::api->godot_method_bind_get_method("Sprite", "set_flip_v");
	___mb.mb_set_frame = godot::api->godot_method_bind_get_method("Sprite", "set_frame");
	___mb.mb_set_frame_coords = godot::api->godot_method_bind_get_method("Sprite", "set_frame_coords");
	___mb.mb_set_hframes = godot::api->godot_method_bind_get_method("Sprite", "set_hframes");
	___mb.mb_set_normal_map = godot::api->godot_method_bind_get_method("Sprite", "set_normal_map");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("Sprite", "set_offset");
	___mb.mb_set_region = godot::api->godot_method_bind_get_method("Sprite", "set_region");
	___mb.mb_set_region_filter_clip = godot::api->godot_method_bind_get_method("Sprite", "set_region_filter_clip");
	___mb.mb_set_region_rect = godot::api->godot_method_bind_get_method("Sprite", "set_region_rect");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Sprite", "set_texture");
	___mb.mb_set_vframes = godot::api->godot_method_bind_get_method("Sprite", "set_vframes");
}

Sprite *Sprite::_new()
{
	return (Sprite *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Sprite")());
}
void Sprite::_texture_changed() {
	___godot_icall_void(___mb.mb__texture_changed, (const Object *) this);
}

int64_t Sprite::get_frame() const {
	return ___godot_icall_int(___mb.mb_get_frame, (const Object *) this);
}

Vector2 Sprite::get_frame_coords() const {
	return ___godot_icall_Vector2(___mb.mb_get_frame_coords, (const Object *) this);
}

int64_t Sprite::get_hframes() const {
	return ___godot_icall_int(___mb.mb_get_hframes, (const Object *) this);
}

Ref<Texture> Sprite::get_normal_map() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_map, (const Object *) this));
}

Vector2 Sprite::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

Rect2 Sprite::get_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_rect, (const Object *) this);
}

Rect2 Sprite::get_region_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_region_rect, (const Object *) this);
}

Ref<Texture> Sprite::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

int64_t Sprite::get_vframes() const {
	return ___godot_icall_int(___mb.mb_get_vframes, (const Object *) this);
}

bool Sprite::is_centered() const {
	return ___godot_icall_bool(___mb.mb_is_centered, (const Object *) this);
}

bool Sprite::is_flipped_h() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_h, (const Object *) this);
}

bool Sprite::is_flipped_v() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_v, (const Object *) this);
}

bool Sprite::is_pixel_opaque(const Vector2 pos) const {
	return ___godot_icall_bool_Vector2(___mb.mb_is_pixel_opaque, (const Object *) this, pos);
}

bool Sprite::is_region() const {
	return ___godot_icall_bool(___mb.mb_is_region, (const Object *) this);
}

bool Sprite::is_region_filter_clip_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_region_filter_clip_enabled, (const Object *) this);
}

void Sprite::set_centered(const bool centered) {
	___godot_icall_void_bool(___mb.mb_set_centered, (const Object *) this, centered);
}

void Sprite::set_flip_h(const bool flip_h) {
	___godot_icall_void_bool(___mb.mb_set_flip_h, (const Object *) this, flip_h);
}

void Sprite::set_flip_v(const bool flip_v) {
	___godot_icall_void_bool(___mb.mb_set_flip_v, (const Object *) this, flip_v);
}

void Sprite::set_frame(const int64_t frame) {
	___godot_icall_void_int(___mb.mb_set_frame, (const Object *) this, frame);
}

void Sprite::set_frame_coords(const Vector2 coords) {
	___godot_icall_void_Vector2(___mb.mb_set_frame_coords, (const Object *) this, coords);
}

void Sprite::set_hframes(const int64_t hframes) {
	___godot_icall_void_int(___mb.mb_set_hframes, (const Object *) this, hframes);
}

void Sprite::set_normal_map(const Ref<Texture> normal_map) {
	___godot_icall_void_Object(___mb.mb_set_normal_map, (const Object *) this, normal_map.ptr());
}

void Sprite::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void Sprite::set_region(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_region, (const Object *) this, enabled);
}

void Sprite::set_region_filter_clip(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_region_filter_clip, (const Object *) this, enabled);
}

void Sprite::set_region_rect(const Rect2 rect) {
	___godot_icall_void_Rect2(___mb.mb_set_region_rect, (const Object *) this, rect);
}

void Sprite::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Sprite::set_vframes(const int64_t vframes) {
	___godot_icall_void_int(___mb.mb_set_vframes, (const Object *) this, vframes);
}

}