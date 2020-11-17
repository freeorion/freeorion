#include "AudioEffectHighPassFilter.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioEffectHighPassFilter::___method_bindings AudioEffectHighPassFilter::___mb = {};

void AudioEffectHighPassFilter::___init_method_bindings() {
}

AudioEffectHighPassFilter *AudioEffectHighPassFilter::_new()
{
	return (AudioEffectHighPassFilter *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioEffectHighPassFilter")());
}
}