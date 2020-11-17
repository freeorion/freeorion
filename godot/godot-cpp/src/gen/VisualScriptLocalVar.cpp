#include "VisualScriptLocalVar.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptLocalVar::___method_bindings VisualScriptLocalVar::___mb = {};

void VisualScriptLocalVar::___init_method_bindings() {
	___mb.mb_get_var_name = godot::api->godot_method_bind_get_method("VisualScriptLocalVar", "get_var_name");
	___mb.mb_get_var_type = godot::api->godot_method_bind_get_method("VisualScriptLocalVar", "get_var_type");
	___mb.mb_set_var_name = godot::api->godot_method_bind_get_method("VisualScriptLocalVar", "set_var_name");
	___mb.mb_set_var_type = godot::api->godot_method_bind_get_method("VisualScriptLocalVar", "set_var_type");
}

VisualScriptLocalVar *VisualScriptLocalVar::_new()
{
	return (VisualScriptLocalVar *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptLocalVar")());
}
String VisualScriptLocalVar::get_var_name() const {
	return ___godot_icall_String(___mb.mb_get_var_name, (const Object *) this);
}

Variant::Type VisualScriptLocalVar::get_var_type() const {
	return (Variant::Type) ___godot_icall_int(___mb.mb_get_var_type, (const Object *) this);
}

void VisualScriptLocalVar::set_var_name(const String name) {
	___godot_icall_void_String(___mb.mb_set_var_name, (const Object *) this, name);
}

void VisualScriptLocalVar::set_var_type(const int64_t type) {
	___godot_icall_void_int(___mb.mb_set_var_type, (const Object *) this, type);
}

}