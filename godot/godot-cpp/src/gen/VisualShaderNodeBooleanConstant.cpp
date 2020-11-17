#include "VisualShaderNodeBooleanConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeBooleanConstant::___method_bindings VisualShaderNodeBooleanConstant::___mb = {};

void VisualShaderNodeBooleanConstant::___init_method_bindings() {
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeBooleanConstant", "get_constant");
	___mb.mb_set_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeBooleanConstant", "set_constant");
}

VisualShaderNodeBooleanConstant *VisualShaderNodeBooleanConstant::_new()
{
	return (VisualShaderNodeBooleanConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeBooleanConstant")());
}
bool VisualShaderNodeBooleanConstant::get_constant() const {
	return ___godot_icall_bool(___mb.mb_get_constant, (const Object *) this);
}

void VisualShaderNodeBooleanConstant::set_constant(const bool value) {
	___godot_icall_void_bool(___mb.mb_set_constant, (const Object *) this, value);
}

}