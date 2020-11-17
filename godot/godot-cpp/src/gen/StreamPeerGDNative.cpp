#include "StreamPeerGDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


StreamPeerGDNative::___method_bindings StreamPeerGDNative::___mb = {};

void StreamPeerGDNative::___init_method_bindings() {
}

StreamPeerGDNative *StreamPeerGDNative::_new()
{
	return (StreamPeerGDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"StreamPeerGDNative")());
}
}