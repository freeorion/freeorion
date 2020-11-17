#include "TriangleMesh.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


TriangleMesh::___method_bindings TriangleMesh::___mb = {};

void TriangleMesh::___init_method_bindings() {
}

TriangleMesh *TriangleMesh::_new()
{
	return (TriangleMesh *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"TriangleMesh")());
}
}