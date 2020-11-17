#include "SpriteFrames.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"


#include "Texture.hpp"


namespace godot {


SpriteFrames::___method_bindings SpriteFrames::___mb = {};

void SpriteFrames::___init_method_bindings() {
	___mb.mb__get_animations = godot::api->godot_method_bind_get_method("SpriteFrames", "_get_animations");
	___mb.mb__get_frames = godot::api->godot_method_bind_get_method("SpriteFrames", "_get_frames");
	___mb.mb__set_animations = godot::api->godot_method_bind_get_method("SpriteFrames", "_set_animations");
	___mb.mb__set_frames = godot::api->godot_method_bind_get_method("SpriteFrames", "_set_frames");
	___mb.mb_add_animation = godot::api->godot_method_bind_get_method("SpriteFrames", "add_animation");
	___mb.mb_add_frame = godot::api->godot_method_bind_get_method("SpriteFrames", "add_frame");
	___mb.mb_clear = godot::api->godot_method_bind_get_method("SpriteFrames", "clear");
	___mb.mb_clear_all = godot::api->godot_method_bind_get_method("SpriteFrames", "clear_all");
	___mb.mb_get_animation_loop = godot::api->godot_method_bind_get_method("SpriteFrames", "get_animation_loop");
	___mb.mb_get_animation_names = godot::api->godot_method_bind_get_method("SpriteFrames", "get_animation_names");
	___mb.mb_get_animation_speed = godot::api->godot_method_bind_get_method("SpriteFrames", "get_animation_speed");
	___mb.mb_get_frame = godot::api->godot_method_bind_get_method("SpriteFrames", "get_frame");
	___mb.mb_get_frame_count = godot::api->godot_method_bind_get_method("SpriteFrames", "get_frame_count");
	___mb.mb_has_animation = godot::api->godot_method_bind_get_method("SpriteFrames", "has_animation");
	___mb.mb_remove_animation = godot::api->godot_method_bind_get_method("SpriteFrames", "remove_animation");
	___mb.mb_remove_frame = godot::api->godot_method_bind_get_method("SpriteFrames", "remove_frame");
	___mb.mb_rename_animation = godot::api->godot_method_bind_get_method("SpriteFrames", "rename_animation");
	___mb.mb_set_animation_loop = godot::api->godot_method_bind_get_method("SpriteFrames", "set_animation_loop");
	___mb.mb_set_animation_speed = godot::api->godot_method_bind_get_method("SpriteFrames", "set_animation_speed");
	___mb.mb_set_frame = godot::api->godot_method_bind_get_method("SpriteFrames", "set_frame");
}

SpriteFrames *SpriteFrames::_new()
{
	return (SpriteFrames *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpriteFrames")());
}
Array SpriteFrames::_get_animations() const {
	return ___godot_icall_Array(___mb.mb__get_animations, (const Object *) this);
}

Array SpriteFrames::_get_frames() const {
	return ___godot_icall_Array(___mb.mb__get_frames, (const Object *) this);
}

void SpriteFrames::_set_animations(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_animations, (const Object *) this, arg0);
}

void SpriteFrames::_set_frames(const Array arg0) {
	___godot_icall_void_Array(___mb.mb__set_frames, (const Object *) this, arg0);
}

void SpriteFrames::add_animation(const String anim) {
	___godot_icall_void_String(___mb.mb_add_animation, (const Object *) this, anim);
}

void SpriteFrames::add_frame(const String anim, const Ref<Texture> frame, const int64_t at_position) {
	___godot_icall_void_String_Object_int(___mb.mb_add_frame, (const Object *) this, anim, frame.ptr(), at_position);
}

void SpriteFrames::clear(const String anim) {
	___godot_icall_void_String(___mb.mb_clear, (const Object *) this, anim);
}

void SpriteFrames::clear_all() {
	___godot_icall_void(___mb.mb_clear_all, (const Object *) this);
}

bool SpriteFrames::get_animation_loop(const String anim) const {
	return ___godot_icall_bool_String(___mb.mb_get_animation_loop, (const Object *) this, anim);
}

PoolStringArray SpriteFrames::get_animation_names() const {
	return ___godot_icall_PoolStringArray(___mb.mb_get_animation_names, (const Object *) this);
}

real_t SpriteFrames::get_animation_speed(const String anim) const {
	return ___godot_icall_float_String(___mb.mb_get_animation_speed, (const Object *) this, anim);
}

Ref<Texture> SpriteFrames::get_frame(const String anim, const int64_t idx) const {
	return Ref<Texture>::__internal_constructor(___godot_icall_Object_String_int(___mb.mb_get_frame, (const Object *) this, anim, idx));
}

int64_t SpriteFrames::get_frame_count(const String anim) const {
	return ___godot_icall_int_String(___mb.mb_get_frame_count, (const Object *) this, anim);
}

bool SpriteFrames::has_animation(const String anim) const {
	return ___godot_icall_bool_String(___mb.mb_has_animation, (const Object *) this, anim);
}

void SpriteFrames::remove_animation(const String anim) {
	___godot_icall_void_String(___mb.mb_remove_animation, (const Object *) this, anim);
}

void SpriteFrames::remove_frame(const String anim, const int64_t idx) {
	___godot_icall_void_String_int(___mb.mb_remove_frame, (const Object *) this, anim, idx);
}

void SpriteFrames::rename_animation(const String anim, const String newname) {
	___godot_icall_void_String_String(___mb.mb_rename_animation, (const Object *) this, anim, newname);
}

void SpriteFrames::set_animation_loop(const String anim, const bool loop) {
	___godot_icall_void_String_bool(___mb.mb_set_animation_loop, (const Object *) this, anim, loop);
}

void SpriteFrames::set_animation_speed(const String anim, const real_t speed) {
	___godot_icall_void_String_float(___mb.mb_set_animation_speed, (const Object *) this, anim, speed);
}

void SpriteFrames::set_frame(const String anim, const int64_t idx, const Ref<Texture> txt) {
	___godot_icall_void_String_int_Object(___mb.mb_set_frame, (const Object *) this, anim, idx, txt.ptr());
}

}