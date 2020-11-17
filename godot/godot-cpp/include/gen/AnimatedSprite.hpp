#ifndef GODOT_CPP_ANIMATEDSPRITE_HPP
#define GODOT_CPP_ANIMATEDSPRITE_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Node2D.hpp"
namespace godot {

class SpriteFrames;

class AnimatedSprite : public Node2D {
	struct ___method_bindings {
		godot_method_bind *mb__is_playing;
		godot_method_bind *mb__res_changed;
		godot_method_bind *mb__set_playing;
		godot_method_bind *mb_get_animation;
		godot_method_bind *mb_get_frame;
		godot_method_bind *mb_get_offset;
		godot_method_bind *mb_get_speed_scale;
		godot_method_bind *mb_get_sprite_frames;
		godot_method_bind *mb_is_centered;
		godot_method_bind *mb_is_flipped_h;
		godot_method_bind *mb_is_flipped_v;
		godot_method_bind *mb_is_playing;
		godot_method_bind *mb_play;
		godot_method_bind *mb_set_animation;
		godot_method_bind *mb_set_centered;
		godot_method_bind *mb_set_flip_h;
		godot_method_bind *mb_set_flip_v;
		godot_method_bind *mb_set_frame;
		godot_method_bind *mb_set_offset;
		godot_method_bind *mb_set_speed_scale;
		godot_method_bind *mb_set_sprite_frames;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimatedSprite"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static AnimatedSprite *_new();

	// methods
	bool _is_playing() const;
	void _res_changed();
	void _set_playing(const bool playing);
	String get_animation() const;
	int64_t get_frame() const;
	Vector2 get_offset() const;
	real_t get_speed_scale() const;
	Ref<SpriteFrames> get_sprite_frames() const;
	bool is_centered() const;
	bool is_flipped_h() const;
	bool is_flipped_v() const;
	bool is_playing() const;
	void play(const String anim = "", const bool backwards = false);
	void set_animation(const String animation);
	void set_centered(const bool centered);
	void set_flip_h(const bool flip_h);
	void set_flip_v(const bool flip_v);
	void set_frame(const int64_t frame);
	void set_offset(const Vector2 offset);
	void set_speed_scale(const real_t speed_scale);
	void set_sprite_frames(const Ref<SpriteFrames> sprite_frames);
	void stop();

};

}

#endif