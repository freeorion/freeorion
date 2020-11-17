#include "VisualShaderNodeScalarConstant.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeScalarConstant::___method_bindings VisualShaderNodeScalarConstant::___mb = {};

void VisualShaderNodeScalarConstant::___init_method_bindings() {
	___mb.mb_get_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarConstant", "get_constant");
	___mb.mb_set_constant = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarConstant", "set_constant");
}

VisualShaderNodeScalarConstant *VisualShaderNodeScalarConstant::_new()
{
	return (VisualShaderNodeScalarConstant *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeScalarConstant")());
}
real_t VisualShaderNodeScalarConstant::get_constant() const {
	return ___godot_icall_float(___mb.mb_get_constant, (const Object *) this);
}

void VisualShaderNodeScalarConstant::set_constant(const real_t value) {
	___godot_icall_void_float(___mb.mb_set_constant, (const Object *) this, value);
}

}