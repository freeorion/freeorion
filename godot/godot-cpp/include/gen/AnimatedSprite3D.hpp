#ifndef GODOT_CPP_ANIMATEDSPRITE3D_HPP
#define GODOT_CPP_ANIMATEDSPRITE3D_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "SpriteBase3D.hpp"
namespace godot {

class SpriteFrames;

class AnimatedSprite3D : public SpriteBase3D {
	struct ___method_bindings {
		godot_method_bind *mb__is_playing;
		godot_method_bind *mb__res_changed;
		godot_method_bind *mb__set_playing;
		godot_method_bind *mb_get_animation;
		godot_method_bind *mb_get_frame;
		godot_method_bind *mb_get_sprite_frames;
		godot_method_bind *mb_is_playing;
		godot_method_bind *mb_play;
		godot_method_bind *mb_set_animation;
		godot_method_bind *mb_set_frame;
		godot_method_bind *mb_set_sprite_frames;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimatedSprite3D"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AnimatedSprite3D *_new();

	// methods
	bool _is_playing() const;
	void _res_changed();
	void _set_playing(const bool playing);
	String get_animation() const;
	int64_t get_frame() const;
	Ref<SpriteFrames> get_sprite_frames() const;
	bool is_playing() const;
	void play(const String anim = "");
	void set_animation(const String animation);
	void set_frame(const int64_t frame);
	void set_sprite_frames(const Ref<SpriteFrames> sprite_frames);
	void stop();

};

}

#endif