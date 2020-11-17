#include "VisualScriptWhile.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptWhile::___method_bindings VisualScriptWhile::___mb = {};

void VisualScriptWhile::___init_method_bindings() {
}

VisualScriptWhile *VisualScriptWhile::_new()
{
	return (VisualScriptWhile *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptWhile")());
}
}