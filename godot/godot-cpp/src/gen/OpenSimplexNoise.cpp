#include "OpenSimplexNoise.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Image.hpp"


namespace godot {


OpenSimplexNoise::___method_bindings OpenSimplexNoise::___mb = {};

void OpenSimplexNoise::___init_method_bindings() {
	___mb.mb_get_image = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_image");
	___mb.mb_get_lacunarity = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_lacunarity");
	___mb.mb_get_noise_1d = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_1d");
	___mb.mb_get_noise_2d = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_2d");
	___mb.mb_get_noise_2dv = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_2dv");
	___mb.mb_get_noise_3d = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_3d");
	___mb.mb_get_noise_3dv = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_3dv");
	___mb.mb_get_noise_4d = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_noise_4d");
	___mb.mb_get_octaves = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_octaves");
	___mb.mb_get_period = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_period");
	___mb.mb_get_persistence = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_persistence");
	___mb.mb_get_seamless_image = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_seamless_image");
	___mb.mb_get_seed = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "get_seed");
	___mb.mb_set_lacunarity = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "set_lacunarity");
	___mb.mb_set_octaves = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "set_octaves");
	___mb.mb_set_period = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "set_period");
	___mb.mb_set_persistence = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "set_persistence");
	___mb.mb_set_seed = godot::api->godot_method_bind_get_method("OpenSimplexNoise", "set_seed");
}

OpenSimplexNoise *OpenSimplexNoise::_new()
{
	return (OpenSimplexNoise *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"OpenSimplexNoise")());
}
Ref<Image> OpenSimplexNoise::get_image(const int64_t width, const int64_t height) {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_get_image, (const Object *) this, width, height));
}

real_t OpenSimplexNoise::get_lacunarity() const {
	return ___godot_icall_float(___mb.mb_get_lacunarity, (const Object *) this);
}

real_t OpenSimplexNoise::get_noise_1d(const real_t x) {
	return ___godot_icall_float_float(___mb.mb_get_noise_1d, (const Object *) this, x);
}

real_t OpenSimplexNoise::get_noise_2d(const real_t x, const real_t y) {
	return ___godot_icall_float_float_float(___mb.mb_get_noise_2d, (const Object *) this, x, y);
}

real_t OpenSimplexNoise::get_noise_2dv(const Vector2 pos) {
	return ___godot_icall_float_Vector2(___mb.mb_get_noise_2dv, (const Object *) this, pos);
}

real_t OpenSimplexNoise::get_noise_3d(const real_t x, const real_t y, const real_t z) {
	return ___godot_icall_float_float_float_float(___mb.mb_get_noise_3d, (const Object *) this, x, y, z);
}

real_t OpenSimplexNoise::get_noise_3dv(const Vector3 pos) {
	return ___godot_icall_float_Vector3(___mb.mb_get_noise_3dv, (const Object *) this, pos);
}

real_t OpenSimplexNoise::get_noise_4d(const real_t x, const real_t y, const real_t z, const real_t w) {
	return ___godot_icall_float_float_float_float_float(___mb.mb_get_noise_4d, (const Object *) this, x, y, z, w);
}

int64_t OpenSimplexNoise::get_octaves() const {
	return ___godot_icall_int(___mb.mb_get_octaves, (const Object *) this);
}

real_t OpenSimplexNoise::get_period() const {
	return ___godot_icall_float(___mb.mb_get_period, (const Object *) this);
}

real_t OpenSimplexNoise::get_persistence() const {
	return ___godot_icall_float(___mb.mb_get_persistence, (const Object *) this);
}

Ref<Image> OpenSimplexNoise::get_seamless_image(const int64_t size) {
	return Ref<Image>::__internal_constructor(___godot_icall_Object_int(___mb.mb_get_seamless_image, (const Object *) this, size));
}

int64_t OpenSimplexNoise::get_seed() {
	return ___godot_icall_int(___mb.mb_get_seed, (const Object *) this);
}

void OpenSimplexNoise::set_lacunarity(const real_t lacunarity) {
	___godot_icall_void_float(___mb.mb_set_lacunarity, (const Object *) this, lacunarity);
}

void OpenSimplexNoise::set_octaves(const int64_t octave_count) {
	___godot_icall_void_int(___mb.mb_set_octaves, (const Object *) this, octave_count);
}

void OpenSimplexNoise::set_period(const real_t period) {
	___godot_icall_void_float(___mb.mb_set_period, (const Object *) this, period);
}

void OpenSimplexNoise::set_persistence(const real_t persistence) {
	___godot_icall_void_float(___mb.mb_set_persistence, (const Object *) this, persistence);
}

void OpenSimplexNoise::set_seed(const int64_t seed) {
	___godot_icall_void_int(___mb.mb_set_seed, (const Object *) this, seed);
}

}