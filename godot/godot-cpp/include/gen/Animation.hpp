#ifndef GODOT_CPP_ANIMATION_HPP
#define GODOT_CPP_ANIMATION_HPP


#include <gdnative_api_struct.gen.h>
#include <stdint.h>

#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include "Animation.hpp"

#include "Resource.hpp"
namespace godot {

class Resource;
class Animation;

class Animation : public Resource {
	struct ___method_bindings {
		godot_method_bind *mb_add_track;
		godot_method_bind *mb_animation_track_get_key_animation;
		godot_method_bind *mb_animation_track_insert_key;
		godot_method_bind *mb_animation_track_set_key_animation;
		godot_method_bind *mb_audio_track_get_key_end_offset;
		godot_method_bind *mb_audio_track_get_key_start_offset;
		godot_method_bind *mb_audio_track_get_key_stream;
		godot_method_bind *mb_audio_track_insert_key;
		godot_method_bind *mb_audio_track_set_key_end_offset;
		godot_method_bind *mb_audio_track_set_key_start_offset;
		godot_method_bind *mb_audio_track_set_key_stream;
		godot_method_bind *mb_bezier_track_get_key_in_handle;
		godot_method_bind *mb_bezier_track_get_key_out_handle;
		godot_method_bind *mb_bezier_track_get_key_value;
		godot_method_bind *mb_bezier_track_insert_key;
		godot_method_bind *mb_bezier_track_interpolate;
		godot_method_bind *mb_bezier_track_set_key_in_handle;
		godot_method_bind *mb_bezier_track_set_key_out_handle;
		godot_method_bind *mb_bezier_track_set_key_value;
		godot_method_bind *mb_clear;
		godot_method_bind *mb_copy_track;
		godot_method_bind *mb_find_track;
		godot_method_bind *mb_get_length;
		godot_method_bind *mb_get_step;
		godot_method_bind *mb_get_track_count;
		godot_method_bind *mb_has_loop;
		godot_method_bind *mb_method_track_get_key_indices;
		godot_method_bind *mb_method_track_get_name;
		godot_method_bind *mb_method_track_get_params;
		godot_method_bind *mb_remove_track;
		godot_method_bind *mb_set_length;
		godot_method_bind *mb_set_loop;
		godot_method_bind *mb_set_step;
		godot_method_bind *mb_track_find_key;
		godot_method_bind *mb_track_get_interpolation_loop_wrap;
		godot_method_bind *mb_track_get_interpolation_type;
		godot_method_bind *mb_track_get_key_count;
		godot_method_bind *mb_track_get_key_time;
		godot_method_bind *mb_track_get_key_transition;
		godot_method_bind *mb_track_get_key_value;
		godot_method_bind *mb_track_get_path;
		godot_method_bind *mb_track_get_type;
		godot_method_bind *mb_track_insert_key;
		godot_method_bind *mb_track_is_enabled;
		godot_method_bind *mb_track_is_imported;
		godot_method_bind *mb_track_move_down;
		godot_method_bind *mb_track_move_to;
		godot_method_bind *mb_track_move_up;
		godot_method_bind *mb_track_remove_key;
		godot_method_bind *mb_track_remove_key_at_position;
		godot_method_bind *mb_track_set_enabled;
		godot_method_bind *mb_track_set_imported;
		godot_method_bind *mb_track_set_interpolation_loop_wrap;
		godot_method_bind *mb_track_set_interpolation_type;
		godot_method_bind *mb_track_set_key_time;
		godot_method_bind *mb_track_set_key_transition;
		godot_method_bind *mb_track_set_key_value;
		godot_method_bind *mb_track_set_path;
		godot_method_bind *mb_track_swap;
		godot_method_bind *mb_transform_track_insert_key;
		godot_method_bind *mb_transform_track_interpolate;
		godot_method_bind *mb_value_track_get_key_indices;
		godot_method_bind *mb_value_track_get_update_mode;
		godot_method_bind *mb_value_track_set_update_mode;
	};
	static ___method_bindings ___mb;

public:
	static void ___init_method_bindings();

	static inline const char *___get_class_name() { return (const char *) "Animation"; }
	static inline Object *___get_from_variant(Variant a) { godot_object *o = (godot_object*) a; return (o) ? (Object *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, o) : nullptr; }

	// enums
	enum TrackType {
		TYPE_VALUE = 0,
		TYPE_TRANSFORM = 1,
		TYPE_METHOD = 2,
		TYPE_BEZIER = 3,
		TYPE_AUDIO = 4,
		TYPE_ANIMATION = 5,
	};
	enum UpdateMode {
		UPDATE_CONTINUOUS = 0,
		UPDATE_DISCRETE = 1,
		UPDATE_TRIGGER = 2,
		UPDATE_CAPTURE = 3,
	};
	enum InterpolationType {
		INTERPOLATION_NEAREST = 0,
		INTERPOLATION_LINEAR = 1,
		INTERPOLATION_CUBIC = 2,
	};

	// constants


	static Animation *_new();

