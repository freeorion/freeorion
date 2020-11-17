#include "Particles.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Mesh.hpp"
#include "Material.hpp"


namespace godot {


Particles::___method_bindings Particles::___mb = {};

void Particles::___init_method_bindings() {
	___mb.mb_capture_aabb = godot::api->godot_method_bind_get_method("Particles", "capture_aabb");
	___mb.mb_get_amount = godot::api->godot_method_bind_get_method("Particles", "get_amount");
	___mb.mb_get_draw_order = godot::api->godot_method_bind_get_method("Particles", "get_draw_order");
	___mb.mb_get_draw_pass_mesh = godot::api->godot_method_bind_get_method("Particles", "get_draw_pass_mesh");
	___mb.mb_get_draw_passes = godot::api->godot_method_bind_get_method("Particles", "get_draw_passes");
	___mb.mb_get_explosiveness_ratio = godot::api->godot_method_bind_get_method("Particles", "get_explosiveness_ratio");
	___mb.mb_get_fixed_fps = godot::api->godot_method_bind_get_method("Particles", "get_fixed_fps");
	___mb.mb_get_fractional_delta = godot::api->godot_method_bind_get_method("Particles", "get_fractional_delta");
	___mb.mb_get_lifetime = godot::api->godot_method_bind_get_method("Particles", "get_lifetime");
	___mb.mb_get_one_shot = godot::api->godot_method_bind_get_method("Particles", "get_one_shot");
	___mb.mb_get_pre_process_time = godot::api->godot_method_bind_get_method("Particles", "get_pre_process_time");
	___mb.mb_get_process_material = godot::api->godot_method_bind_get_method("Particles", "get_process_material");
	___mb.mb_get_randomness_ratio = godot::api->godot_method_bind_get_method("Particles", "get_randomness_ratio");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("Particles", "get_speed_scale");
	___mb.mb_get_use_local_coordinates = godot::api->godot_method_bind_get_method("Particles", "get_use_local_coordinates");
	___mb.mb_get_visibility_aabb = godot::api->godot_method_bind_get_method("Particles", "get_visibility_aabb");
	___mb.mb_is_emitting = godot::api->godot_method_bind_get_method("Particles", "is_emitting");
	___mb.mb_restart = godot::api->godot_method_bind_get_method("Particles", "restart");
	___mb.mb_set_amount = godot::api->godot_method_bind_get_method("Particles", "set_amount");
	___mb.mb_set_draw_order = godot::api->godot_method_bind_get_method("Particles", "set_draw_order");
	___mb.mb_set_draw_pass_mesh = godot::api->godot_method_bind_get_method("Particles", "set_draw_pass_mesh");
	___mb.mb_set_draw_passes = godot::api->godot_method_bind_get_method("Particles", "set_draw_passes");
	___mb.mb_set_emitting = godot::api->godot_method_bind_get_method("Particles", "set_emitting");
	___mb.mb_set_explosiveness_ratio = godot::api->godot_method_bind_get_method("Particles", "set_explosiveness_ratio");
	___mb.mb_set_fixed_fps = godot::api->godot_method_bind_get_method("Particles", "set_fixed_fps");
	___mb.mb_set_fractional_delta = godot::api->godot_method_bind_get_method("Particles", "set_fractional_delta");
	___mb.mb_set_lifetime = godot::api->godot_method_bind_get_method("Particles", "set_lifetime");
	___mb.mb_set_one_shot = godot::api->godot_method_bind_get_method("Particles", "set_one_shot");
	___mb.mb_set_pre_process_time = godot::api->godot_method_bind_get_method("Particles", "set_pre_process_time");
	___mb.mb_set_process_material = godot::api->godot_method_bind_get_method("Particles", "set_process_material");
	___mb.mb_set_randomness_ratio = godot::api->godot_method_bind_get_method("Particles", "set_randomness_ratio");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("Particles", "set_speed_scale");
	___mb.mb_set_use_local_coordinates = godot::api->godot_method_bind_get_method("Particles", "set_use_local_coordinates");
	___mb.mb_set_visibility_aabb = godot::api->godot_method_bind_get_method("Particles", "set_visibility_aabb");
}

Particles *Particles::_new()
{
	return (Particles *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Particles")());
}
AABB Particles::capture_aabb() const {
	return ___godot_icall_AABB(___mb.mb_capture_aabb, (const Object *) this);
}

int64_t Particles::get_amount() const {
	return ___godot_icall_int(___mb.mb_get_amount, (const Object *) this);
}

Particles::DrawOrder Particles::get_draw_order() const {
	return (Particles::DrawOrder) ___godot_icall_int(___mb.mb_get_draw_order, (const Object *) this);
}

Ref<Mesh> Particles::get_draw_pass_mesh(const int64_t pass) const {
	return Ref<Mesh>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_draw_pass_mesh, (const Object *) this, pass));
}

