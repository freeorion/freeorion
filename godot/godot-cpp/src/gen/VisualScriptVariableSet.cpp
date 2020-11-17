#include "VisualScriptVariableSet.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptVariableSet::___method_bindings VisualScriptVariableSet::___mb = {};

void VisualScriptVariableSet::___init_method_bindings() {
	___mb.mb_get_variable = godot::api->godot_method_bind_get_method("VisualScriptVariableSet", "get_variable");
	___mb.mb_set_variable = godot::api->godot_method_bind_get_method("VisualScriptVariableSet", "set_variable");
}

VisualScriptVariableSet *VisualScriptVariableSet::_new()
{
	return (VisualScriptVariableSet *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptVariableSet")());
}
String VisualScriptVariableSet::get_variable() const {
	return ___godot_icall_String(___mb.mb_get_variable, (const Object *) this);
}

void VisualScriptVariableSet::set_variable(const String name) {
	___godot_icall_void_String(___mb.mb_set_variable, (const Object *) this, name);
}

}