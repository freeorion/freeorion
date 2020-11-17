#include "AudioEffectBandPassFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectBandPassFilter::___method_bindings AudioEffectBandPassFilter::___mb = {};

void AudioEffectBandPassFilter::___init_method_bindings() {
}

AudioEffectBandPassFilter *AudioEffectBandPassFilter::_new()
{
	return (AudioEffectBandPassFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectBandPassFilter")());
}
}