int64_t Particles::get_draw_passes() const {
	return ___godot_icall_int(___mb.mb_get_draw_passes, (const Object *) this);
}

real_t Particles::get_explosiveness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_explosiveness_ratio, (const Object *) this);
}

int64_t Particles::get_fixed_fps() const {
	return ___godot_icall_int(___mb.mb_get_fixed_fps, (const Object *) this);
}

bool Particles::get_fractional_delta() const {
	return ___godot_icall_bool(___mb.mb_get_fractional_delta, (const Object *) this);
}

real_t Particles::get_lifetime() const {
	return ___godot_icall_float(___mb.mb_get_lifetime, (const Object *) this);
}

bool Particles::get_one_shot() const {
	return ___godot_icall_bool(___mb.mb_get_one_shot, (const Object *) this);
}

real_t Particles::get_pre_process_time() const {
	return ___godot_icall_float(___mb.mb_get_pre_process_time, (const Object *) this);
}

Ref<Material> Particles::get_process_material() const {
	return Ref<Material>::__internal_constructor(___godot_icall_Object(___mb.mb_get_process_material, (const Object *) this));
}

real_t Particles::get_randomness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_randomness_ratio, (const Object *) this);
}

real_t Particles::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

bool Particles::get_use_local_coordinates() const {
	return ___godot_icall_bool(___mb.mb_get_use_local_coordinates, (const Object *) this);
}

AABB Particles::get_visibility_aabb() const {
	return ___godot_icall_AABB(___mb.mb_get_visibility_aabb, (const Object *) this);
}

bool Particles::is_emitting() const {
	return ___godot_icall_bool(___mb.mb_is_emitting, (const Object *) this);
}

void Particles::restart() {
	___godot_icall_void(___mb.mb_restart, (const Object *) this);
}

void Particles::set_amount(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_amount, (const Object *) this, amount);
}

void Particles::set_draw_order(const int64_t order) {
	___godot_icall_void_int(___mb.mb_set_draw_order, (const Object *) this, order);
}

void Particles::set_draw_pass_mesh(const int64_t pass, const Ref<Mesh> mesh) {
	___godot_icall_void_int_Object(___mb.mb_set_draw_pass_mesh, (const Object *) this, pass, mesh.ptr());
}

void Particles::set_draw_passes(const int64_t passes) {
	___godot_icall_void_int(___mb.mb_set_draw_passes, (const Object *) this, passes);
}

void Particles::set_emitting(const bool emitting) {
	___godot_icall_void_bool(___mb.mb_set_emitting, (const Object *) this, emitting);
}

void Particles::set_explosiveness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_explosiveness_ratio, (const Object *) this, ratio);
}

void Particles::set_fixed_fps(const int64_t fps) {
	___godot_icall_void_int(___mb.mb_set_fixed_fps, (const Object *) this, fps);
}

void Particles::set_fractional_delta(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_fractional_delta, (const Object *) this, enable);
}

void Particles::set_lifetime(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_lifetime, (const Object *) this, secs);
}

void Particles::set_one_shot(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_one_shot, (const Object *) this, enable);
}

void Particles::set_pre_process_time(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_pre_process_time, (const Object *) this, secs);
}

void Particles::set_process_material(const Ref<Material> material) {
	___godot_icall_void_Object(___mb.mb_set_process_material, (const Object *) this, material.ptr());
}

void Particles::set_randomness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_randomness_ratio, (const Object *) this, ratio);
}

void Particles::set_speed_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, scale);
}

void Particles::set_use_local_coordinates(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_local_coordinates, (const Object *) this, enable);
}

void Particles::set_visibility_aabb(const AABB aabb) {
	___godot_icall_void_AABB(___mb.mb_set_visibility_aabb, (const Object *) this, aabb);
}

}