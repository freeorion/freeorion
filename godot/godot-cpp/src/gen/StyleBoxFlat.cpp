#include "StyleBoxFlat.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StyleBoxFlat::___method_bindings StyleBoxFlat::___mb = {};

void StyleBoxFlat::___init_method_bindings() {
	___mb.mb_get_aa_size = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_aa_size");
	___mb.mb_get_bg_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_bg_color");
	___mb.mb_get_border_blend = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_border_blend");
	___mb.mb_get_border_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_border_color");
	___mb.mb_get_border_width = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_border_width");
	___mb.mb_get_border_width_min = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_border_width_min");
	___mb.mb_get_corner_detail = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_corner_detail");
	___mb.mb_get_corner_radius = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_corner_radius");
	___mb.mb_get_expand_margin = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_expand_margin");
	___mb.mb_get_shadow_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_shadow_color");
	___mb.mb_get_shadow_offset = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_shadow_offset");
	___mb.mb_get_shadow_size = godot::api->godot_method_bind_get_method("StyleBoxFlat", "get_shadow_size");
	___mb.mb_is_anti_aliased = godot::api->godot_method_bind_get_method("StyleBoxFlat", "is_anti_aliased");
	___mb.mb_is_draw_center_enabled = godot::api->godot_method_bind_get_method("StyleBoxFlat", "is_draw_center_enabled");
	___mb.mb_set_aa_size = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_aa_size");
	___mb.mb_set_anti_aliased = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_anti_aliased");
	___mb.mb_set_bg_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_bg_color");
	___mb.mb_set_border_blend = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_border_blend");
	___mb.mb_set_border_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_border_color");
	___mb.mb_set_border_width = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_border_width");
	___mb.mb_set_border_width_all = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_border_width_all");
	___mb.mb_set_corner_detail = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_corner_detail");
	___mb.mb_set_corner_radius = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_corner_radius");
	___mb.mb_set_corner_radius_all = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_corner_radius_all");
	___mb.mb_set_corner_radius_individual = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_corner_radius_individual");
	___mb.mb_set_draw_center = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_draw_center");
	___mb.mb_set_expand_margin = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_expand_margin");
	___mb.mb_set_expand_margin_all = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_expand_margin_all");
	___mb.mb_set_expand_margin_individual = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_expand_margin_individual");
	___mb.mb_set_shadow_color = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_shadow_color");
	___mb.mb_set_shadow_offset = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_shadow_offset");
	___mb.mb_set_shadow_size = godot::api->godot_method_bind_get_method("StyleBoxFlat", "set_shadow_size");
}

StyleBoxFlat *StyleBoxFlat::_new()
{
	return (StyleBoxFlat *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StyleBoxFlat")());
}
int64_t StyleBoxFlat::get_aa_size() const {
	return ___godot_icall_int(___mb.mb_get_aa_size, (const Object *) this);
}

Color StyleBoxFlat::get_bg_color() const {
	return ___godot_icall_Color(___mb.mb_get_bg_color, (const Object *) this);
}

bool StyleBoxFlat::get_border_blend() const {
	return ___godot_icall_bool(___mb.mb_get_border_blend, (const Object *) this);
}

Color StyleBoxFlat::get_border_color() const {
	return ___godot_icall_Color(___mb.mb_get_border_color, (const Object *) this);
}

int64_t StyleBoxFlat::get_border_width(const int64_t margin) const {
	return ___godot_icall_int_int(___mb.mb_get_border_width, (const Object *) this, margin);
}

int64_t StyleBoxFlat::get_border_width_min() const {
	return ___godot_icall_int(___mb.mb_get_border_width_min, (const Object *) this);
}

int64_t StyleBoxFlat::get_corner_detail() const {
	return ___godot_icall_int(___mb.mb_get_corner_detail, (const Object *) this);
}

int64_t StyleBoxFlat::get_corner_radius(const int64_t corner) const {
	return ___godot_icall_int_int(___mb.mb_get_corner_radius, (const Object *) this, corner);
}

real_t StyleBoxFlat::get_expand_margin(const int64_t margin) const {
	return ___godot_icall_float_int(___mb.mb_get_expand_margin, (const Object *) this, margin);
}

Color StyleBoxFlat::get_shadow_color() const {
	return ___godot_icall_Color(___mb.mb_get_shadow_color, (const Object *) this);
}

Vector2 StyleBoxFlat::get_shadow_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_shadow_offset, (const Object *) this);
}

