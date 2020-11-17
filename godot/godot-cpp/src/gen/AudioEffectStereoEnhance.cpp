#include "AudioEffectStereoEnhance.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectStereoEnhance::___method_bindings AudioEffectStereoEnhance::___mb = {};

void AudioEffectStereoEnhance::___init_method_bindings() {
	___mb.mb_get_pan_pullout = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "get_pan_pullout");
	___mb.mb_get_surround = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "get_surround");
	___mb.mb_get_time_pullout = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "get_time_pullout");
	___mb.mb_set_pan_pullout = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "set_pan_pullout");
	___mb.mb_set_surround = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "set_surround");
	___mb.mb_set_time_pullout = godot::api->godot_method_bind_get_method("AudioEffectStereoEnhance", "set_time_pullout");
}

AudioEffectStereoEnhance *AudioEffectStereoEnhance::_new()
{
	return (AudioEffectStereoEnhance *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectStereoEnhance")());
}
real_t AudioEffectStereoEnhance::get_pan_pullout() const {
	return ___godot_icall_float(___mb.mb_get_pan_pullout, (const Object *) this);
}

real_t AudioEffectStereoEnhance::get_surround() const {
	return ___godot_icall_float(___mb.mb_get_surround, (const Object *) this);
}

real_t AudioEffectStereoEnhance::get_time_pullout() const {
	return ___godot_icall_float(___mb.mb_get_time_pullout, (const Object *) this);
}

void AudioEffectStereoEnhance::set_pan_pullout(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_pan_pullout, (const Object *) this, amount);
}

void AudioEffectStereoEnhance::set_surround(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_surround, (const Object *) this, amount);
}

void AudioEffectStereoEnhance::set_time_pullout(const real_t amount) {
	___godot_icall_void_float(___mb.mb_set_time_pullout, (const Object *) this, amount);
}

}