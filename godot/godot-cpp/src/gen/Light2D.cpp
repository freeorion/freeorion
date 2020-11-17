#include "Light2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


Light2D::___method_bindings Light2D::___mb = {};

void Light2D::___init_method_bindings() {
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("Light2D", "get_color");
	___mb.mb_get_energy = godot::api->godot_method_bind_get_method("Light2D", "get_energy");
	___mb.mb_get_height = godot::api->godot_method_bind_get_method("Light2D", "get_height");
	___mb.mb_get_item_cull_mask = godot::api->godot_method_bind_get_method("Light2D", "get_item_cull_mask");
	___mb.mb_get_item_shadow_cull_mask = godot::api->godot_method_bind_get_method("Light2D", "get_item_shadow_cull_mask");
	___mb.mb_get_layer_range_max = godot::api->godot_method_bind_get_method("Light2D", "get_layer_range_max");
	___mb.mb_get_layer_range_min = godot::api->godot_method_bind_get_method("Light2D", "get_layer_range_min");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("Light2D", "get_mode");
	___mb.mb_get_shadow_buffer_size = godot::api->godot_method_bind_get_method("Light2D", "get_shadow_buffer_size");
	___mb.mb_get_shadow_color = godot::api->godot_method_bind_get_method("Light2D", "get_shadow_color");
	___mb.mb_get_shadow_filter = godot::api->godot_method_bind_get_method("Light2D", "get_shadow_filter");
	___mb.mb_get_shadow_gradient_length = godot::api->godot_method_bind_get_method("Light2D", "get_shadow_gradient_length");
	___mb.mb_get_shadow_smooth = godot::api->godot_method_bind_get_method("Light2D", "get_shadow_smooth");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Light2D", "get_texture");
	___mb.mb_get_texture_offset = godot::api->godot_method_bind_get_method("Light2D", "get_texture_offset");
	___mb.mb_get_texture_scale = godot::api->godot_method_bind_get_method("Light2D", "get_texture_scale");
	___mb.mb_get_z_range_max = godot::api->godot_method_bind_get_method("Light2D", "get_z_range_max");
	___mb.mb_get_z_range_min = godot::api->godot_method_bind_get_method("Light2D", "get_z_range_min");
	___mb.mb_is_editor_only = godot::api->godot_method_bind_get_method("Light2D", "is_editor_only");
	___mb.mb_is_enabled = godot::api->godot_method_bind_get_method("Light2D", "is_enabled");
	___mb.mb_is_shadow_enabled = godot::api->godot_method_bind_get_method("Light2D", "is_shadow_enabled");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("Light2D", "set_color");
	___mb.mb_set_editor_only = godot::api->godot_method_bind_get_method("Light2D", "set_editor_only");
	___mb.mb_set_enabled = godot::api->godot_method_bind_get_method("Light2D", "set_enabled");
	___mb.mb_set_energy = godot::api->godot_method_bind_get_method("Light2D", "set_energy");
	___mb.mb_set_height = godot::api->godot_method_bind_get_method("Light2D", "set_height");
	___mb.mb_set_item_cull_mask = godot::api->godot_method_bind_get_method("Light2D", "set_item_cull_mask");
	___mb.mb_set_item_shadow_cull_mask = godot::api->godot_method_bind_get_method("Light2D", "set_item_shadow_cull_mask");
	___mb.mb_set_layer_range_max = godot::api->godot_method_bind_get_method("Light2D", "set_layer_range_max");
	___mb.mb_set_layer_range_min = godot::api->godot_method_bind_get_method("Light2D", "set_layer_range_min");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("Light2D", "set_mode");
	___mb.mb_set_shadow_buffer_size = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_buffer_size");
	___mb.mb_set_shadow_color = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_color");
	___mb.mb_set_shadow_enabled = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_enabled");
	___mb.mb_set_shadow_filter = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_filter");
	___mb.mb_set_shadow_gradient_length = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_gradient_length");
	___mb.mb_set_shadow_smooth = godot::api->godot_method_bind_get_method("Light2D", "set_shadow_smooth");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Light2D", "set_texture");
	___mb.mb_set_texture_offset = godot::api->godot_method_bind_get_method("Light2D", "set_texture_offset");
	___mb.mb_set_texture_scale = godot::api->godot_method_bind_get_method("Light2D", "set_texture_scale");
	___mb.mb_set_z_range_max = godot::api->godot_method_bind_get_method("Light2D", "set_z_range_max");
	___mb.mb_set_z_range_min = godot::api->godot_method_bind_get_method("Light2D", "set_z_range_min");
}

