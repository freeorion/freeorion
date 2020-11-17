#include "TextureRect.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


TextureRect::___method_bindings TextureRect::___mb = {};

void TextureRect::___init_method_bindings() {
	___mb.mb__texture_changed = godot::api->godot_method_bind_get_method("TextureRect", "_texture_changed");
	___mb.mb_get_stretch_mode = godot::api->godot_method_bind_get_method("TextureRect", "get_stretch_mode");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("TextureRect", "get_texture");
	___mb.mb_has_expand = godot::api->godot_method_bind_get_method("TextureRect", "has_expand");
	___mb.mb_is_flipped_h = godot::api->godot_method_bind_get_method("TextureRect", "is_flipped_h");
	___mb.mb_is_flipped_v = godot::api->godot_method_bind_get_method("TextureRect", "is_flipped_v");
	___mb.mb_set_expand = godot::api->godot_method_bind_get_method("TextureRect", "set_expand");
	___mb.mb_set_flip_h = godot::api->godot_method_bind_get_method("TextureRect", "set_flip_h");
	___mb.mb_set_flip_v = godot::api->godot_method_bind_get_method("TextureRect", "set_flip_v");
	___mb.mb_set_stretch_mode = godot::api->godot_method_bind_get_method("TextureRect", "set_stretch_mode");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("TextureRect", "set_texture");
}

TextureRect *TextureRect::_new()
{
	return (TextureRect *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextureRect")());
}
void TextureRect::_texture_changed() {
	___godot_icall_void(___mb.mb__texture_changed, (const Object *) this);
}

TextureRect::StretchMode TextureRect::get_stretch_mode() const {
	return (TextureRect::StretchMode) ___godot_icall_int(___mb.mb_get_stretch_mode, (const Object *) this);
}

Ref<Texture> TextureRect::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

bool TextureRect::has_expand() const {
	return ___godot_icall_bool(___mb.mb_has_expand, (const Object *) this);
}

bool TextureRect::is_flipped_h() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_h, (const Object *) this);
}

bool TextureRect::is_flipped_v() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_v, (const Object *) this);
}

void TextureRect::set_expand(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_expand, (const Object *) this, enable);
}

void TextureRect::set_flip_h(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_flip_h, (const Object *) this, enable);
}

void TextureRect::set_flip_v(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_flip_v, (const Object *) this, enable);
}

void TextureRect::set_stretch_mode(const int64_t stretch_mode) {
	___godot_icall_void_int(___mb.mb_set_stretch_mode, (const Object *) this, stretch_mode);
}

void TextureRect::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

}