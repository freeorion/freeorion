#include "LargeTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


LargeTexture::___method_bindings LargeTexture::___mb = {};

void LargeTexture::___init_method_bindings() {
	___mb.mb__get_data = godot::api->godot_method_bind_get_method("LargeTexture", "_get_data");
	___mb.mb__set_data = godot::api->godot_method_bind_get_method("LargeTexture", "_set_data");
	___mb.mb_add_piece = godot::api->godot_method_bind_get_method("LargeTexture", "add_piece");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("LargeTexture", "clear");
	___mb.mb_get_piece_count = godot::api->godot_method_bind_get_method("LargeTexture", "get_piece_count");
	___mb.mb_get_piece_offset = godot::api->godot_method_bind_get_method("LargeTexture", "get_piece_offset");
	___mb.mb_get_piece_texture = godot::api->godot_method_bind_get_method("LargeTexture", "get_piece_texture");
	___mb.mb_set_piece_offset = godot::api->godot_method_bind_get_method("LargeTexture", "set_piece_offset");
	___mb.mb_set_piece_texture = godot::api->godot_method_bind_get_method("LargeTexture", "set_piece_texture");
	___mb.mb_set_size = godot::api->godot_method_bind_get_method("LargeTexture", "set_size");
}

LargeTexture *LargeTexture::_new()
{
	return (LargeTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"LargeTexture")());
}
Array LargeTexture::_get_data() const {
	return ___godot_icall_Array(___mb.mb__get_data, (const Object *) this);
}

void LargeTexture::_set_data(const Array data) {
	___godot_icall_void_Array(___mb.mb__set_data, (const Object *) this, data);
}

int64_t LargeTexture::add_piece(const Vector2 ofs, const Ref<Texture> texture) {
	return ___godot_icall_int_Vector2_Object(___mb.mb_add_piece, (const Object *) this, ofs, texture.ptr());
}

void LargeTexture::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

int64_t LargeTexture::get_piece_count() const {
	return ___godot_icall_int(___mb.mb_get_piece_count, (const Object *) this);
}

Vector2 LargeTexture::get_piece_offset(const int64_t idx) const {
	return ___godot_icall_Vector2_int(___mb.mb_get_piece_offset, (const Object *) this, idx);
}

Ref<Texture> LargeTexture::get_piece_texture(const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_piece_texture, (const Object *) this, idx));
}

void LargeTexture::set_piece_offset(const int64_t idx, const Vector2 ofs) {
	___godot_icall_void_int_Vector2(___mb.mb_set_piece_offset, (const Object *) this, idx, ofs);
}

void LargeTexture::set_piece_texture(const int64_t idx, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_piece_texture, (const Object *) this, idx, texture.ptr());
}

void LargeTexture::set_size(const Vector2 size) {
	___godot_icall_void_Vector2(___mb.mb_set_size, (const Object *) this, size);
}

}