Light2D *Light2D::_new()
{
	return (Light2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Light2D")());
}
Color Light2D::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

real_t Light2D::get_energy() const {
	return ___godot_icall_float(___mb.mb_get_energy, (const Object *) this);
}

real_t Light2D::get_height() const {
	return ___godot_icall_float(___mb.mb_get_height, (const Object *) this);
}

int64_t Light2D::get_item_cull_mask() const {
	return ___godot_icall_int(___mb.mb_get_item_cull_mask, (const Object *) this);
}

int64_t Light2D::get_item_shadow_cull_mask() const {
	return ___godot_icall_int(___mb.mb_get_item_shadow_cull_mask, (const Object *) this);
}

int64_t Light2D::get_layer_range_max() const {
	return ___godot_icall_int(___mb.mb_get_layer_range_max, (const Object *) this);
}

int64_t Light2D::get_layer_range_min() const {
	return ___godot_icall_int(___mb.mb_get_layer_range_min, (const Object *) this);
}

Light2D::Mode Light2D::get_mode() const {
	return (Light2D::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

int64_t Light2D::get_shadow_buffer_size() const {
	return ___godot_icall_int(___mb.mb_get_shadow_buffer_size, (const Object *) this);
}

Color Light2D::get_shadow_color() const {
	return ___godot_icall_Color(___mb.mb_get_shadow_color, (const Object *) this);
}

Light2D::ShadowFilter Light2D::get_shadow_filter() const {
	return (Light2D::ShadowFilter) ___godot_icall_int(___mb.mb_get_shadow_filter, (const Object *) this);
}

real_t Light2D::get_shadow_gradient_length() const {
	return ___godot_icall_float(___mb.mb_get_shadow_gradient_length, (const Object *) this);
}

real_t Light2D::get_shadow_smooth() const {
	return ___godot_icall_float(___mb.mb_get_shadow_smooth, (const Object *) this);
}

Ref<Texture> Light2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

Vector2 Light2D::get_texture_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_texture_offset, (const Object *) this);
}

real_t Light2D::get_texture_scale() const {
	return ___godot_icall_float(___mb.mb_get_texture_scale, (const Object *) this);
}

int64_t Light2D::get_z_range_max() const {
	return ___godot_icall_int(___mb.mb_get_z_range_max, (const Object *) this);
}

int64_t Light2D::get_z_range_min() const {
	return ___godot_icall_int(___mb.mb_get_z_range_min, (const Object *) this);
}

bool Light2D::is_editor_only() const {
	return ___godot_icall_bool(___mb.mb_is_editor_only, (const Object *) this);
}

bool Light2D::is_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_enabled, (const Object *) this);
}

bool Light2D::is_shadow_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_shadow_enabled, (const Object *) this);
}

void Light2D::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void Light2D::set_editor_only(const bool editor_only) {
	___godot_icall_void_bool(___mb.mb_set_editor_only, (const Object *) this, editor_only);
}

void Light2D::set_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_enabled, (const Object *) this, enabled);
}

void Light2D::set_energy(const real_t energy) {
	___godot_icall_void_float(___mb.mb_set_energy, (const Object *) this, energy);
}

void Light2D::set_height(const real_t height) {
	___godot_icall_void_float(___mb.mb_set_height, (const Object *) this, height);
}

void Light2D::set_item_cull_mask(const int64_t item_cull_mask) {
	___godot_icall_void_int(___mb.mb_set_item_cull_mask, (const Object *) this, item_cull_mask);
}

void Light2D::set_item_shadow_cull_mask(const int64_t item_shadow_cull_mask) {
	___godot_icall_void_int(___mb.mb_set_item_shadow_cull_mask, (const Object *) this, item_shadow_cull_mask);
}

void Light2D::set_layer_range_max(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_layer_range_max, (const Object *) this, layer);
}

void Light2D::set_layer_range_min(const int64_t layer) {
	___godot_icall_void_int(___mb.mb_set_layer_range_min, (const Object *) this, layer);
}

void Light2D::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void Light2D::set_shadow_buffer_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_shadow_buffer_size, (const Object *) this, size);
}

void Light2D::set_shadow_color(const Color shadow_color) {
	___godot_icall_void_Color(___mb.mb_set_shadow_color, (const Object *) this, shadow_color);
}

void Light2D::set_shadow_enabled(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_shadow_enabled, (const Object *) this, enabled);
}

void Light2D::set_shadow_filter(const int64_t filter) {
	___godot_icall_void_int(___mb.mb_set_shadow_filter, (const Object *) this, filter);
}

void Light2D::set_shadow_gradient_length(const real_t multiplier) {
	___godot_icall_void_float(___mb.mb_set_shadow_gradient_length, (const Object *) this, multiplier);
}

void Light2D::set_shadow_smooth(const real_t smooth) {
	___godot_icall_void_float(___mb.mb_set_shadow_smooth, (const Object *) this, smooth);
}

void Light2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Light2D::set_texture_offset(const Vector2 texture_offset) {
	___godot_icall_void_Vector2(___mb.mb_set_texture_offset, (const Object *) this, texture_offset);
}

void Light2D::set_texture_scale(const real_t texture_scale) {
	___godot_icall_void_float(___mb.mb_set_texture_scale, (const Object *) this, texture_scale);
}

void Light2D::set_z_range_max(const int64_t z) {
	___godot_icall_void_int(___mb.mb_set_z_range_max, (const Object *) this, z);
}

void Light2D::set_z_range_min(const int64_t z) {
	___godot_icall_void_int(___mb.mb_set_z_range_min, (const Object *) this, z);
}

}