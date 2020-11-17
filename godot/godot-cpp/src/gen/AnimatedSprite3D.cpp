#include "AnimatedSprite3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "SpriteFrames.hpp"


namespace godot {


AnimatedSprite3D::___method_bindings AnimatedSprite3D::___mb = {};

void AnimatedSprite3D::___init_method_bindings() {
	___mb.mb__is_playing = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "_is_playing");
	___mb.mb__res_changed = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "_res_changed");
	___mb.mb__set_playing = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "_set_playing");
	___mb.mb_get_animation = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "get_animation");
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "get_frame");
	___mb.mb_get_sprite_frames = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "get_sprite_frames");
	___mb.mb_is_playing = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "is_playing");
	___mb.mb_play = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "play");
	___mb.mb_set_animation = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "set_animation");
	___mb.mb_set_frame = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "set_frame");
	___mb.mb_set_sprite_frames = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "set_sprite_frames");
	___mb.mb_stop = godot::api->godot_method_bind_get_method("AnimatedSprite3D", "stop");
}

AnimatedSprite3D *AnimatedSprite3D::_new()
{
	return (AnimatedSprite3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimatedSprite3D")());
}
bool AnimatedSprite3D::_is_playing() const {
	return ___godot_icall_bool(___mb.mb__is_playing, (const Object *) this);
}

void AnimatedSprite3D::_res_changed() {
	___godot_icall_void(___mb.mb__res_changed, (const Object *) this);
}

void AnimatedSprite3D::_set_playing(const bool playing) {
	___godot_icall_void_bool(___mb.mb__set_playing, (const Object *) this, playing);
}

String AnimatedSprite3D::get_animation() const {
	return ___godot_icall_String(___mb.mb_get_animation, (const Object *) this);
}

int64_t AnimatedSprite3D::get_frame() const {
	return ___godot_icall_int(___mb.mb_get_frame, (const Object *) this);
}

Ref<SpriteFrames> AnimatedSprite3D::get_sprite_frames() const {
	return Ref<SpriteFrames>::__internal_constructor(___godot_icall_Object(___mb.mb_get_sprite_frames, (const Object *) this));
}

bool AnimatedSprite3D::is_playing() const {
	return ___godot_icall_bool(___mb.mb_is_playing, (const Object *) this);
}

void AnimatedSprite3D::play(const String anim) {
	___godot_icall_void_String(___mb.mb_play, (const Object *) this, anim);
}

void AnimatedSprite3D::set_animation(const String animation) {
	___godot_icall_void_String(___mb.mb_set_animation, (const Object *) this, animation);
}

void AnimatedSprite3D::set_frame(const int64_t frame) {
	___godot_icall_void_int(___mb.mb_set_frame, (const Object *) this, frame);
}

void AnimatedSprite3D::set_sprite_frames(const Ref<SpriteFrames> sprite_frames) {
	___godot_icall_void_Object(___mb.mb_set_sprite_frames, (const Object *) this, sprite_frames.ptr());
}

void AnimatedSprite3D::stop() {
	___godot_icall_void(___mb.mb_stop, (const Object *) this);
}

}