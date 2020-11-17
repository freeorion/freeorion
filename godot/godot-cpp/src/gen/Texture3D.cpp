#include "Texture3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Texture3D::___method_bindings Texture3D::___mb = {};

void Texture3D::___init_method_bindings() {
}

Texture3D *Texture3D::_new()
{
	return (Texture3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Texture3D")());
}
}