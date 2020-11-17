#ifndef GODOT_CPP_ANIMATEDTEXTURE_HPP
#define GODOT_CPP_ANIMATEDTEXTURE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Texture.hpp"
namespace godot {

class Texture;

class AnimatedTexture : public Texture {
	struct ___method_bindings {
		godot_method_bind *mb__update_proxy;
		godot_method_bind *mb_get_fps;
		godot_method_bind *mb_get_frame_delay;
		godot_method_bind *mb_get_frame_texture;
		godot_method_bind *mb_get_frames;
		godot_method_bind *mb_set_fps;
		godot_method_bind *mb_set_frame_delay;
		godot_method_bind *mb_set_frame_texture;
		godot_method_bind *mb_set_frames;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimatedTexture"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants
	const static int MAX_FRAMES = 256;


	static AnimatedTexture *_new();

	// methods
	void _update_proxy();
	real_t get_fps() const;
	real_t get_frame_delay(const int64_t frame) const;
	Ref<Texture> get_frame_texture(const int64_t frame) const;
	int64_t get_frames() const;
	void set_fps(const real_t fps);
	void set_frame_delay(const int64_t frame, const real_t delay);
	void set_frame_texture(const int64_t frame, const Ref<Texture> texture);
	void set_frames(const int64_t frames);

};

}

#endif