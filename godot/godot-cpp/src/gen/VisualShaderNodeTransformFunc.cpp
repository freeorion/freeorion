#include "VisualShaderNodeTransformFunc.hpp"


#include <core/GodotGlobal.hpp>
#include <core/CoreTypes.hpp>
#include <core/Ref.hpp>
#include <core/Godot.hpp>

#include "__icalls.hpp"




namespace godot {


VisualShaderNodeTransformFunc::___method_bindings VisualShaderNodeTransformFunc::___mb = {};

void VisualShaderNodeTransformFunc::___init_method_bindings() {
	___mb.mb_get_function = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformFunc", "get_function");
	___mb.mb_set_function = godot::api->godot_method_bind_get_method("VisualShaderNodeTransformFunc", "set_function");
}

VisualShaderNodeTransformFunc *VisualShaderNodeTransformFunc::_new()
{
	return (VisualShaderNodeTransformFunc *) godot::nativescript_1_1_api->godot_nativescript_get_instance_binding_data(godot::_RegisterState::language_index, godot::api->godot_get_class_constructor((char *)"VisualShaderNodeTransformFunc")());
}
VisualShaderNodeTransformFunc::Function VisualShaderNodeTransformFunc::get_function() const {
	return (VisualShaderNodeTransformFunc::Function) ___godot_icall_int(___mb.mb_get_function, (const Object *) this);
}

void VisualShaderNodeTransformFunc::set_function(const int64_t func) {
	___godot_icall_void_int(___mb.mb_set_function, (const Object *) this, func);
}

}