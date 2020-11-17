#include "AudioEffectPhaser.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectPhaser::___method_bindings AudioEffectPhaser::___mb = {};

void AudioEffectPhaser::___init_method_bindings() {
	___mb.mb_get_depth = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "get_depth");
	___mb.mb_get_feedback = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "get_feedback");
	___mb.mb_get_range_max_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "get_range_max_hz");
	___mb.mb_get_range_min_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "get_range_min_hz");
	___mb.mb_get_rate_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "get_rate_hz");
	___mb.mb_set_depth = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "set_depth");
	___mb.mb_set_feedback = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "set_feedback");
	___mb.mb_set_range_max_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "set_range_max_hz");
	___mb.mb_set_range_min_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "set_range_min_hz");
	___mb.mb_set_rate_hz = godot::api->godot_method_bind_get_method("AudioEffectPhaser", "set_rate_hz");
}

AudioEffectPhaser *AudioEffectPhaser::_new()
{
	return (AudioEffectPhaser *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectPhaser")());
}
real_t AudioEffectPhaser::get_depth() const {
	return ___godot_icall_float(___mb.mb_get_depth, (const Object *) this);
}

real_t AudioEffectPhaser::get_feedback() const {
	return ___godot_icall_float(___mb.mb_get_feedback, (const Object *) this);
}

real_t AudioEffectPhaser::get_range_max_hz() const {
	return ___godot_icall_float(___mb.mb_get_range_max_hz, (const Object *) this);
}

real_t AudioEffectPhaser::get_range_min_hz() const {
	return ___godot_icall_float(___mb.mb_get_range_min_hz, (const Object *) this);
}

real_t AudioEffectPhaser::get_rate_hz() const {
	return ___godot_icall_float(___mb.mb_get_rate_hz, (const Object *) this);
}

void AudioEffectPhaser::set_depth(const real_t depth) {
	___godot_icall_void_float(___mb.mb_set_depth, (const Object *) this, depth);
}

void AudioEffectPhaser::set_feedback(const real_t fbk) {
	___godot_icall_void_float(___mb.mb_set_feedback, (const Object *) this, fbk);
}

void AudioEffectPhaser::set_range_max_hz(const real_t hz) {
	___godot_icall_void_float(___mb.mb_set_range_max_hz, (const Object *) this, hz);
}

void AudioEffectPhaser::set_range_min_hz(const real_t hz) {
	___godot_icall_void_float(___mb.mb_set_range_min_hz, (const Object *) this, hz);
}

void AudioEffectPhaser::set_rate_hz(const real_t hz) {
	___godot_icall_void_float(___mb.mb_set_rate_hz, (const Object *) this, hz);
}

}