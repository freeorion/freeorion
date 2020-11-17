#include "AudioBusLayout.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


AudioBusLayout::___method_bindings AudioBusLayout::___mb = {};

void AudioBusLayout::___init_method_bindings() {
}

AudioBusLayout *AudioBusLayout::_new()
{
	return (AudioBusLayout *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"AudioBusLayout")());
}
}