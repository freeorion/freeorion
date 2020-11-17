#include "VisualShaderNodeColorFunc.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeColorFunc::___method_bindings VisualShaderNodeColorFunc::___mb = {};

void VisualShaderNodeColorFunc::___init_method_bindings() {
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeColorFunc", "get_function");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeColorFunc", "set_function");
}

VisualShaderNodeColorFunc *VisualShaderNodeColorFunc::_new()
{
	return (VisualShaderNodeColorFunc *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeColorFunc")());
}
VisualShaderNodeColorFunc::Function VisualShaderNodeColorFunc::get_function() const {
	return (VisualShaderNodeColorFunc::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeColorFunc::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}