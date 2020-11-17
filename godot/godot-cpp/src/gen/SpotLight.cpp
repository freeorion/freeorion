#include "SpotLight.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


SpotLight::___method_bindings SpotLight::___mb = {};

void SpotLight::___init_method_bindings() {
}

SpotLight *SpotLight::_new()
{
	return (SpotLight *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"SpotLight")());
}
}