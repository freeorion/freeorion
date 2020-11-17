#include "BitmapFont.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "BitmapFont.hpp"


namespace godot {


BitmapFont::___method_bindings BitmapFont::___mb = {};

void BitmapFont::___init_method_bindings() {
	___mb.mb__get_chars = godot::api->godot_method_bind_get_method("BitmapFont", "_get_chars");
	___mb.mb__get_kernings = godot::api->godot_method_bind_get_method("BitmapFont", "_get_kernings");
	___mb.mb__get_textures = godot::api->godot_method_bind_get_method("BitmapFont", "_get_textures");
	___mb.mb__set_chars = godot::api->godot_method_bind_get_method("BitmapFont", "_set_chars");
	___mb.mb__set_kernings = godot::api->godot_method_bind_get_method("BitmapFont", "_set_kernings");
	___mb.mb__set_textures = godot::api->godot_method_bind_get_method("BitmapFont", "_set_textures");
	___mb.mb_add_char = godot::api->godot_method_bind_get_method("BitmapFont", "add_char");
	___mb.mb_add_kerning_pair = godot::api->godot_method_bind_get_method("BitmapFont", "add_kerning_pair");
	___mb.mb_add_texture = godot::api->godot_method_bind_get_method("BitmapFont", "add_texture");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("BitmapFont", "clear");
	___mb.mb_create_from_fnt = godot::api->godot_method_bind_get_method("BitmapFont", "create_from_fnt");
	___mb.mb_get_char_size = godot::api->godot_method_bind_get_method("BitmapFont", "get_char_size");
	___mb.mb_get_fallback = godot::api->godot_method_bind_get_method("BitmapFont", "get_fallback");
	___mb.mb_get_kerning_pair = godot::api->godot_method_bind_get_method("BitmapFont", "get_kerning_pair");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("BitmapFont", "get_texture");
	___mb.mb_get_texture_count = godot::api->godot_method_bind_get_method("BitmapFont", "get_texture_count");
	___mb.mb_set_ascent = godot::api->godot_method_bind_get_method("BitmapFont", "set_ascent");
	___mb.mb_set_distance_field_hint = godot::api->godot_method_bind_get_method("BitmapFont", "set_distance_field_hint");
	___mb.mb_set_fallback = godot::api->godot_method_bind_get_method("BitmapFont", "set_fallback");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("BitmapFont", "set_height");
}

BitmapFont *BitmapFont::_new()
{
	return (BitmapFont *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"BitmapFont")());
}
PoolIntArray BitmapFont::_get_chars() const {
	return ___godot_icall_PoolIntArray(___mb.mb__get_chars, (const Object *) this);
}

PoolIntArray BitmapFont::_get_kernings() const {
	return ___godot_icall_PoolIntArray(___mb.mb__get_kernings, (const Object *) this);
}

Array BitmapFont::_get_textures() const {
	return ___godot_icall_Array(___mb.mb__get_textures, (const Object *) this);
}

void BitmapFont::_set_chars(const PoolIntArray arg0) {
	___godot_icall_void_PoolIntArray(___mb.mb__set_chars, (const Object *) this, arg0);
}

void BitmapFont::_set_kernings(const PoolIntArray arg0) {
	___godot_icall_void_PoolIntArray(___mb.mb__set_kernings, (const Object *) this, arg0);
}

void BitmapFont::_set_textures(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_textures, (const Object *) this, arg0);
}

void BitmapFont::add_char(const int64_t character, const int64_t texture, const Rect2 rect, const Vector2 align, const real_t advance) {
	___godot_icall_void_int_int_Rect2_Vector2_float(___mb.mb_add_char, (const Object *) this, character, texture, rect, align, advance);
}

void BitmapFont::add_kerning_pair(const int64_t char_a, const int64_t char_b, const int64_t kerning) {
	___godot_icall_void_int_int_int(___mb.mb_add_kerning_pair, (const Object *) this, char_a, char_b, kerning);
}

void BitmapFont::add_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_add_texture, (const Object *) this, texture.ptr());
}

void BitmapFont::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

Error BitmapFont::create_from_fnt(const String path) {
	return (Error) ___godot_icall_int_String(___mb.mb_create_from_fnt, (const Object *) this, path);
}

Vector2 BitmapFont::get_char_size(const int64_t _char, const int64_t next) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_get_char_size, (const Object *) this, _char, next);
}

Ref<BitmapFont> BitmapFont::get_fallback() const {
	return Ref<BitmapFont>::__internal_constructor(___godot_icall_Object(___mb.mb_get_fallback, (const Object *) this));
}

int64_t BitmapFont::get_kerning_pair(const int64_t char_a, const int64_t char_b) const {
	return ___godot_icall_int_int_int(___mb.mb_get_kerning_pair, (const Object *) this, char_a, char_b);
}

Ref<Texture> BitmapFont::get_texture(const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_texture, (const Object *) this, idx));
}

int64_t BitmapFont::get_texture_count() const {
	return ___godot_icall_int(___mb.mb_get_texture_count, (const Object *) this);
}

void BitmapFont::set_ascent(const real_t px) {
	___godot_icall_void_float(___mb.mb_set_ascent, (const Object *) this, px);
}

void BitmapFont::set_distance_field_hint(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_distance_field_hint, (const Object *) this, enable);
}

void BitmapFont::set_fallback(const Ref<BitmapFont> fallback) {
	___godot_icall_void_Object(___mb.mb_set_fallback, (const Object *) this, fallback.ptr());
}

void BitmapFont::set_height(const real_t px) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, px);
}

}