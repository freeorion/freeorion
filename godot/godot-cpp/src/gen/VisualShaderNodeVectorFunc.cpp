#include "VisualShaderNodeVectorFunc.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeVectorFunc::___method_bindings VisualShaderNodeVectorFunc::___mb = {};

void VisualShaderNodeVectorFunc::___init_method_bindings() {
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeVectorFunc", "get_function");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeVectorFunc", "set_function");
}

VisualShaderNodeVectorFunc *VisualShaderNodeVectorFunc::_new()
{
	return (VisualShaderNodeVectorFunc *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeVectorFunc")());
}
VisualShaderNodeVectorFunc::Function VisualShaderNodeVectorFunc::get_function() const {
	return (VisualShaderNodeVectorFunc::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeVectorFunc::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}