	// methods
	int64_t add_track(const int64_t type, const int64_t at_position = -1);
	String animation_track_get_key_animation(const int64_t track_idx, const int64_t key_idx) const;
	int64_t animation_track_insert_key(const int64_t track_idx, const real_t time, const String animation);
	void animation_track_set_key_animation(const int64_t track_idx, const int64_t key_idx, const String animation);
	real_t audio_track_get_key_end_offset(const int64_t track_idx, const int64_t key_idx) const;
	real_t audio_track_get_key_start_offset(const int64_t track_idx, const int64_t key_idx) const;
	Ref<Resource> audio_track_get_key_stream(const int64_t track_idx, const int64_t key_idx) const;
	int64_t audio_track_insert_key(const int64_t track_idx, const real_t time, const Ref<Resource> stream, const real_t start_offset = 0, const real_t end_offset = 0);
	void audio_track_set_key_end_offset(const int64_t track_idx, const int64_t key_idx, const real_t offset);
	void audio_track_set_key_start_offset(const int64_t track_idx, const int64_t key_idx, const real_t offset);
	void audio_track_set_key_stream(const int64_t track_idx, const int64_t key_idx, const Ref<Resource> stream);
	Vector2 bezier_track_get_key_in_handle(const int64_t track_idx, const int64_t key_idx) const;
	Vector2 bezier_track_get_key_out_handle(const int64_t track_idx, const int64_t key_idx) const;
	real_t bezier_track_get_key_value(const int64_t track_idx, const int64_t key_idx) const;
	int64_t bezier_track_insert_key(const int64_t track_idx, const real_t time, const real_t value, const Vector2 in_handle = Vector2(0, 0), const Vector2 out_handle = Vector2(0, 0));
	real_t bezier_track_interpolate(const int64_t track_idx, const real_t time) const;
	void bezier_track_set_key_in_handle(const int64_t track_idx, const int64_t key_idx, const Vector2 in_handle);
	void bezier_track_set_key_out_handle(const int64_t track_idx, const int64_t key_idx, const Vector2 out_handle);
	void bezier_track_set_key_value(const int64_t track_idx, const int64_t key_idx, const real_t value);
	void clear();
	void copy_track(const int64_t track_idx, const Ref<Animation> to_animation);
	int64_t find_track(const NodePath path) const;
	real_t get_length() const;
	real_t get_step() const;
	int64_t get_track_count() const;
	bool has_loop() const;
	PoolIntArray method_track_get_key_indices(const int64_t track_idx, const real_t time_sec, const real_t delta) const;
	String method_track_get_name(const int64_t track_idx, const int64_t key_idx) const;
	Array method_track_get_params(const int64_t track_idx, const int64_t key_idx) const;
	void remove_track(const int64_t track_idx);
	void set_length(const real_t time_sec);
	void set_loop(const bool enabled);
	void set_step(const real_t size_sec);
	int64_t track_find_key(const int64_t track_idx, const real_t time, const bool exact = false) const;
	bool track_get_interpolation_loop_wrap(const int64_t track_idx) const;
	Animation::InterpolationType track_get_interpolation_type(const int64_t track_idx) const;
	int64_t track_get_key_count(const int64_t track_idx) const;
	real_t track_get_key_time(const int64_t track_idx, const int64_t key_idx) const;
	real_t track_get_key_transition(const int64_t track_idx, const int64_t key_idx) const;
	Variant track_get_key_value(const int64_t track_idx, const int64_t key_idx) const;
	NodePath track_get_path(const int64_t track_idx) const;
	Animation::TrackType track_get_type(const int64_t track_idx) const;
	void track_insert_key(const int64_t track_idx, const real_t time, const Variant key, const real_t transition = 1);
	bool track_is_enabled(const int64_t track_idx) const;
	bool track_is_imported(const int64_t track_idx) const;
	void track_move_down(const int64_t track_idx);
	void track_move_to(const int64_t track_idx, const int64_t to_idx);
	void track_move_up(const int64_t track_idx);
	void track_remove_key(const int64_t track_idx, const int64_t key_idx);
	void track_remove_key_at_position(const int64_t track_idx, const real_t position);
	void track_set_enabled(const int64_t track_idx, const bool enabled);
	void track_set_imported(const int64_t track_idx, const bool imported);
	void track_set_interpolation_loop_wrap(const int64_t track_idx, const bool interpolation);
	void track_set_interpolation_type(const int64_t track_idx, const int64_t interpolation);
	void track_set_key_time(const int64_t track_idx, const int64_t key_idx, const real_t time);
	void track_set_key_transition(const int64_t track_idx, const int64_t key_idx, const real_t transition);
	void track_set_key_value(const int64_t track_idx, const int64_t key, const Variant value);
	void track_set_path(const int64_t track_idx, const NodePath path);
	void track_swap(const int64_t track_idx, const int64_t with_idx);
	int64_t transform_track_insert_key(const int64_t track_idx, const real_t time, const Vector3 location, const Quat rotation, const Vector3 scale);
	Array transform_track_interpolate(const int64_t track_idx, const real_t time_sec) const;
	PoolIntArray value_track_get_key_indices(const int64_t track_idx, const real_t time_sec, const real_t delta) const;
	Animation::UpdateMode value_track_get_update_mode(const int64_t track_idx) const;
	void value_track_set_update_mode(const int64_t track_idx, const int64_t mode);

};

}

#endif