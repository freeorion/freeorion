#include "PointMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


PointMesh::___method_bindings PointMesh::___mb = {};

void PointMesh::___init_method_bindings() {
}

PointMesh *PointMesh::_new()
{
	return (PointMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"PointMesh")());
}
}