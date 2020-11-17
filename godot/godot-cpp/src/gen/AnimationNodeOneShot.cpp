#include "AnimationNodeOneShot.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AnimationNodeOneShot::___method_bindings AnimationNodeOneShot::___mb = {};

void AnimationNodeOneShot::___init_method_bindings() {
	___mb.mb_get_autorestart_delay = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "get_autorestart_delay");
	___mb.mb_get_autorestart_random_delay = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "get_autorestart_random_delay");
	___mb.mb_get_fadein_time = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "get_fadein_time");
	___mb.mb_get_fadeout_time = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "get_fadeout_time");
	___mb.mb_get_mix_mode = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "get_mix_mode");
	___mb.mb_has_autorestart = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "has_autorestart");
	___mb.mb_is_using_sync = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "is_using_sync");
	___mb.mb_set_autorestart = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_autorestart");
	___mb.mb_set_autorestart_delay = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_autorestart_delay");
	___mb.mb_set_autorestart_random_delay = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_autorestart_random_delay");
	___mb.mb_set_fadein_time = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_fadein_time");
	___mb.mb_set_fadeout_time = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_fadeout_time");
	___mb.mb_set_mix_mode = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_mix_mode");
	___mb.mb_set_use_sync = godot::api->godot_method_bind_get_method("AnimationNodeOneShot", "set_use_sync");
}

AnimationNodeOneShot *AnimationNodeOneShot::_new()
{
	return (AnimationNodeOneShot *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AnimationNodeOneShot")());
}
real_t AnimationNodeOneShot::get_autorestart_delay() const {
	return ___godot_icall_float(___mb.mb_get_autorestart_delay, (const Object *) this);
}

real_t AnimationNodeOneShot::get_autorestart_random_delay() const {
	return ___godot_icall_float(___mb.mb_get_autorestart_random_delay, (const Object *) this);
}

real_t AnimationNodeOneShot::get_fadein_time() const {
	return ___godot_icall_float(___mb.mb_get_fadein_time, (const Object *) this);
}

real_t AnimationNodeOneShot::get_fadeout_time() const {
	return ___godot_icall_float(___mb.mb_get_fadeout_time, (const Object *) this);
}

AnimationNodeOneShot::MixMode AnimationNodeOneShot::get_mix_mode() const {
	return (AnimationNodeOneShot::MixMode) ___godot_icall_int(___mb.mb_get_mix_mode, (const Object *) this);
}

bool AnimationNodeOneShot::has_autorestart() const {
	return ___godot_icall_bool(___mb.mb_has_autorestart, (const Object *) this);
}

bool AnimationNodeOneShot::is_using_sync() const {
	return ___godot_icall_bool(___mb.mb_is_using_sync, (const Object *) this);
}

void AnimationNodeOneShot::set_autorestart(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_autorestart, (const Object *) this, enable);
}

void AnimationNodeOneShot::set_autorestart_delay(const real_t enable) {
	___godot_icall_void_float(___mb.mb_set_autorestart_delay, (const Object *) this, enable);
}

void AnimationNodeOneShot::set_autorestart_random_delay(const real_t enable) {
	___godot_icall_void_float(___mb.mb_set_autorestart_random_delay, (const Object *) this, enable);
}

void AnimationNodeOneShot::set_fadein_time(const real_t time) {
	___godot_icall_void_float(___mb.mb_set_fadein_time, (const Object *) this, time);
}

void AnimationNodeOneShot::set_fadeout_time(const real_t time) {
	___godot_icall_void_float(___mb.mb_set_fadeout_time, (const Object *) this, time);
}

void AnimationNodeOneShot::set_mix_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mix_mode, (const Object *) this, mode);
}

void AnimationNodeOneShot::set_use_sync(const bool enable) {
	___godot_icall_void_bool(___mb.mb_set_use_sync, (const Object *) this, enable);
}

}