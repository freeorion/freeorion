#include "PacketPeerGDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PacketPeerGDNative::___method_bindings PacketPeerGDNative::___mb = {};

void PacketPeerGDNative::___init_method_bindings() {
}

PacketPeerGDNative *PacketPeerGDNative::_new()
{
	return (PacketPeerGDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PacketPeerGDNative")());
}
}