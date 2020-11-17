#include "AudioEffectLowShelfFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectLowShelfFilter::___method_bindings AudioEffectLowShelfFilter::___mb = {};

void AudioEffectLowShelfFilter::___init_method_bindings() {
}

AudioEffectLowShelfFilter *AudioEffectLowShelfFilter::_new()
{
	return (AudioEffectLowShelfFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectLowShelfFilter")());
}
}