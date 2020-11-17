#include "AudioEffectFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectFilter::___method_bindings AudioEffectFilter::___mb = {};

void AudioEffectFilter::___init_method_bindings() {
	___mb.mb_get_cutoff = godot::api->godot_method_bind_get_method("AudioEffectFilter", "get_cutoff");
	___mb.mb_get_db = godot::api->godot_method_bind_get_method("AudioEffectFilter", "get_db");
	___mb.mb_get_gain = godot::api->godot_method_bind_get_method("AudioEffectFilter", "get_gain");
	___mb.mb_get_resonance = godot::api->godot_method_bind_get_method("AudioEffectFilter", "get_resonance");
	___mb.mb_set_cutoff = godot::api->godot_method_bind_get_method("AudioEffectFilter", "set_cutoff");
	___mb.mb_set_db = godot::api->godot_method_bind_get_method("AudioEffectFilter", "set_db");
	___mb.mb_set_gain = godot::api->godot_method_bind_get_method("AudioEffectFilter", "set_gain");
	___mb.mb_set_resonance = godot::api->godot_method_bind_get_method("AudioEffectFilter", "set_resonance");
}

AudioEffectFilter *AudioEffectFilter::_new()
{
	return (AudioEffectFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectFilter")());
}
real_t AudioEffectFilter::get_cutoff() const {
	return ___godot_icall_float(___mb.mb_get_cutoff, (const Object *) this);
}

AudioEffectFilter::FilterDB AudioEffectFilter::get_db() const {
	return (AudioEffectFilter::FilterDB) ___godot_icall_int(___mb.mb_get_db, (const Object *) this);
}

real_t AudioEffectFilter::get_gain() const {
	return ___godot_icall_float(___mb.mb_get_gain, (const Object *) this);
}

real_t AudioEffectFilter::get_resonance() const {
	return ___godot_icall_float(___mb.mb_get_resonance, (const Object *) this);
}

void AudioEffectFilter::set_cutoff(const real_t freq) {
	___godot_icall_void_float(___mb.mb_set_cutoff, (const Object *) this, freq);
}

void AudioEffectFilter::set_db(const int64_t amount) {
	___godot_icall_void_int(___mb.mb_set_db, (const Object *) this, amount);
}

void AudioEffectFilter::set_gain(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_gain, (const Object *) this, amount);
}

void AudioEffectFilter::set_resonance(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_resonance, (const Object *) this, amount);
}

}