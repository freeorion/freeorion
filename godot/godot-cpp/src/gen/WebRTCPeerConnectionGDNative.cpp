#include "WebRTCPeerConnectionGDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


WebRTCPeerConnectionGDNative::___method_bindings WebRTCPeerConnectionGDNative::___mb = {};

void WebRTCPeerConnectionGDNative::___init_method_bindings() {
}

WebRTCPeerConnectionGDNative *WebRTCPeerConnectionGDNative::_new()
{
	return (WebRTCPeerConnectionGDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"WebRTCPeerConnectionGDNative")());
}
}