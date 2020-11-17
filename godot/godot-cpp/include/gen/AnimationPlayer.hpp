#ifndef GODOT_CPP_ANIMATIONPLAYER_HPP
#define GODOT_CPP_ANIMATIONPLAYER_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "AnimationPlayer.hpp"

#include "Node.hpp"
namespace godot {

class Node;
class Animation;

class AnimationPlayer : public Node {
	struct ___method_bindings {
		godot_method_bind *mb__animation_changed;
		godot_method_bind *mb__node_removed;
		godot_method_bind *mb_add_animation;
		godot_method_bind *mb_advance;
		godot_method_bind *mb_animation_get_next;
		godot_method_bind *mb_animation_set_next;
		godot_method_bind *mb_clear_caches;
		godot_method_bind *mb_clear_queue;
		godot_method_bind *mb_find_animation;
		godot_method_bind *mb_get_animation;
		godot_method_bind *mb_get_animation_list;
		godot_method_bind *mb_get_animation_process_mode;
		godot_method_bind *mb_get_assigned_animation;
		godot_method_bind *mb_get_autoplay;
		godot_method_bind *mb_get_blend_time;
		godot_method_bind *mb_get_current_animation;
		godot_method_bind *mb_get_current_animation_length;
		godot_method_bind *mb_get_current_animation_position;
		godot_method_bind *mb_get_default_blend_time;
		godot_method_bind *mb_get_method_call_mode;
		godot_method_bind *mb_get_playing_speed;
		godot_method_bind *mb_get_queue;
		godot_method_bind *mb_get_root;
		godot_method_bind *mb_get_speed_scale;
		godot_method_bind *mb_has_animation;
		godot_method_bind *mb_is_active;
		godot_method_bind *mb_is_playing;
		godot_method_bind *mb_play;
		godot_method_bind *mb_play_backwards;
		godot_method_bind *mb_queue;
		godot_method_bind *mb_remove_animation;
		godot_method_bind *mb_rename_animation;
		godot_method_bind *mb_seek;
		godot_method_bind *mb_set_active;
		godot_method_bind *mb_set_animation_process_mode;
		godot_method_bind *mb_set_assigned_animation;
		godot_method_bind *mb_set_autoplay;
		godot_method_bind *mb_set_blend_time;
		godot_method_bind *mb_set_current_animation;
		godot_method_bind *mb_set_default_blend_time;
		godot_method_bind *mb_set_method_call_mode;
		godot_method_bind *mb_set_root;
		godot_method_bind *mb_set_speed_scale;
		godot_method_bind *mb_stop;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "AnimationPlayer"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum AnimationProcessMode {
		ANIMATION_PROCESS_PHYSICS = 0,
		ANIMATION_PROCESS_IDLE = 1,
		ANIMATION_PROCESS_MANUAL = 2,
	};
	enum AnimationMethodCallMode {
		ANIMATION_METHOD_CALL_DEFERRED = 0,
		ANIMATION_METHOD_CALL_IMMEDIATE = 1,
	};

	// constants


	static AnimationPlayer *_new();

	// methods
	void _animation_changed();
	void _node_removed(const Node *arg0);
	Error add_animation(const String name, const Ref<Animation> animation);
	void advance(const real_t delta);
	String animation_get_next(const String anim_from) const;
	void animation_set_next(const String anim_from, const String anim_to);
	void clear_caches();
	void clear_queue();
	String find_animation(const Ref<Animation> animation) const;
	Ref<Animation> get_animation(const String name) const;
	PoolStringArray get_animation_list() const;
	AnimationPlayer::AnimationProcessMode get_animation_process_mode() const;
	String get_assigned_animation() const;
	String get_autoplay() const;
	real_t get_blend_time(const String anim_from, const String anim_to) const;
	String get_current_animation() const;
	real_t get_current_animation_length() const;
	real_t get_current_animation_position() const;
	real_t get_default_blend_time() const;
	AnimationPlayer::AnimationMethodCallMode get_method_call_mode() const;
	real_t get_playing_speed() const;
	PoolStringArray get_queue();
	NodePath get_root() const;
	real_t get_speed_scale() const;
	bool has_animation(const String name) const;
	bool is_active() const;
	bool is_playing() const;
	void play(const String name = "", const real_t custom_blend = -1, const real_t custom_speed = 1, const bool from_end = false);
	void play_backwards(const String name = "", const real_t custom_blend = -1);
	void queue(const String name);
	void remove_animation(const String name);
	void rename_animation(const String name, const String newname);
	void seek(const real_t seconds, const bool update = false);
	void set_active(const bool active);
	void set_animation_process_mode(const int64_t mode);
	void set_assigned_animation(const String anim);
	void set_autoplay(const String name);
	void set_blend_time(const String anim_from, const String anim_to, const real_t sec);
	void set_current_animation(const String anim);
	void set_default_blend_time(const real_t sec);
	void set_method_call_mode(const int64_t mode);
	void set_root(const NodePath path);
	void set_speed_scale(const real_t speed);
	void stop(const bool reset = true);

};

}

#endif