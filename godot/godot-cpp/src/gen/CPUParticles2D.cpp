#include "CPUParticles2D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "Gradient.hpp"
#include "Texture.hpp"
#include "Curve.hpp"


namespace godot {


CPUParticles2D::___method_bindings CPUParticles2D::___mb = {};

void CPUParticles2D::___init_method_bindings() {
	___mb.mb__update_render_thread = godot::api->godot_method_bind_get_method("CPUParticles2D", "_update_render_thread");
	___mb.mb_convert_from_particles = godot::api->godot_method_bind_get_method("CPUParticles2D", "convert_from_particles");
	___mb.mb_get_amount = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_amount");
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_color");
	___mb.mb_get_color_ramp = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_color_ramp");
	___mb.mb_get_direction = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_direction");
	___mb.mb_get_draw_order = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_draw_order");
	___mb.mb_get_emission_colors = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_colors");
	___mb.mb_get_emission_normals = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_normals");
	___mb.mb_get_emission_points = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_points");
	___mb.mb_get_emission_rect_extents = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_rect_extents");
	___mb.mb_get_emission_shape = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_shape");
	___mb.mb_get_emission_sphere_radius = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_emission_sphere_radius");
	___mb.mb_get_explosiveness_ratio = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_explosiveness_ratio");
	___mb.mb_get_fixed_fps = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_fixed_fps");
	___mb.mb_get_fractional_delta = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_fractional_delta");
	___mb.mb_get_gravity = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_gravity");
	___mb.mb_get_lifetime = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_lifetime");
	___mb.mb_get_lifetime_randomness = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_lifetime_randomness");
	___mb.mb_get_normalmap = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_normalmap");
	___mb.mb_get_one_shot = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_one_shot");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_param");
	___mb.mb_get_param_curve = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_param_curve");
	___mb.mb_get_param_randomness = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_param_randomness");
	___mb.mb_get_particle_flag = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_particle_flag");
	___mb.mb_get_pre_process_time = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_pre_process_time");
	___mb.mb_get_randomness_ratio = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_randomness_ratio");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_speed_scale");
	___mb.mb_get_spread = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_spread");
	___mb.mb_get_texture = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_texture");
	___mb.mb_get_use_local_coordinates = godot::api->godot_method_bind_get_method("CPUParticles2D", "get_use_local_coordinates");
	___mb.mb_is_emitting = godot::api->godot_method_bind_get_method("CPUParticles2D", "is_emitting");
	___mb.mb_restart = godot::api->godot_method_bind_get_method("CPUParticles2D", "restart");
	___mb.mb_set_amount = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_amount");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_color");
	___mb.mb_set_color_ramp = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_color_ramp");
	___mb.mb_set_direction = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_direction");
	___mb.mb_set_draw_order = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_draw_order");
	___mb.mb_set_emission_colors = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_colors");
	___mb.mb_set_emission_normals = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_normals");
	___mb.mb_set_emission_points = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_points");
	___mb.mb_set_emission_rect_extents = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_rect_extents");
	___mb.mb_set_emission_shape = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_shape");
	___mb.mb_set_emission_sphere_radius = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emission_sphere_radius");
	___mb.mb_set_emitting = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_emitting");
	___mb.mb_set_explosiveness_ratio = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_explosiveness_ratio");
	___mb.mb_set_fixed_fps = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_fixed_fps");
	___mb.mb_set_fractional_delta = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_fractional_delta");
	___mb.mb_set_gravity = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_gravity");
	___mb.mb_set_lifetime = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_lifetime");
	___mb.mb_set_lifetime_randomness = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_lifetime_randomness");
	___mb.mb_set_normalmap = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_normalmap");
	___mb.mb_set_one_shot = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_one_shot");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_param");
	___mb.mb_set_param_curve = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_param_curve");
	___mb.mb_set_param_randomness = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_param_randomness");
	___mb.mb_set_particle_flag = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_particle_flag");
	___mb.mb_set_pre_process_time = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_pre_process_time");
	___mb.mb_set_randomness_ratio = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_randomness_ratio");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_speed_scale");
	___mb.mb_set_spread = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_spread");
	___mb.mb_set_texture = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_texture");
	___mb.mb_set_use_local_coordinates = godot::api->godot_method_bind_get_method("CPUParticles2D", "set_use_local_coordinates");
}

