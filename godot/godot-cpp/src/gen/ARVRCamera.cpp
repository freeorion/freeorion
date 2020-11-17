#include "ARVRCamera.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


ARVRCamera::___method_bindings ARVRCamera::___mb = {};

void ARVRCamera::___init_method_bindings() {
}

ARVRCamera *ARVRCamera::_new()
{
	return (ARVRCamera *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"ARVRCamera")());
}
}