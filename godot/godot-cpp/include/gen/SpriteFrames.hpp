#ifndef GODOT_CPP_SPRITEFRAMES_HPP
#define GODOT_CPP_SPRITEFRAMES_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>

#include "Resource.hpp"
namespace godot {

class Texture;

class SpriteFrames : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb__get_animations;
		godot_method_bind *mb__get_frames;
		godot_method_bind *mb__set_animations;
		godot_method_bind *mb__set_frames;
		godot_method_bind *mb_add_animation;
		godot_method_bind *mb_add_frame;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_clear_all;
		godot_method_bind *mb_get_animation_loop;
		godot_method_bind *mb_get_animation_names;
		godot_method_bind *mb_get_animation_speed;
		godot_method_bind *mb_get_frame;
		godot_method_bind *mb_get_frame_count;
		godot_method_bind *mb_has_animation;
		godot_method_bind *mb_remove_animation;
		godot_method_bind *mb_remove_frame;
		godot_method_bind *mb_rename_animation;
		godot_method_bind *mb_set_animation_loop;
		godot_method_bind *mb_set_animation_speed;
		godot_method_bind *mb_set_frame;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "SpriteFrames"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums

	// constants


	static SpriteFrames *_new();

	// methods
	Array _get_animations() const;
	Array _get_frames() const;
	void _set_animations(const Array arg0);
	void _set_frames(const Array arg0);
	void add_animation(const String anim);
	void add_frame(const String anim, const Ref<Texture> frame, const int64_t at_position = -1);
	void clear(const String anim);
	void clear_all();
	bool get_animation_loop(const String anim) const;
	PoolStringArray get_animation_names() const;
	real_t get_animation_speed(const String anim) const;
	Ref<Texture> get_frame(const String anim, const int64_t idx) const;
	int64_t get_frame_count(const String anim) const;
	bool has_animation(const String anim) const;
	void remove_animation(const String anim);
	void remove_frame(const String anim, const int64_t idx);
	void rename_animation(const String anim, const String newname);
	void set_animation_loop(const String anim, const bool loop);
	void set_animation_speed(const String anim, const real_t speed);
	void set_frame(const String anim, const int64_t idx, const Ref<Texture> txt);

};

}

#endif