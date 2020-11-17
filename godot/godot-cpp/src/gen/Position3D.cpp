#include "Position3D.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


Position3D::___method_bindings Position3D::___mb = {};

void Position3D::___init_method_bindings() {
}

Position3D *Position3D::_new()
{
	return (Position3D *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"Position3D")());
}
}