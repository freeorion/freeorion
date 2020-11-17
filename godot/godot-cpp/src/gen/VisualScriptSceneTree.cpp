#include "VisualScriptSceneTree.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptSceneTree::___method_bindings VisualScriptSceneTree::___mb = {};

void VisualScriptSceneTree::___init_method_bindings() {
}

VisualScriptSceneTree *VisualScriptSceneTree::_new()
{
	return (VisualScriptSceneTree *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptSceneTree")());
}
}