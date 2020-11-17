#include "TextureProgress.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


TextureProgress::___method_bindings TextureProgress::___mb = {};

void TextureProgress::___init_method_bindings() {
	___mb.mb_get_fill_degrees = godot::api->godot_method_bind_get_method("TextureProgress", "get_fill_degrees");
	___mb.mb_get_fill_mode = godot::api->godot_method_bind_get_method("TextureProgress", "get_fill_mode");
	___mb.mb_get_nine_patch_stretch = godot::api->godot_method_bind_get_method("TextureProgress", "get_nine_patch_stretch");
	___mb.mb_get_over_texture = godot::api->godot_method_bind_get_method("TextureProgress", "get_over_texture");
	___mb.mb_get_progress_texture = godot::api->godot_method_bind_get_method("TextureProgress", "get_progress_texture");
	___mb.mb_get_radial_center_offset = godot::api->godot_method_bind_get_method("TextureProgress", "get_radial_center_offset");
	___mb.mb_get_radial_initial_angle = godot::api->godot_method_bind_get_method("TextureProgress", "get_radial_initial_angle");
	___mb.mb_get_stretch_margin = godot::api->godot_method_bind_get_method("TextureProgress", "get_stretch_margin");
	___mb.mb_get_tint_over = godot::api->godot_method_bind_get_method("TextureProgress", "get_tint_over");
	___mb.mb_get_tint_progress = godot::api->godot_method_bind_get_method("TextureProgress", "get_tint_progress");
	___mb.mb_get_tint_under = godot::api->godot_method_bind_get_method("TextureProgress", "get_tint_under");
	___mb.mb_get_under_texture = godot::api->godot_method_bind_get_method("TextureProgress", "get_under_texture");
	___mb.mb_set_fill_degrees = godot::api->godot_method_bind_get_method("TextureProgress", "set_fill_degrees");
	___mb.mb_set_fill_mode = godot::api->godot_method_bind_get_method("TextureProgress", "set_fill_mode");
	___mb.mb_set_nine_patch_stretch = godot::api->godot_method_bind_get_method("TextureProgress", "set_nine_patch_stretch");
	___mb.mb_set_over_texture = godot::api->godot_method_bind_get_method("TextureProgress", "set_over_texture");
	___mb.mb_set_progress_texture = godot::api->godot_method_bind_get_method("TextureProgress", "set_progress_texture");
	___mb.mb_set_radial_center_offset = godot::api->godot_method_bind_get_method("TextureProgress", "set_radial_center_offset");
	___mb.mb_set_radial_initial_angle = godot::api->godot_method_bind_get_method("TextureProgress", "set_radial_initial_angle");
	___mb.mb_set_stretch_margin = godot::api->godot_method_bind_get_method("TextureProgress", "set_stretch_margin");
	___mb.mb_set_tint_over = godot::api->godot_method_bind_get_method("TextureProgress", "set_tint_over");
	___mb.mb_set_tint_progress = godot::api->godot_method_bind_get_method("TextureProgress", "set_tint_progress");
	___mb.mb_set_tint_under = godot::api->godot_method_bind_get_method("TextureProgress", "set_tint_under");
	___mb.mb_set_under_texture = godot::api->godot_method_bind_get_method("TextureProgress", "set_under_texture");
}

TextureProgress *TextureProgress::_new()
{
	return (TextureProgress *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TextureProgress")());
}
real_t TextureProgress::get_fill_degrees() {
	return ___godot_icall_float(___mb.mb_get_fill_degrees, (const Object *) this);
}

int64_t TextureProgress::get_fill_mode() {
	return ___godot_icall_int(___mb.mb_get_fill_mode, (const Object *) this);
}

bool TextureProgress::get_nine_patch_stretch() const {
	return ___godot_icall_bool(___mb.mb_get_nine_patch_stretch, (const Object *) this);
}

Ref<Texture> TextureProgress::get_over_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_over_texture, (const Object *) this));
}

Ref<Texture> TextureProgress::get_progress_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_progress_texture, (const Object *) this));
}

Vector2 TextureProgress::get_radial_center_offset() {
	return ___godot_icall_Vector2(___mb.mb_get_radial_center_offset, (const Object *) this);
}

real_t TextureProgress::get_radial_initial_angle() {
	return ___godot_icall_float(___mb.mb_get_radial_initial_angle, (const Object *) this);
}

int64_t TextureProgress::get_stretch_margin(const int64_t margin) const {
	return ___godot_icall_int_int(___mb.mb_get_stretch_margin, (const Object *) this, margin);
}

Color TextureProgress::get_tint_over() const {
	return ___godot_icall_Color(___mb.mb_get_tint_over, (const Object *) this);
}

Color TextureProgress::get_tint_progress() const {
	return ___godot_icall_Color(___mb.mb_get_tint_progress, (const Object *) this);
}

Color TextureProgress::get_tint_under() const {
	return ___godot_icall_Color(___mb.mb_get_tint_under, (const Object *) this);
}

Ref<Texture> TextureProgress::get_under_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_under_texture, (const Object *) this));
}

void TextureProgress::set_fill_degrees(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_fill_degrees, (const Object *) this, mode);
}

void TextureProgress::set_fill_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_fill_mode, (const Object *) this, mode);
}

void TextureProgress::set_nine_patch_stretch(const bool stretch) {
	___godot_icall_void_bool(___mb.mb_set_nine_patch_stretch, (const Object *) this, stretch);
}

void TextureProgress::set_over_texture(const Ref<Texture> tex) {
	___godot_icall_void_Object(___mb.mb_set_over_texture, (const Object *) this, tex.ptr());
}

void TextureProgress::set_progress_texture(const Ref<Texture> tex) {
	___godot_icall_void_Object(___mb.mb_set_progress_texture, (const Object *) this, tex.ptr());
}

void TextureProgress::set_radial_center_offset(const Vector2 mode) {
	___godot_icall_void_Vector2(___mb.mb_set_radial_center_offset, (const Object *) this, mode);
}

void TextureProgress::set_radial_initial_angle(const real_t mode) {
	___godot_icall_void_float(___mb.mb_set_radial_initial_angle, (const Object *) this, mode);
}

void TextureProgress::set_stretch_margin(const int64_t margin, const int64_t value) {
	___godot_icall_void_int_int(___mb.mb_set_stretch_margin, (const Object *) this, margin, value);
}

void TextureProgress::set_tint_over(const Color tint) {
	___godot_icall_void_Color(___mb.mb_set_tint_over, (const Object *) this, tint);
}

void TextureProgress::set_tint_progress(const Color tint) {
	___godot_icall_void_Color(___mb.mb_set_tint_progress, (const Object *) this, tint);
}

void TextureProgress::set_tint_under(const Color tint) {
	___godot_icall_void_Color(___mb.mb_set_tint_under, (const Object *) this, tint);
}

void TextureProgress::set_under_texture(const Ref<Texture> tex) {
	___godot_icall_void_Object(___mb.mb_set_under_texture, (const Object *) this, tex.ptr());
}

}