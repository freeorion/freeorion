#include "AudioEffectChorus.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectChorus::___method_bindings AudioEffectChorus::___mb = {};

void AudioEffectChorus::___init_method_bindings() {
	___mb.mb_get_dry = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_dry");
	___mb.mb_get_voice_count = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_count");
	___mb.mb_get_voice_cutoff_hz = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_cutoff_hz");
	___mb.mb_get_voice_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_delay_ms");
	___mb.mb_get_voice_depth_ms = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_depth_ms");
	___mb.mb_get_voice_level_db = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_level_db");
	___mb.mb_get_voice_pan = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_pan");
	___mb.mb_get_voice_rate_hz = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_voice_rate_hz");
	___mb.mb_get_wet = godot::api->godot_method_bind_get_method("AudioEffectChorus", "get_wet");
	___mb.mb_set_dry = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_dry");
	___mb.mb_set_voice_count = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_count");
	___mb.mb_set_voice_cutoff_hz = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_cutoff_hz");
	___mb.mb_set_voice_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_delay_ms");
	___mb.mb_set_voice_depth_ms = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_depth_ms");
	___mb.mb_set_voice_level_db = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_level_db");
	___mb.mb_set_voice_pan = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_pan");
	___mb.mb_set_voice_rate_hz = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_voice_rate_hz");
	___mb.mb_set_wet = godot::api->godot_method_bind_get_method("AudioEffectChorus", "set_wet");
}

AudioEffectChorus *AudioEffectChorus::_new()
{
	return (AudioEffectChorus *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectChorus")());
}
real_t AudioEffectChorus::get_dry() const {
	return ___godot_icall_float(___mb.mb_get_dry, (const Object *) this);
}

int64_t AudioEffectChorus::get_voice_count() const {
	return ___godot_icall_int(___mb.mb_get_voice_count, (const Object *) this);
}

real_t AudioEffectChorus::get_voice_cutoff_hz(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_cutoff_hz, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_voice_delay_ms(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_delay_ms, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_voice_depth_ms(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_depth_ms, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_voice_level_db(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_level_db, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_voice_pan(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_pan, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_voice_rate_hz(const int64_t voice_idx) const {
	return ___godot_icall_float_int(___mb.mb_get_voice_rate_hz, (const Object *) this, voice_idx);
}

real_t AudioEffectChorus::get_wet() const {
	return ___godot_icall_float(___mb.mb_get_wet, (const Object *) this);
}

void AudioEffectChorus::set_dry(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_dry, (const Object *) this, amount);
}

void AudioEffectChorus::set_voice_count(const int64_t voices) {
	___godot_icall_void_int(___mb.mb_set_voice_count, (const Object *) this, voices);
}

void AudioEffectChorus::set_voice_cutoff_hz(const int64_t voice_idx, const real_t cutoff_hz) {
	___godot_icall_void_int_float(___mb.mb_set_voice_cutoff_hz, (const Object *) this, voice_idx, cutoff_hz);
}

void AudioEffectChorus::set_voice_delay_ms(const int64_t voice_idx, const real_t delay_ms) {
	___godot_icall_void_int_float(___mb.mb_set_voice_delay_ms, (const Object *) this, voice_idx, delay_ms);
}

void AudioEffectChorus::set_voice_depth_ms(const int64_t voice_idx, const real_t depth_ms) {
	___godot_icall_void_int_float(___mb.mb_set_voice_depth_ms, (const Object *) this, voice_idx, depth_ms);
}

void AudioEffectChorus::set_voice_level_db(const int64_t voice_idx, const real_t level_db) {
	___godot_icall_void_int_float(___mb.mb_set_voice_level_db, (const Object *) this, voice_idx, level_db);
}

void AudioEffectChorus::set_voice_pan(const int64_t voice_idx, const real_t pan) {
	___godot_icall_void_int_float(___mb.mb_set_voice_pan, (const Object *) this, voice_idx, pan);
}

void AudioEffectChorus::set_voice_rate_hz(const int64_t voice_idx, const real_t rate_hz) {
	___godot_icall_void_int_float(___mb.mb_set_voice_rate_hz, (const Object *) this, voice_idx, rate_hz);
}

void AudioEffectChorus::set_wet(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_wet, (const Object *) this, amount);
}

}