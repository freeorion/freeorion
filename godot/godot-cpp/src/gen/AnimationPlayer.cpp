#include "AnimationPlayer.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Node.hpp"
#include "Animation.hpp"


namespace godot {


AnimationPlayer::___method_bindings AnimationPlayer::___mb = {};

void AnimationPlayer::___init_method_bindings() {
	___mb.mb__animation_changed = godot::api->godot_method_bind_get_method("AnimationPlayer", "_animation_changed");
	___mb.mb__node_removed = godot::api->godot_method_bind_get_method("AnimationPlayer", "_node_removed");
	___mb.mb_add_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "add_animation");
	___mb.mb_advance = godot::api->godot_method_bind_get_method("AnimationPlayer", "advance");
	___mb.mb_animation_get_next = godot::api->godot_method_bind_get_method("AnimationPlayer", "animation_get_next");
	___mb.mb_animation_set_next = godot::api->godot_method_bind_get_method("AnimationPlayer", "animation_set_next");
	___mb.mb_clear_caches = godot::api->godot_method_bind_get_method("AnimationPlayer", "clear_caches");
	___mb.mb_clear_queue = godot::api->godot_method_bind_get_method("AnimationPlayer", "clear_queue");
	___mb.mb_find_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "find_animation");
	___mb.mb_get_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_animation");
	___mb.mb_get_animation_list = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_animation_list");
	___mb.mb_get_animation_process_mode = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_animation_process_mode");
	___mb.mb_get_assigned_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_assigned_animation");
	___mb.mb_get_autoplay = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_autoplay");
	___mb.mb_get_blend_time = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_blend_time");
	___mb.mb_get_current_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_current_animation");
	___mb.mb_get_current_animation_length = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_current_animation_length");
	___mb.mb_get_current_animation_position = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_current_animation_position");
	___mb.mb_get_default_blend_time = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_default_blend_time");
	___mb.mb_get_method_call_mode = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_method_call_mode");
	___mb.mb_get_playing_speed = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_playing_speed");
	___mb.mb_get_queue = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_queue");
	___mb.mb_get_root = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_root");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("AnimationPlayer", "get_speed_scale");
	___mb.mb_has_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "has_animation");
	___mb.mb_is_active = godot::api->godot_method_bind_get_method("AnimationPlayer", "is_active");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("AnimationPlayer", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("AnimationPlayer", "play");
	___mb.mb_play_backwards = godot::api->godot_method_bind_get_method("AnimationPlayer", "play_backwards");
	___mb.mb_queue = godot::api->godot_method_bind_get_method("AnimationPlayer", "queue");
	___mb.mb_remove_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "remove_animation");
	___mb.mb_rename_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "rename_animation");
	___mb.mb_seek = godot::api->godot_method_bind_get_method("AnimationPlayer", "seek");
	___mb.mb_set_active = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_active");
	___mb.mb_set_animation_process_mode = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_animation_process_mode");
	___mb.mb_set_assigned_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_assigned_animation");
	___mb.mb_set_autoplay = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_autoplay");
	___mb.mb_set_blend_time = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_blend_time");
	___mb.mb_set_current_animation = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_current_animation");
	___mb.mb_set_default_blend_time = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_default_blend_time");
	___mb.mb_set_method_call_mode = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_method_call_mode");
	___mb.mb_set_root = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_root");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("AnimationPlayer", "set_speed_scale");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("AnimationPlayer", "stop");
}

AnimationPlayer *AnimationPlayer::_new()
{
	return (AnimationPlayer *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationPlayer")());
}
void AnimationPlayer::_animation_changed() {
	___godot_icall_void(___mb.mb__animation_changed, (const Object *) this);
}

void AnimationPlayer::_node_removed(const Node *arg0) {
	___godot_icall_void_Object(___mb.mb__node_removed, (const Object *) this, arg0);
}

Error AnimationPlayer::add_animation(const String name, const Ref<Animation> animation) {
	return (Error) ___godot_icall_int_String_Object(___mb.mb_add_animation, (const Object *) this, name, animation.ptr());
}

void AnimationPlayer::advance(const real_t delta) {
	___godot_icall_void_float(___mb.mb_advance, (const Object *) this, delta);
}

String AnimationPlayer::animation_get_next(const String anim_from) const {
	return ___godot_icall_String_String(___mb.mb_animation_get_next, (const Object *) this, anim_from);
}

void AnimationPlayer::animation_set_next(const String anim_from, const String anim_to) {
	___godot_icall_void_String_String(___mb.mb_animation_set_next, (const Object *) this, anim_from, anim_to);
}

void AnimationPlayer::clear_caches() {
	___godot_icall_void(___mb.mb_clear_caches, (const Object *) this);
}

void AnimationPlayer::clear_queue() {
	___godot_icall_void(___mb.mb_clear_queue, (const Object *) this);
}

String AnimationPlayer::find_animation(const Ref<Animation> animation) const {
	return ___godot_icall_String_Object(___mb.mb_find_animation, (const Object *) this, animation.ptr());
}

Ref<Animation> AnimationPlayer::get_animation(const String name) const {
	return Ref<Animation>::__internal_constructor(___godot_icall_Object_String(___mb.mb_get_animation, (const Object *) this, name));
}

PoolStringArray AnimationPlayer::get_animation_list() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_animation_list, (const Object *) this);
}

