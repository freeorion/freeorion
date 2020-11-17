#include "AudioEffectReverb.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectReverb::___method_bindings AudioEffectReverb::___mb = {};

void AudioEffectReverb::___init_method_bindings() {
	___mb.mb_get_damping = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_damping");
	___mb.mb_get_dry = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_dry");
	___mb.mb_get_hpf = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_hpf");
	___mb.mb_get_predelay_feedback = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_predelay_feedback");
	___mb.mb_get_predelay_msec = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_predelay_msec");
	___mb.mb_get_room_size = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_room_size");
	___mb.mb_get_spread = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_spread");
	___mb.mb_get_wet = godot::api->godot_method_bind_get_method("AudioEffectReverb", "get_wet");
	___mb.mb_set_damping = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_damping");
	___mb.mb_set_dry = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_dry");
	___mb.mb_set_hpf = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_hpf");
	___mb.mb_set_predelay_feedback = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_predelay_feedback");
	___mb.mb_set_predelay_msec = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_predelay_msec");
	___mb.mb_set_room_size = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_room_size");
	___mb.mb_set_spread = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_spread");
	___mb.mb_set_wet = godot::api->godot_method_bind_get_method("AudioEffectReverb", "set_wet");
}

AudioEffectReverb *AudioEffectReverb::_new()
{
	return (AudioEffectReverb *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectReverb")());
}
real_t AudioEffectReverb::get_damping() const {
	return ___godot_icall_float(___mb.mb_get_damping, (const Object *) this);
}

real_t AudioEffectReverb::get_dry() const {
	return ___godot_icall_float(___mb.mb_get_dry, (const Object *) this);
}

real_t AudioEffectReverb::get_hpf() const {
	return ___godot_icall_float(___mb.mb_get_hpf, (const Object *) this);
}

real_t AudioEffectReverb::get_predelay_feedback() const {
	return ___godot_icall_float(___mb.mb_get_predelay_feedback, (const Object *) this);
}

real_t AudioEffectReverb::get_predelay_msec() const {
	return ___godot_icall_float(___mb.mb_get_predelay_msec, (const Object *) this);
}

real_t AudioEffectReverb::get_room_size() const {
	return ___godot_icall_float(___mb.mb_get_room_size, (const Object *) this);
}

real_t AudioEffectReverb::get_spread() const {
	return ___godot_icall_float(___mb.mb_get_spread, (const Object *) this);
}

real_t AudioEffectReverb::get_wet() const {
	return ___godot_icall_float(___mb.mb_get_wet, (const Object *) this);
}

void AudioEffectReverb::set_damping(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_damping, (const Object *) this, amount);
}

void AudioEffectReverb::set_dry(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_dry, (const Object *) this, amount);
}

void AudioEffectReverb::set_hpf(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_hpf, (const Object *) this, amount);
}

void AudioEffectReverb::set_predelay_feedback(const real_t feedback) {
	___godot_icall_void_float(___mb.mb_set_predelay_feedback, (const Object *) this, feedback);
}

void AudioEffectReverb::set_predelay_msec(const real_t msec) {
	___godot_icall_void_float(___mb.mb_set_predelay_msec, (const Object *) this, msec);
}

void AudioEffectReverb::set_room_size(const real_t size) {
	___godot_icall_void_float(___mb.mb_set_room_size, (const Object *) this, size);
}

void AudioEffectReverb::set_spread(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_spread, (const Object *) this, amount);
}

void AudioEffectReverb::set_wet(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_wet, (const Object *) this, amount);
}

}