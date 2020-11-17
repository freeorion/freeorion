#include "ParticlesMaterial.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"
#include "GradientTexture.hpp"
#include "CurveTexture.hpp"


namespace godot {


ParticlesMaterial::___method_bindings ParticlesMaterial::___mb = {};

void ParticlesMaterial::___init_method_bindings() {
	___mb.mb_get_color = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_color");
	___mb.mb_get_color_ramp = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_color_ramp");
	___mb.mb_get_direction = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_direction");
	___mb.mb_get_emission_box_extents = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_box_extents");
	___mb.mb_get_emission_color_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_color_texture");
	___mb.mb_get_emission_normal_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_normal_texture");
	___mb.mb_get_emission_point_count = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_point_count");
	___mb.mb_get_emission_point_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_point_texture");
	___mb.mb_get_emission_shape = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_shape");
	___mb.mb_get_emission_sphere_radius = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_emission_sphere_radius");
	___mb.mb_get_flag = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_flag");
	___mb.mb_get_flatness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_flatness");
	___mb.mb_get_gravity = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_gravity");
	___mb.mb_get_lifetime_randomness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_lifetime_randomness");
	___mb.mb_get_param = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_param");
	___mb.mb_get_param_randomness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_param_randomness");
	___mb.mb_get_param_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_param_texture");
	___mb.mb_get_spread = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_spread");
	___mb.mb_get_trail_color_modifier = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_trail_color_modifier");
	___mb.mb_get_trail_divisor = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_trail_divisor");
	___mb.mb_get_trail_size_modifier = godot::api->godot_method_bind_get_method("ParticlesMaterial", "get_trail_size_modifier");
	___mb.mb_set_color = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_color");
	___mb.mb_set_color_ramp = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_color_ramp");
	___mb.mb_set_direction = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_direction");
	___mb.mb_set_emission_box_extents = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_box_extents");
	___mb.mb_set_emission_color_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_color_texture");
	___mb.mb_set_emission_normal_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_normal_texture");
	___mb.mb_set_emission_point_count = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_point_count");
	___mb.mb_set_emission_point_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_point_texture");
	___mb.mb_set_emission_shape = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_shape");
	___mb.mb_set_emission_sphere_radius = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_emission_sphere_radius");
	___mb.mb_set_flag = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_flag");
	___mb.mb_set_flatness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_flatness");
	___mb.mb_set_gravity = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_gravity");
	___mb.mb_set_lifetime_randomness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_lifetime_randomness");
	___mb.mb_set_param = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_param");
	___mb.mb_set_param_randomness = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_param_randomness");
	___mb.mb_set_param_texture = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_param_texture");
	___mb.mb_set_spread = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_spread");
	___mb.mb_set_trail_color_modifier = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_trail_color_modifier");
	___mb.mb_set_trail_divisor = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_trail_divisor");
	___mb.mb_set_trail_size_modifier = godot::api->godot_method_bind_get_method("ParticlesMaterial", "set_trail_size_modifier");
}

ParticlesMaterial *ParticlesMaterial::_new()
{
	return (ParticlesMaterial *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ParticlesMaterial")());
}
Color ParticlesMaterial::get_color() const {
	return ___godot_icall_Color(___mb.mb_get_color, (const Object *) this);
}

Ref<Texture> ParticlesMaterial::get_color_ramp() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_color_ramp, (const Object *) this));
}

Vector3 ParticlesMaterial::get_direction() const {
	return ___godot_icall_Vector3(___mb.mb_get_direction, (const Object *) this);
}

Vector3 ParticlesMaterial::get_emission_box_extents() const {
	return ___godot_icall_Vector3(___mb.mb_get_emission_box_extents, (const Object *) this);
}

Ref<Texture> ParticlesMaterial::get_emission_color_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_emission_color_texture, (const Object *) this));
}

Ref<Texture> ParticlesMaterial::get_emission_normal_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_emission_normal_texture, (const Object *) this));
}

int64_t ParticlesMaterial::get_emission_point_count() const {
	return ___godot_icall_int(___mb.mb_get_emission_point_count, (const Object *) this);
}

Ref<Texture> ParticlesMaterial::get_emission_point_texture() const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_emission_point_texture, (const Object *) this));
}

ParticlesMaterial::EmissionShape ParticlesMaterial::get_emission_shape() const {
	return (ParticlesMaterial::EmissionShape) ___godot_icall_int(___mb.mb_get_emission_shape, (const Object *) this);
}

real_t ParticlesMaterial::get_emission_sphere_radius() const {
	return ___godot_icall_float(___mb.mb_get_emission_sphere_radius, (const Object *) this);
}

bool ParticlesMaterial::get_flag(const int64_t flag) const {
	return ___godot_icall_bool_int(___mb.mb_get_flag, (const Object *) this, flag);
}

real_t ParticlesMaterial::get_flatness() const {
	return ___godot_icall_float(___mb.mb_get_flatness, (const Object *) this);
}

Vector3 ParticlesMaterial::get_gravity() const {
	return ___godot_icall_Vector3(___mb.mb_get_gravity, (const Object *) this);
}

