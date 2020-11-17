#include "CSGCombiner.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


CSGCombiner::___method_bindings CSGCombiner::___mb = {};

void CSGCombiner::___init_method_bindings() {
}

CSGCombiner *CSGCombiner::_new()
{
	return (CSGCombiner *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"CSGCombiner")());
}
}