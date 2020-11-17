#include "Animation.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Resource.hpp"
#include "Animation.hpp"


namespace godot {


Animation::___method_bindings Animation::___mb = {};

void Animation::___init_method_bindings() {
	___mb.mb_add_track = godot::api->godot_method_bind_get_method("Animation", "add_track");
	___mb.mb_animation_track_get_key_animation = godot::api->godot_method_bind_get_method("Animation", "animation_track_get_key_animation");
	___mb.mb_animation_track_insert_key = godot::api->godot_method_bind_get_method("Animation", "animation_track_insert_key");
	___mb.mb_animation_track_set_key_animation = godot::api->godot_method_bind_get_method("Animation", "animation_track_set_key_animation");
	___mb.mb_audio_track_get_key_end_offset = godot::api->godot_method_bind_get_method("Animation", "audio_track_get_key_end_offset");
	___mb.mb_audio_track_get_key_start_offset = godot::api->godot_method_bind_get_method("Animation", "audio_track_get_key_start_offset");
	___mb.mb_audio_track_get_key_stream = godot::api->godot_method_bind_get_method("Animation", "audio_track_get_key_stream");
	___mb.mb_audio_track_insert_key = godot::api->godot_method_bind_get_method("Animation", "audio_track_insert_key");
	___mb.mb_audio_track_set_key_end_offset = godot::api->godot_method_bind_get_method("Animation", "audio_track_set_key_end_offset");
	___mb.mb_audio_track_set_key_start_offset = godot::api->godot_method_bind_get_method("Animation", "audio_track_set_key_start_offset");
	___mb.mb_audio_track_set_key_stream = godot::api->godot_method_bind_get_method("Animation", "audio_track_set_key_stream");
	___mb.mb_bezier_track_get_key_in_handle = godot::api->godot_method_bind_get_method("Animation", "bezier_track_get_key_in_handle");
	___mb.mb_bezier_track_get_key_out_handle = godot::api->godot_method_bind_get_method("Animation", "bezier_track_get_key_out_handle");
	___mb.mb_bezier_track_get_key_value = godot::api->godot_method_bind_get_method("Animation", "bezier_track_get_key_value");
	___mb.mb_bezier_track_insert_key = godot::api->godot_method_bind_get_method("Animation", "bezier_track_insert_key");
	___mb.mb_bezier_track_interpolate = godot::api->godot_method_bind_get_method("Animation", "bezier_track_interpolate");
	___mb.mb_bezier_track_set_key_in_handle = godot::api->godot_method_bind_get_method("Animation", "bezier_track_set_key_in_handle");
	___mb.mb_bezier_track_set_key_out_handle = godot::api->godot_method_bind_get_method("Animation", "bezier_track_set_key_out_handle");
	___mb.mb_bezier_track_set_key_value = godot::api->godot_method_bind_get_method("Animation", "bezier_track_set_key_value");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("Animation", "clear");
	___mb.mb_copy_track = godot::api->godot_method_bind_get_method("Animation", "copy_track");
	___mb.mb_find_track = godot::api->godot_method_bind_get_method("Animation", "find_track");
	___mb.mb_get_length = godot::api->godot_method_bind_get_method("Animation", "get_length");
	___mb.mb_get_step = godot::api->godot_method_bind_get_method("Animation", "get_step");
	___mb.mb_get_track_count = godot::api->godot_method_bind_get_method("Animation", "get_track_count");
	___mb.mb_has_loop = godot::api->godot_method_bind_get_method("Animation", "has_loop");
	___mb.mb_method_track_get_key_indices = godot::api->godot_method_bind_get_method("Animation", "method_track_get_key_indices");
	___mb.mb_method_track_get_name = godot::api->godot_method_bind_get_method("Animation", "method_track_get_name");
	___mb.mb_method_track_get_params = godot::api->godot_method_bind_get_method("Animation", "method_track_get_params");
	___mb.mb_remove_track = godot::api->godot_method_bind_get_method("Animation", "remove_track");
	___mb.mb_set_length = godot::api->godot_method_bind_get_method("Animation", "set_length");
	___mb.mb_set_loop = godot::api->godot_method_bind_get_method("Animation", "set_loop");
	___mb.mb_set_step = godot::api->godot_method_bind_get_method("Animation", "set_step");
	___mb.mb_track_find_key = godot::api->godot_method_bind_get_method("Animation", "track_find_key");
	___mb.mb_track_get_interpolation_loop_wrap = godot::api->godot_method_bind_get_method("Animation", "track_get_interpolation_loop_wrap");
	___mb.mb_track_get_interpolation_type = godot::api->godot_method_bind_get_method("Animation", "track_get_interpolation_type");
	___mb.mb_track_get_key_count = godot::api->godot_method_bind_get_method("Animation", "track_get_key_count");
	___mb.mb_track_get_key_time = godot::api->godot_method_bind_get_method("Animation", "track_get_key_time");
	___mb.mb_track_get_key_transition = godot::api->godot_method_bind_get_method("Animation", "track_get_key_transition");
	___mb.mb_track_get_key_value = godot::api->godot_method_bind_get_method("Animation", "track_get_key_value");
	___mb.mb_track_get_path = godot::api->godot_method_bind_get_method("Animation", "track_get_path");
	___mb.mb_track_get_type = godot::api->godot_method_bind_get_method("Animation", "track_get_type");
	___mb.mb_track_insert_key = godot::api->godot_method_bind_get_method("Animation", "track_insert_key");
	___mb.mb_track_is_enabled = godot::api->godot_method_bind_get_method("Animation", "track_is_enabled");
	___mb.mb_track_is_imported = godot::api->godot_method_bind_get_method("Animation", "track_is_imported");
	___mb.mb_track_move_down = godot::api->godot_method_bind_get_method("Animation", "track_move_down");
	___mb.mb_track_move_to = godot::api->godot_method_bind_get_method("Animation", "track_move_to");
	___mb.mb_track_move_up = godot::api->godot_method_bind_get_method("Animation", "track_move_up");
	___mb.mb_track_remove_key = godot::api->godot_method_bind_get_method("Animation", "track_remove_key");
	___mb.mb_track_remove_key_at_position = godot::api->godot_method_bind_get_method("Animation", "track_remove_key_at_position");
	___mb.mb_track_set_enabled = godot::api->godot_method_bind_get_method("Animation", "track_set_enabled");
	___mb.mb_track_set_imported = godot::api->godot_method_bind_get_method("Animation", "track_set_imported");
	___mb.mb_track_set_interpolation_loop_wrap = godot::api->godot_method_bind_get_method("Animation", "track_set_interpolation_loop_wrap");
	___mb.mb_track_set_interpolation_type = godot::api->godot_method_bind_get_method("Animation", "track_set_interpolation_type");
	___mb.mb_track_set_key_time = godot::api->godot_method_bind_get_method("Animation", "track_set_key_time");
	___mb.mb_track_set_key_transition = godot::api->godot_method_bind_get_method("Animation", "track_set_key_transition");
	___mb.mb_track_set_key_value = godot::api->godot_method_bind_get_method("Animation", "track_set_key_value");
	___mb.mb_track_set_path = godot::api->godot_method_bind_get_method("Animation", "track_set_path");
	___mb.mb_track_swap = godot::api->godot_method_bind_get_method("Animation", "track_swap");
	___mb.mb_transform_track_insert_key = godot::api->godot_method_bind_get_method("Animation", "transform_track_insert_key");
	___mb.mb_transform_track_interpolate = godot::api->godot_method_bind_get_method("Animation", "transform_track_interpolate");
	___mb.mb_value_track_get_key_indices = godot::api->godot_method_bind_get_method("Animation", "value_track_get_key_indices");
	___mb.mb_value_track_get_update_mode = godot::api->godot_method_bind_get_method("Animation", "value_track_get_update_mode");
	___mb.mb_value_track_set_update_mode = godot::api->godot_method_bind_get_method("Animation", "value_track_set_update_mode");
}

Animation *Animation::_new()
{
	return (Animation *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Animation")());
}
int64_t Animation::add_track(const int64_t type, const int64_t at_position) {
	return ___godot_icall_int_int_int(___mb.mb_add_track, (const Object *) this, type, at_position);
}

String Animation::animation_track_get_key_animation(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_String_int_int(___mb.mb_animation_track_get_key_animation, (const Object *) this, track_idx, key_idx);
}

int64_t Animation::animation_track_insert_key(const int64_t track_idx, const real_t time, const String animation) {
	return ___godot_icall_int_int_float_String(___mb.mb_animation_track_insert_key, (const Object *) this, track_idx, time, animation);
}

void Animation::animation_track_set_key_animation(const int64_t track_idx, const int64_t key_idx, const String animation) {
	___godot_icall_void_int_int_String(___mb.mb_animation_track_set_key_animation, (const Object *) this, track_idx, key_idx, animation);
}

real_t Animation::audio_track_get_key_end_offset(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_float_int_int(___mb.mb_audio_track_get_key_end_offset, (const Object *) this, track_idx, key_idx);
}

real_t Animation::audio_track_get_key_start_offset(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_float_int_int(___mb.mb_audio_track_get_key_start_offset, (const Object *) this, track_idx, key_idx);
}

Ref<Resource> Animation::audio_track_get_key_stream(const int64_t track_idx, const int64_t key_idx) const {
	return Ref<Resource>::__internal_constructor(___godot_icall_Object_int_int(___mb.mb_audio_track_get_key_stream, (const Object *) this, track_idx, key_idx));
}

int64_t Animation::audio_track_insert_key(const int64_t track_idx, const real_t time, const Ref<Resource> stream, const real_t start_offset, const real_t end_offset) {
	return ___godot_icall_int_int_float_Object_float_float(___mb.mb_audio_track_insert_key, (const Object *) this, track_idx, time, stream.ptr(), start_offset, end_offset);
}

void Animation::audio_track_set_key_end_offset(const int64_t track_idx, const int64_t key_idx, const real_t offset) {
	___godot_icall_void_int_int_float(___mb.mb_audio_track_set_key_end_offset, (const Object *) this, track_idx, key_idx, offset);
}

void Animation::audio_track_set_key_start_offset(const int64_t track_idx, const int64_t key_idx, const real_t offset) {
	___godot_icall_void_int_int_float(___mb.mb_audio_track_set_key_start_offset, (const Object *) this, track_idx, key_idx, offset);
}

void Animation::audio_track_set_key_stream(const int64_t track_idx, const int64_t key_idx, const Ref<Resource> stream) {
	___godot_icall_void_int_int_Object(___mb.mb_audio_track_set_key_stream, (const Object *) this, track_idx, key_idx, stream.ptr());
}

Vector2 Animation::bezier_track_get_key_in_handle(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_bezier_track_get_key_in_handle, (const Object *) this, track_idx, key_idx);
}

Vector2 Animation::bezier_track_get_key_out_handle(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_Vector2_int_int(___mb.mb_bezier_track_get_key_out_handle, (const Object *) this, track_idx, key_idx);
}

real_t Animation::bezier_track_get_key_value(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_float_int_int(___mb.mb_bezier_track_get_key_value, (const Object *) this, track_idx, key_idx);
}

int64_t Animation::bezier_track_insert_key(const int64_t track_idx, const real_t time, const real_t value, const Vector2 in_handle, const Vector2 out_handle) {
	return ___godot_icall_int_int_float_float_Vector2_Vector2(___mb.mb_bezier_track_insert_key, (const Object *) this, track_idx, time, value, in_handle, out_handle);
}

real_t Animation::bezier_track_interpolate(const int64_t track_idx, const real_t time) const {
	return ___godot_icall_float_int_float(___mb.mb_bezier_track_interpolate, (const Object *) this, track_idx, time);
}

void Animation::bezier_track_set_key_in_handle(const int64_t track_idx, const int64_t key_idx, const Vector2 in_handle) {
	___godot_icall_void_int_int_Vector2(___mb.mb_bezier_track_set_key_in_handle, (const Object *) this, track_idx, key_idx, in_handle);
}

void Animation::bezier_track_set_key_out_handle(const int64_t track_idx, const int64_t key_idx, const Vector2 out_handle) {
	___godot_icall_void_int_int_Vector2(___mb.mb_bezier_track_set_key_out_handle, (const Object *) this, track_idx, key_idx, out_handle);
}

void Animation::bezier_track_set_key_value(const int64_t track_idx, const int64_t key_idx, const real_t value) {
	___godot_icall_void_int_int_float(___mb.mb_bezier_track_set_key_value, (const Object *) this, track_idx, key_idx, value);
}

void Animation::clear() {
	___godot_icall_void(___mb.mb_clear, (const Object *) this);
}

void Animation::copy_track(const int64_t track_idx, const Ref<Animation> to_animation) {
	___godot_icall_void_int_Object(___mb.mb_copy_track, (const Object *) this, track_idx, to_animation.ptr());
}

int64_t Animation::find_track(const NodePath path) const {
	return ___godot_icall_int_NodePath(___mb.mb_find_track, (const Object *) this, path);
}

real_t Animation::get_length() const {
	return ___godot_icall_float(___mb.mb_get_length, (const Object *) this);
}

real_t Animation::get_step() const {
	return ___godot_icall_float(___mb.mb_get_step, (const Object *) this);
}

int64_t Animation::get_track_count() const {
	return ___godot_icall_int(___mb.mb_get_track_count, (const Object *) this);
}

bool Animation::has_loop() const {
	return ___godot_icall_bool(___mb.mb_has_loop, (const Object *) this);
}

PoolIntArray Animation::method_track_get_key_indices(const int64_t track_idx, const real_t time_sec, const real_t delta) const {
	return ___godot_icall_PoolIntArray_int_float_float(___mb.mb_method_track_get_key_indices, (const Object *) this, track_idx, time_sec, delta);
}

String Animation::method_track_get_name(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_String_int_int(___mb.mb_method_track_get_name, (const Object *) this, track_idx, key_idx);
}

Array Animation::method_track_get_params(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_Array_int_int(___mb.mb_method_track_get_params, (const Object *) this, track_idx, key_idx);
}

void Animation::remove_track(const int64_t track_idx) {
	___godot_icall_void_int(___mb.mb_remove_track, (const Object *) this, track_idx);
}

void Animation::set_length(const real_t time_sec) {
	___godot_icall_void_float(___mb.mb_set_length, (const Object *) this, time_sec);
}

void Animation::set_loop(const bool enabled) {
	___godot_icall_void_bool(___mb.mb_set_loop, (const Object *) this, enabled);
}

void Animation::set_step(const real_t size_sec) {
	___godot_icall_void_float(___mb.mb_set_step, (const Object *) this, size_sec);
}

int64_t Animation::track_find_key(const int64_t track_idx, const real_t time, const bool exact) const {
	return ___godot_icall_int_int_float_bool(___mb.mb_track_find_key, (const Object *) this, track_idx, time, exact);
}

bool Animation::track_get_interpolation_loop_wrap(const int64_t track_idx) const {
	return ___godot_icall_bool_int(___mb.mb_track_get_interpolation_loop_wrap, (const Object *) this, track_idx);
}

Animation::InterpolationType Animation::track_get_interpolation_type(const int64_t track_idx) const {
	return (Animation::InterpolationType) ___godot_icall_int_int(___mb.mb_track_get_interpolation_type, (const Object *) this, track_idx);
}

int64_t Animation::track_get_key_count(const int64_t track_idx) const {
	return ___godot_icall_int_int(___mb.mb_track_get_key_count, (const Object *) this, track_idx);
}

real_t Animation::track_get_key_time(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_float_int_int(___mb.mb_track_get_key_time, (const Object *) this, track_idx, key_idx);
}

real_t Animation::track_get_key_transition(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_float_int_int(___mb.mb_track_get_key_transition, (const Object *) this, track_idx, key_idx);
}

Variant Animation::track_get_key_value(const int64_t track_idx, const int64_t key_idx) const {
	return ___godot_icall_Variant_int_int(___mb.mb_track_get_key_value, (const Object *) this, track_idx, key_idx);
}

NodePath Animation::track_get_path(const int64_t track_idx) const {
	return ___godot_icall_NodePath_int(___mb.mb_track_get_path, (const Object *) this, track_idx);
}

Animation::TrackType Animation::track_get_type(const int64_t track_idx) const {
	return (Animation::TrackType) ___godot_icall_int_int(___mb.mb_track_get_type, (const Object *) this, track_idx);
}

void Animation::track_insert_key(const int64_t track_idx, const real_t time, const Variant key, const real_t transition) {
	___godot_icall_void_int_float_Variant_float(___mb.mb_track_insert_key, (const Object *) this, track_idx, time, key, transition);
}

bool Animation::track_is_enabled(const int64_t track_idx) const {
	return ___godot_icall_bool_int(___mb.mb_track_is_enabled, (const Object *) this, track_idx);
}

bool Animation::track_is_imported(const int64_t track_idx) const {
	return ___godot_icall_bool_int(___mb.mb_track_is_imported, (const Object *) this, track_idx);
}

void Animation::track_move_down(const int64_t track_idx) {
	___godot_icall_void_int(___mb.mb_track_move_down, (const Object *) this, track_idx);
}

void Animation::track_move_to(const int64_t track_idx, const int64_t to_idx) {
	___godot_icall_void_int_int(___mb.mb_track_move_to, (const Object *) this, track_idx, to_idx);
}

void Animation::track_move_up(const int64_t track_idx) {
	___godot_icall_void_int(___mb.mb_track_move_up, (const Object *) this, track_idx);
}

void Animation::track_remove_key(const int64_t track_idx, const int64_t key_idx) {
	___godot_icall_void_int_int(___mb.mb_track_remove_key, (const Object *) this, track_idx, key_idx);
}

void Animation::track_remove_key_at_position(const int64_t track_idx, const real_t position) {
	___godot_icall_void_int_float(___mb.mb_track_remove_key_at_position, (const Object *) this, track_idx, position);
}

void Animation::track_set_enabled(const int64_t track_idx, const bool enabled) {
	___godot_icall_void_int_bool(___mb.mb_track_set_enabled, (const Object *) this, track_idx, enabled);
}

void Animation::track_set_imported(const int64_t track_idx, const bool imported) {
	___godot_icall_void_int_bool(___mb.mb_track_set_imported, (const Object *) this, track_idx, imported);
}

void Animation::track_set_interpolation_loop_wrap(const int64_t track_idx, const bool interpolation) {
	___godot_icall_void_int_bool(___mb.mb_track_set_interpolation_loop_wrap, (const Object *) this, track_idx, interpolation);
}

void Animation::track_set_interpolation_type(const int64_t track_idx, const int64_t interpolation) {
	___godot_icall_void_int_int(___mb.mb_track_set_interpolation_type, (const Object *) this, track_idx, interpolation);
}

void Animation::track_set_key_time(const int64_t track_idx, const int64_t key_idx, const real_t time) {
	___godot_icall_void_int_int_float(___mb.mb_track_set_key_time, (const Object *) this, track_idx, key_idx, time);
}

void Animation::track_set_key_transition(const int64_t track_idx, const int64_t key_idx, const real_t transition) {
	___godot_icall_void_int_int_float(___mb.mb_track_set_key_transition, (const Object *) this, track_idx, key_idx, transition);
}

void Animation::track_set_key_value(const int64_t track_idx, const int64_t key, const Variant value) {
	___godot_icall_void_int_int_Variant(___mb.mb_track_set_key_value, (const Object *) this, track_idx, key, value);
}

void Animation::track_set_path(const int64_t track_idx, const NodePath path) {
	___godot_icall_void_int_NodePath(___mb.mb_track_set_path, (const Object *) this, track_idx, path);
}

void Animation::track_swap(const int64_t track_idx, const int64_t with_idx) {
	___godot_icall_void_int_int(___mb.mb_track_swap, (const Object *) this, track_idx, with_idx);
}

int64_t Animation::transform_track_insert_key(const int64_t track_idx, const real_t time, const Vector3 location, const Quat rotation, const Vector3 scale) {
	return ___godot_icall_int_int_float_Vector3_Quat_Vector3(___mb.mb_transform_track_insert_key, (const Object *) this, track_idx, time, location, rotation, scale);
}

Array Animation::transform_track_interpolate(const int64_t track_idx, const real_t time_sec) const {
	return ___godot_icall_Array_int_float(___mb.mb_transform_track_interpolate, (const Object *) this, track_idx, time_sec);
}

PoolIntArray Animation::value_track_get_key_indices(const int64_t track_idx, const real_t time_sec, const real_t delta) const {
	return ___godot_icall_PoolIntArray_int_float_float(___mb.mb_value_track_get_key_indices, (const Object *) this, track_idx, time_sec, delta);
}

Animation::UpdateMode Animation::value_track_get_update_mode(const int64_t track_idx) const {
	return (Animation::UpdateMode) ___godot_icall_int_int(___mb.mb_value_track_get_update_mode, (const Object *) this, track_idx);
}

void Animation::value_track_set_update_mode(const int64_t track_idx, const int64_t mode) {
	___godot_icall_void_int_int(___mb.mb_value_track_set_update_mode, (const Object *) this, track_idx, mode);
}

}