real_t ParticlesMaterial::get_lifetime_randomness() const {
	return ___godot_icall_float(___mb.mb_get_lifetime_randomness, (const Object *) this);
}

real_t ParticlesMaterial::get_param(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param, (const Object *) this, param);
}

real_t ParticlesMaterial::get_param_randomness(const int64_t param) const {
	return ___godot_icall_float_int(___mb.mb_get_param_randomness, (const Object *) this, param);
}

Ref<Texture> ParticlesMaterial::get_param_texture(const int64_t param) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_param_texture, (const Object *) this, param));
}

real_t ParticlesMaterial::get_spread() const {
	return ___godot_icall_float(___mb.mb_get_spread, (const Object *) this);
}

Ref<GradientTexture> ParticlesMaterial::get_trail_color_modifier() const {
	return Ref<GradientTexture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_trail_color_modifier, (const Object *) this));
}

int64_t ParticlesMaterial::get_trail_divisor() const {
	return ___godot_icall_int(___mb.mb_get_trail_divisor, (const Object *) this);
}

Ref<CurveTexture> ParticlesMaterial::get_trail_size_modifier() const {
	return Ref<CurveTexture>::__internal_constructor(___godot_icall_Object(___mb.mb_get_trail_size_modifier, (const Object *) this));
}

void ParticlesMaterial::set_color(const Color color) {
	___godot_icall_void_Color(___mb.mb_set_color, (const Object *) this, color);
}

void ParticlesMaterial::set_color_ramp(const Ref<Texture> ramp) {
	___godot_icall_void_Object(___mb.mb_set_color_ramp, (const Object *) this, ramp.ptr());
}

void ParticlesMaterial::set_direction(const Vector3 degrees) {
	___godot_icall_void_Vector3(___mb.mb_set_direction, (const Object *) this, degrees);
}

void ParticlesMaterial::set_emission_box_extents(const Vector3 extents) {
	___godot_icall_void_Vector3(___mb.mb_set_emission_box_extents, (const Object *) this, extents);
}

void ParticlesMaterial::set_emission_color_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_emission_color_texture, (const Object *) this, texture.ptr());
}

void ParticlesMaterial::set_emission_normal_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_emission_normal_texture, (const Object *) this, texture.ptr());
}

void ParticlesMaterial::set_emission_point_count(const int64_t point_count) {
	___godot_icall_void_int(___mb.mb_set_emission_point_count, (const Object *) this, point_count);
}

void ParticlesMaterial::set_emission_point_texture(const Ref<Texture> texture) {
	___godot_icall_void_Object(___mb.mb_set_emission_point_texture, (const Object *) this, texture.ptr());
}

void ParticlesMaterial::set_emission_shape(const int64_t shape) {
	___godot_icall_void_int(___mb.mb_set_emission_shape, (const Object *) this, shape);
}

void ParticlesMaterial::set_emission_sphere_radius(const real_t radius) {
	___godot_icall_void_float(___mb.mb_set_emission_sphere_radius, (const Object *) this, radius);
}

void ParticlesMaterial::set_flag(const int64_t flag, const bool enable) {
	___godot_icall_void_int_bool(___mb.mb_set_flag, (const Object *) this, flag, enable);
}

void ParticlesMaterial::set_flatness(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_flatness, (const Object *) this, amount);
}

void ParticlesMaterial::set_gravity(const Vector3 accel_vec) {
	___godot_icall_void_Vector3(___mb.mb_set_gravity, (const Object *) this, accel_vec);
}

void ParticlesMaterial::set_lifetime_randomness(const real_t randomness) {
	___godot_icall_void_float(___mb.mb_set_lifetime_randomness, (const Object *) this, randomness);
}

void ParticlesMaterial::set_param(const int64_t param, const real_t value) {
	___godot_icall_void_int_float(___mb.mb_set_param, (const Object *) this, param, value);
}

void ParticlesMaterial::set_param_randomness(const int64_t param, const real_t randomness) {
	___godot_icall_void_int_float(___mb.mb_set_param_randomness, (const Object *) this, param, randomness);
}

void ParticlesMaterial::set_param_texture(const int64_t param, const Ref<Texture> texture) {
	___godot_icall_void_int_Object(___mb.mb_set_param_texture, (const Object *) this, param, texture.ptr());
}

void ParticlesMaterial::set_spread(const real_t degrees) {
	___godot_icall_void_float(___mb.mb_set_spread, (const Object *) this, degrees);
}

void ParticlesMaterial::set_trail_color_modifier(const Ref<GradientTexture> texture) {
	___godot_icall_void_Object(___mb.mb_set_trail_color_modifier, (const Object *) this, texture.ptr());
}

void ParticlesMaterial::set_trail_divisor(const int64_t divisor) {
	___godot_icall_void_int(___mb.mb_set_trail_divisor, (const Object *) this, divisor);
}

void ParticlesMaterial::set_trail_size_modifier(const Ref<CurveTexture> texture) {
	___godot_icall_void_Object(___mb.mb_set_trail_size_modifier, (const Object *) this, texture.ptr());
}

}