CPUParticles2D *CPUParticles2D::_new()
{
	return (CPUParticles2D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CPUParticles2D")());
}
void CPUParticles2D::_update_render_thread() {
	___godot_icall_void(___mb.mb__update_render_thread, (const Object *) this);
}

void CPUParticles2D::convert_from_particles(const Node *particles) {
	___godot_icall_void_Object(___mb.mb_convert_from_particles, (const Object *) this, particles);
}

int64_t CPUParticles2D::get_amount() const {
	return ___godot_icall_int(___mb.mb_get_amount, (const Object *) this);
}

Color CPUParticles2D::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

Ref<Gradient> CPUParticles2D::get_color_ramp() const {
	return Ref<Gradient>::__internal_constructor(___godot_icall_Object(___mb.mb_get_color_ramp, (const Object *) this));
}

Vector2 CPUParticles2D::get_direction() const {
	return ___godot_icall_Vector2(___mb.mb_get_direction, (const Object *) this);
}

CPUParticles2D::DrawOrder CPUParticles2D::get_draw_order() const {
	return (CPUParticles2D::DrawOrder) ___godot_icall_int(___mb.mb_get_draw_order, (const Object *) this);
}

PoolColorArray CPUParticles2D::get_emission_colors() const {
	return ___godot_icall_PoolColorArray(___mb.mb_get_emission_colors, (const Object *) this);
}

PoolVector2Array CPUParticles2D::get_emission_normals() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_emission_normals, (const Object *) this);
}

PoolVector2Array CPUParticles2D::get_emission_points() const {
	return ___godot_icall_PoolVector2Array(___mb.mb_get_emission_points, (const Object *) this);
}

Vector2 CPUParticles2D::get_emission_rect_extents() const {
	return ___godot_icall_Vector2(___mb.mb_get_emission_rect_extents, (const Object *) this);
}

CPUParticles2D::EmissionShape CPUParticles2D::get_emission_shape() const {
	return (CPUParticles2D::EmissionShape) ___godot_icall_int(___mb.mb_get_emission_shape, (const Object *) this);
}

real_t CPUParticles2D::get_emission_sphere_radius() const {
	return ___godot_icall_float(___mb.mb_get_emission_sphere_radius, (const Object *) this);
}

real_t CPUParticles2D::get_explosiveness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_explosiveness_ratio, (const Object *) this);
}

int64_t CPUParticles2D::get_fixed_fps() const {
	return ___godot_icall_int(___mb.mb_get_fixed_fps, (const Object *) this);
}

bool CPUParticles2D::get_fractional_delta() const {
	return ___godot_icall_bool(___mb.mb_get_fractional_delta, (const Object *) this);
}

Vector2 CPUParticles2D::get_gravity() const {
	return ___godot_icall_Vector2(___mb.mb_get_gravity, (const Object *) this);
}

real_t CPUParticles2D::get_lifetime() const {
	return ___godot_icall_float(___mb.mb_get_lifetime, (const Object *) this);
}

real_t CPUParticles2D::get_lifetime_randomness() const {
	return ___godot_icall_float(___mb.mb_get_lifetime_randomness, (const Object *) this);
}

Ref<Texture> CPUParticles2D::get_normalmap() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_normalmap, (const Object *) this));
}

bool CPUParticles2D::get_one_shot() const {
	return ___godot_icall_bool(___mb.mb_get_one_shot, (const Object *) this);
}

real_t CPUParticles2D::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

Ref<Curve> CPUParticles2D::get_param_curve(const int64_t param) const {
	return Ref<Curve>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_param_curve, (const Object *) this, param));
}

real_t CPUParticles2D::get_param_randomness(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param_randomness, (const Object *) this, param);
}

bool CPUParticles2D::get_particle_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_particle_flag, (const Object *) this, flag);
}

real_t CPUParticles2D::get_pre_process_time() const {
	return ___godot_icall_float(___mb.mb_get_pre_process_time, (const Object *) this);
}

real_t CPUParticles2D::get_randomness_ratio() const {
	return ___godot_icall_float(___mb.mb_get_randomness_ratio, (const Object *) this);
}

real_t CPUParticles2D::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

real_t CPUParticles2D::get_spread() const {
	return ___godot_icall_float(___mb.mb_get_spread, (const Object *) this);
}

Ref<Texture> CPUParticles2D::get_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_texture, (const Object *) this));
}

bool CPUParticles2D::get_use_local_coordinates() const {
	return ___godot_icall_bool(___mb.mb_get_use_local_coordinates, (const Object *) this);
}

bool CPUParticles2D::is_emitting() const {
	return ___godot_icall_bool(___mb.mb_is_emitting, (const Object *) this);
}