int64_t StyleBoxFlat::get_shadow_size() const {
	return ___godot_icall_int(___mb.mb_get_shadow_size, (const Object *) this);
}

bool StyleBoxFlat::is_anti_aliased() const {
	return ___godot_icall_bool(___mb.mb_is_anti_aliased, (const Object *) this);
}

bool StyleBoxFlat::is_draw_center_enabled() const {
	return ___godot_icall_bool(___mb.mb_is_draw_center_enabled, (const Object *) this);
}

void StyleBoxFlat::set_aa_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_aa_size, (const Object *) this, size);
}

void StyleBoxFlat::set_anti_aliased(const bool anti_aliased) {
	___godot_icall_void_bool(___mb.mb_set_anti_aliased, (const Object *) this, anti_aliased);
}

void StyleBoxFlat::set_bg_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_bg_color, (const Object *) this, color);
}

void StyleBoxFlat::set_border_blend(const bool blend) {
	___godot_icall_void_bool(___mb.mb_set_border_blend, (const Object *) this, blend);
}

void StyleBoxFlat::set_border_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_border_color, (const Object *) this, color);
}

void StyleBoxFlat::set_border_width(const int64_t margin, const int64_t width) {
	___godot_icall_void_int_int(___mb.mb_set_border_width, (const Object *) this, margin, width);
}

void StyleBoxFlat::set_border_width_all(const int64_t width) {
	___godot_icall_void_int(___mb.mb_set_border_width_all, (const Object *) this, width);
}

void StyleBoxFlat::set_corner_detail(const int64_t detail) {
	___godot_icall_void_int(___mb.mb_set_corner_detail, (const Object *) this, detail);
}

void StyleBoxFlat::set_corner_radius(const int64_t corner, const int64_t radius) {
	___godot_icall_void_int_int(___mb.mb_set_corner_radius, (const Object *) this, corner, radius);
}

void StyleBoxFlat::set_corner_radius_all(const int64_t radius) {
	___godot_icall_void_int(___mb.mb_set_corner_radius_all, (const Object *) this, radius);
}

void StyleBoxFlat::set_corner_radius_individual(const int64_t radius_top_left, const int64_t radius_top_right, const int64_t radius_bottom_right, const int64_t radius_bottom_left) {
	___godot_icall_void_int_int_int_int(___mb.mb_set_corner_radius_individual, (const Object *) this, radius_top_left, radius_top_right, radius_bottom_right, radius_bottom_left);
}

void StyleBoxFlat::set_draw_center(const bool draw_center) {
	___godot_icall_void_bool(___mb.mb_set_draw_center, (const Object *) this, draw_center);
}

void StyleBoxFlat::set_expand_margin(const int64_t margin, const real_t size) {
	___godot_icall_void_int_float(___mb.mb_set_expand_margin, (const Object *) this, margin, size);
}

void StyleBoxFlat::set_expand_margin_all(const real_t size) {
	___godot_icall_void_float(___mb.mb_set_expand_margin_all, (const Object *) this, size);
}

void StyleBoxFlat::set_expand_margin_individual(const real_t size_left, const real_t size_top, const real_t size_right, const real_t size_bottom) {
	___godot_icall_void_float_float_float_float(___mb.mb_set_expand_margin_individual, (const Object *) this, size_left, size_top, size_right, size_bottom);
}

void StyleBoxFlat::set_shadow_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_shadow_color, (const Object *) this, color);
}

void StyleBoxFlat::set_shadow_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_shadow_offset, (const Object *) this, offset);
}

void StyleBoxFlat::set_shadow_size(const int64_t size) {
	___godot_icall_void_int(___mb.mb_set_shadow_size, (const Object *) this, size);
}

}