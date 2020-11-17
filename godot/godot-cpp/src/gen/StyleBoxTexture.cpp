#include "StyleBoxTexture.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


StyleBoxTexture::___method_bindings StyleBoxTexture::___mb = {};

void StyleBoxTexture::___init_method_bindings() {
	___mb.mb_get_expand_margin_size = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_expand_margin_size");
	___mb.mb_get_h_axis_stretch_mode = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_h_axis_stretch_mode");
	___mb.mb_get_margin_size = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_margin_size");
	___mb.mb_get_modulate = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_modulate");
	___mb.mb_get_normal_map = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_normal_map");
	___mb.mb_get_region_rect = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_region_rect");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_texture");
	___mb.mb_get_v_axis_stretch_mode = godot::api->godot_method_bind_get_method("StyleBoxTexture", "get_v_axis_stretch_mode");
	___mb.mb_is_draw_center_enabled = godot::api->godot_method_bind_get_method("StyleBoxTexture", "is_draw_center_enabled");
	___mb.mb_set_draw_center = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_draw_center");
	___mb.mb_set_expand_margin_all = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_expand_margin_all");
	___mb.mb_set_expand_margin_individual = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_expand_margin_individual");
	___mb.mb_set_expand_margin_size = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_expand_margin_size");
	___mb.mb_set_h_axis_stretch_mode = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_h_axis_stretch_mode");
	___mb.mb_set_margin_size = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_margin_size");
	___mb.mb_set_modulate = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_modulate");
	___mb.mb_set_normal_map = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_normal_map");
	___mb.mb_set_region_rect = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_region_rect");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_texture");
	___mb.mb_set_v_axis_stretch_mode = godot::api->godot_method_bind_get_method("StyleBoxTexture", "set_v_axis_stretch_mode");
}

StyleBoxTexture *StyleBoxTexture::_new()
{
	return (StyleBoxTexture *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StyleBoxTexture")());
}
real_t StyleBoxTexture::get_expand_margin_size(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_expand_margin_size, (const Object *) this, margin);
}

StyleBoxTexture::AxisStretchMode StyleBoxTexture::get_h_axis_stretch_mode() const {
	return (StyleBoxTexture::AxisStretchMode) ___godot_icall_int(___mb.mb_get_h_axis_stretch_mode, (const Object *) this);
}

real_t StyleBoxTexture::get_margin_size(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_margin_size, (const Object *) this, margin);
}

Color StyleBoxTexture::get_modulate() const {
	return ___godot_icall_Color(___mb.mb_get_modulate, (const Object *) this);
}

Ref<Texture> StyleBoxTexture::get_normal_map() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_map, (const Object *) this));
}

Rect2 StyleBoxTexture::get_region_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_region_rect, (const Object *) this);
}

Ref<Texture> StyleBoxTexture::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

StyleBoxTexture::AxisStretchMode StyleBoxTexture::get_v_axis_stretch_mode() const {
	return (StyleBoxTexture::AxisStretchMode) ___godot_icall_int(___mb.mb_get_v_axis_stretch_mode, (const Object *) this);
}

bool StyleBoxTexture::is_draw_center_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_draw_center_enabled, (const Object *) this);
}

void StyleBoxTexture::set_draw_center(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_draw_center, (const Object *) this, enable);
}

void StyleBoxTexture::set_expand_margin_all(const real_t size) {
	___godot_icall_void_float(___mb.mb_set_expand_margin_all, (const Object *) this, size);
}

void StyleBoxTexture::set_expand_margin_individual(const real_t size_left, const real_t size_top, const real_t size_right, const real_t size_bottom) {
	___godot_icall_void_float_float_float_float(___mb.mb_set_expand_margin_individual, (const Object *) this, size_left, size_top, size_right, size_bottom);
}

void StyleBoxTexture::set_expand_margin_size(const int64_t margin, const real_t size) {
	___godot_icall_void_int_float(___mb.mb_set_expand_margin_size, (const Object *) this, margin, size);
}

void StyleBoxTexture::set_h_axis_stretch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_h_axis_stretch_mode, (const Object *) this, mode);
}

void StyleBoxTexture::set_margin_size(const int64_t margin, const real_t size) {
	___godot_icall_void_int_float(___mb.mb_set_margin_size, (const Object *) this, margin, size);
}

void StyleBoxTexture::set_modulate(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_modulate, (const Object *) this, color);
}

void StyleBoxTexture::set_normal_map(const Ref<Texture> normal_map) {
	___godot_icall_void_Object(___mb.mb_set_normal_map, (const Object *) this, normal_map.ptr());
}

void StyleBoxTexture::set_region_rect(const Rect2 region) {
	___godot_icall_void_Rect2(___mb.mb_set_region_rect, (const Object *) this, region);
}

void StyleBoxTexture::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void StyleBoxTexture::set_v_axis_stretch_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_v_axis_stretch_mode, (const Object *) this, mode);
}

}