void CPUParticles2D::restart() {
	___godot_icall_void(___mb.mb_restart, (const Object *) this);
}

void CPUParticles2D::set_amount(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_amount, (const Object *) this, amount);
}

void CPUParticles2D::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void CPUParticles2D::set_color_ramp(const Ref<Gradient> ramp) {
	___godot_icall_void_Object(___mb.mb_set_color_ramp, (const Object *) this, ramp.ptr());
}

void CPUParticles2D::set_direction(const Vector2 direction) {
	___godot_icall_void_Vector2(___mb.mb_set_direction, (const Object *) this, direction);
}

void CPUParticles2D::set_draw_order(const int64_t order) {
	___godot_icall_void_int(___mb.mb_set_draw_order, (const Object *) this, order);
}

void CPUParticles2D::set_emission_colors(const PoolColorArray array) {
	___godot_icall_void_PoolColorArray(___mb.mb_set_emission_colors, (const Object *) this, array);
}

void CPUParticles2D::set_emission_normals(const PoolVector2Array array) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_emission_normals, (const Object *) this, array);
}

void CPUParticles2D::set_emission_points(const PoolVector2Array array) {
	___godot_icall_void_PoolVector2Array(___mb.mb_set_emission_points, (const Object *) this, array);
}

void CPUParticles2D::set_emission_rect_extents(const Vector2 extents) {
	___godot_icall_void_Vector2(___mb.mb_set_emission_rect_extents, (const Object *) this, extents);
}

void CPUParticles2D::set_emission_shape(const int64_t shape) {
	___godot_icall_void_int(___mb.mb_set_emission_shape, (const Object *) this, shape);
}

void CPUParticles2D::set_emission_sphere_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_emission_sphere_radius, (const Object *) this, radius);
}

void CPUParticles2D::set_emitting(const bool emitting) {
	___godot_icall_void_bool(___mb.mb_set_emitting, (const Object *) this, emitting);
}

void CPUParticles2D::set_explosiveness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_explosiveness_ratio, (const Object *) this, ratio);
}

void CPUParticles2D::set_fixed_fps(const int64_t fps) {
	___godot_icall_void_int(___mb.mb_set_fixed_fps, (const Object *) this, fps);
}

void CPUParticles2D::set_fractional_delta(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_fractional_delta, (const Object *) this, enable);
}

void CPUParticles2D::set_gravity(const Vector2 accel_vec) {
	___godot_icall_void_Vector2(___mb.mb_set_gravity, (const Object *) this, accel_vec);
}

void CPUParticles2D::set_lifetime(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_lifetime, (const Object *) this, secs);
}

void CPUParticles2D::set_lifetime_randomness(const real_t random) {
	___godot_icall_void_float(___mb.mb_set_lifetime_randomness, (const Object *) this, random);
}

void CPUParticles2D::set_normalmap(const Ref<Texture> normalmap) {
	___godot_icall_void_Object(___mb.mb_set_normalmap, (const Object *) this, normalmap.ptr());
}

void CPUParticles2D::set_one_shot(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_one_shot, (const Object *) this, enable);
}

void CPUParticles2D::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

void CPUParticles2D::set_param_curve(const int64_t param, const Ref<Curve> curve) {
	___godot_icall_void_int_Object(___mb.mb_set_param_curve, (const Object *) this, param, curve.ptr());
}

void CPUParticles2D::set_param_randomness(const int64_t param, const real_t randomness) {
	___godot_icall_void_int_float(___mb.mb_set_param_randomness, (const Object *) this, param, randomness);
}

void CPUParticles2D::set_particle_flag(const int64_t flag, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_particle_flag, (const Object *) this, flag, enable);
}

void CPUParticles2D::set_pre_process_time(const real_t secs) {
	___godot_icall_void_float(___mb.mb_set_pre_process_time, (const Object *) this, secs);
}

void CPUParticles2D::set_randomness_ratio(const real_t ratio) {
	___godot_icall_void_float(___mb.mb_set_randomness_ratio, (const Object *) this, ratio);
}

void CPUParticles2D::set_speed_scale(const real_t scale) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, scale);
}

void CPUParticles2D::set_spread(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_spread, (const Object *) this, degrees);
}

void CPUParticles2D::set_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_texture, (const Object *) this, texture.ptr());
}

void CPUParticles2D::set_use_local_coordinates(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_local_coordinates, (const Object *) this, enable);
}

}