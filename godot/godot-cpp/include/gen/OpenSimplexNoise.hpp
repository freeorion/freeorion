#ifndef GODOT_CPP_OPENSIMPLEXNOISE_HPP
#define GODOT_CPP_OPENSIMPLEXNOISE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Image;

class OpenSimplexNoise : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_get_image;
		godot_method_bind *mb_get_lacunarity;
		godot_method_bind *mb_get_noise_1d;
		godot_method_bind *mb_get_noise_2d;
		godot_method_bind *mb_get_noise_2dv;
		godot_method_bind *mb_get_noise_3d;
		godot_method_bind *mb_get_noise_3dv;
		godot_method_bind *mb_get_noise_4d;
		godot_method_bind *mb_get_octaves;
		godot_method_bind *mb_get_period;
		godot_method_bind *mb_get_persistence;
		godot_method_bind *mb_get_seamless_image;
		godot_method_bind *mb_get_seed;
		godot_method_bind *mb_set_lacunarity;
		godot_method_bind *mb_set_octaves;
		godot_method_bind *mb_set_period;
		godot_method_bind *mb_set_persistence;
		godot_method_bind *mb_set_seed;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "OpenSimplexNoise"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static OpenSimplexNoise *_new();

	// methods
	Ref<Image> get_image(const int64_t width, const int64_t height);
	real_t get_lacunarity() const;
	real_t get_noise_1d(const real_t x);
	real_t get_noise_2d(const real_t x, const real_t y);
	real_t get_noise_2dv(const Vector2 pos);
	real_t get_noise_3d(const real_t x, const real_t y, const real_t z);
	real_t get_noise_3dv(const Vector3 pos);
	real_t get_noise_4d(const real_t x, const real_t y, const real_t z, const real_t w);
	int64_t get_octaves() const;
	real_t get_period() const;
	real_t get_persistence() const;
	Ref<Image> get_seamless_image(const int64_t size);
	int64_t get_seed();
	void set_lacunarity(const real_t lacunarity);
	void set_octaves(const int64_t octave_count);
	void set_period(const real_t period);
	void set_persistence(const real_t persistence);
	void set_seed(const int64_t seed);

};

}

#endif