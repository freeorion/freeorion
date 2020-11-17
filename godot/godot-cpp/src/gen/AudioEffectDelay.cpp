#include "AudioEffectDelay.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectDelay::___method_bindings AudioEffectDelay::___mb = {};

void AudioEffectDelay::___init_method_bindings() {
	___mb.mb_get_dry = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_dry");
	___mb.mb_get_feedback_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_feedback_delay_ms");
	___mb.mb_get_feedback_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_feedback_level_db");
	___mb.mb_get_feedback_lowpass = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_feedback_lowpass");
	___mb.mb_get_tap1_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap1_delay_ms");
	___mb.mb_get_tap1_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap1_level_db");
	___mb.mb_get_tap1_pan = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap1_pan");
	___mb.mb_get_tap2_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap2_delay_ms");
	___mb.mb_get_tap2_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap2_level_db");
	___mb.mb_get_tap2_pan = godot::api->godot_method_bind_get_method("AudioEffectDelay", "get_tap2_pan");
	___mb.mb_is_feedback_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "is_feedback_active");
	___mb.mb_is_tap1_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "is_tap1_active");
	___mb.mb_is_tap2_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "is_tap2_active");
	___mb.mb_set_dry = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_dry");
	___mb.mb_set_feedback_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_feedback_active");
	___mb.mb_set_feedback_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_feedback_delay_ms");
	___mb.mb_set_feedback_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_feedback_level_db");
	___mb.mb_set_feedback_lowpass = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_feedback_lowpass");
	___mb.mb_set_tap1_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap1_active");
	___mb.mb_set_tap1_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap1_delay_ms");
	___mb.mb_set_tap1_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap1_level_db");
	___mb.mb_set_tap1_pan = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap1_pan");
	___mb.mb_set_tap2_active = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap2_active");
	___mb.mb_set_tap2_delay_ms = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap2_delay_ms");
	___mb.mb_set_tap2_level_db = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap2_level_db");
	___mb.mb_set_tap2_pan = godot::api->godot_method_bind_get_method("AudioEffectDelay", "set_tap2_pan");
}

AudioEffectDelay *AudioEffectDelay::_new()
{
	return (AudioEffectDelay *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectDelay")());
}
real_t AudioEffectDelay::get_dry() {
	return ___godot_icall_float(___mb.mb_get_dry, (const Object *) this);
}

real_t AudioEffectDelay::get_feedback_delay_ms() const {
	return ___godot_icall_float(___mb.mb_get_feedback_delay_ms, (const Object *) this);
}

real_t AudioEffectDelay::get_feedback_level_db() const {
	return ___godot_icall_float(___mb.mb_get_feedback_level_db, (const Object *) this);
}

real_t AudioEffectDelay::get_feedback_lowpass() const {
	return ___godot_icall_float(___mb.mb_get_feedback_lowpass, (const Object *) this);
}

real_t AudioEffectDelay::get_tap1_delay_ms() const {
	return ___godot_icall_float(___mb.mb_get_tap1_delay_ms, (const Object *) this);
}

real_t AudioEffectDelay::get_tap1_level_db() const {
	return ___godot_icall_float(___mb.mb_get_tap1_level_db, (const Object *) this);
}

real_t AudioEffectDelay::get_tap1_pan() const {
	return ___godot_icall_float(___mb.mb_get_tap1_pan, (const Object *) this);
}

real_t AudioEffectDelay::get_tap2_delay_ms() const {
	return ___godot_icall_float(___mb.mb_get_tap2_delay_ms, (const Object *) this);
}

real_t AudioEffectDelay::get_tap2_level_db() const {
	return ___godot_icall_float(___mb.mb_get_tap2_level_db, (const Object *) this);
}

real_t AudioEffectDelay::get_tap2_pan() const {
	return ___godot_icall_float(___mb.mb_get_tap2_pan, (const Object *) this);
}

bool AudioEffectDelay::is_feedback_active() const {
	return ___godot_icall_bool(___mb.mb_is_feedback_active, (const Object *) this);
}

bool AudioEffectDelay::is_tap1_active() const {
	return ___godot_icall_bool(___mb.mb_is_tap1_active, (const Object *) this);
}

bool AudioEffectDelay::is_tap2_active() const {
	return ___godot_icall_bool(___mb.mb_is_tap2_active, (const Object *) this);
}

void AudioEffectDelay::set_dry(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_dry, (const Object *) this, amount);
}

void AudioEffectDelay::set_feedback_active(const bool amount) {
	___godot_icall_void_bool(___mb.mb_set_feedback_active, (const Object *) this, amount);
}

void AudioEffectDelay::set_feedback_delay_ms(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_feedback_delay_ms, (const Object *) this, amount);
}

void AudioEffectDelay::set_feedback_level_db(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_feedback_level_db, (const Object *) this, amount);
}

void AudioEffectDelay::set_feedback_lowpass(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_feedback_lowpass, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap1_active(const bool amount) {
	___godot_icall_void_bool(___mb.mb_set_tap1_active, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap1_delay_ms(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap1_delay_ms, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap1_level_db(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap1_level_db, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap1_pan(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap1_pan, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap2_active(const bool amount) {
	___godot_icall_void_bool(___mb.mb_set_tap2_active, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap2_delay_ms(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap2_delay_ms, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap2_level_db(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap2_level_db, (const Object *) this, amount);
}

void AudioEffectDelay::set_tap2_pan(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_tap2_pan, (const Object *) this, amount);
}

}