#include "ARVRInterfaceGDNative.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ARVRInterfaceGDNative::___method_bindings ARVRInterfaceGDNative::___mb = {};

void ARVRInterfaceGDNative::___init_method_bindings() {
}

ARVRInterfaceGDNative *ARVRInterfaceGDNative::_new()
{
	return (ARVRInterfaceGDNative *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVRInterfaceGDNative")());
}
}