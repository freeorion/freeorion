#include "AudioStreamGenerator.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioStreamGenerator::___method_bindings AudioStreamGenerator::___mb = {};

void AudioStreamGenerator::___init_method_bindings() {
	___mb.mb_get_buffer_length = godot::api->godot_method_bind_get_method("AudioStreamGenerator", "get_buffer_length");
	___mb.mb_get_mix_rate = godot::api->godot_method_bind_get_method("AudioStreamGenerator", "get_mix_rate");
	___mb.mb_set_buffer_length = godot::api->godot_method_bind_get_method("AudioStreamGenerator", "set_buffer_length");
	___mb.mb_set_mix_rate = godot::api->godot_method_bind_get_method("AudioStreamGenerator", "set_mix_rate");
}

AudioStreamGenerator *AudioStreamGenerator::_new()
{
	return (AudioStreamGenerator *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamGenerator")());
}
real_t AudioStreamGenerator::get_buffer_length() const {
	return ___godot_icall_float(___mb.mb_get_buffer_length, (const Object *) this);
}

real_t AudioStreamGenerator::get_mix_rate() const {
	return ___godot_icall_float(___mb.mb_get_mix_rate, (const Object *) this);
}

void AudioStreamGenerator::set_buffer_length(const real_t seconds) {
	___godot_icall_void_float(___mb.mb_set_buffer_length, (const Object *) this, seconds);
}

void AudioStreamGenerator::set_mix_rate(const real_t hz) {
	___godot_icall_void_float(___mb.mb_set_mix_rate, (const Object *) this, hz);
}

}