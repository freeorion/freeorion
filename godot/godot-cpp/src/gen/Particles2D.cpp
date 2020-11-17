#include "Particles2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "Material.hpp"


namespace godot {


Particles2D::___method_bindings Particles2D::___mb = {};

void Particles2D::___init_method_bindings() {
	___mb.mb_capture_rect = godot::api->godot_method_bind_get_method("Particles2D", "capture_rect");
	___mb.mb_get_amount = godot::api->godot_method_bind_get_method("Particles2D", "get_amount");
	___mb.mb_get_draw_order = godot::api->godot_method_bind_get_method("Particles2D", "get_draw_order");
	___mb.mb_get_explosiveness_ratio = godot::api->godot_method_bind_get_method("Particles2D", "get_explosiveness_ratio");
	___mb.mb_get_fixed_fps = godot::api->godot_method_bind_get_method("Particles2D", "get_fixed_fps");
	___mb.mb_get_fractional_delta = godot::api->godot_method_bind_get_method("Particles2D", "get_fractional_delta");
	___mb.mb_get_lifetime = godot::api->godot_method_bind_get_method("Particles2D", "get_lifetime");
	___mb.mb_get_normal_map = godot::api->godot_method_bind_get_method("Particles2D", "get_normal_map");
	___mb.mb_get_one_shot = godot::api->godot_method_bind_get_method("Particles2D", "get_one_shot");
	___mb.mb_get_pre_process_time = godot::api->godot_method_bind_get_method("Particles2D", "get_pre_process_time");
	___mb.mb_get_process_material = godot::api->godot_method_bind_get_method("Particles2D", "get_process_material");
	___mb.mb_get_randomness_ratio = godot::api->godot_method_bind_get_method("Particles2D", "get_randomness_ratio");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("Particles2D", "get_speed_scale");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("Particles2D", "get_texture");
	___mb.mb_get_use_local_coordinates = godot::api->godot_method_bind_get_method("Particles2D", "get_use_local_coordinates");
	___mb.mb_get_visibility_rect = godot::api->godot_method_bind_get_method("Particles2D", "get_visibility_rect");
	___mb.mb_is_emitting = godot::api->godot_method_bind_get_method("Particles2D", "is_emitting");
	___mb.mb_restart = godot::api->godot_method_bind_get_method("Particles2D", "restart");
	___mb.mb_set_amount = godot::api->godot_method_bind_get_method("Particles2D", "set_amount");
	___mb.mb_set_draw_order = godot::api->godot_method_bind_get_method("Particles2D", "set_draw_order");
	___mb.mb_set_emitting = godot::api->godot_method_bind_get_method("Particles2D", "set_emitting");
	___mb.mb_set_explosiveness_ratio = godot::api->godot_method_bind_get_method("Particles2D", "set_explosiveness_ratio");
	___mb.mb_set_fixed_fps = godot::api->godot_method_bind_get_method("Particles2D", "set_fixed_fps");
	___mb.mb_set_fractional_delta = godot::api->godot_method_bind_get_method("Particles2D", "set_fractional_delta");
	___mb.mb_set_lifetime = godot::api->godot_method_bind_get_method("Particles2D", "set_lifetime");
	___mb.mb_set_normal_map = godot::api->godot_method_bind_get_method("Particles2D", "set_normal_map");
	___mb.mb_set_one_shot = godot::api->godot_method_bind_get_method("Particles2D", "set_one_shot");
	___mb.mb_set_pre_process_time = godot::api->godot_method_bind_get_method("Particles2D", "set_pre_process_time");
	___mb.mb_set_process_material = godot::api->godot_method_bind_get_method("Particles2D", "set_process_material");
	___mb.mb_set_randomness_ratio = godot::api->godot_method_bind_get_method("Particles2D", "set_randomness_ratio");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("Particles2D", "set_speed_scale");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("Particles2D", "set_texture");
	___mb.mb_set_use_local_coordinates = godot::api->godot_method_bind_get_method("Particles2D", "set_use_local_coordinates");
	___mb.mb_set_visibility_rect = godot::api->godot_method_bind_get_method("Particles2D", "set_visibility_rect");
}

Particles2D *Particles2D::_new()
{
	return (Particles2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Particles2D")());
}
Rect2 Particles2D::capture_rect() const {
	return ___godot_icall_Rect2(___mb.mb_capture_rect, (const Object *) this);
}

int64_t Particles2D::get_amount() const {
	return ___godot_icall_int(___mb.mb_get_amount, (const Object *) this);
}

Particles2D::DrawOrder Particles2D::get_draw_order() const {
	return (Particles2D::DrawOrder) ___godot_icall_int(___mb.mb_get_draw_order, (const Object *) this);
}

real_t Particles2D::get_explosiveness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_explosiveness_ratio, (const Object *) this);
}