AnimationPlayer::AnimationProcessMode AnimationPlayer::get_animation_process_mode() const {
	return (AnimationPlayer::AnimationProcessMode) ___godot_icall_int(___mb.mb_get_animation_process_mode, (const Object *) this);
}

String AnimationPlayer::get_assigned_animation() const {
	return ___godot_icall_String(___mb.mb_get_assigned_animation, (const Object *) this);
}

String AnimationPlayer::get_autoplay() const {
	return ___godot_icall_String(___mb.mb_get_autoplay, (const Object *) this);
}

real_t AnimationPlayer::get_blend_time(const String anim_from, const String anim_to) const {
	return ___godot_icall_float_String_String(___mb.mb_get_blend_time, (const Object *) this, anim_from, anim_to);
}

String AnimationPlayer::get_current_animation() const {
	return ___godot_icall_String(___mb.mb_get_current_animation, (const Object *) this);
}

real_t AnimationPlayer::get_current_animation_length() const {
	return ___godot_icall_float(___mb.mb_get_current_animation_length, (const Object *) this);
}

real_t AnimationPlayer::get_current_animation_position() const {
	return ___godot_icall_float(___mb.mb_get_current_animation_position, (const Object *) this);
}

real_t AnimationPlayer::get_default_blend_time() const {
	return ___godot_icall_float(___mb.mb_get_default_blend_time, (const Object *) this);
}

AnimationPlayer::AnimationMethodCallMode AnimationPlayer::get_method_call_mode() const {
	return (AnimationPlayer::AnimationMethodCallMode) ___godot_icall_int(___mb.mb_get_method_call_mode, (const Object *) this);
}

real_t AnimationPlayer::get_playing_speed() const {
	return ___godot_icall_float(___mb.mb_get_playing_speed, (const Object *) this);
}

PoolStringArray AnimationPlayer::get_queue() {
	return ___godot_icall_PoolStringArray(___mb.mb_get_queue, (const Object *) this);
}

NodePath AnimationPlayer::get_root() const {
	return ___godot_icall_NodePath(___mb.mb_get_root, (const Object *) this);
}

real_t AnimationPlayer::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

bool AnimationPlayer::has_animation(const String name) const {
	return ___godot_icall_bool_String(___mb.mb_has_animation, (const Object *) this, name);
}

bool AnimationPlayer::is_active() const {
	return ___godot_icall_bool(___mb.mb_is_active, (const Object *) this);
}

bool AnimationPlayer::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void AnimationPlayer::play(const String name, const real_t custom_blend, const real_t custom_speed, const bool from_end) {
	___godot_icall_void_String_float_float_bool(___mb.mb_play, (const Object *) this, name, custom_blend, custom_speed, from_end);
}

void AnimationPlayer::play_backwards(const String name, const real_t custom_blend) {
	___godot_icall_void_String_float(___mb.mb_play_backwards, (const Object *) this, name, custom_blend);
}

void AnimationPlayer::queue(const String name) {
	___godot_icall_void_String(___mb.mb_queue, (const Object *) this, name);
}

void AnimationPlayer::remove_animation(const String name) {
	___godot_icall_void_String(___mb.mb_remove_animation, (const Object *) this, name);
}

void AnimationPlayer::rename_animation(const String name, const String newname) {
	___godot_icall_void_String_String(___mb.mb_rename_animation, (const Object *) this, name, newname);
}

void AnimationPlayer::seek(const real_t seconds, const bool update) {
	___godot_icall_void_float_bool(___mb.mb_seek, (const Object *) this, seconds, update);
}

void AnimationPlayer::set_active(const bool active) {
	___godot_icall_void_bool(___mb.mb_set_active, (const Object *) this, active);
}

void AnimationPlayer::set_animation_process_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_animation_process_mode, (const Object *) this, mode);
}

void AnimationPlayer::set_assigned_animation(const String anim) {
	___godot_icall_void_String(___mb.mb_set_assigned_animation, (const Object *) this, anim);
}

void AnimationPlayer::set_autoplay(const String name) {
	___godot_icall_void_String(___mb.mb_set_autoplay, (const Object *) this, name);
}

void AnimationPlayer::set_blend_time(const String anim_from, const String anim_to, const real_t sec) {
	___godot_icall_void_String_String_float(___mb.mb_set_blend_time, (const Object *) this, anim_from, anim_to, sec);
}

void AnimationPlayer::set_current_animation(const String anim) {
	___godot_icall_void_String(___mb.mb_set_current_animation, (const Object *) this, anim);
}

void AnimationPlayer::set_default_blend_time(const real_t sec) {
	___godot_icall_void_float(___mb.mb_set_default_blend_time, (const Object *) this, sec);
}

void AnimationPlayer::set_method_call_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_method_call_mode, (const Object *) this, mode);
}

void AnimationPlayer::set_root(const NodePath path) {
	___godot_icall_void_NodePath(___mb.mb_set_root, (const Object *) this, path);
}

void AnimationPlayer::set_speed_scale(const real_t speed) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, speed);
}

void AnimationPlayer::stop(const bool reset) {
	___godot_icall_void_bool(___mb.mb_stop, (const Object *) this, reset);
}

}