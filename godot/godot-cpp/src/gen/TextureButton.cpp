#include "TextureButton.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "BitMap.hpp"
#include "Texture.hpp"


namespace godot {


TextureButton::___method_bindings TextureButton::___mb = {};

void TextureButton::___init_method_bindings() {
	___mb.mb_get_click_mask = godot::api->godot_method_bind_get_method("TextureButton", "get_click_mask");
	___mb.mb_get_disabled_texture = godot::api->godot_method_bind_get_method("TextureButton", "get_disabled_texture");
	___mb.mb_get_expand = godot::api->godot_method_bind_get_method("TextureButton", "get_expand");
	___mb.mb_get_focused_texture = godot::api->godot_method_bind_get_method("TextureButton", "get_focused_texture");
	___mb.mb_get_hover_texture = godot::api->godot_method_bind_get_method("TextureButton", "get_hover_texture");
	___mb.mb_get_normal_texture = godot::api->godot_method_bind_get_method("TextureButton", "get_normal_texture");
	___mb.mb_get_pressed_texture = godot::api->godot_method_bind_get_method("TextureButton", "get_pressed_texture");
	___mb.mb_get_stretch_mode = godot::api->godot_method_bind_get_method("TextureButton", "get_stretch_mode");
	___mb.mb_set_click_mask = godot::api->godot_method_bind_get_method("TextureButton", "set_click_mask");
	___mb.mb_set_disabled_texture = godot::api->godot_method_bind_get_method("TextureButton", "set_disabled_texture");
	___mb.mb_set_expand = godot::api->godot_method_bind_get_method("TextureButton", "set_expand");
	___mb.mb_set_focused_texture = godot::api->godot_method_bind_get_method("TextureButton", "set_focused_texture");
	___mb.mb_set_hover_texture = godot::api->godot_method_bind_get_method("TextureButton", "set_hover_texture");
	___mb.mb_set_normal_texture = godot::api->godot_method_bind_get_method("TextureButton", "set_normal_texture");
	___mb.mb_set_pressed_texture = godot::api->godot_method_bind_get_method("TextureButton", "set_pressed_texture");
	___mb.mb_set_stretch_mode = godot::api->godot_method_bind_get_method("TextureButton", "set_stretch_mode");
}

TextureButton *TextureButton::_new()
{
	return (TextureButton *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextureButton")());
}
Ref<BitMap> TextureButton::get_click_mask() const {
	return Ref<BitMap>::__internal_constructor(___godot_icall_Object(___mb.mb_get_click_mask, (const Object *) this));
}

Ref<Texture> TextureButton::get_disabled_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_disabled_texture, (const Object *) this));
}

bool TextureButton::get_expand() const {
	return ___godot_icall_bool(___mb.mb_get_expand, (const Object *) this);
}

Ref<Texture> TextureButton::get_focused_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_focused_texture, (const Object *) this));
}

Ref<Texture> TextureButton::get_hover_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_hover_texture, (const Object *) this));
}

Ref<Texture> TextureButton::get_normal_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_texture, (const Object *) this));
}

Ref<Texture> TextureButton::get_pressed_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_pressed_texture, (const Object *) this));
}

TextureButton::StretchMode TextureButton::get_stretch_mode() const {
	return (TextureButton::StretchMode) ___godot_icall_int(___mb.mb_get_stretch_mode, (const Object *) this);
}

void TextureButton::set_click_mask(const Ref<BitMap> mask) {
	___godot_icall_void_Object(___mb.mb_set_click_mask, (const Object *) this, mask.ptr());
}

void TextureButton::set_disabled_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_disabled_texture, (const Object *) this, texture.ptr());
}

void TextureButton::set_expand(const bool p_expand) {
	___godot_icall_void_bool(___mb.mb_set_expand, (const Object *) this, p_expand);
}

void TextureButton::set_focused_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_focused_texture, (const Object *) this, texture.ptr());
}

void TextureButton::set_hover_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_hover_texture, (const Object *) this, texture.ptr());
}

void TextureButton::set_normal_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_normal_texture, (const Object *) this, texture.ptr());
}

void TextureButton::set_pressed_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_pressed_texture, (const Object *) this, texture.ptr());
}

void TextureButton::set_stretch_mode(const int64_t p_mode) {
	___godot_icall_void_int(___mb.mb_set_stretch_mode, (const Object *) this, p_mode);
}

}