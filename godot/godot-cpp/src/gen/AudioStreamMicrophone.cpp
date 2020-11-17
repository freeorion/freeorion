#include "AudioStreamMicrophone.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioStreamMicrophone::___method_bindings AudioStreamMicrophone::___mb = {};

void AudioStreamMicrophone::___init_method_bindings() {
}

AudioStreamMicrophone *AudioStreamMicrophone::_new()
{
	return (AudioStreamMicrophone *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioStreamMicrophone")());
}
}