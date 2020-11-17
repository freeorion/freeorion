#include "VisualShaderNodeScalarFunc.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeScalarFunc::___method_bindings VisualShaderNodeScalarFunc::___mb = {};

void VisualShaderNodeScalarFunc::___init_method_bindings() {
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarFunc", "get_function");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeScalarFunc", "set_function");
}

VisualShaderNodeScalarFunc *VisualShaderNodeScalarFunc::_new()
{
	return (VisualShaderNodeScalarFunc *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeScalarFunc")());
}
VisualShaderNodeScalarFunc::Function VisualShaderNodeScalarFunc::get_function() const {
	return (VisualShaderNodeScalarFunc::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeScalarFunc::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}