#include "VisualScriptClassConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptClassConstant::___method_bindings VisualScriptClassConstant::___mb = {};

void VisualScriptClassConstant::___init_method_bindings() {
	___mb.mb_get_base_type = godot::api->godot_method_bind_get_method("VisualScriptClassConstant", "get_base_type");
	___mb.mb_get_class_constant = godot::api->godot_method_bind_get_method("VisualScriptClassConstant", "get_class_constant");
	___mb.mb_set_base_type = godot::api->godot_method_bind_get_method("VisualScriptClassConstant", "set_base_type");
	___mb.mb_set_class_constant = godot::api->godot_method_bind_get_method("VisualScriptClassConstant", "set_class_constant");
}

VisualScriptClassConstant *VisualScriptClassConstant::_new()
{
	return (VisualScriptClassConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptClassConstant")());
}
String VisualScriptClassConstant::get_base_type() {
	return ___godot_icall_String(___mb.mb_get_base_type, (const Object *) this);
}

String VisualScriptClassConstant::get_class_constant() {
	return ___godot_icall_String(___mb.mb_get_class_constant, (const Object *) this);
}

void VisualScriptClassConstant::set_base_type(const String name) {
	___godot_icall_void_String(___mb.mb_set_base_type, (const Object *) this, name);
}

void VisualScriptClassConstant::set_class_constant(const String name) {
	___godot_icall_void_String(___mb.mb_set_class_constant, (const Object *) this, name);
}

}