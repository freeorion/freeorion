#include "AnimatedSprite.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "SpriteFrames.hpp"


namespace godot {


AnimatedSprite::___method_bindings AnimatedSprite::___mb = {};

void AnimatedSprite::___init_method_bindings() {
	___mb.mb__is_playing = godot::api->godot_method_bind_get_method("AnimatedSprite", "_is_playing");
	___mb.mb__res_changed = godot::api->godot_method_bind_get_method("AnimatedSprite", "_res_changed");
	___mb.mb__set_playing = godot::api->godot_method_bind_get_method("AnimatedSprite", "_set_playing");
	___mb.mb_get_animation = godot::api->godot_method_bind_get_method("AnimatedSprite", "get_animation");
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("AnimatedSprite", "get_frame");
	___mb.mb_get_offset = godot::api->godot_method_bind_get_method("AnimatedSprite", "get_offset");
	___mb.mb_get_speed_scale = godot::api->godot_method_bind_get_method("AnimatedSprite", "get_speed_scale");
	___mb.mb_get_sprite_frames = godot::api->godot_method_bind_get_method("AnimatedSprite", "get_sprite_frames");
	___mb.mb_is_centered = godot::api->godot_method_bind_get_method("AnimatedSprite", "is_centered");
	___mb.mb_is_flipped_h = godot::api->godot_method_bind_get_method("AnimatedSprite", "is_flipped_h");
	___mb.mb_is_flipped_v = godot::api->godot_method_bind_get_method("AnimatedSprite", "is_flipped_v");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("AnimatedSprite", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("AnimatedSprite", "play");
	___mb.mb_set_animation = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_animation");
	___mb.mb_set_centered = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_centered");
	___mb.mb_set_flip_h = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_flip_h");
	___mb.mb_set_flip_v = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_flip_v");
	___mb.mb_set_frame = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_frame");
	___mb.mb_set_offset = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_offset");
	___mb.mb_set_speed_scale = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_speed_scale");
	___mb.mb_set_sprite_frames = godot::api->godot_method_bind_get_method("AnimatedSprite", "set_sprite_frames");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("AnimatedSprite", "stop");
}

AnimatedSprite *AnimatedSprite::_new()
{
	return (AnimatedSprite *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimatedSprite")());
}
bool AnimatedSprite::_is_playing() const {
	return ___godot_icall_bool(___mb.mb__is_playing, (const Object *) this);
}

void AnimatedSprite::_res_changed() {
	___godot_icall_void(___mb.mb__res_changed, (const Object *) this);
}

void AnimatedSprite::_set_playing(const bool playing) {
	___godot_icall_void_bool(___mb.mb__set_playing, (const Object *) this, playing);
}

String AnimatedSprite::get_animation() const {
	return ___godot_icall_String(___mb.mb_get_animation, (const Object *) this);
}

int64_t AnimatedSprite::get_frame() const {
	return ___godot_icall_int(___mb.mb_get_frame, (const Object *) this);
}

Vector2 AnimatedSprite::get_offset() const {
	return ___godot_icall_Vector2(___mb.mb_get_offset, (const Object *) this);
}

real_t AnimatedSprite::get_speed_scale() const {
	return ___godot_icall_float(___mb.mb_get_speed_scale, (const Object *) this);
}

Ref<SpriteFrames> AnimatedSprite::get_sprite_frames() const {
	return Ref<SpriteFrames>::__internal_constructor(___godot_icall_Object(___mb.mb_get_sprite_frames, (const Object *) this));
}

bool AnimatedSprite::is_centered() const {
	return ___godot_icall_bool(___mb.mb_is_centered, (const Object *) this);
}

bool AnimatedSprite::is_flipped_h() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_h, (const Object *) this);
}

bool AnimatedSprite::is_flipped_v() const {
	return ___godot_icall_bool(___mb.mb_is_flipped_v, (const Object *) this);
}

bool AnimatedSprite::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void AnimatedSprite::play(const String anim, const bool backwards) {
	___godot_icall_void_String_bool(___mb.mb_play, (const Object *) this, anim, backwards);
}

void AnimatedSprite::set_animation(const String animation) {
	___godot_icall_void_String(___mb.mb_set_animation, (const Object *) this, animation);
}

void AnimatedSprite::set_centered(const bool centered) {
	___godot_icall_void_bool(___mb.mb_set_centered, (const Object *) this, centered);
}

void AnimatedSprite::set_flip_h(const bool flip_h) {
	___godot_icall_void_bool(___mb.mb_set_flip_h, (const Object *) this, flip_h);
}

void AnimatedSprite::set_flip_v(const bool flip_v) {
	___godot_icall_void_bool(___mb.mb_set_flip_v, (const Object *) this, flip_v);
}

void AnimatedSprite::set_frame(const int64_t frame) {
	___godot_icall_void_int(___mb.mb_set_frame, (const Object *) this, frame);
}

void AnimatedSprite::set_offset(const Vector2 offset) {
	___godot_icall_void_Vector2(___mb.mb_set_offset, (const Object *) this, offset);
}

void AnimatedSprite::set_speed_scale(const real_t speed_scale) {
	___godot_icall_void_float(___mb.mb_set_speed_scale, (const Object *) this, speed_scale);
}

void AnimatedSprite::set_sprite_frames(const Ref<SpriteFrames> sprite_frames) {
	___godot_icall_void_Object(___mb.mb_set_sprite_frames, (const Object *) this, sprite_frames.ptr());
}

void AnimatedSprite::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}