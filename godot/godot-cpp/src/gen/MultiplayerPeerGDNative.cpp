#include "MultiplayerPeerGDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


MultiplayerPeerGDNative::___method_bindings MultiplayerPeerGDNative::___mb = {};

void MultiplayerPeerGDNative::___init_method_bindings() {
}

MultiplayerPeerGDNative *MultiplayerPeerGDNative::_new()
{
	return (MultiplayerPeerGDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"MultiplayerPeerGDNative")());
}
}