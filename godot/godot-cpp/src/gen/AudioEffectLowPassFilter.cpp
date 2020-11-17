#include "AudioEffectLowPassFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectLowPassFilter::___method_bindings AudioEffectLowPassFilter::___mb = {};

void AudioEffectLowPassFilter::___init_method_bindings() {
}

AudioEffectLowPassFilter *AudioEffectLowPassFilter::_new()
{
	return (AudioEffectLowPassFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectLowPassFilter")());
}
}