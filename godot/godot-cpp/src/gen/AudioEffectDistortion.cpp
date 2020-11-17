#include "AudioEffectDistortion.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectDistortion::___method_bindings AudioEffectDistortion::___mb = {};

void AudioEffectDistortion::___init_method_bindings() {
	___mb.mb_get_drive = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "get_drive");
	___mb.mb_get_keep_hf_hz = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "get_keep_hf_hz");
	___mb.mb_get_mode = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "get_mode");
	___mb.mb_get_post_gain = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "get_post_gain");
	___mb.mb_get_pre_gain = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "get_pre_gain");
	___mb.mb_set_drive = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "set_drive");
	___mb.mb_set_keep_hf_hz = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "set_keep_hf_hz");
	___mb.mb_set_mode = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "set_mode");
	___mb.mb_set_post_gain = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "set_post_gain");
	___mb.mb_set_pre_gain = godot::api->godot_method_bind_get_method("AudioEffectDistortion", "set_pre_gain");
}

AudioEffectDistortion *AudioEffectDistortion::_new()
{
	return (AudioEffectDistortion *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectDistortion")());
}
real_t AudioEffectDistortion::get_drive() const {
	return ___godot_icall_float(___mb.mb_get_drive, (const Object *) this);
}

real_t AudioEffectDistortion::get_keep_hf_hz() const {
	return ___godot_icall_float(___mb.mb_get_keep_hf_hz, (const Object *) this);
}

AudioEffectDistortion::Mode AudioEffectDistortion::get_mode() const {
	return (AudioEffectDistortion::Mode) ___godot_icall_int(___mb.mb_get_mode, (const Object *) this);
}

real_t AudioEffectDistortion::get_post_gain() const {
	return ___godot_icall_float(___mb.mb_get_post_gain, (const Object *) this);
}

real_t AudioEffectDistortion::get_pre_gain() const {
	return ___godot_icall_float(___mb.mb_get_pre_gain, (const Object *) this);
}

void AudioEffectDistortion::set_drive(const real_t drive) {
	___godot_icall_void_float(___mb.mb_set_drive, (const Object *) this, drive);
}

void AudioEffectDistortion::set_keep_hf_hz(const real_t keep_hf_hz) {
	___godot_icall_void_float(___mb.mb_set_keep_hf_hz, (const Object *) this, keep_hf_hz);
}

void AudioEffectDistortion::set_mode(const int64_t mode) {
	___godot_icall_void_int(___mb.mb_set_mode, (const Object *) this, mode);
}

void AudioEffectDistortion::set_post_gain(const real_t post_gain) {
	___godot_icall_void_float(___mb.mb_set_post_gain, (const Object *) this, post_gain);
}

void AudioEffectDistortion::set_pre_gain(const real_t pre_gain) {
	___godot_icall_void_float(___mb.mb_set_pre_gain, (const Object *) this, pre_gain);
}

}