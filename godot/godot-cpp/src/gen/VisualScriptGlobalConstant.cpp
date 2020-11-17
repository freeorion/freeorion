#include "VisualScriptGlobalConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualScriptGlobalConstant::___method_bindings VisualScriptGlobalConstant::___mb = {};

void VisualScriptGlobalConstant::___init_method_bindings() {
	___mb.mb_get_global_constant = godot::api->godot_method_bind_get_method("VisualScriptGlobalConstant", "get_global_constant");
	___mb.mb_set_global_constant = godot::api->godot_method_bind_get_method("VisualScriptGlobalConstant", "set_global_constant");
}

VisualScriptGlobalConstant *VisualScriptGlobalConstant::_new()
{
	return (VisualScriptGlobalConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualScriptGlobalConstant")());
}
int64_t VisualScriptGlobalConstant::get_global_constant() {
	return ___godot_icall_int(___mb.mb_get_global_constant, (const Object *) this);
}

void VisualScriptGlobalConstant::set_global_constant(const int64_t index) {
	___godot_icall_void_int(___mb.mb_set_global_constant, (const Object *) this, index);
}

}