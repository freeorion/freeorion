#ifndef GODOT_CPP_NOISETEXTURE_HPP
#define GODOT_CPP_NOISETEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {

class Image;
class OpenSimplexNoise;

class NoiseTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb__generate_texture;
		godot_method_bind *mb__queue_update;
		godot_method_bind *mb__thread_done;
		godot_method_bind *mb__update_texture;
		godot_method_bind *mb_get_bump_strength;
		godot_method_bind *mb_get_noise;
		godot_method_bind *mb_get_seamless;
		godot_method_bind *mb_is_normalmap;
		godot_method_bind *mb_set_as_normalmap;
		godot_method_bind *mb_set_bump_strength;
		godot_method_bind *mb_set_height;
		godot_method_bind *mb_set_noise;
		godot_method_bind *mb_set_seamless;
		godot_method_bind *mb_set_width;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "NoiseTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static NoiseTexture *_new();

	// methods
	Ref<Image> _generate_texture();
	void _queue_update();
	void _thread_done(const Ref<Image> image);
	void _update_texture();
	real_t get_bump_strength();
	Ref<OpenSimplexNoise> get_noise();
	bool get_seamless();
	bool is_normalmap();
	void set_as_normalmap(const bool as_normalmap);
	void set_bump_strength(const real_t bump_strength);
	void set_height(const int64_t height);
	void set_noise(const Ref<OpenSimplexNoise> noise);
	void set_seamless(const bool seamless);
	void set_width(const int64_t width);

};

}

#endif