int64_t Particles2D::get_fixed_fps() const {
	return ___godot_icall_int(___mb.mb_get_fixed_fps, (const Object *) this);
}

bool Particles2D::get_fractional_delta() const {
	return ___godot_icall_bool(___mb.mb_get_fractional_delta, (const Object *) this);
}

real_t Particles2D::get_lifetime() const {
	return ___godot_icall_float(___mb.mb_get_lifetime, (const Object *) this);
}

Ref<Texture> Particles2D::get_normal_map() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normal_map, (const Object *) this));
}

bool Particles2D::get_one_shot() const {
	return ___godot_icall_bool(___mb.mb_get_one_shot, (const Object *) this);
}

real_t Particles2D::get_pre_process_time() const {
	return ___godot_icall_float(___mb.mb_get_pre_process_time, (const Object *) this);
}

Ref<Material> Particles2D::get_process_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_process_material, (const Object *) this));
}

real_t Particles2D::get_randomness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_randomness_ratio, (const Object *) this);
}

real_t Particles2D::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

Ref<Texture> Particles2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

bool Particles2D::get_use_local_coordinates() const {
	return ___godot_icall_bool(___mb.mb_get_use_local_coordinates, (const Object *) this);
}

Rect2 Particles2D::get_visibility_rect() const {
	return ___godot_icall_Rect2(___mb.mb_get_visibility_rect, (const Object *) this);
}

bool Particles2D::is_emitting() const {
	return ___godot_icall_bool(___mb.mb_is_emitting, (const Object *) this);
}

void Particles2D::restart() {
	___godot_icall_void(___mb.mb_restart, (const Object *) this);
}

void Particles2D::set_amount(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_amount, (const Object *) this, amount);
}

void Particles2D::set_draw_order(const int64_t order) {
	___godot_icall_void_int(___mb.mb_set_draw_order, (const Object *) this, order);
}

void Particles2D::set_emitting(const bool emitting) {
	___godot_icall_void_bool(___mb.mb_set_emitting, (const Object *) this, emitting);
}

void Particles2D::set_explosiveness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_explosiveness_ratio, (const Object *) this, ratio);
}

void Particles2D::set_fixed_fps(const int64_t fps) {
	___godot_icall_void_int(___mb.mb_set_fixed_fps, (const Object *) this, fps);
}

void Particles2D::set_fractional_delta(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_fractional_delta, (const Object *) this, enable);
}

void Particles2D::set_lifetime(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_lifetime, (const Object *) this, secs);
}

void Particles2D::set_normal_map(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_normal_map, (const Object *) this, texture.ptr());
}

void Particles2D::set_one_shot(const bool secs) {
	___godot_icall_void_bool(___mb.mb_set_one_shot, (const Object *) this, secs);
}

void Particles2D::set_pre_process_time(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_pre_process_time, (const Object *) this, secs);
}

void Particles2D::set_process_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_process_material, (const Object *) this, material.ptr());
}

void Particles2D::set_randomness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_randomness_ratio, (const Object *) this, ratio);
}

void Particles2D::set_speed_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, scale);
}

void Particles2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void Particles2D::set_use_local_coordinates(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_local_coordinates, (const Object *) this, enable);
}

void Particles2D::set_visibility_rect(const Rect2 visibility_rect) {
	___godot_icall_void_Rect2(___mb.mb_set_visibility_rect, (const Object *) this, visibility_rect);
}

}